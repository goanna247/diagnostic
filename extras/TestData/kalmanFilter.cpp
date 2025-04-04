/*
g++ -o kalmanFilter kalmanFilter.cpp -lgsl && ./kalmanFilter raw.log | tee data.txt
cat cutecom.log | sed -n 's|.*ic: x: \([-0-9]*\).*|\1|p' > angle.txt
gnuplot
set title "InfoCrank Electronics - Accelerometer Testing\n100 Cadence, σ@^2_α = 0.1, σ@^2_{acc} = 10.0"
set style data lines
set grid
set xlabel "time (seconds)"
set ylabel "See legend"
f(x) = m*x + b
fit [5.0:] f(x) "data.txt" using 1:6 via m,b
plot "angle.txt" using ($0/32):($1*360/8192)  lw 3 title "nRF52840", "data.txt" using ($0/32):($1*57.29) lw 3 title "PC C++"
plot \
"data.txt" u 1:2 t "@^{..}_{x_1} m/s^2", \
"" u 1:3 t "@^{..}_{y_1} m/s^2", \
"" u 1:4 t "@^{..}_{x_2} m/s^2", \
"" u 1:5 t "@^{..}_{y_2} m/s^2", \
"" u 1:6 lw 3 t "θ rad", \
"" u 1:7 lw 3 t "@^._θ rad/s", \
"" u 1:8 lw 3 t "@^{..}_θ rad/s^2", \
f(x) ls 0 t sprintf("fit = %0.1f RPM",m*9.5493)

 */
#include <iostream>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <vector>
#include <gsl/gsl_linalg.h>

#define GRAVITY 9.81
#define r1 0.0284
#define r2 0.0614
#define RATIO 0 //0.9545
#define DT 0.0078125
#define DT2 (DT*DT)
#define DT3 (DT*DT*DT)
#define DT4 (DT*DT*DT*DT)
#define VA 0.10  // Variance of rotational acceleration α
#define VACC 10.0  // Variance of accelerometers

double F_data[] = {
  1, DT, 0.5*DT*DT,
  0,  1,        DT,
  0,  0,         1
};

double Q_data[] = {
  VA*DT4/4.0, VA*DT3/2.0, VA*DT2/2.0,
  VA*DT3/2.0, VA*DT2,     VA*DT,
  VA*DT2/2.0, VA*DT,      VA
};

double R_data[] = {
  VACC, 0.0, 0.0, 0.0,
  0.0, VACC, 0.0, 0.0,
  0.0, 0.0, VACC, 0.0,
  0.0, 0.0, 0.0, VACC
};

struct observation_s {
  double   t;
  double   x1;
  double   y1;
  double   x2;
  double   y2;
};


#pragma pack(push, 1)
struct raw_data_s {
  uint8_t op_code;
  union {
    struct {
      int32_t unused: 6;
      int32_t strain: 18;
    } strain;
    struct {
      int16_t accel1_x;
      int16_t accel1_y;
      int16_t accel1_z;
      int16_t accel2_x;
      int16_t accel2_y;
      int16_t accel2_z;
    } acceleration;
    struct {
      int16_t integral;
      int32_t fractional;
    } temperature;
    struct {
      int32_t theta;
      int32_t omega;
      int32_t alpha;
    } state;
  } data;
};
#pragma pack(pop)

enum op_codes {
  STRAIN_DATA = 0,
  ACCELERATION_DATA,
  TEMPERATURE_DATA,
  STATE_DATA,
};

double a1[9] = {
  -19445, 512, -674,
  621, -19409, -1373,
  -163, -767, 19580
};

double a2[9] = {
  19241, 307, 320,
  -1101, -19107, 1088,
  -246, -136, -19854
};

int main (int argc, char *argv[]) {
  char buffer[256];
  struct observation_s observation;
  std::vector<struct observation_s> observations;

  char filename[256] = "raw.log";
  if (argc > 1) {
    strcpy(filename,argv[1]);
  }

  // Create noise free simulation data
//  for (double t=0; t < 60.0; t += DT) {
//    double thetadot = 100.0/60.0*2.0*M_PI;
//    double theta = t*thetadot;
//    observation.t = t;
//    observation.x1 = 	GRAVITY*sin(theta)-thetadot*thetadot*r1;
//    observation.y1 = 	GRAVITY*cos(theta);
//    observation.x2 = 	GRAVITY*sin(theta)-thetadot*thetadot*r2;
//    observation.y2 = 	GRAVITY*cos(theta);
//    observations.push_back(observation);
//    printf("%0.7lf %0.4lf %0.4lf %0.4lf %0.4lf\n",t,observation.x1,observation.y1,observation.x2,observation.y2);
//  }

  // Read the input file and calibrate the accelerometers
  {
    double t = 0.0;
    gsl_matrix_view A1 = gsl_matrix_view_array (a1, 3, 3);
    gsl_matrix_view A2 = gsl_matrix_view_array (a2, 3, 3);
          gsl_vector *a = gsl_vector_alloc (3);
          gsl_vector *a_ = gsl_vector_alloc (3);
    FILE *infile = fopen(filename, "rb");
    struct raw_data_s raw_data;
    int n=0;
    do {
      fread(&raw_data.op_code,1,1,infile);
      switch (raw_data.op_code) {
        case STRAIN_DATA:
          fread(&raw_data.data.strain,sizeof(raw_data.data.strain),1,infile);
          break;
        case ACCELERATION_DATA:
          fread(&raw_data.data.acceleration,sizeof(raw_data.data.acceleration),1,infile);
//          printf("%hd %hd %hd %hd %hd %hd\n",
//                 raw_data.data.acceleration.accel1_x,
//                 raw_data.data.acceleration.accel1_y,
//                 raw_data.data.acceleration.accel1_z,
//                 raw_data.data.acceleration.accel2_x,
//                 raw_data.data.acceleration.accel2_y,
//                 raw_data.data.acceleration.accel2_z);
          gsl_vector_set(a, 0, raw_data.data.acceleration.accel1_x);
          gsl_vector_set(a, 1, raw_data.data.acceleration.accel1_y);
          gsl_vector_set(a, 2, raw_data.data.acceleration.accel1_z);
          gsl_blas_dgemv(CblasNoTrans, 1.0/(double) 0x00800000, &A1.matrix, a, 0.0, a_);
          observation.t = t;
      observation.x1 = gsl_vector_get(a_,0);
      observation.y1 = gsl_vector_get(a_,1);
//          printf("%0.7lf %lf %lf ",t,
//                 gsl_vector_get(a_,0),
//                 gsl_vector_get(a_,1),
//                 gsl_vector_get(a_,2));
          gsl_vector_set(a, 0, raw_data.data.acceleration.accel2_x);
          gsl_vector_set(a, 1, raw_data.data.acceleration.accel2_y);
          gsl_vector_set(a, 2, raw_data.data.acceleration.accel2_z);
          gsl_blas_dgemv(CblasNoTrans, 1.0/(double) 0x00800000, &A2.matrix, a, 0.0, a_);
      observation.x2 = gsl_vector_get(a_,0);
      observation.y2 = gsl_vector_get(a_,1);
      observations.push_back(observation);
//          printf("%lf %lf\n",
//                 gsl_vector_get(a_,0),
//                 gsl_vector_get(a_,1),
//                 gsl_vector_get(a_,2));
          t += DT;
          break;
        case TEMPERATURE_DATA:
          fread(&raw_data.data.temperature,sizeof(raw_data.data.temperature),1,infile);
          break;
        case STATE_DATA:
          fread(&raw_data.data.state,sizeof(raw_data.data.state),1,infile);
          printf("%lf %lf %lf %lf\n", t,raw_data.data.state.theta/8192.0,raw_data.data.state.omega*60.0*128.0/524288.0,raw_data.data.state.alpha* 16384.0 / 1677216.0);
          break;
      }
    } while (!feof(infile));
          gsl_vector_free(a);
          gsl_vector_free(a_);

    fclose(infile);
  }

  return 0;





  // Kalman filter
  {
    // These matrices don't change
    gsl_matrix_view F = gsl_matrix_view_array (F_data, 3, 3);
    gsl_matrix_view Q = gsl_matrix_view_array (Q_data, 3, 3);
    gsl_matrix_view R = gsl_matrix_view_array (R_data, 4, 4);

    // Working vectors, matrices and permutations
    gsl_vector *x = gsl_vector_alloc (3);
    gsl_vector *xkk1 = gsl_vector_alloc (3);
    gsl_vector *h = gsl_vector_alloc (4);
    gsl_vector *y = gsl_vector_alloc (4);
    gsl_matrix *P = gsl_matrix_alloc(3,3);
    gsl_matrix *H = gsl_matrix_alloc(4,3);
    gsl_matrix *S = gsl_matrix_alloc(4,4);
    gsl_matrix *S1 = gsl_matrix_alloc(4,4);
    gsl_matrix *K = gsl_matrix_alloc(3,4);
    gsl_matrix *M33 = gsl_matrix_alloc(3,3);
    gsl_matrix *M33_ = gsl_matrix_alloc(3,3);
    gsl_matrix *M43 = gsl_matrix_alloc(4,3);
    gsl_matrix *M34 = gsl_matrix_alloc(3,4);
    gsl_permutation *p = gsl_permutation_alloc(4);

    // Initialisation
    // State - stationary with zero angle
    gsl_vector_set_zero(x);
    gsl_vector_set(x,1,0.0);
    // Covariance - zero
    gsl_matrix_set_zero(P);

    for (std::vector<struct observation_s>::iterator it = observations.begin(); it < observations.end(); ++it) {

      // Predicted state estimate
      gsl_blas_dgemv(CblasNoTrans, 1.0, &F.matrix, x, 0.0, xkk1);
      //std::cout << "x " << gsl_vector_get(xkk1,0) << " " << gsl_vector_get(xkk1,1) << " " << gsl_vector_get(xkk1,2) << std::endl;

      // Predicted covariance estimate
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &F.matrix, P, 0.0, M33);
      gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, M33, &F.matrix, 0.0, P);
      gsl_matrix_add(P, &Q.matrix);
      //std::cout << "P " << gsl_matrix_get(P,0,0) << " " << gsl_matrix_get(P,0,1) << " " << gsl_matrix_get(P,0,2) << " " << std::endl
      //          << "  " << gsl_matrix_get(P,1,0) << " " << gsl_matrix_get(P,1,1) << " " << gsl_matrix_get(P,1,2) << " " << std::endl
      //          << "  " << gsl_matrix_get(P,2,0) << " " << gsl_matrix_get(P,2,1) << " " << gsl_matrix_get(P,2,2) << " " << std::endl;

      // Innovation or measurement residual
      gsl_vector_set(y, 0, it->x1);
      gsl_vector_set(y, 1, it->y1);
      gsl_vector_set(y, 2, it->x2);
      gsl_vector_set(y, 3, it->y2);
      //std::cout << "z " << gsl_vector_get(y,0) << " " << gsl_vector_get(y,1) << " " << gsl_vector_get(y,2) << " " << gsl_vector_get(y,3) << std::endl;
      gsl_vector_set(h, 0,  RATIO*gsl_vector_get(xkk1,2)*cos(gsl_vector_get(xkk1,0)) + GRAVITY*sin(gsl_vector_get(xkk1,0)) - r1*gsl_vector_get(xkk1,1)*gsl_vector_get(xkk1,1));
      gsl_vector_set(h, 1, -RATIO*gsl_vector_get(xkk1,2)*sin(gsl_vector_get(xkk1,0)) + GRAVITY*cos(gsl_vector_get(xkk1,0)) + r1*gsl_vector_get(xkk1,2));
      gsl_vector_set(h, 2,  RATIO*gsl_vector_get(xkk1,2)*cos(gsl_vector_get(xkk1,0)) + GRAVITY*sin(gsl_vector_get(xkk1,0)) - r2*gsl_vector_get(xkk1,1)*gsl_vector_get(xkk1,1));
      gsl_vector_set(h, 3, -RATIO*gsl_vector_get(xkk1,2)*sin(gsl_vector_get(xkk1,0)) + GRAVITY*cos(gsl_vector_get(xkk1,0)) + r2*gsl_vector_get(xkk1,2));
      //std::cout << " h " << gsl_vector_get(h,0) << " " << gsl_vector_get(h,1) << " " << gsl_vector_get(h,2) << " " << gsl_vector_get(h,3);
      gsl_vector_sub(y, h);
      //std::cout << "y " << gsl_vector_get(y,0) << " " << gsl_vector_get(y,1) << " " << gsl_vector_get(y,2) << " " << gsl_vector_get(y,3) << std::endl;

      // Innovation (or residual) covariance
      gsl_matrix_set(H, 0, 0, -RATIO*gsl_vector_get(xkk1,2)*sin(gsl_vector_get(xkk1,0)) + GRAVITY*cos(gsl_vector_get(xkk1,0)));
      gsl_matrix_set(H, 1, 0, -RATIO*gsl_vector_get(xkk1,2)*cos(gsl_vector_get(xkk1,0)) - GRAVITY*sin(gsl_vector_get(xkk1,0)));
      gsl_matrix_set(H, 2, 0, -RATIO*gsl_vector_get(xkk1,2)*sin(gsl_vector_get(xkk1,0)) + GRAVITY*cos(gsl_vector_get(xkk1,0)));
      gsl_matrix_set(H, 3, 0, -RATIO*gsl_vector_get(xkk1,2)*cos(gsl_vector_get(xkk1,0)) - GRAVITY*sin(gsl_vector_get(xkk1,0)));

      gsl_matrix_set(H, 0, 1, -2.0*r1*gsl_vector_get(xkk1,1));
      gsl_matrix_set(H, 1, 1,  0.0);
      gsl_matrix_set(H, 2, 1, -2.0*r2*gsl_vector_get(xkk1,1));
      gsl_matrix_set(H, 3, 1,  0.0);

      gsl_matrix_set(H, 0, 2,  RATIO*cos(gsl_vector_get(xkk1,0)));
      gsl_matrix_set(H, 1, 2, -RATIO*sin(gsl_vector_get(xkk1,0)));
      gsl_matrix_set(H, 2, 2,  RATIO*cos(gsl_vector_get(xkk1,0)));
      gsl_matrix_set(H, 3, 2, -RATIO*sin(gsl_vector_get(xkk1,0)));
      //std::cout << "H " << gsl_matrix_get(H,0,0) << " " << gsl_matrix_get(H,0,1) << " " << gsl_matrix_get(H,0,2) << std::endl
      //          << "  " << gsl_matrix_get(H,1,0) << " " << gsl_matrix_get(H,1,1) << " " << gsl_matrix_get(H,1,2) << std::endl
      //          << "  " << gsl_matrix_get(H,2,0) << " " << gsl_matrix_get(H,2,1) << " " << gsl_matrix_get(H,2,2) << std::endl
      //          << "  " << gsl_matrix_get(H,3,0) << " " << gsl_matrix_get(H,3,1) << " " << gsl_matrix_get(H,3,2) << std::endl;


      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, H, P, 0.0, M43);
      gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, M43, H, 0.0, S);
      gsl_matrix_add(S, &R.matrix);
      //std::cout << "S " << gsl_matrix_get(S,0,0) << " " << gsl_matrix_get(S,0,1) << " " << gsl_matrix_get(S,0,2) << " " << gsl_matrix_get(S,0,3) << std::endl
      //          << "  " << gsl_matrix_get(S,1,0) << " " << gsl_matrix_get(S,1,1) << " " << gsl_matrix_get(S,1,2) << " " << gsl_matrix_get(S,1,3) << std::endl
      //          << "  " << gsl_matrix_get(S,2,0) << " " << gsl_matrix_get(S,2,1) << " " << gsl_matrix_get(S,2,2) << " " << gsl_matrix_get(S,2,3) << std::endl
      //          << "  " << gsl_matrix_get(S,3,0) << " " << gsl_matrix_get(S,3,1) << " " << gsl_matrix_get(S,3,2) << " " << gsl_matrix_get(S,3,3) << std::endl;

      // Near-optimal Kalman gain
      int s;
      gsl_linalg_LU_decomp(S, p, &s);
      gsl_linalg_LU_invert(S, p, S1);

      gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, P, H, 0.0, M34);
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, M34, S1, 0.0, K);
      //std::cout << "K " << gsl_matrix_get(K,0,0) << " " << gsl_matrix_get(K,0,1) << " " << gsl_matrix_get(K,0,2) << " " << gsl_matrix_get(K,0,3) << std::endl
      //          << "  " << gsl_matrix_get(K,1,0) << " " << gsl_matrix_get(K,1,1) << " " << gsl_matrix_get(K,1,2) << " " << gsl_matrix_get(K,1,3) << std::endl
      //          << "  " << gsl_matrix_get(K,2,0) << " " << gsl_matrix_get(K,2,1) << " " << gsl_matrix_get(K,2,2) << " " << gsl_matrix_get(K,2,3) << std::endl;


      // Updated state estimate
      gsl_blas_dgemv(CblasNoTrans, 1.0, K, y, 1.0, xkk1);
      gsl_vector_memcpy(x, xkk1);

      // Updated covariance estimate
      gsl_matrix_set_identity(M33);
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, -1.0, K, H, 1.0, M33);
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, M33, P, 0.0, M33_);
      gsl_matrix_set_identity(M33);
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, -1.0, K, H, 1.0, M33);
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, K, &R.matrix, 0.0, M34);
      gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, M34, K, 0.0, P);
      gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, M33_, M33, 1.0, P);
      //std::cout << "P " << gsl_matrix_get(P,0,0) << " " << gsl_matrix_get(P,0,1) << " " << gsl_matrix_get(P,0,2) << " " << std::endl
      //          << "  " << gsl_matrix_get(P,1,0) << " " << gsl_matrix_get(P,1,1) << " " << gsl_matrix_get(P,1,2) << " " << std::endl
      //          << "  " << gsl_matrix_get(P,2,0) << " " << gsl_matrix_get(P,2,1) << " " << gsl_matrix_get(P,2,2) << " " << std::endl;


      // Output
      printf("%0.7lf %0.6lf %0.6lf %0.6lf %0.6lf %0.6lf %0.6lf %0.6lf\n",it->t,it->x1,it->y1,it->x2,it->y2,gsl_vector_get(x,0),gsl_vector_get(x,1),gsl_vector_get(x,2));
//      std::cout << gsl_vector_get(x,0) << " " << gsl_vector_get(x,1) << " " << gsl_vector_get(x,2) << std::endl;
    }
    gsl_vector_free(x);
    gsl_vector_free(xkk1);
    gsl_vector_free(h);
    gsl_vector_free(y);
    gsl_matrix_free(P);
    gsl_matrix_free(H);
    gsl_matrix_free(S);
    gsl_matrix_free(S1);
    gsl_matrix_free(K);
    gsl_matrix_free(M33);
    gsl_matrix_free(M33_);
    gsl_matrix_free(M43);
    gsl_matrix_free(M34);
    gsl_permutation_free(p);
  }

}


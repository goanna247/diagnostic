#ifndef _MAIN_H
#define _MAIN_H

#include <wx/wx.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/tglbtn.h>
#include <wx/notebook.h>
#include <wx/file.h>
#include <wx/clntdata.h>
#include <wx/sizer.h>

#include "crank-canvas.h"
#include "gui-helper.h"

//--------------------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------------------
class IC2App;
class IC2Frame;

//--------------------------------------------------------------------------------------------------
// The application
//--------------------------------------------------------------------------------------------------
class IC2App: public wxApp
{
public:
    bool OnInit();
};


class DraggableOverlayPanel : public wxPanel {
public:
    DraggableOverlayPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size), dragging(false), dragTimer(this)
    {
        SetBackgroundColour(wxColour(0, 0, 0, 128));

        Bind(wxEVT_LEFT_DOWN, &DraggableOverlayPanel::OnMouseDown, this);
        Bind(wxEVT_LEFT_UP, &DraggableOverlayPanel::OnMouseUp, this);
        Bind(wxEVT_TIMER, &DraggableOverlayPanel::OnDragTimer, this, dragTimer.GetId());
        Bind(wxEVT_MOUSE_CAPTURE_LOST, &DraggableOverlayPanel::OnCaptureLost, this);
    }

private:
    bool dragging = false;
    wxPoint mouseOffsetInOverlay;
    wxTimer dragTimer;

    void OnMouseDown(wxMouseEvent& event) {
        dragging = true;
        mouseOffsetInOverlay = event.GetPosition();
        CaptureMouse();
        dragTimer.Start(10);  // Poll every 10ms
    }

    void OnMouseUp(wxMouseEvent&) {
        CancelDrag();
    }

    void OnCaptureLost(wxMouseCaptureLostEvent&) {
        CancelDrag();
    }

    void OnDragTimer(wxTimerEvent&) {
        if (!dragging) return;

        wxPoint mouseScreenPos = wxGetMousePosition();
        wxPoint parentMousePos = GetParent()->ScreenToClient(mouseScreenPos);
        wxPoint newOverlayPos = parentMousePos - mouseOffsetInOverlay;

        // Clamp to parent bounds
        wxSize parentSize = GetParent()->GetClientSize();
        wxSize overlaySize = GetSize();

        newOverlayPos.x = std::clamp(newOverlayPos.x, 0, parentSize.GetWidth() - overlaySize.GetWidth());
        newOverlayPos.y = std::clamp(newOverlayPos.y, 0, parentSize.GetHeight() - overlaySize.GetHeight());

        Move(newOverlayPos);
    }

    void CancelDrag() {
        dragging = false;
        if (HasCapture()) ReleaseMouse();
        if (dragTimer.IsRunning()) dragTimer.Stop();
    }
};






//--------------------------------------------------------------------------------------------------
// The main frame
//--------------------------------------------------------------------------------------------------
class IC2Frame : public wxFrame
{
public:
    wxStaticText* overlayText;
    wxNotebook* notebook;

    wxPanel* devices;
    wxPanel* devInfo;
    wxPanel* battery;
    wxPanel* features;
    wxPanel* measurement;
    wxPanel* sensor_location;
    wxPanel* control;
    wxPanel* vector;
    wxPanel* infoCrank_control;
    wxPanel* infoCrank_raw;
    wxPanel* page12;
    CrankCanvas* crank_graphics;
    wxButton* refreshDeviceInformation;

    wxButton* refreshBatteryInformation;
    wxCheckBox* loggingBattery;
    wxButton* logFileBattery;

    wxButton* refreshFeatures;


    int sockfd;

    struct sensorLocations_s {
        int index;
        wxString location;
    };
    static struct sensorLocations_s sensorLocations[17];


    class FileDialogParameters : public wxObject
    {
    public:
        wxString m_fileName;
        wxCheckBox *m_checkBox;
        FileDialogParameters(wxString fileName, wxCheckBox *checkBox)
        {
            m_fileName = fileName;
            m_checkBox = checkBox;
        }
    };

    //    class LogFile : public wxObject
    //    {
    //    public:
    //        wxFile *m_file;
    //        LogFile(wxFile *file)
    //        {
    //            m_file = file;
    //        }
    //    };

    //    class wxStringObject : public wxObject
    //    {
    //    public:
    //        wxString *m_string;
    //        wxStringObject(const char *str)
    //        {
    //            m_string = new wxString(str);
    //        }
    //    };

    //    class wxTextCtrlObject : public wxObject
    //    {
    //    public:
    //        wxTextCtrl *m_textCtrl;
    //        wxTextCtrlObject(wxTextCtrl *textCtrl)
    //        {
    //            m_textCtrl = textCtrl;
    //        }
    //    };

    //    wxNotebook *notebook;

    // Devices page

    wxWrapSizer *devicesSizer;

    // Device infomation page
    wxStaticText *manufacturerName;
    wxStaticText *modelNumber;
    wxStaticText *serialNumber;
    wxStaticText *hardwareRevisionNumber;
    wxStaticText *firmwareRevisionNumber;
    wxStaticText *softwareRevisionNumber;
    wxStaticText *systemID;
    wxStaticText *PNPVendorIDSource;
    wxStaticText *PNPVendorID;
    wxStaticText *PNPProductID;
    wxStaticText *PNPProductVersion;

    // Battery information page
    wxStaticText *batteryLevel;
    wxFile logBattery;

    // Features page
    wxCheckBox *balanceFeature;
    wxCheckBox *torqueFeature;
    wxCheckBox *wheelRevolutionFeature;
    wxCheckBox *crankRevolutionFeature;
    wxCheckBox *magnitudesFeature;
    wxCheckBox *anglesFeature;
    wxCheckBox *deadSpotsFeature;
    wxCheckBox *energyFeature;
    wxCheckBox *compensationIndicatorFeature;
    wxCheckBox *compensationFeature;
    wxCheckBox *maskingFeature;
    wxCheckBox *locationsFeature;
    wxCheckBox *crankLengthFeature;
    wxCheckBox *chainLengthFeature;
    wxCheckBox *chainWeightFeature;
    wxCheckBox *spanFeature;
    wxStaticText *contextFeature;
    wxCheckBox *directionFeature;
    wxCheckBox *dateFeature;
    wxCheckBox *enhancedCompensationFeature;
    wxStaticText *distributedFeature;

    // Cycling power measurement page

    wxCheckBox *pedalPowerBalancePresent;
    wxCheckBox *accumulatedTorquePresent;
    wxCheckBox *wheelRevolutionDataPresent;
    wxCheckBox *crankRevolutionDataPresent;
    wxCheckBox *extremeForceMagnitudesPresent;
    wxCheckBox *extremeTorqueMagnitudesPresent;
    wxCheckBox *extremeAnglesPresent;
    wxCheckBox *topDeadSpotAnglePresent;
    wxCheckBox *bottomDeadSpotAnglePresent;
    wxCheckBox *accumulatedEnergyPresent;
    wxCheckBox *offsetCompensationIndicator;

    wxStaticText *instantaneousPower;
    wxStaticText *pedalPowerBalance;
    wxStaticText *accumulatedTorque;
    wxStaticText *torque;
    wxStaticText *cumulativeWheelRevolutions;
    wxStaticText *lastWheelEventTime;
    wxStaticText *wheelSpeed;
    wxStaticText *cumulativeCrankRevolutions;
    wxStaticText *lastCrankEventTime;
    wxStaticText *cadence;
    wxStaticText *maximumForceMagnitude;
    wxStaticText *minimumForceMagnitude;
    wxStaticText *maximumTorqueMagnitude;
    wxStaticText *minimumTorqueMagnitude;
    wxStaticText *maximumAngle;
    wxStaticText *minimumAngle;
    wxStaticText *topDeadSpotAngle;
    wxStaticText *bottomDeadSpotAngle;
    wxStaticText *accumulatedEnergy;
    wxFile logMeasurement;

    // Sensor location page
    wxStaticText *sensorLocation;

    // Cycling power control point page
    wxComboBox *location;
    wxTextCtrl *crankLength;
    wxTextCtrl *chainLength;
    wxTextCtrl *chainWeight;
    wxTextCtrl *span;
    wxStaticText *offsetCompensationValue;
    wxCheckBox *pedalPowerBalanceMask;
    wxCheckBox *accumulatedTorqueMask;
    wxCheckBox *wheelRevolutionDataMask;
    wxCheckBox *crankRevolutionDataMask;
    wxCheckBox *extremeMagnitudesMask;
    wxCheckBox *extremeAnglesMask;
    wxCheckBox *topDeadSpotAngleMask;
    wxCheckBox *bottomDeadSpotAngleMask;
    wxCheckBox *accumulatedEnergyMask;
    wxStaticText *samplingRate;
    wxStaticText *calibrationDate;
    wxStaticText *enhancedOffsetCompensationValue;

    // Cycling power vector page

    wxCheckBox *crankRevolutionDataVectorPresent;
    wxCheckBox *firstCrankMeasurementAnglePresent;
    wxCheckBox *instantaneousForceMagnitudeArrayPresent;
    wxCheckBox *instantaneousTorqueMagnitudeArrayPresent;
    wxStaticText *instantaneousMeasurementDirection;
    wxStaticText *cumulativeCrankVectorRevolutions;
    wxStaticText *lastCrankEventVectorTime;
    wxStaticText *firstCrankMeasurementAngle;
    wxWrapSizer *forceArraySizer;
    wxWrapSizer *torqueArraySizer;
    wxFile logVector;

    // InfoCrank control point page
    wxTextCtrl *setSerialNumber;
    wxTextCtrl *setFactoryCalibrationDate;
    wxTextCtrl *k1;
    wxTextCtrl *k2;
    wxTextCtrl *k3;
    wxTextCtrl *k4;
    wxTextCtrl *k5;
    wxTextCtrl *k6;
    wxTextCtrl *s2alpha;
    wxTextCtrl *s2accel;
    wxTextCtrl *driveRatio;
    wxTextCtrl *r1;
    wxTextCtrl *r2;
    wxTextCtrl *a1[12];
    wxTextCtrl *a2[12];
    wxTextCtrl *ble_addr[6];
    wxTextCtrl *cpvSize;
    wxTextCtrl *cpvDownsample;


    // InfoCrank raw data page
    wxTextCtrl *strain;
    wxTextCtrl *strain_avg;
    wxTextCtrl *strain_sd;
    wxTextCtrl *temperature;
    wxTextCtrl *batteryVoltage;
    wxTextCtrl *accel1[3];
    wxTextCtrl *accel1_avg[3];
    wxTextCtrl *accel1_sd[3];
    wxTextCtrl *accel2[3];
    wxTextCtrl *accel2_avg[3];
    wxTextCtrl *accel2_sd[3];
    wxRadioBox *orientation;

    wxTextCtrl *x;
    wxTextCtrl *x_dot;
    wxTextCtrl *x_ddot;

    wxFile logRaw;

    int strain_num;
    long strain_sum;
    long strain_sum2;

    double temp;
    double volts;

    double accel1_num;
    double accel1X_sum;
    double accel1X_sum2;
    double accel1Y_sum;
    double accel1Y_sum2;
    double accel1Z_sum;
    double accel1Z_sum2;

    double accel2_num;
    double accel2X_sum;
    double accel2X_sum2;
    double accel2Y_sum;
    double accel2Y_sum2;
    double accel2Z_sum;
    double accel2Z_sum2;

    // The accelerometer output in each of the 6 orientations
    double gravity1matrix[18] = {
        1.0,0.0,0.0,
        -1.0,0.0,0.0,
        0.0,1.0,0.0,
        0.0,-1.0,0.0,
        0.0,0.0,1.0,
        0.0,0.0,-1.0
    };
    double gravity2matrix[18] = {
        1.0,0.0,0.0,
        -1.0,0.0,0.0,
        0.0,1.0,0.0,
        0.0,-1.0,0.0,
        0.0,0.0,1.0,
        0.0,0.0,-1.0
    };

    // InfoCrank graphics page

    //    CrankTimer *crank_timer;


    IC2Frame();
    void ConnectSocket();
    void SendCommand(const char *);

    void LogFileName(wxCommandEvent &evt);

    // Menu
    void OnQuit(wxCommandEvent &evt);
    void OnDisconnect(wxCommandEvent &evt);

    // Devices page
    //void OnScan(wxCommandEvent &evt);
    void AddDevice(const char *name, const char *address);
    void RemoveDevice(const char *address);
    void OnConnect(wxCommandEvent &evt);

    // Device information page

    // Battery information page
    void SetBatteryLevel(uint8_t level);

    // Cycling power features page

    // Device information page
    void SetManufacturerName(const char *str);
    void SetModelNumber(const char *str);
    void SetSerialNumber(const char *str);
    void SetHardwareRevisionNumber(const char *str);
    void SetFirmwareRevisionNumber(const char *str);
    void SetSoftwareRevisionNumber(const char *str);
    void SetSystemID(const char *str);
    void SetIEEE(const char *str);
    void SetPNP(void *str);

    // Cycling power feature page
    void SetCyclingPowerFeature(void *str);

    // Cycling power measurement page
    void SetCyclingPowerMeasurement(void *str, int length);

    // Sensor location page
    void SetSensorLocation(uint8_t idx);

    // Cycling power control point page
    void SetCyclingPowerControlPoint(void *str, int length);
    void SetSensorLocation(wxCommandEvent &evt);
    void MaskMeasurement(wxCommandEvent &evt);

    // Cycling power vector page
    void SetCyclingPowerVector(void *str, int length);

    // InfoCrank control point page
    void SetInfoCrankControlPoint(void *str, int length);

    // InfoCrank raw data page
    void SetInfoCrankRawData(void *str, int length);

    // InfoCrank graphics page
    void OnCountdownTimer(wxTimerEvent& event);
    wxDECLARE_EVENT_TABLE();



private:
    wxStaticText* disconnectCountdownLabel;
    wxTimer* countdownTimer;
    int countdownSeconds;

    wxPanel* overlayPanel;
    bool isDraggingOverlay = false;
    wxPoint dragStartPos;

    void SetOverlayText(const wxString& text);
    // void OnOverlayMouseDown(wxMouseEvent& event);
    // void OnOverlayMouseMove(wxMouseEvent& event);
    // void OnOverlayMouseUp(wxMouseEvent& event);

};


//--------------------------------------------------------------------------------------------------
// A Bluetooth Low Energy Device
//--------------------------------------------------------------------------------------------------
class BLEDevice : public wxStaticBoxSizer
{
public:
    class UserData : public wxObject
    {
    public:
        char address[20];
    public:
        UserData(const char *addr)
        {
            strcpy(address, addr);
        };
    };
public:
    BLEDevice(IC2Frame *context, wxWindow *parent, const wxString &name, const wxString &address);
    ~BLEDevice();
};

//--------------------------------------------------------------------------------------------------
// controls and menu constants
//--------------------------------------------------------------------------------------------------
enum {
    DISCONNECT = wxID_HIGHEST + 1,
    ADD_DEVICE,
    LAYOUT_TEST_NB_SIZER,
    LAYOUT_TEST_GB_SIZER,
    LAYOUT_TEST_PROPORTIONS,
    LAYOUT_TEST_SET_MINIMAL,
    LAYOUT_TEST_NESTED,
    LAYOUT_TEST_WRAP,
};

#endif // _MAIN_H

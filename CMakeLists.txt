cmake_minimum_required(VERSION 3.28.1)

project(diagnostic)#cb

find_package(wxWidgets REQUIRED gl core base OPTIONAL_COMPONENTS net)
include(${wxWidgets_USE_FILE})

find_package(PkgConfig REQUIRED)
#pkg_check_modules(GLIB REQUIRED glib-2.0 gio-2.0)

include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})
add_definitions(${GLIB_CFLAGS_OTHER})

#target_link_libraries(diagnostic ${GLIB_LIBRARIES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0 gio-2.0)

include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})  # Optional — usually not needed
add_definitions(${GLIB_CFLAGS_OTHER})

#target_link_libraries(diagnostic )

#include_directories(${GLIB_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS})

link_directories(/usr/local/lib /usr/lib)
link_directories(/home/anna/Downloads/new_folder/bluez-source/src/libshared-mainloop.la)

include_directories(
  ~/bluez-5.80
  /usr/include/dbus-1.0/
 ${PROJECT_SOURCE_DIR}/src
  /usr/include/glib-2.0
  /usr/lib/x86_64-linux-gnu/glib-2.0/include
  /usr/lib/x86_64-linux-gnu/dbus-1.0/include
  /usr/include/gio-unix-2.0
  /home/anna/Downloads/new_folder/bluez-5.80
  /usr/include/glib-2.0
  /usr/lib/x86_64-linux-gnu/glib-2.0/include
  /home/anna/Downloads/new_folder/bluez-5.80/gdbus
  #/usr/lib/x86_64-linux-gnu/libglib-2.0.so
  #/usr/lib/x86_64-linux-gnu/libgio-2.0.so
)


add_executable(diagnostic
  ${PROJECT_SOURCE_DIR}/src/main.cpp
  ${PROJECT_SOURCE_DIR}/src/thread.cpp
  ${PROJECT_SOURCE_DIR}/src/crank-canvas.cpp
)

target_link_directories(diagnostic PUBLIC
  ~/bluez-5.80/gdbus/.libs
  ~/bluez-5.80/src/.libs
)


target_link_libraries(diagnostic PUBLIC
  ${wxWidgets_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GIO_LIBRARIES}
  glib-2.0
 gio-2.0  # Ensure this is added for GDBus and GIO functionality
  dbus-1
  #gdbus-internal
  explain
  gsl
  GL
  GLEW
  ${GLIB_LIBRARIES}
 ${PROJECT_SOURCE_DIR}/../bluez-5.80/gdbus/.libs/libgdbus-internal.a
  ${PROJECT_SOURCE_DIR}/../bluez-5.80/src/.libs/libshared-mainloop.a
)




#target_link_libraries(diagnostic PUBLIC
#  ${wxWidgets_LIBRARIES}
 # ${GLIB_LIBRARIES}
 # ${GIO_LIBRARIES}
 # glib-2.0
 #dbus-1
 #gdbus-internal
  #shared-mainloop
 #explain
 # gsl
 # GL
#  glfw
 # GLEW
#)

install(TARGETS diagnostic RUNTIME DESTINATION bin)

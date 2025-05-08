#ifndef HELPERS_H
#define HELPERS_H


#include <wx/menu.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>

// Function declarations here
class IC2Frame;  // Forward declaration

// Helper function declarations
void CreateMenuBar(IC2Frame* frame);
void CreateNotebookPages(IC2Frame* frame);
void SetupDevicePanels(IC2Frame* frame);
void SetupDeviceInfoPage(IC2Frame* frame, wxSizerFlags& fieldFlags, wxSizerFlags& bottomRightFlags);
void SetupBatteryPage(IC2Frame* frame, wxSizerFlags& fieldFlags, wxSizerFlags& groupBoxFlags, wxSizerFlags& groupBoxInnerFlags, wxSizerFlags& rightFlags);
void SetupFeaturesPage(IC2Frame* frame, wxSizerFlags& fieldFlags, wxSizerFlags& bottomRightFlags);
void SetupMeasurementPage(IC2Frame* frame);
void SetupSensorLocationPage(IC2Frame* frame, wxSizerFlags& fieldFlags);
void SetupBindControls(IC2Frame* frame);
void SetupControlPointPage(IC2Frame* frame);
void SetupVectorPage(IC2Frame* frame);
void SetupInfoCrankControlPage(IC2Frame* frame);
void SetupInfoCrankRawPage(IC2Frame* frame);
void SetupCountdownTimer(IC2Frame* frame);

void setupFunction(IC2Frame* frame, wxSizerFlags& fieldFlags,wxSizerFlags& bottomRightFlags); //TODO - rename
void bindControls(IC2Frame* frame);
void layoutPage(IC2Frame* frame, wxSizerFlags& fieldFlags, wxSizerFlags& groupBoxFlags, wxSizerFlags& groupBoxInnerFlags, wxSizerFlags& gridFlags, wxSizerFlags& rightFlags);

void InitializeSizerFlags(wxSizerFlags& fieldFlags,wxSizerFlags& groupBoxFlags,wxSizerFlags& groupBoxInnerFlags,wxSizerFlags& rightFlags,wxSizerFlags& bottomRightFlags,wxSizerFlags& centreFlags,wxSizerFlags& gridFlags);


#endif // HELPERS_H

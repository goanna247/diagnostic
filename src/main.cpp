/*
cd /mnt/fractal/Projects/BicyclePowerMeter/InfoCrank2021/Software/DiagnosticGUI/build/

g++ -c -I ~/Software/bluez-source $(pkg-config --cflags glib-2.0) $(pkg-config --cflags dbus-1) $(wx-config --cxxflags) ../src/main.cpp ../src/thread.cpp ../src/crank-canvas.cpp && \
g++ -o diagnostic \
main.o \
thread.o \
crank-canvas.o \
-L ~/Software/bluez-source/gdbus/.libs \
-lgdbus-internal -lexplain -lgsl -lGL -lGLEW \
$(wx-config --libs --gl-libs) \
$(pkg-config --libs glib-2.0) \
$(pkg-config --libs dbus-1) && \
./diagnostic | tee log.txt

*/

#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iomanip>
#include <cmath>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>

#include "icon.xpm"
#include "view-refresh.xpm"
#include "arrow-right.xpm"

#include "thread.h"
#include "crank-canvas.h"

// -------------------------------------------------------------------------------------------------
// The application
// -------------------------------------------------------------------------------------------------
IMPLEMENT_APP(IC2App)

bool IC2App::OnInit()
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    // Create the frame
    IC2Frame *frame = new IC2Frame();
    frame->Show();

    // create the thread for Bluetooth communication with InfoCrank
    IC2Thread *thread = new IC2Thread(frame);
    wxThreadError err = thread->Create();

    if (err != wxTHREAD_NO_ERROR) {
        wxMessageBox(_("Couldn't create thread!"));
        return false;
    }

    err = thread->Run();

    if (err != wxTHREAD_NO_ERROR) {
        wxMessageBox(_("Couldn't run thread!"));
        return false;
    }

    // Let the thread start before opening the socket connection
    usleep(500000);
    frame->ConnectSocket();
    return true;
}





// -------------------------------------------------------------------------------------------------
// The main frame
// -------------------------------------------------------------------------------------------------
IC2Frame::IC2Frame() : wxFrame(NULL, wxID_ANY,  wxT("Verve IC2 Diagnostic Tool"), wxPoint(50, 50), wxSize(800, 600))
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    // The icon in the title bar
    SetIcon(wxICON(icon));

    // The menubar
    wxMenu *file_menu = new wxMenu;

    file_menu->Append(DISCONNECT, "&Disconnect...\tF1", "Disconnect all", wxITEM_CHECK);
    file_menu->Append(ADD_DEVICE, "&Add Device...\tF2");
    file_menu->Append(LAYOUT_TEST_NB_SIZER, "Test &notebook sizers...\tF3");
    file_menu->Append(LAYOUT_TEST_GB_SIZER, "Test &gridbag sizer...\tF4");
    file_menu->Append(LAYOUT_TEST_SET_MINIMAL, "Test Set&ItemMinSize...\tF5");
    file_menu->Append(LAYOUT_TEST_NESTED, "Test nested sizer in a wxPanel...\tF6");
    file_menu->Append(LAYOUT_TEST_WRAP, "Test wrap sizers...\tF7");
    file_menu->AppendSeparator();
    file_menu->Append(wxID_EXIT, "E&xit", "Quit program");

    // Bind menu events
    Bind(wxEVT_MENU, [&](wxCommandEvent & evt) {
        SendCommand("Disconnect all\n");
    }, DISCONNECT);
    Bind(wxEVT_MENU, [&](wxCommandEvent & evt) {
        SendCommand("Quit\n");
    }, wxID_EXIT);

    wxMenu *help_menu = new wxMenu;
    help_menu->Append(wxID_ABOUT, "&About", "About layout demo...");

    wxMenuBar *menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, "&File");
    menu_bar->Append(help_menu, "&Help");
    SetMenuBar(menu_bar);

    // The status bar
    CreateStatusBar(2);
//    SetStatusText("Status Bar Section 0", 0);
//    SetStatusText("Status Bar Section 1", 1);

    // The main panel
    wxNotebook *notebook = new wxNotebook(this, wxID_ANY);
    devices = new wxPanel(notebook, wxID_ANY);
    wxPanel *devInfo = new wxPanel(notebook, wxID_ANY);
    wxPanel *battery = new wxPanel(notebook, wxID_ANY);
    wxPanel *features = new wxPanel(notebook, wxID_ANY);
    wxPanel *control = new wxPanel(notebook, wxID_ANY);
    wxPanel *sensor_location = new wxPanel(notebook, wxID_ANY);
    measurement = new wxPanel(notebook, wxID_ANY);
    vector = new wxPanel(notebook, wxID_ANY);
    wxPanel *infoCrank_control = new wxPanel(notebook, wxID_ANY);
    wxPanel *infoCrank_raw = new wxPanel(notebook, wxID_ANY);

    crank_graphics = new CrankCanvas(notebook, wxID_ANY);

    wxPanel *page12 = new wxPanel(notebook, wxID_ANY);

    notebook->AddPage(devices, "Devices");
    notebook->AddPage(devInfo, "Device Information");
    notebook->AddPage(battery, "Battery Information");
    notebook->AddPage(features, "Features");
    notebook->AddPage(measurement, "Measurement");
    notebook->AddPage(sensor_location, "Sensor Location");
    notebook->AddPage(control, "Control Point");
    notebook->AddPage(vector, "Vector");
    notebook->AddPage(infoCrank_control, "InfoCrank Control Point");
    notebook->AddPage(infoCrank_raw, "InfoCrank Raw Data");
    notebook->AddPage(crank_graphics, "Graphics");
    notebook->AddPage(page12, "Page 12");

    // Common sizer flags
    wxSizerFlags fieldFlags;
    fieldFlags.Border(wxLEFT | wxRIGHT, 10);
    wxSizerFlags groupBoxFlags(0);
    groupBoxFlags.Expand().Border(wxTOP | wxLEFT | wxRIGHT, 10);
    wxSizerFlags groupBoxInnerFlags(0);
    groupBoxInnerFlags.Expand().Border(wxBOTTOM, 10);
    wxSizerFlags rightFlags;
    rightFlags.Align(wxALIGN_RIGHT).Border(wxRIGHT, 10);
    wxSizerFlags bottomRightFlags;
    bottomRightFlags.Align(wxALIGN_RIGHT | wxALIGN_BOTTOM).Border(wxBOTTOM | wxRIGHT, 10);
    wxSizerFlags centreFlags;
    centreFlags.Align(wxALIGN_CENTRE | wxALIGN_BOTTOM).Border(wxALL, 5);
    wxSizerFlags gridFlags;
    gridFlags.Expand().Border(wxALL, 0);


    // Devices panel
    devicesSizer = new wxWrapSizer(wxHORIZONTAL);
    devices->SetSizerAndFit(devicesSizer);

    // Device information panel
    manufacturerName = new wxStaticText(devInfo, wxID_ANY, "-");
    modelNumber = new wxStaticText(devInfo, wxID_ANY, "-");
    serialNumber = new wxStaticText(devInfo, wxID_ANY, "-");
    hardwareRevisionNumber = new wxStaticText(devInfo, wxID_ANY, "-");
    firmwareRevisionNumber = new wxStaticText(devInfo, wxID_ANY, "-");
    softwareRevisionNumber = new wxStaticText(devInfo, wxID_ANY, "-");
    systemID = new wxStaticText(devInfo, wxID_ANY, "-");
    PNPVendorIDSource = new wxStaticText(devInfo, wxID_ANY, "-");
    PNPVendorID = new wxStaticText(devInfo, wxID_ANY, "-");
    PNPProductID = new wxStaticText(devInfo, wxID_ANY, "-");
    PNPProductVersion = new wxStaticText(devInfo, wxID_ANY, "-");
    wxButton *refreshDeviceInformation = new wxButton(devInfo, wxID_ANY, "Refresh");

    // Bind the controls
    refreshDeviceInformation->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Refresh device information\n");
    });

    {
        wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 0, 0);
        sizer->AddGrowableCol(1);
        sizer->AddGrowableRow(11);
        sizer->Add(new wxStaticText(devInfo, wxID_ANY, "Manufacturer name"), fieldFlags);
        sizer->Add(manufacturerName, fieldFlags);
        sizer->Add(new wxStaticText(devInfo, wxID_ANY, "Model number"), fieldFlags);
        sizer->Add(modelNumber, fieldFlags);
        sizer->Add(new wxStaticText(devInfo, wxID_ANY, "Serial number"), fieldFlags);
        sizer->Add(serialNumber, fieldFlags);
        sizer->Add(new wxStaticText(devInfo, wxID_ANY, "Hardware revision number"), fieldFlags);
        sizer->Add(hardwareRevisionNumber, fieldFlags);
        sizer->Add(new wxStaticText(devInfo, wxID_ANY, "Firmware revision number"), fieldFlags);
        sizer->Add(firmwareRevisionNumber, fieldFlags);
        sizer->Add(new wxStaticText(devInfo, wxID_ANY, "Software revision"), fieldFlags);
        sizer->Add(softwareRevisionNumber, fieldFlags);
        sizer->Add(new wxStaticText(devInfo, wxID_ANY, "System ID"), fieldFlags);
        sizer->Add(systemID, fieldFlags);
        sizer->Add(new wxStaticText(devInfo, wxID_ANY, "PNP vendor ID source"), fieldFlags);
        sizer->Add(PNPVendorIDSource, fieldFlags);
        sizer->Add(new wxStaticText(devInfo, wxID_ANY, "PNP vendor ID"), fieldFlags);
        sizer->Add(PNPVendorID, fieldFlags);
        sizer->Add(new wxStaticText(devInfo, wxID_ANY, "PNP product ID"), fieldFlags);
        sizer->Add(PNPProductID, fieldFlags);
        sizer->Add(new wxStaticText(devInfo, wxID_ANY, "PNP product version"), fieldFlags);
        sizer->Add(PNPProductVersion, fieldFlags);
        sizer->AddStretchSpacer();
        sizer->Add(refreshDeviceInformation, bottomRightFlags);
        devInfo->SetSizerAndFit(sizer);
    }

    // TODO Battery panel
    batteryLevel = new wxStaticText(battery, wxID_ANY, "-");
    wxButton *refreshBatteryInformation = new wxButton(battery, wxID_ANY, "Refresh");
    wxCheckBox *loggingBattery = new wxCheckBox(battery, wxID_ANY, "/dev/null");
    wxButton *logFileBattery = new wxButton(battery, wxID_ANY, "...", wxDefaultPosition, wxSize(50, 20));

    // Bind the controls
    refreshBatteryInformation->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Refresh battery information\n");
    });

    logFileBattery->Bind(wxEVT_BUTTON, &IC2Frame::LogFileName, this, wxID_ANY, wxID_ANY, new FileDialogParameters("battery.log", loggingBattery));
    loggingBattery->Bind(wxEVT_CHECKBOX,
    [&](wxCommandEvent & evt) {
        wxCheckBox *checkBox = (wxCheckBox *) evt.GetEventObject();
        if (checkBox->IsChecked()) {
//            logBattery.Create(checkBox->GetLabel(), true);
            logBattery.Open(checkBox->GetLabel(), wxFile::write);
        } else {
            logBattery.Close();
        }
    });

    // Layout the page
    {
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        {
        wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
            flexGridSizer->AddGrowableCol(1);
            //flexGridSizer->AddGrowableRow(12);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "Battery level"), fieldFlags);
            flexGridSizer->Add(batteryLevel, fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "Battery level status"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "-"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "Estimated Service Date"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "-"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "Battery critical status"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "-"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "Battery energy status"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "-"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "Battery time status"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "-"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "Battery health status"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "-"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "Battery health information"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "-"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "Battery information"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "-"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "Manufacturer name"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "-"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "Model number"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "-"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "Serial number"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(battery, wxID_ANY, "-"), fieldFlags);
            sizer->Add(flexGridSizer, groupBoxFlags);
        }

        sizer->AddStretchSpacer();

        {
            wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
            boxSizer->Add(loggingBattery, fieldFlags);
            boxSizer->Add(logFileBattery, fieldFlags);
            boxSizer->AddStretchSpacer();
            boxSizer->Add(refreshBatteryInformation, rightFlags);
            sizer->Add(boxSizer, groupBoxInnerFlags);
        }

        battery->SetSizerAndFit(sizer);
    }

    // Cycling Power Features
    balanceFeature = new wxCheckBox(features, wxID_ANY, "Bit 0");
//    balanceFeature->Enable(false);
    torqueFeature = new wxCheckBox(features, wxID_ANY, "Bit 1");
//    torqueFeature->Enable(false);
    wheelRevolutionFeature = new wxCheckBox(features, wxID_ANY, "Bit 2");
//    wheelRevolutionFeature->Enable(false);
    crankRevolutionFeature = new wxCheckBox(features, wxID_ANY, "Bit 3");
//    crankRevolutionFeature->Enable(false);
    magnitudesFeature = new wxCheckBox(features, wxID_ANY, "Bit 4");
//    magnitudesFeature->Enable(false);
    anglesFeature = new wxCheckBox(features, wxID_ANY, "Bit 5");
//    anglesFeature->Enable(false);
    deadSpotsFeature = new wxCheckBox(features, wxID_ANY, "Bit 6");
//    deadSpotsFeature->Enable(false);
    energyFeature = new wxCheckBox(features, wxID_ANY, "Bit 7");
//    energyFeature->Enable(false);
    compensationIndicatorFeature = new wxCheckBox(features, wxID_ANY, "Bit 8");
//    compensationIndicatorFeature->Enable(false);
    compensationFeature = new wxCheckBox(features, wxID_ANY, "Bit 9");
//    compensationFeature->Enable(false);
    maskingFeature = new wxCheckBox(features, wxID_ANY, "Bit 10");
//    maskingFeature->Enable(false);
    locationsFeature = new wxCheckBox(features, wxID_ANY, "Bit 11");
//    locationsFeature->Enable(false);
    crankLengthFeature = new wxCheckBox(features, wxID_ANY, "Bit 12");
//    crankLengthFeature->Enable(false);
    chainLengthFeature = new wxCheckBox(features, wxID_ANY, "Bit 13");
//    chainLengthFeature->Enable(false);
    chainWeightFeature = new wxCheckBox(features, wxID_ANY, "Bit 14");
//    chainWeightFeature->Enable(false);
    spanFeature = new wxCheckBox(features, wxID_ANY, "Bit 15");
//    spanFeature->Enable(false);
    contextFeature = new wxStaticText(features, wxID_ANY, "Bit 16");
    directionFeature = new wxCheckBox(features, wxID_ANY, "Bit 17");
//    directionFeature->Enable(false);
    dateFeature = new wxCheckBox(features, wxID_ANY, "Bit 18");
//    dateFeature->Enable(false);
    enhancedCompensationFeature = new wxCheckBox(features, wxID_ANY, "Bit 19");
//    enhancedCompensationFeature->Enable(false);
    distributedFeature = new wxStaticText(features, wxID_ANY, "Bit 20-21");
    wxButton *refreshFeatures = new wxButton(features, wxID_ANY, "Refresh");

    // Bind the controls
    refreshFeatures->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Refresh features\n");
    });

    {
        wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 0, 0);
        sizer->AddGrowableCol(1);
        sizer->AddGrowableRow(20);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Pedal power balance supported"), fieldFlags);
        sizer->Add(balanceFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Accumulated torque supported"), fieldFlags);
        sizer->Add(torqueFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Wheel revolution data supported"), fieldFlags);
        sizer->Add(wheelRevolutionFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Crank revolution data supported"), fieldFlags);
        sizer->Add(crankRevolutionFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Extreme magnitudes supported"), fieldFlags);
        sizer->Add(magnitudesFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Extreme angles supported"), fieldFlags);
        sizer->Add(anglesFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Top and bottom dead spot angles supported"), fieldFlags);
        sizer->Add(deadSpotsFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Accumulated energy supported"), fieldFlags);
        sizer->Add(energyFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Offset compensation indicator supported"), fieldFlags);
        sizer->Add(compensationIndicatorFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Offset compensation supported"), fieldFlags);
        sizer->Add(compensationFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Cycling power measurement characteristic content masking supported"), fieldFlags);
        sizer->Add(maskingFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Multiple sensor locations supported"), fieldFlags);
        sizer->Add(locationsFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Crank length adjustment supported"), fieldFlags);
        sizer->Add(crankLengthFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Chain length adjustment supported"), fieldFlags);
        sizer->Add(chainLengthFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Chain weight adjustment supported"), fieldFlags);
        sizer->Add(chainWeightFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Span length adjustment supported"), fieldFlags);
        sizer->Add(spanFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Sensor measurement context"), fieldFlags);
        sizer->Add(contextFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Instantaneous measurement direction supported"), fieldFlags);
        sizer->Add(directionFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Factory calibration date supported"), fieldFlags);
        sizer->Add(dateFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Enhanced offset compensation procedure supported"), fieldFlags);
        sizer->Add(enhancedCompensationFeature, fieldFlags);
        sizer->Add(new wxStaticText(features, wxID_ANY, "Distributed system support"), fieldFlags);
        sizer->Add(distributedFeature, fieldFlags);
        sizer->AddStretchSpacer();
        sizer->Add(refreshFeatures, bottomRightFlags);
        features->SetSizerAndFit(sizer);
    }

    // Cycling power measurement
    pedalPowerBalancePresent = new wxCheckBox(measurement, wxID_ANY, "Balance: Unknown");
//    pedalPowerBalancePresent->Enable(false);
    accumulatedTorquePresent = new wxCheckBox(measurement, wxID_ANY, "Accumulated torque: Wheel based");
//    accumulatedTorquePresent->Enable(false);
    wheelRevolutionDataPresent = new wxCheckBox(measurement, wxID_ANY, "Wheel revolution data");
//    wheelRevolutionDataPresent->Enable(false);
    crankRevolutionDataPresent = new wxCheckBox(measurement, wxID_ANY, "Crank revolution data");
//    crankRevolutionDataPresent->Enable(false);
    extremeForceMagnitudesPresent = new wxCheckBox(measurement, wxID_ANY, "Extreme force magnitudes");
//    extremeForceMagnitudesPresent->Enable(false);
    extremeTorqueMagnitudesPresent = new wxCheckBox(measurement, wxID_ANY, "Extreme torque magnitudes");
//    extremeTorqueMagnitudesPresent->Enable(false);
    extremeAnglesPresent = new wxCheckBox(measurement, wxID_ANY, "Extreme angles");
//    extremeAnglesPresent->Enable(false);
    topDeadSpotAnglePresent = new wxCheckBox(measurement, wxID_ANY, "Top dead spot angle");
//    topDeadSpotAnglePresent->Enable(false);
    bottomDeadSpotAnglePresent = new wxCheckBox(measurement, wxID_ANY, "Bottom dead spot angle");
//    bottomDeadSpotAnglePresent->Enable(false);
    accumulatedEnergyPresent = new wxCheckBox(measurement, wxID_ANY, "Accumulated energy");
//    accumulatedEnergyPresent->Enable(false);
    offsetCompensationIndicator = new wxCheckBox(measurement, wxID_ANY, "Offset compensation indicator");
//    offsetCompensationIndicator->Enable(false);
    instantaneousPower = new wxStaticText(measurement, wxID_ANY, "-");
    pedalPowerBalance = new wxStaticText(measurement, wxID_ANY, "-");
    accumulatedTorque = new wxStaticText(measurement, wxID_ANY, "-");
    wxPanel *torquePanel = new wxPanel(measurement);
    torquePanel->SetBackgroundColour(wxColour(wxT("YELLOW")));
    torque = new wxStaticText(torquePanel, wxID_ANY, "-");
    torque->SetForegroundColour(wxColour(wxT("RED")));
    cumulativeWheelRevolutions = new wxStaticText(measurement, wxID_ANY, "-");
    lastWheelEventTime = new wxStaticText(measurement, wxID_ANY, "-");
    wxPanel *wheelSpeedPanel = new wxPanel(measurement);
    wheelSpeedPanel->SetBackgroundColour(wxColour(wxT("YELLOW")));
    wheelSpeed = new wxStaticText(wheelSpeedPanel, wxID_ANY, "-");
    wheelSpeed->SetForegroundColour(wxColour(wxT("RED")));
    cumulativeCrankRevolutions = new wxStaticText(measurement, wxID_ANY, "-");
    lastCrankEventTime = new wxStaticText(measurement, wxID_ANY, "-");
    wxPanel *cadencePanel = new wxPanel(measurement);
    cadencePanel->SetBackgroundColour(wxColour(wxT("YELLOW")));
    cadence = new wxStaticText(cadencePanel, wxID_ANY, "-");
    cadence->SetForegroundColour(wxColour(wxT("RED")));
    maximumForceMagnitude = new wxStaticText(measurement, wxID_ANY, "-");
    minimumForceMagnitude = new wxStaticText(measurement, wxID_ANY, "-");
    maximumTorqueMagnitude = new wxStaticText(measurement, wxID_ANY, "-");
    minimumTorqueMagnitude = new wxStaticText(measurement, wxID_ANY, "-");
    maximumAngle = new wxStaticText(measurement, wxID_ANY, "-");
    minimumAngle = new wxStaticText(measurement, wxID_ANY, "-");
    topDeadSpotAngle = new wxStaticText(measurement, wxID_ANY, "-");
    bottomDeadSpotAngle = new wxStaticText(measurement, wxID_ANY, "-");
    accumulatedEnergy = new wxStaticText(measurement, wxID_ANY, "-");
    wxToggleButton *notifyMeasurement = new wxToggleButton(measurement, wxID_ANY, "Notify");
    wxToggleButton *broadcastMeasurement = new wxToggleButton(measurement, wxID_ANY, "Broadcast");
    wxCheckBox *loggingMeasurement = new wxCheckBox(measurement, wxID_ANY, "/dev/null");
    wxButton *logFileMeasurement = new wxButton(measurement, wxID_ANY, "...", wxDefaultPosition, wxSize(50, 20));

    // Bind the controls
    notifyMeasurement->Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent & evt) {
        switch (evt.GetInt()) {
        case TRUE:
            ((wxToggleButton *) evt.GetEventObject())->SetLabel("Stop");
            SendCommand("Notify measurement on\n");
            break;
        case FALSE:
            ((wxToggleButton *) evt.GetEventObject())->SetLabel("Notify");
            SendCommand("Notify measurement off\n");
            break;
        }
    });
    broadcastMeasurement->Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent & evt) {
        switch (evt.GetInt()) {
        case TRUE:
            ((wxToggleButton *) evt.GetEventObject())->SetLabel("Stop");
            SendCommand("Broadcast measurement on\n");
            break;
        case FALSE:
            ((wxToggleButton *) evt.GetEventObject())->SetLabel("Broadcast");
            SendCommand("Broadcast measurement off\n");
            break;
        }
    });
    logFileMeasurement->Bind(wxEVT_BUTTON, &IC2Frame::LogFileName, this, wxID_ANY, wxID_ANY, new FileDialogParameters("measurement.log", loggingMeasurement));
    loggingMeasurement->Bind(wxEVT_CHECKBOX,
    [&](wxCommandEvent & evt) {
        wxCheckBox *checkBox = (wxCheckBox *) evt.GetEventObject();
        if (checkBox->IsChecked()) {
//            logMeasurement.Create(checkBox->GetLabel(), true);
            logMeasurement.Open(checkBox->GetLabel(), wxFile::write);
        } else {
            logMeasurement.Close();
        }
    });

    // Layout the page
    {
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

        {
            wxStaticBoxSizer  *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, measurement, "Flags");
            {
                wxWrapSizer *wrapSizer = new wxWrapSizer(wxVERTICAL);
                wrapSizer->Add(pedalPowerBalancePresent, fieldFlags);
                wrapSizer->Add(accumulatedTorquePresent, fieldFlags);
                wrapSizer->Add(wheelRevolutionDataPresent, fieldFlags);
                wrapSizer->Add(crankRevolutionDataPresent, fieldFlags);
                wrapSizer->Add(extremeForceMagnitudesPresent, fieldFlags);
                wrapSizer->Add(extremeTorqueMagnitudesPresent, fieldFlags);
                wrapSizer->Add(extremeAnglesPresent, fieldFlags);
                wrapSizer->Add(topDeadSpotAnglePresent, fieldFlags);
                wrapSizer->Add(bottomDeadSpotAnglePresent, fieldFlags);
                wrapSizer->Add(accumulatedEnergyPresent, fieldFlags);
                wrapSizer->Add(offsetCompensationIndicator, fieldFlags);
                staticBoxSizer->Add(wrapSizer, groupBoxInnerFlags);
            }
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }

        {
            wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
            flexGridSizer->AddGrowableCol(0);
            flexGridSizer->AddGrowableCol(1);

            {
                wxStaticBoxSizer  *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, measurement, "Instantaneous power");
                staticBoxSizer->Add(instantaneousPower, fieldFlags);
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer  *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, measurement, "Pedal power balance");
                staticBoxSizer->Add(pedalPowerBalance, fieldFlags);
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer  *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, measurement, "Accumulated torque");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Accumulated torque"), fieldFlags);
                    flexGridSizer->Add(accumulatedTorque, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Instantaneous torque"), fieldFlags);
                    flexGridSizer->Add(torquePanel, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer  *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, measurement, "Accumulated energy");
                staticBoxSizer->Add(accumulatedEnergy, fieldFlags);
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, measurement, "Wheel revolution data");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Cumulative wheel revolutions"), fieldFlags);
                    flexGridSizer->Add(cumulativeWheelRevolutions, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Last wheel event time"), fieldFlags);
                    flexGridSizer->Add(lastWheelEventTime, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Wheel speed"), fieldFlags);
                    flexGridSizer->Add(wheelSpeedPanel, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, measurement, "Crank revolution data");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Cumulative crank revolutions"), fieldFlags);
                    flexGridSizer->Add(cumulativeCrankRevolutions, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Last crank event time"), fieldFlags);
                    flexGridSizer->Add(lastCrankEventTime, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Cadence"), fieldFlags);
                    //crankRevolutionSizerInnerSizer->Add(cadence, fieldFlags);
                    flexGridSizer->Add(cadencePanel, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, measurement, "Extreme force magnitudes");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Maximum force magnitude"), fieldFlags);
                    flexGridSizer->Add(maximumForceMagnitude, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Minimum force magnitude"), fieldFlags);
                    flexGridSizer->Add(minimumForceMagnitude, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, measurement, "Extreme torque magnitudes");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Maximum torque magnitude"), fieldFlags);
                    flexGridSizer->Add(maximumTorqueMagnitude, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Minimum torque magnitude"), fieldFlags);
                    flexGridSizer->Add(minimumTorqueMagnitude, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, measurement, "Extreme angles");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Maximum angle"), fieldFlags);
                    flexGridSizer->Add(maximumAngle, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Minimum angle"), fieldFlags);
                    flexGridSizer->Add(minimumAngle, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, measurement, "Dead spot angles");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Top dead spot angle"), fieldFlags);
                    flexGridSizer->Add(topDeadSpotAngle, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(measurement, wxID_ANY, "Bottom dead spot angle"), fieldFlags);
                    flexGridSizer->Add(bottomDeadSpotAngle, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            sizer->Add(flexGridSizer, gridFlags);
        }

        sizer->AddStretchSpacer();

        {
            wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
            boxSizer->Add(loggingMeasurement, fieldFlags);
            boxSizer->Add(logFileMeasurement, fieldFlags);
            boxSizer->AddStretchSpacer();
            boxSizer->Add(notifyMeasurement, fieldFlags);
            boxSizer->Add(broadcastMeasurement, rightFlags);
            sizer->Add(boxSizer, groupBoxInnerFlags);
        }

        measurement->SetSizerAndFit(sizer);
        measurement->PostSizeEvent();        // wxWrapSizer needs a resize to layout correctly
    }


    // Sensor location page
    wxButton *requestSensorLocation = new wxButton(sensor_location, wxID_ANY, "Get sensor location");
    sensorLocation = new wxStaticText(sensor_location, wxID_ANY, "-");

    // Bind the controls
    requestSensorLocation->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get sensor location\n");
    });

    // Layout the page
    {
        wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 0, 0);
        sizer->Add(requestSensorLocation, fieldFlags);
        sizer->Add(sensorLocation, fieldFlags);

        sensor_location->SetSizerAndFit(sizer);
    }

    // Cycling power control point
    wxTextCtrl *cumulative = new wxTextCtrl(control, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
    wxButton *supportedLocations = new wxButton(control, wxID_ANY, "Get supported sensor locations");
    location = new wxComboBox(control, wxID_ANY);
    crankLength = new wxTextCtrl(control, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC));
    wxButton *requestCrankLength = new wxButton(control, wxID_ANY, "Get crank length");
    chainLength = new wxTextCtrl(control, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
    wxButton *requestChainLength = new wxButton(control, wxID_ANY, "Get chain length");
    chainWeight = new wxTextCtrl(control, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
    wxButton *requestChainWeight = new wxButton(control, wxID_ANY, "Get chain weight");
    span = new wxTextCtrl(control, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
    wxButton *requestSpan = new wxButton(control, wxID_ANY, "Get span");
    wxButton *offsetCompensation = new wxButton(control, wxID_ANY, "Start offset compensation");
    offsetCompensationValue = new wxStaticText(control, wxID_ANY, "-");
    wxButton *maskMeasurement = new wxButton(control, wxID_ANY, "Mask cycling power measurement characteristic content");
    pedalPowerBalanceMask   = new wxCheckBox(control, wxID_ANY, "Balance");
    accumulatedTorqueMask   = new wxCheckBox(control, wxID_ANY, "Accumulated torque");
    wheelRevolutionDataMask = new wxCheckBox(control, wxID_ANY, "Wheel revolution");
    crankRevolutionDataMask = new wxCheckBox(control, wxID_ANY, "Crank revolution");
    extremeMagnitudesMask   = new wxCheckBox(control, wxID_ANY, "Extreme magnitudes");
    extremeAnglesMask       = new wxCheckBox(control, wxID_ANY, "Extreme angles");
    topDeadSpotAngleMask    = new wxCheckBox(control, wxID_ANY, "Top dead spot angle");
    bottomDeadSpotAngleMask = new wxCheckBox(control, wxID_ANY, "Bottom dead spot angle");
    accumulatedEnergyMask   = new wxCheckBox(control, wxID_ANY, "Accumulated energy");
    samplingRate = new wxStaticText(control, wxID_ANY, "-");
    wxButton *requestSamplingRate = new wxButton(control, wxID_ANY, "Get sampling rate");
    calibrationDate = new wxStaticText(control, wxID_ANY, "-");
    wxButton *requestCalibrationDate = new wxButton(control, wxID_ANY, "Get factory calibration date");
    wxButton *enhancedOffsetCompensation = new wxButton(control, wxID_ANY, "Start enhanced offset compensation");
    enhancedOffsetCompensationValue = new wxStaticText(control, wxID_ANY, "-");

    // Bind the controls
    cumulative->Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent & evt) {
        SendCommand(wxString().Format("Set cumulative value %s\n", cumulative->GetValue()));
    });
    supportedLocations->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get supported sensor locations\n");
    });
    location->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &IC2Frame::SetSensorLocation, this);
    requestCrankLength->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get crank length\n");
    });
    crankLength->Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent & evt) {
        SendCommand(wxString().Format("Set crank length %s\n", crankLength->GetValue()));
    });
    requestChainLength->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get chain length\n");
    });
    chainLength->Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent & evt) {
        SendCommand(wxString().Format("Set chain length %s\n", chainLength->GetValue()));
    });
    requestChainWeight->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get chain weight\n");
    });
    chainWeight->Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent & evt) {
        SendCommand(wxString().Format("Set chain weight %s\n", chainWeight->GetValue()));
    });
    requestSpan->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get span\n");
    });
    span->Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent & evt) {
        SendCommand(wxString().Format("Set span %s\n", span->GetValue()));
    });
    offsetCompensation->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Start offset compensation\n");
    });
    maskMeasurement->Bind(wxEVT_BUTTON, &IC2Frame::MaskMeasurement, this);

    requestSamplingRate->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get sampling rate\n");
    });
    requestCalibrationDate->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get factory calibration date\n");
    });
    enhancedOffsetCompensation->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Start enhanced offset compensation\n");
    });


    // Layout the page
    {
        wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 0, 0);
        sizer->AddGrowableCol(1);
        sizer->Add(new wxStaticText(control, wxID_ANY, "Set cumulative value"), fieldFlags);
        sizer->Add(cumulative, fieldFlags);
        sizer->Add(supportedLocations, fieldFlags);
        sizer->Add(location, fieldFlags);
        sizer->Add(requestCrankLength, fieldFlags);
        sizer->Add(crankLength, fieldFlags);
        sizer->Add(requestChainLength, fieldFlags);
        sizer->Add(chainLength, fieldFlags);
        sizer->Add(requestChainWeight, fieldFlags);
        sizer->Add(chainWeight, fieldFlags);
        sizer->Add(requestSpan, fieldFlags);
        sizer->Add(span, fieldFlags);
        sizer->Add(offsetCompensation, fieldFlags);
        sizer->Add(offsetCompensationValue, fieldFlags);
        sizer->Add(maskMeasurement, fieldFlags);

        {
            wxGridSizer *maskSizer = new wxGridSizer(2, 0, 0);
            maskSizer->Add(pedalPowerBalanceMask, fieldFlags);
            maskSizer->Add(accumulatedTorqueMask, fieldFlags);
            maskSizer->Add(wheelRevolutionDataMask, fieldFlags);
            maskSizer->Add(crankRevolutionDataMask, fieldFlags);
            maskSizer->Add(extremeMagnitudesMask, fieldFlags);
            maskSizer->Add(extremeAnglesMask, fieldFlags);
            maskSizer->Add(topDeadSpotAngleMask, fieldFlags);
            maskSizer->Add(bottomDeadSpotAngleMask, fieldFlags);
            maskSizer->Add(accumulatedEnergyMask, fieldFlags);
            sizer->Add(maskSizer, groupBoxInnerFlags);
        }

        sizer->Add(requestSamplingRate, fieldFlags);
        sizer->Add(samplingRate, fieldFlags);
        sizer->Add(requestCalibrationDate, fieldFlags);
        sizer->Add(calibrationDate, fieldFlags);
        sizer->Add(enhancedOffsetCompensation, fieldFlags);
        sizer->Add(enhancedOffsetCompensationValue, fieldFlags);

        control->SetSizerAndFit(sizer);
    }

    // Cycling power vector page
    crankRevolutionDataVectorPresent = new wxCheckBox(vector, wxID_ANY, "Crank revolution data present");
//    crankRevolutionDataVectorPresent->Enable(false);
    firstCrankMeasurementAnglePresent = new wxCheckBox(vector, wxID_ANY, "First crank angle measurement present");
//    firstCrankMeasurementAnglePresent->Enable(false);
    instantaneousForceMagnitudeArrayPresent = new wxCheckBox(vector, wxID_ANY, "Instantaneous force magnitude array present");
//    instantaneousForceMagnitudeArrayPresent->Enable(false);
    instantaneousTorqueMagnitudeArrayPresent = new wxCheckBox(vector, wxID_ANY, "Instantaneous torque magnitude array present");
//    instantaneousTorqueMagnitudeArrayPresent->Enable(false);
    instantaneousMeasurementDirection = new wxStaticText(vector, wxID_ANY, "-");
    cumulativeCrankVectorRevolutions = new wxStaticText(vector, wxID_ANY, "-");
    lastCrankEventVectorTime = new wxStaticText(vector, wxID_ANY, "-");
    firstCrankMeasurementAngle = new wxStaticText(vector, wxID_ANY, "-");
    forceArraySizer = new wxWrapSizer(wxHORIZONTAL);
    torqueArraySizer = new wxWrapSizer(wxHORIZONTAL);
    wxToggleButton *notifyVector = new wxToggleButton(vector, wxID_ANY, "Notify");
    wxCheckBox *loggingVector = new wxCheckBox(vector, wxID_ANY, "/dev/null");
    wxButton *logFileVector = new wxButton(vector, wxID_ANY, "...", wxDefaultPosition, wxSize(50, 20));

    // Bind the controls
    notifyVector->Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent & evt) {
        switch (evt.GetInt()) {
        case TRUE:
            ((wxToggleButton *) evt.GetEventObject())->SetLabel("Stop");
            SendCommand("Notify vector on\n");
            break;
        case FALSE:
            ((wxToggleButton *) evt.GetEventObject())->SetLabel("Notify");
            SendCommand("Notify vector off\n");
            break;
        }
    });
    logFileVector->Bind(wxEVT_BUTTON, &IC2Frame::LogFileName, this, wxID_ANY, wxID_ANY, new FileDialogParameters("vector.log", loggingVector));
//    loggingVector->Bind(wxEVT_CHECKBOX, &IC2Frame::LogFileOpen, this, wxID_ANY, wxID_ANY, new LogFile(&logVector));

    loggingVector->Bind(wxEVT_CHECKBOX,
    [&](wxCommandEvent & evt) {
        wxCheckBox *checkBox = (wxCheckBox *) evt.GetEventObject();
        if (checkBox->IsChecked()) {
//            logMeasurement.Create(checkBox->GetLabel(), true);
            logVector.Open(checkBox->GetLabel(), wxFile::write);
        } else {
            logVector.Close();
        }
    });


    // Layout the page
    {
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

        {
            wxStaticBoxSizer *vectorFlagsSizer = new wxStaticBoxSizer(wxVERTICAL, vector, "Flags");
            wxWrapSizer *vectorFlagsCheckBoxSizer = new wxWrapSizer(wxVERTICAL);
            vectorFlagsCheckBoxSizer->Add(crankRevolutionDataVectorPresent, fieldFlags);
            vectorFlagsCheckBoxSizer->Add(firstCrankMeasurementAnglePresent, fieldFlags);
            vectorFlagsCheckBoxSizer->Add(instantaneousForceMagnitudeArrayPresent, fieldFlags);
            vectorFlagsCheckBoxSizer->Add(instantaneousTorqueMagnitudeArrayPresent, fieldFlags);
            vectorFlagsSizer->Add(vectorFlagsCheckBoxSizer, groupBoxInnerFlags);
            wxBoxSizer *vectorFlagsDirectionSizer = new wxBoxSizer(wxHORIZONTAL);
            vectorFlagsDirectionSizer->Add(new wxStaticText(vector, wxID_ANY, "Instantaneous measurement direction"), fieldFlags);
            vectorFlagsDirectionSizer->Add(instantaneousMeasurementDirection, fieldFlags);
            vectorFlagsSizer->Add(vectorFlagsDirectionSizer, groupBoxInnerFlags);
            sizer->Add(vectorFlagsSizer, groupBoxFlags);
        }


        {
            wxGridSizer *gridSizer = new wxGridSizer(2, 0, 0);
            {
                wxStaticBoxSizer *vectorCrankRevolutionSizer = new wxStaticBoxSizer(wxVERTICAL, vector, "Crank revolution data");
                wxFlexGridSizer *vectorCrankRevolutionInnerSizer = new wxFlexGridSizer(2, 0, 0);
                vectorCrankRevolutionInnerSizer->Add(new wxStaticText(vector, wxID_ANY, "Cumulative crank revolutions"), fieldFlags);
                vectorCrankRevolutionInnerSizer->Add(cumulativeCrankVectorRevolutions, fieldFlags);
                vectorCrankRevolutionInnerSizer->Add(new wxStaticText(vector, wxID_ANY, "Last crank event time"), fieldFlags);
                vectorCrankRevolutionInnerSizer->Add(lastCrankEventVectorTime, fieldFlags);
                vectorCrankRevolutionSizer->Add(vectorCrankRevolutionInnerSizer, groupBoxInnerFlags);
                gridSizer->Add(vectorCrankRevolutionSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, vector, "First crank measurement angle");
                staticBoxSizer->Add(firstCrankMeasurementAngle, fieldFlags);
                gridSizer->Add(staticBoxSizer, groupBoxFlags);
            }
            sizer->Add(gridSizer, gridFlags);
        }

        {
            wxStaticBoxSizer *boxSizer = new wxStaticBoxSizer(wxVERTICAL, vector, "Instantaneous force measurement array");
            forceArraySizer->Add(new wxStaticText(vector, wxID_ANY, ""), fieldFlags);
            boxSizer->Add(forceArraySizer, groupBoxFlags);
            sizer->Add(boxSizer, groupBoxFlags);
        }

        {
            wxStaticBoxSizer *boxSizer = new wxStaticBoxSizer(wxVERTICAL, vector, "Instantaneous torque measurement array");
            torqueArraySizer->Add(new wxStaticText(vector, wxID_ANY, ""), fieldFlags);
            boxSizer->Add(torqueArraySizer, groupBoxInnerFlags);
            sizer->Add(boxSizer, groupBoxFlags);
        }

        sizer->AddStretchSpacer();

        {
            wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
            boxSizer->Add(loggingVector, fieldFlags);
            boxSizer->Add(logFileVector, fieldFlags);
            boxSizer->AddStretchSpacer();
            boxSizer->Add(notifyVector, rightFlags);
            sizer->Add(boxSizer, groupBoxInnerFlags);
        }

        vector->SetSizerAndFit(sizer);
        vector->PostSizeEvent();        // wxWrapSizer needs a resize to layout correctly
    }


    // InfoCrank control point
    wxTextValidator integers(wxFILTER_EMPTY | wxFILTER_INCLUDE_CHAR_LIST);
    integers.SetCharIncludes("+-0123456789");
    wxTextValidator hexadecimal(wxFILTER_EMPTY | wxFILTER_INCLUDE_CHAR_LIST);
    hexadecimal.SetCharIncludes("0123456789abcdefABCDEF");
    setSerialNumber = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxSize(150, -1), wxTE_PROCESS_ENTER);
    setFactoryCalibrationDate = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxSize(150, -1), wxTE_PROCESS_ENTER);
    setFactoryCalibrationDate->SetInitialSize(setFactoryCalibrationDate->GetSizeFromTextSize(setFactoryCalibrationDate->GetTextExtent("8888-88-88T88:88:88")));
    setFactoryCalibrationDate->SetToolTip("YYYY-MM-DDTHH:MM:SS");
    wxButton *setFactoryCalibrationDateNow = new wxButton(infoCrank_control, wxID_ANY, "Now");
    k1 = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    k1->SetToolTip("K1");
    k2 = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    k2->SetToolTip("K2");
    k3 = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    k3->SetToolTip("K3");
    k4 = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    k4->SetToolTip("K4");
    k5 = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    k5->SetToolTip("K5");
    k6 = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    k6->SetToolTip("K6");
    wxButton *requestStrainCalibrationParameters = new wxButton(infoCrank_control, wxID_ANY, "Get");
    wxButton *setStrainCalibrationParameters = new wxButton(infoCrank_control, wxID_ANY, "Set");
    s2alpha = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    s2alpha->SetToolTip(L"σ² alpha\nUnits: (rad/sec²)²");
    s2accel = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    s2accel->SetToolTip(L"σ² accel\nUnits: (m/sec²)²");
    driveRatio = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    driveRatio->SetToolTip("Drive ratio (metres of development)\nUnits: m/revolution");
    r1 = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    r1->SetToolTip("Radius accelerometer 1\nUnits: m\n28.4mm");
    r2 = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    r2->SetToolTip("Radius accelerometer 1\nUnits: m\n61.4mm");
    wxButton *requestKalmanFilterParameters = new wxButton(infoCrank_control, wxID_ANY, "Get");
    wxButton *setKalmanFilterParameters = new wxButton(infoCrank_control, wxID_ANY, "Set");
    wchar_t acc_labels[][32] = {
        L"A₀₀\nUnits: 2^-11 m/sec²/g",
        L"A₀₁\nUnits: 2^-11 m/sec²/g",
        L"A₀₂\nUnits: 2^-11 m/sec²/g",
        L"δ₀\nUnits: 2^-15 m/sec²",
        L"A₁₀\nUnits: 2^-11 m/sec²/g",
        L"A₁₁\nUnits: 2^-11 m/sec²/g",
        L"A₁₂\nUnits: 2^-11 m/sec²/g",
        L"δ₁\nUnits: 2^-15 m/sec²",
        L"A₂₀\nUnits: 2^-11 m/sec²/g",
        L"A₂₁\nUnits: 2^-11 m/sec²/g",
        L"A₂₂\nUnits: 2^-11 m/sec²/g",
        L"δ₂\nUnits: 2^-15 m/sec²",
    };
    for (int i = 0; i < 12; i ++) {
        a1[i] = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, integers);
        a1[i]->SetToolTip(acc_labels[i]);
        a2[i] = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, integers);
        a2[i]->SetToolTip(acc_labels[i]);
    }
    wxButton *requestAccel1CalibrationParameters = new wxButton(infoCrank_control, wxID_ANY, "Get");
    wxButton *setAccel1CalibrationParameters = new wxButton(infoCrank_control, wxID_ANY, "Set");
    wxButton *requestAccel2CalibrationParameters = new wxButton(infoCrank_control, wxID_ANY, "Get");
    wxButton *setAccel2CalibrationParameters = new wxButton(infoCrank_control, wxID_ANY, "Set");
    for (int i = 5; i > -1; i --) {
        ble_addr[i] = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxSize(10, -1), 0, hexadecimal);
        ble_addr[i]->SetInitialSize(ble_addr[i]->GetSizeFromTextSize(ble_addr[i]->GetTextExtent("DD")));
    }
    wxButton *requestPartnerAddress = new wxButton(infoCrank_control, wxID_ANY, "Get");
    wxButton *setPartnerAddress = new wxButton(infoCrank_control, wxID_ANY, "Set");
    wxButton *deletePartnerAddress = new wxButton(infoCrank_control, wxID_ANY, "Delete");
    deletePartnerAddress->SetToolTip("Distributed system\nNot paired");
    cpvSize = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_DIGITS));
    cpvSize->SetToolTip("Number of samples");
    cpvDownsample = new wxTextCtrl(infoCrank_control, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_DIGITS));
    cpvDownsample->SetToolTip("Downsampling");
    wxButton *requestCyclingPowerVectorParameters = new wxButton(infoCrank_control, wxID_ANY, "Get");
    wxButton *setCyclingPowerVectorParameters = new wxButton(infoCrank_control, wxID_ANY, "Set");

    // Bind the controls
    setSerialNumber->Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent & evt) {
        SendCommand(wxString().Format("Set serial number %s\n", setSerialNumber->GetValue()));
    });
    setFactoryCalibrationDate->Bind(wxEVT_TEXT_ENTER,  [&](wxCommandEvent & evt) {
        SendCommand(wxString().Format("Set factory calibration date %s\n", setFactoryCalibrationDate->GetValue()));
    });
    setFactoryCalibrationDateNow->Bind(wxEVT_BUTTON,  [&](wxCommandEvent & evt) {
        wxString now = wxDateTime().Now().Format("%FT%T", wxDateTime::UTC);
        setFactoryCalibrationDate->SetValue(now);
        SendCommand(wxString().Format("Set factory calibration date %s\n", setFactoryCalibrationDate->GetValue()));
    });
    requestStrainCalibrationParameters->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get strain parameters\n");
    });
    setStrainCalibrationParameters->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand(
            wxString().Format(
                "Set strain parameters %s %s %s %s %s %s\n",
                k1->GetValue(),
                k2->GetValue(),
                k3->GetValue(),
                k4->GetValue(),
                k5->GetValue(),
                k6->GetValue()));
    });
    requestKalmanFilterParameters->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get KF parameters\n");
    });
    setKalmanFilterParameters->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand(
            wxString().Format(
                "Set KF parameters %s %s %s %s %s\n",
                s2alpha->GetValue(),
                s2accel->GetValue(),
                driveRatio->GetValue(),
                r1->GetValue(),
                r2->GetValue()));
    });
    requestAccel1CalibrationParameters->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get accelerometer 1 transform\n");
    });
    setAccel1CalibrationParameters->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand(
            wxString().Format(
                "Set accelerometer 1 transform %s %s %s %s %s %s %s %s %s %s %s %s\n",
                a1[0]->GetValue(), a1[1]->GetValue(), a1[2]->GetValue(), a1[3]->GetValue(),
                a1[4]->GetValue(), a1[5]->GetValue(), a1[6]->GetValue(), a1[7]->GetValue(),
                a1[8]->GetValue(), a1[9]->GetValue(), a1[10]->GetValue(), a1[11]->GetValue()));
    });
    requestAccel2CalibrationParameters->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get accelerometer 2 transform\n");
    });
    setAccel2CalibrationParameters->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand(
            wxString().Format(
                "Set accelerometer 2 transform %s %s %s %s %s %s %s %s %s %s %s %s\n",
                a2[0]->GetValue(), a2[1]->GetValue(), a2[2]->GetValue(), a2[3]->GetValue(),
                a2[4]->GetValue(), a2[5]->GetValue(), a2[6]->GetValue(), a2[7]->GetValue(),
                a2[8]->GetValue(), a2[9]->GetValue(), a2[10]->GetValue(), a2[11]->GetValue()));
    });
    requestPartnerAddress->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get partner address\n");
    });
    setPartnerAddress->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand(
            wxString().Format(
                "Set partner address %s %s %s %s %s %s\n",
                ble_addr[0]->GetValue(), ble_addr[1]->GetValue(), ble_addr[2]->GetValue(),
                ble_addr[3]->GetValue(), ble_addr[4]->GetValue(), ble_addr[5]->GetValue()));
    });
    deletePartnerAddress->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Delete partner address\n");
        for (int i = 0; i < 6; i ++) {
            ble_addr[i]->SetValue("");
        }
    });
    requestCyclingPowerVectorParameters->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand("Get cycling power vector parameters\n");
    });
    setCyclingPowerVectorParameters->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        SendCommand(
            wxString().Format(
                "Set cycling power vector parameters %s %s\n",
                cpvSize->GetValue(),
                cpvDownsample->GetValue()));
    });

    // Layout the page
    {
        wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 0, 0);
        sizer->SetFlexibleDirection(wxVERTICAL);
        sizer->AddGrowableCol(0);
        sizer->AddGrowableCol(1);
        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_control, "Serial number");
            staticBoxSizer->Add(setSerialNumber, fieldFlags);
            staticBoxSizer->Add(-1, 5);
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }
        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_control, "Factory calibration date UTC");
            wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
            boxSizer->Add(setFactoryCalibrationDate, fieldFlags);
            boxSizer->Add(setFactoryCalibrationDateNow, fieldFlags);
            staticBoxSizer->Add(boxSizer);
            staticBoxSizer->Add(-1, 5);
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }
        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_control, "Strain gauge calibration");
            {
                wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
                boxSizer->Add(k1);
                boxSizer->Add(k2);
                boxSizer->Add(k3);
                boxSizer->Add(k4);
                boxSizer->Add(k5);
                boxSizer->Add(k6);
                staticBoxSizer->Add(boxSizer, centreFlags);
            }
            {
                wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
//            wxPanel *panel = new wxPanel(infoCrank_control, wxID_ANY);
//            panel->SetBackgroundColour(wxColour(wxT("RED")));
//            boxSizer->Add(panel, 1, wxEXPAND | wxALL ,10);
//            wxStaticText *staticText = new wxStaticText(panel, wxID_ANY, "1.00");
                boxSizer->Add(requestStrainCalibrationParameters);
                boxSizer->Add(setStrainCalibrationParameters);
                staticBoxSizer->Add(boxSizer, centreFlags);
            }
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }
        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_control, "Kalman filter parameters");
            {
                wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
                boxSizer->Add(s2alpha);
                boxSizer->Add(s2accel);
                boxSizer->Add(driveRatio);
                boxSizer->Add(r1);
                boxSizer->Add(r2);
                staticBoxSizer->Add(boxSizer, centreFlags);
            }
            {
                wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
                boxSizer->Add(requestKalmanFilterParameters);
                boxSizer->Add(setKalmanFilterParameters);
                staticBoxSizer->Add(boxSizer, centreFlags);
            }
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }
        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_control, "Accelerometer 1 calibration");
            {
                wxGridSizer *gridSizer = new wxGridSizer(4, 0, 0);
                for (int i = 0; i < 12; i ++) {
                    gridSizer->Add(a1[i]);
                }
                staticBoxSizer->Add(gridSizer, centreFlags);
            }
            {
                wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
                boxSizer->Add(requestAccel1CalibrationParameters);
                boxSizer->Add(setAccel1CalibrationParameters);
                staticBoxSizer->Add(boxSizer, centreFlags);
            }
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }
        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_control, "Accelerometer 2 calibration");
            {
                wxGridSizer *gridSizer = new wxGridSizer(4, 0, 0);
                for (int i = 0; i < 12; i ++) {
                    gridSizer->Add(a2[i]);
                }
                staticBoxSizer->Add(gridSizer, centreFlags);
            }
            {
                wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
                boxSizer->Add(requestAccel2CalibrationParameters);
                boxSizer->Add(setAccel2CalibrationParameters);
                staticBoxSizer->Add(boxSizer, centreFlags);
            }
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }
        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_control, "Partner crank Bluetooth address");
            {
                wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
                boxSizer->Add(ble_addr[5]);
                for (int i = 4; i > -1; i --) {
                    boxSizer->Add(new wxStaticText(infoCrank_control, wxID_ANY, ":"));
                    boxSizer->Add(ble_addr[i]);
                }
                staticBoxSizer->Add(boxSizer, centreFlags);
            }
            {
                wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
                boxSizer->Add(requestPartnerAddress);
                boxSizer->Add(setPartnerAddress);
                boxSizer->Add(deletePartnerAddress);
                staticBoxSizer->Add(boxSizer, centreFlags);
            }
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }
        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_control, "Cycling Power Vector Parameters");
            {
                wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
                boxSizer->Add(cpvSize);
                boxSizer->Add(cpvDownsample);
                staticBoxSizer->Add(boxSizer, centreFlags);
            }
            {
                wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
                boxSizer->Add(requestCyclingPowerVectorParameters);
                boxSizer->Add(setCyclingPowerVectorParameters);
                staticBoxSizer->Add(boxSizer, centreFlags);
            }
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }

        infoCrank_control->SetSizerAndFit(sizer);
    }


    // InfoCrank raw data
    wxBitmap refresh;
    refresh.CopyFromIcon(wxICON(view_refresh));
    wxBitmap arrow;
    arrow.CopyFromIcon(wxICON(arrow_right));

    strain = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    strain->SetToolTip("A/D Count");
    strain_avg = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    strain_avg->SetToolTip("Mean");
    strain_sd = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    strain_sd->SetToolTip("Standard deviation");
    wxButton *resetStrain = new wxButton(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxSize(32, 32));
    resetStrain->SetToolTip("Reset statistics");
    resetStrain->SetBitmap(refresh);
    temperature = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    temperature->SetToolTip(L"°C");
    batteryVoltage = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    batteryVoltage->SetToolTip(L"Volts");
    for (int i = 0; i < 3; i ++) {
        accel1[i] = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
        accel1[i]->SetToolTip(wxString().Format("%c", 'X' + i));
        accel1_avg[i] = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
        accel1_avg[i]->SetToolTip(wxString().Format("%c average", 'X' + i));
        accel1_sd[i] = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
        accel1_sd[i]->SetToolTip(wxString().Format("%c standard deviation", 'X' + i));
        accel2[i] = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
        accel2[i]->SetToolTip(wxString().Format("%c", 'X' + i));
        accel2_avg[i] = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
        accel2_avg[i]->SetToolTip(wxString().Format("%c average", 'X' + i));
        accel2_sd[i] = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
        accel2_sd[i]->SetToolTip(wxString().Format("%c standard deviation", 'X' + i));
    }
    wxButton *resetAccel1 = new wxButton(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxSize(32, 32));
    resetAccel1->SetToolTip("Reset statistics");
    resetAccel1->SetBitmap(refresh);
    wxButton *processAccel1 = new wxButton(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxSize(32, 32));
    processAccel1->SetToolTip("Process");
    processAccel1->SetBitmap(arrow);
    wxButton *resetAccel2 = new wxButton(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxSize(32, 32));
    resetAccel2->SetToolTip("Reset statistics");
    resetAccel2->SetBitmap(refresh);
    wxButton *processAccel2 = new wxButton(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxSize(32, 32));
    processAccel2->SetToolTip("Process");
    processAccel2->SetBitmap(arrow);
    wxString choices[] = {wxString("+X"), wxString("-X"), wxString("+Y"), wxString("-Y"), wxString("+Z"), wxString("-Z"),};
    orientation = new wxRadioBox(infoCrank_raw, wxID_ANY, "Orientation", wxDefaultPosition, wxDefaultSize, 6, choices);

    x = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    x_dot = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
    x_ddot = new wxTextCtrl(infoCrank_raw, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));

    wxToggleButton *notifyRaw = new wxToggleButton(infoCrank_raw, wxID_ANY, "Notify");
    wxCheckBox *loggingRaw = new wxCheckBox(infoCrank_raw, wxID_ANY, "/dev/null");
    wxButton *logFileRaw = new wxButton(infoCrank_raw, wxID_ANY, "...", wxDefaultPosition, wxSize(50, 20));


    // Bind the controls
    resetStrain->Bind(
        wxEVT_BUTTON,
    [&](wxCommandEvent & evt) {
        strain_num = 0;
        strain_sum = 0.0;
        strain_sum2 = 0.0;
        strain->SetValue("");
        strain_avg->SetValue("");
        strain_sd->SetValue("");
    }
    );
    resetAccel1->Bind(
        wxEVT_BUTTON,
    [&](wxCommandEvent & evt) {
        accel1_num = 0.001;
        accel1X_sum = 0.0;
        accel1X_sum2 = 0.0;
        accel1Y_sum = 0.0;
        accel1Y_sum2 = 0.0;
        accel1Z_sum = 0.0;
        accel1Z_sum2 = 0.0;
        accel1[0]->SetValue("");
        accel1_avg[0]->SetValue("");
        accel1_sd[0]->SetValue("");
        accel1[1]->SetValue("");
        accel1_avg[1]->SetValue("");
        accel1_sd[1]->SetValue("");
        accel1[2]->SetValue("");
        accel1_avg[2]->SetValue("");
        accel1_sd[2]->SetValue("");
    }
    );
    resetAccel2->Bind(
        wxEVT_BUTTON,
    [&](wxCommandEvent & evt) {
        accel2_num = 0.001;
        accel2X_sum = 0.0;
        accel2X_sum2 = 0.0;
        accel2Y_sum = 0.0;
        accel2Y_sum2 = 0.0;
        accel2Z_sum = 0.0;
        accel2Z_sum2 = 0.0;
        accel2[0]->SetValue("");
        accel2_avg[0]->SetValue("");
        accel2_sd[0]->SetValue("");
        accel2[1]->SetValue("");
        accel2_avg[1]->SetValue("");
        accel2_sd[1]->SetValue("");
        accel2[2]->SetValue("");
        accel2_avg[2]->SetValue("");
        accel2_sd[2]->SetValue("");
    }
    );

    processAccel1->Bind(
        wxEVT_BUTTON,
    [&](wxCommandEvent & evt) {
        // Refer to CrankAngleEstimate.odt - Transforming Sensor Data to Crank Reference Frame
        // Copy row based on orientation to the gravity matrix
        gravity1matrix[orientation->GetSelection() * 3 + 0] = accel1X_sum / accel1_num;
        gravity1matrix[orientation->GetSelection() * 3 + 1] = accel1Y_sum / accel1_num;
        gravity1matrix[orientation->GetSelection() * 3 + 2] = accel1Z_sum / accel1_num;

        // Build the coefficient matrix
        double sx = + gravity1matrix[0]
                    + gravity1matrix[3]
                    + gravity1matrix[6]
                    + gravity1matrix[9]
                    + gravity1matrix[12]
                    + gravity1matrix[15];
        double sy = + gravity1matrix[1]
                    + gravity1matrix[4]
                    + gravity1matrix[7]
                    + gravity1matrix[10]
                    + gravity1matrix[13]
                    + gravity1matrix[16];
        double sz = + gravity1matrix[2]
                    + gravity1matrix[5]
                    + gravity1matrix[8]
                    + gravity1matrix[11]
                    + gravity1matrix[14]
                    + gravity1matrix[17];
        double sxx = + gravity1matrix[0] * gravity1matrix[0]
                     + gravity1matrix[3] * gravity1matrix[3]
                     + gravity1matrix[6] * gravity1matrix[6]
                     + gravity1matrix[9] * gravity1matrix[9]
                     + gravity1matrix[12] * gravity1matrix[12]
                     + gravity1matrix[15] * gravity1matrix[15];
        double syy = + gravity1matrix[1] * gravity1matrix[1]
                     + gravity1matrix[4] * gravity1matrix[4]
                     + gravity1matrix[7] * gravity1matrix[7]
                     + gravity1matrix[10] * gravity1matrix[10]
                     + gravity1matrix[13] * gravity1matrix[13]
                     + gravity1matrix[16] * gravity1matrix[16];
        double szz = + gravity1matrix[2] * gravity1matrix[2]
                     + gravity1matrix[5] * gravity1matrix[5]
                     + gravity1matrix[8] * gravity1matrix[8]
                     + gravity1matrix[11] * gravity1matrix[11]
                     + gravity1matrix[14] * gravity1matrix[14]
                     + gravity1matrix[17] * gravity1matrix[17];
        double sxy = + gravity1matrix[0] * gravity1matrix[1]
                     + gravity1matrix[3] * gravity1matrix[4]
                     + gravity1matrix[6] * gravity1matrix[7]
                     + gravity1matrix[9] * gravity1matrix[10]
                     + gravity1matrix[12] * gravity1matrix[13]
                     + gravity1matrix[15] * gravity1matrix[16];
        double sxz = + gravity1matrix[0] * gravity1matrix[2]
                     + gravity1matrix[3] * gravity1matrix[5]
                     + gravity1matrix[6] * gravity1matrix[8]
                     + gravity1matrix[9] * gravity1matrix[11]
                     + gravity1matrix[12] * gravity1matrix[14]
                     + gravity1matrix[15] * gravity1matrix[17];
        double syz = + gravity1matrix[1] * gravity1matrix[2]
                     + gravity1matrix[4] * gravity1matrix[5]
                     + gravity1matrix[7] * gravity1matrix[8]
                     + gravity1matrix[10] * gravity1matrix[11]
                     + gravity1matrix[13] * gravity1matrix[14]
                     + gravity1matrix[16] * gravity1matrix[17];

        gsl_matrix *A = gsl_matrix_alloc(4, 4);
        gsl_matrix_set(A, 0, 0, sxx);
        gsl_matrix_set(A, 0, 1, sxy);
        gsl_matrix_set(A, 0, 2, sxz);
        gsl_matrix_set(A, 0, 3, sx * 256.0); // Note: the 256.0 scales the deltas so that they fit into an int16_t
        gsl_matrix_set(A, 1, 0, sxy);
        gsl_matrix_set(A, 1, 1, syy);
        gsl_matrix_set(A, 1, 2, syz);
        gsl_matrix_set(A, 1, 3, sy * 256.0);
        gsl_matrix_set(A, 2, 0, sxz);
        gsl_matrix_set(A, 2, 1, syz);
        gsl_matrix_set(A, 2, 2, szz);
        gsl_matrix_set(A, 2, 3, sz * 256.0);
        gsl_matrix_set(A, 3, 0, sx);
        gsl_matrix_set(A, 3, 1, sy);
        gsl_matrix_set(A, 3, 2, sz);
        gsl_matrix_set(A, 3, 3, 6.0 * 256.0);

        // Build the result matrix
        double sXx = + 9.81 * 0x01p23 * gravity1matrix[0]
                     - 9.81 * 0x01p23 * gravity1matrix[3];
        double sXy = + 9.81 * 0x01p23 * gravity1matrix[1]
                     - 9.81 * 0x01p23 * gravity1matrix[4];
        double sXz = + 9.81 * 0x01p23 * gravity1matrix[2]
                     - 9.81 * 0x01p23 * gravity1matrix[5];
        double sYx = + 9.81 * 0x01p23 * gravity1matrix[6]
                     - 9.81 * 0x01p23 * gravity1matrix[9];
        double sYy = + 9.81 * 0x01p23 * gravity1matrix[7]
                     - 9.81 * 0x01p23 * gravity1matrix[10];
        double sYz = + 9.81 * 0x01p23 * gravity1matrix[8]
                     - 9.81 * 0x01p23 * gravity1matrix[11];
        double sZx = + 9.81 * 0x01p23 * gravity1matrix[12]
                     - 9.81 * 0x01p23 * gravity1matrix[15];
        double sZy = + 9.81 * 0x01p23 * gravity1matrix[13]
                     - 9.81 * 0x01p23 * gravity1matrix[16];
        double sZz = + 9.81 * 0x01p23 * gravity1matrix[14]
                     - 9.81 * 0x01p23 * gravity1matrix[17];
        gsl_matrix *B = gsl_matrix_alloc(4, 3);
        gsl_matrix_set(B, 0, 0, sXx);
        gsl_matrix_set(B, 0, 1, sYx);
        gsl_matrix_set(B, 0, 2, sZx);
        gsl_matrix_set(B, 1, 0, sXy);
        gsl_matrix_set(B, 1, 1, sYy);
        gsl_matrix_set(B, 1, 2, sZy);
        gsl_matrix_set(B, 2, 0, sXz);
        gsl_matrix_set(B, 2, 1, sYz);
        gsl_matrix_set(B, 2, 2, sZz);
        gsl_matrix_set(B, 3, 0, 0.0);
        gsl_matrix_set(B, 3, 1, 0.0);
        gsl_matrix_set(B, 3, 2, 0.0);

        // Invert the coefficient matrix
        gsl_permutation *p = gsl_permutation_alloc(4);
        int s;
        gsl_linalg_LU_decomp(A, p, &s);
        gsl_matrix *Inv = gsl_matrix_alloc(4, 4);
        gsl_linalg_LU_invert(A, p, Inv);

        // Multiply inverse with result matrix
        gsl_matrix *C = gsl_matrix_alloc(4, 3);
        gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, Inv, B, 0.0, C);

        /*//gsl_matrix_transpose(Inv);
        // Apply scaling to m/s². Scale to fit in int16_t.
        gsl_matrix_scale(Inv, 9.81 * 0x00800000);*/
        for (int i = 0; i < 12; i ++) {
            a1[i]->SetValue(wxString().Format("%0.0lf", gsl_matrix_get(C, i % 4, i / 4)));
        }
        gsl_matrix_free(C);
        gsl_matrix_free(Inv);
        gsl_permutation_free(p);
        gsl_matrix_free(B);
        gsl_matrix_free(A);
    }
    );

    processAccel2->Bind(
        wxEVT_BUTTON,
    [&](wxCommandEvent & evt) {
//        // Refer to CrankAngleEstimate.odt - Transforming Sensor Data to Crank Reference Frame
//        // Copy row based on orientation to the gravity matrix
//        gravity2matrix[orientation->GetSelection() * 3 + 0] = accel2X_sum / accel2_num;
//        gravity2matrix[orientation->GetSelection() * 3 + 1] = accel2Y_sum / accel2_num;
//        gravity2matrix[orientation->GetSelection() * 3 + 2] = accel2Z_sum / accel2_num;
//        // Invert the gravity matrix
//        gsl_matrix *A = gsl_matrix_alloc(3, 3);
//        gsl_matrix_view B = gsl_matrix_view_array(gravity2matrix, 3, 3);
//        gsl_matrix_memcpy(A, &B.matrix);
//        gsl_permutation *p = gsl_permutation_alloc(3);
//        int s;
//        gsl_linalg_LU_decomp(A, p, &s);
//        gsl_matrix *Inv = gsl_matrix_alloc(3, 3);
//        gsl_linalg_LU_invert(A, p, Inv);
//        gsl_matrix_transpose(Inv);
//        // Apply scaling to m/s²
//        gsl_matrix_scale(Inv, 9.81 * 0x00800000);
//        for (int i = 0; i < 9; i ++) {
//            a2[i]->SetValue(wxString().Format("%0.0lf", gsl_matrix_get(Inv, i / 3, i % 3)));
//        }
//        gsl_matrix_free(Inv);
//        gsl_permutation_free(p);
//        gsl_matrix_free(A);


        // Refer to CrankAngleEstimate.odt - Transforming Sensor Data to Crank Reference Frame
        // Copy row based on orientation to the gravity matrix
        gravity2matrix[orientation->GetSelection() * 3 + 0] = accel2X_sum / accel2_num;
        gravity2matrix[orientation->GetSelection() * 3 + 1] = accel2Y_sum / accel2_num;
        gravity2matrix[orientation->GetSelection() * 3 + 2] = accel2Z_sum / accel2_num;

        // Build the coefficient matrix
        double sx = + gravity2matrix[0]
                    + gravity2matrix[3]
                    + gravity2matrix[6]
                    + gravity2matrix[9]
                    + gravity2matrix[12]
                    + gravity2matrix[15];
        double sy = + gravity2matrix[1]
                    + gravity2matrix[4]
                    + gravity2matrix[7]
                    + gravity2matrix[10]
                    + gravity2matrix[13]
                    + gravity2matrix[16];
        double sz = + gravity2matrix[2]
                    + gravity2matrix[5]
                    + gravity2matrix[8]
                    + gravity2matrix[11]
                    + gravity2matrix[14]
                    + gravity2matrix[17];
        double sxx = + gravity2matrix[0] * gravity2matrix[0]
                     + gravity2matrix[3] * gravity2matrix[3]
                     + gravity2matrix[6] * gravity2matrix[6]
                     + gravity2matrix[9] * gravity2matrix[9]
                     + gravity2matrix[12] * gravity2matrix[12]
                     + gravity2matrix[15] * gravity2matrix[15];
        double syy = + gravity2matrix[1] * gravity2matrix[1]
                     + gravity2matrix[4] * gravity2matrix[4]
                     + gravity2matrix[7] * gravity2matrix[7]
                     + gravity2matrix[10] * gravity2matrix[10]
                     + gravity2matrix[13] * gravity2matrix[13]
                     + gravity2matrix[16] * gravity2matrix[16];
        double szz = + gravity2matrix[2] * gravity2matrix[2]
                     + gravity2matrix[5] * gravity2matrix[5]
                     + gravity2matrix[8] * gravity2matrix[8]
                     + gravity2matrix[11] * gravity2matrix[11]
                     + gravity2matrix[14] * gravity2matrix[14]
                     + gravity2matrix[17] * gravity2matrix[17];
        double sxy = + gravity2matrix[0] * gravity2matrix[1]
                     + gravity2matrix[3] * gravity2matrix[4]
                     + gravity2matrix[6] * gravity2matrix[7]
                     + gravity2matrix[9] * gravity2matrix[10]
                     + gravity2matrix[12] * gravity2matrix[13]
                     + gravity2matrix[15] * gravity2matrix[16];
        double sxz = + gravity2matrix[0] * gravity2matrix[2]
                     + gravity2matrix[3] * gravity2matrix[5]
                     + gravity2matrix[6] * gravity2matrix[8]
                     + gravity2matrix[9] * gravity2matrix[11]
                     + gravity2matrix[12] * gravity2matrix[14]
                     + gravity2matrix[15] * gravity2matrix[17];
        double syz = + gravity2matrix[1] * gravity2matrix[2]
                     + gravity2matrix[4] * gravity2matrix[5]
                     + gravity2matrix[7] * gravity2matrix[8]
                     + gravity2matrix[10] * gravity2matrix[11]
                     + gravity2matrix[13] * gravity2matrix[14]
                     + gravity2matrix[16] * gravity2matrix[17];

        gsl_matrix *A = gsl_matrix_alloc(4, 4);
        gsl_matrix_set(A, 0, 0, sxx);
        gsl_matrix_set(A, 0, 1, sxy);
        gsl_matrix_set(A, 0, 2, sxz);
        gsl_matrix_set(A, 0, 3, sx * 256.0); // Note: the 256.0 scales the deltas so that they fit into an int16_t
        gsl_matrix_set(A, 1, 0, sxy);
        gsl_matrix_set(A, 1, 1, syy);
        gsl_matrix_set(A, 1, 2, syz);
        gsl_matrix_set(A, 1, 3, sy * 256.0);
        gsl_matrix_set(A, 2, 0, sxz);
        gsl_matrix_set(A, 2, 1, syz);
        gsl_matrix_set(A, 2, 2, szz);
        gsl_matrix_set(A, 2, 3, sz * 256.0);
        gsl_matrix_set(A, 3, 0, sx);
        gsl_matrix_set(A, 3, 1, sy);
        gsl_matrix_set(A, 3, 2, sz);
        gsl_matrix_set(A, 3, 3, 6.0 * 256.0);

        // Build the result matrix
        double sXx = + 9.81 * 0x01p23 * gravity2matrix[0]
                     - 9.81 * 0x01p23 * gravity2matrix[3];
        double sXy = + 9.81 * 0x01p23 * gravity2matrix[1]
                     - 9.81 * 0x01p23 * gravity2matrix[4];
        double sXz = + 9.81 * 0x01p23 * gravity2matrix[2]
                     - 9.81 * 0x01p23 * gravity2matrix[5];
        double sYx = + 9.81 * 0x01p23 * gravity2matrix[6]
                     - 9.81 * 0x01p23 * gravity2matrix[9];
        double sYy = + 9.81 * 0x01p23 * gravity2matrix[7]
                     - 9.81 * 0x01p23 * gravity2matrix[10];
        double sYz = + 9.81 * 0x01p23 * gravity2matrix[8]
                     - 9.81 * 0x01p23 * gravity2matrix[11];
        double sZx = + 9.81 * 0x01p23 * gravity2matrix[12]
                     - 9.81 * 0x01p23 * gravity2matrix[15];
        double sZy = + 9.81 * 0x01p23 * gravity2matrix[13]
                     - 9.81 * 0x01p23 * gravity2matrix[16];
        double sZz = + 9.81 * 0x01p23 * gravity2matrix[14]
                     - 9.81 * 0x01p23 * gravity2matrix[17];
        gsl_matrix *B = gsl_matrix_alloc(4, 3);
        gsl_matrix_set(B, 0, 0, sXx);
        gsl_matrix_set(B, 0, 1, sYx);
        gsl_matrix_set(B, 0, 2, sZx);
        gsl_matrix_set(B, 1, 0, sXy);
        gsl_matrix_set(B, 1, 1, sYy);
        gsl_matrix_set(B, 1, 2, sZy);
        gsl_matrix_set(B, 2, 0, sXz);
        gsl_matrix_set(B, 2, 1, sYz);
        gsl_matrix_set(B, 2, 2, sZz);
        gsl_matrix_set(B, 3, 0, 0.0);
        gsl_matrix_set(B, 3, 1, 0.0);
        gsl_matrix_set(B, 3, 2, 0.0);

        // Invert the coefficient matrix
        gsl_permutation *p = gsl_permutation_alloc(4);
        int s;
        gsl_linalg_LU_decomp(A, p, &s);
        gsl_matrix *Inv = gsl_matrix_alloc(4, 4);
        gsl_linalg_LU_invert(A, p, Inv);

        // Multiply inverse with result matrix
        gsl_matrix *C = gsl_matrix_alloc(4, 3);
        gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, Inv, B, 0.0, C);

        /*//gsl_matrix_transpose(Inv);
        // Apply scaling to m/s². Scale to fit in int16_t.
        gsl_matrix_scale(Inv, 9.81 * 0x00800000);*/
        for (int i = 0; i < 12; i ++) {
            a2[i]->SetValue(wxString().Format("%0.0lf", gsl_matrix_get(C, i % 4, i / 4)));
        }
        gsl_matrix_free(C);
        gsl_matrix_free(Inv);
        gsl_permutation_free(p);
        gsl_matrix_free(B);
        gsl_matrix_free(A);

    }
    );

    notifyRaw->Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent & evt) {
        switch (evt.GetInt()) {
        case TRUE:
            ((wxToggleButton *) evt.GetEventObject())->SetLabel("Stop");
            SendCommand("Notify raw on\n");
            break;
        case FALSE:
            ((wxToggleButton *) evt.GetEventObject())->SetLabel("Notify");
            SendCommand("Notify raw off\n");
            break;
        }
    });
    logFileRaw->Bind(wxEVT_BUTTON, &IC2Frame::LogFileName, this, wxID_ANY, wxID_ANY, new FileDialogParameters("raw.log", loggingRaw));
//    loggingRaw->Bind(wxEVT_CHECKBOX, &IC2Frame::LogFileOpen, this, wxID_ANY, wxID_ANY, new LogFile(&logRaw));


    loggingRaw->Bind(wxEVT_CHECKBOX,
    [&](wxCommandEvent & evt) {
        wxCheckBox *checkBox = (wxCheckBox *) evt.GetEventObject();
        if (checkBox->IsChecked()) {
//            logMeasurement.Create(checkBox->GetLabel(), true);
            logRaw.Open(checkBox->GetLabel(), wxFile::write);
        } else {
            logRaw.Close();
        }
    });

    // Layout the page
    {
        wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 0, 0);
        sizer->SetFlexibleDirection(wxVERTICAL);
        sizer->AddGrowableCol(0);
        sizer->AddGrowableCol(1);
        sizer->AddGrowableRow(3);
        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_raw, "Strain");
            {
                wxFlexGridSizer *gridSizer = new wxFlexGridSizer(3, 0, 0);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Instantaneous"), centreFlags);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Mean"), centreFlags);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Deviation"), centreFlags);
                gridSizer->Add(strain);
                gridSizer->Add(strain_avg);
                gridSizer->Add(strain_sd);
                gridSizer->Add(resetStrain, centreFlags);
                staticBoxSizer->Add(gridSizer, centreFlags);
            }
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }
        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_raw, "nRF52840 Sensors");
            {
                wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2, 0, 0);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Temperature"), centreFlags);
                gridSizer->Add(temperature);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Voltage"), centreFlags);
                gridSizer->Add(batteryVoltage);
                staticBoxSizer->Add(gridSizer, centreFlags);
            }
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }
        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_raw, "Accelerometer 1");
            {
                wxFlexGridSizer *gridSizer = new wxFlexGridSizer(4, 0, 0);
                gridSizer->Add(-1, -1);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "X"), centreFlags);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Y"), centreFlags);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Z"), centreFlags);

                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Instantaneous"), centreFlags);
                gridSizer->Add(accel1[0]);
                gridSizer->Add(accel1[1]);
                gridSizer->Add(accel1[2]);

                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Mean"), centreFlags);
                gridSizer->Add(accel1_avg[0]);
                gridSizer->Add(accel1_avg[1]);
                gridSizer->Add(accel1_avg[2]);

                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Deviation"), centreFlags);
                gridSizer->Add(accel1_sd[0]);
                gridSizer->Add(accel1_sd[1]);
                gridSizer->Add(accel1_sd[2]);

                gridSizer->Add(-1, -1);
                gridSizer->Add(resetAccel1, centreFlags);
                gridSizer->Add(-1, -1);
                gridSizer->Add(processAccel1, centreFlags);

                staticBoxSizer->Add(gridSizer, centreFlags);
            }
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }
        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_raw, "Accelerometer 2");
            {
                wxFlexGridSizer *gridSizer = new wxFlexGridSizer(4, 0, 0);
                gridSizer->Add(-1, -1);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "X"), centreFlags);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Y"), centreFlags);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Z"), centreFlags);

                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Instantaneous"), centreFlags);
                gridSizer->Add(accel2[0]);
                gridSizer->Add(accel2[1]);
                gridSizer->Add(accel2[2]);

                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Mean"), centreFlags);
                gridSizer->Add(accel2_avg[0]);
                gridSizer->Add(accel2_avg[1]);
                gridSizer->Add(accel2_avg[2]);

                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Deviation"), centreFlags);
                gridSizer->Add(accel2_sd[0]);
                gridSizer->Add(accel2_sd[1]);
                gridSizer->Add(accel2_sd[2]);

                gridSizer->Add(-1, -1);
                gridSizer->Add(resetAccel2, centreFlags);
                gridSizer->Add(-1, -1);
                gridSizer->Add(processAccel2, centreFlags);

                staticBoxSizer->Add(gridSizer, centreFlags);
            }
            staticBoxSizer->Add(-1, 5);
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }
        {
            sizer->Add(orientation, groupBoxFlags);
        }


        {
            wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, infoCrank_raw, "State");
            {
                wxFlexGridSizer *gridSizer = new wxFlexGridSizer(3, 0, 0);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Position"), centreFlags);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Velocity"), centreFlags);
                gridSizer->Add(new wxStaticText(infoCrank_raw, wxID_ANY, "Acceleration"), centreFlags);
                gridSizer->Add(x);
                gridSizer->Add(x_dot);
                gridSizer->Add(x_ddot);
                staticBoxSizer->Add(gridSizer, centreFlags);
            }
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }


//        sizer->AddStretchSpacer();
        sizer->AddStretchSpacer();
        sizer->AddStretchSpacer();

        {
            wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
            boxSizer->Add(loggingRaw, fieldFlags);
            boxSizer->Add(logFileRaw, fieldFlags);
            sizer->Add(boxSizer, groupBoxInnerFlags);
        }
        {
            wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
            boxSizer->AddStretchSpacer();
            boxSizer->Add(notifyRaw, rightFlags);
            sizer->Add(boxSizer, groupBoxInnerFlags);
        }

        infoCrank_raw->SetSizerAndFit(sizer);
    }

    // InfoCrank graphics
//    crank_timer = new CrankTimer(crank_graphics);
//    crank_timer->Start(10);

}

// -------------------------------------------------------------------------------------------------
// Socket connection for communications with the DBus thread
// -------------------------------------------------------------------------------------------------
void IC2Frame::ConnectSocket()
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    // Create the socket for communication with the thread
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("ERROR opening socket\n");
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(9876);;
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("ERROR connecting\n");
    }
    printf("Frame socket connected to thread\n");
}


//--------------------------------------------------------------------------------------------------
// Send command to DBus thread
//--------------------------------------------------------------------------------------------------
void IC2Frame::SendCommand(const char *cmd)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    // Send the command to the thread
    int n = write(sockfd, cmd, strlen(cmd));
    if (n < 0) {
        printf("ERROR writing to socket\n");
    }
}

//--------------------------------------------------------------------------------------------------
// Prepare command from wxTextCtrl
// Command in (wxString *) EventUserData
// Value in (wxTextCtrl *) EventObject
//--------------------------------------------------------------------------------------------------
//void IC2Frame::SetFromTextCtrl(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//
//    wxString cmd = wxString().Format("%s %s\n", *(((wxStringObject *) evt.GetEventUserData())->m_string), ((wxTextCtrl *) evt.GetEventObject())->GetValue());
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd.c_str().AsChar());
//}

//--------------------------------------------------------------------------------------------------
// Prepare command from wxButton
// If EventUserData supplied:
//     Command in (wxString *) EventUserData
// Else:
//     Command in (wxString *) (wxTextCtrl *) EventObject->GetLabel()
//--------------------------------------------------------------------------------------------------
//void IC2Frame::GetFromButton(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//
//    wxString cmd = wxString().Format("%s\n", ((wxButton *) evt.GetEventObject())->GetLabel());
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd.c_str().AsChar());
//}



void IC2Frame::LogFileName(wxCommandEvent &evt)
{
    FileDialogParameters *userData = (FileDialogParameters *) evt.GetEventUserData();
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    if (userData->m_checkBox->IsChecked()) {
        wxMessageDialog(this, "Loging in progress.\nDisable logging first.", "File already open").ShowModal();
        return;
    }

    wxFileDialog logFileDialog(this, "Open logging file", "~/Documents", userData->m_fileName, "Log files (*.log)|*.log", wxFD_SAVE);
    if (logFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    userData->m_checkBox->SetLabel(logFileDialog.GetPath());
    userData->m_checkBox->GetParent()->PostSizeEvent();
}

//void IC2Frame::LogFileOpen(wxCommandEvent &evt)
//{
//    wxCheckBox *checkBox = (wxCheckBox *) evt.GetEventObject();
//    wxFile *file = ((LogFile *) evt.GetEventUserData())->m_file;
//    if (checkBox->IsChecked()) {
//        file->Create(checkBox->GetLabel(), true);
//        file->Open(checkBox->GetLabel(), wxFile::write);
//    } else {
//        file->Close();
//    }
//
//}

//##################################################################################################
// Menu
//##################################################################################################

//--------------------------------------------------------------------------------------------------
// Quit event handler
//--------------------------------------------------------------------------------------------------
//void IC2Frame::OnQuit(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    // Send the command to the thread
//    int n = write(sockfd, "Quit\n", 5);
//    if (n < 0) {
//        printf("ERROR writing to socket\n");
//    }
//}

//--------------------------------------------------------------------------------------------------
// Disconnect event handler
//--------------------------------------------------------------------------------------------------
void IC2Frame::OnDisconnect(wxCommandEvent &evt)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    // Send the command to the thread
    int n = write(sockfd, "Disconnect all\n", 15);
    if (n < 0) {
        printf("ERROR writing to socket\n");
    }
}

//##################################################################################################
// Devices page
//##################################################################################################
// -------------------------------------------------------------------------------------------------
// Scan event handler
// -------------------------------------------------------------------------------------------------
//void IC2Frame::OnScan(wxCommandEvent &evt)
//{
//    printf("Scanning: %s\n", evt.IsChecked() ? "True" : "False");
//    printf("sockfd: %d\n", sockfd);
//    if (evt.IsChecked()) {
//        int n = write(sockfd, "Scan on\n", 8);
//        if (n < 0) {
//            printf("ERROR writing to socket\n");
//        }
//    } else {
//        int n = write(sockfd, "Scan off\n", 9);
//        if (n < 0) {
//            printf("ERROR writing to socket\n");
//        }
//    }
//}

//--------------------------------------------------------------------------------------------------
// Device added
//--------------------------------------------------------------------------------------------------
void IC2Frame::AddDevice(const char *name, const char *address)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    BLEDevice *device = new BLEDevice(this, devices, name, address);
//    wxSizerItem *sizerItem = devicesSizer->Add((wxSizer *) device, 0, /*wxGROW |*/ wxALL, 20, device->userData);
    wxSizerItem *sizerItem = devicesSizer->Add((wxSizer *) device, 0, /*wxGROW |*/ wxALL, 20, new BLEDevice::UserData(address));
    printf("sizerItem: %p\n", sizerItem);
    printf("sizerItem->IsWindow(): %d\n", sizerItem->IsWindow());
    printf("sizerItem->IsSizer(): %d\n", sizerItem->IsSizer());
    printf("sizerItem->GetUserData(): %p\n", sizerItem->GetUserData());

    devices->Layout();
}

//void IC2Frame::OnAddDevice(wxCommandEvent &evt)
//{
//    BLEDevice *device = new BLEDevice(this, devices, "Hello", "21:23:24:25");
//    devicesSizer->Add(device, 0, /*wxGROW |*/ wxALL, 20);
//    devices->Layout();
//}

//--------------------------------------------------------------------------------------------------
// Device removed
//--------------------------------------------------------------------------------------------------
void IC2Frame::RemoveDevice(const char *address)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    printf("devicesSizer->GetItemCount(): %zu\n", devicesSizer->GetItemCount());
    for (int i = 0; i < devicesSizer->GetItemCount(); i ++) {
        printf("Checking %d %p\n", i, devicesSizer->GetItem(i));
        //printf("Checking %d %p\n",i, devicesSizer->GetItem(i)->GetUserData());
        BLEDevice *device = (BLEDevice *) devicesSizer->GetItem(i)->GetSizer();
        wxObject *userData = devicesSizer->GetItem(i)->GetUserData();
        printf("userData %p\n", userData);

        if (!strcmp(((BLEDevice::UserData *) userData)->address, address)) {
            printf("Match on address: %s\n", address);
//            printf("about to: unbind\n");
//            printf("%d\n", device->button->Unbind(wxEVT_BUTTON, &IC2Frame::OnConnect, this));
            printf("about to: clear\n");
            device->Clear(TRUE);
            printf("about to: devicesSizer->Remove(i)\n");
            devicesSizer->Remove(i);
            printf("about to: layout\n");
            devicesSizer->Layout();
            break;
        }
    }
    printf("About to: devices->Layout();\n");
    devices->Layout();

    printf("Layout();\n");
    Layout();
}

//--------------------------------------------------------------------------------------------------
// Connection event handler
//--------------------------------------------------------------------------------------------------
void IC2Frame::OnConnect(wxCommandEvent &evt)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    char cmd[32];
    // Concatenate the button label with the address stored in the user data
    sprintf(cmd, "%s %s\n", ((wxButton *) evt.GetEventObject())->GetLabel().c_str().AsChar(), ((BLEDevice::UserData *) evt.GetEventUserData())->address);

    // Toggle the button label between Connect and Disconnect
    ((wxToggleButton *) evt.GetEventObject())->SetLabel(evt.GetInt() ? "Disconnect" : "Connect");

    SendCommand(cmd);
}


//##################################################################################################
// Device information page
//##################################################################################################
//void IC2Frame::RefreshDeviceInformation(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Refresh device information\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//##################################################################################################
// Battery information page
//##################################################################################################
void IC2Frame::SetBatteryLevel(uint8_t level)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);

    if (logBattery.IsOpened()) {
        logBattery.Write(&level, sizeof(level));
    }

    batteryLevel->SetLabel(wxString().Format("%hhu%%", level));
}

//void IC2Frame::RefreshBatteryInformation(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Refresh battery information\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}


//##################################################################################################
// Cycling power features page
//##################################################################################################
//void IC2Frame::RefreshFeatures(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Refresh features\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//##################################################################################################
// Devices information page
//##################################################################################################
void IC2Frame::SetManufacturerName(const char *str)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    manufacturerName->SetLabel(str);
}
void IC2Frame::SetModelNumber(const char *str)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    modelNumber->SetLabel(str);
}
void IC2Frame::SetSerialNumber(const char *str)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    serialNumber->SetLabel(str);
}
void IC2Frame::SetHardwareRevisionNumber(const char *str)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    hardwareRevisionNumber->SetLabel(str);
}
void IC2Frame::SetFirmwareRevisionNumber(const char *str)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    firmwareRevisionNumber->SetLabel(str);
}
void IC2Frame::SetSoftwareRevisionNumber(const char *str)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    softwareRevisionNumber->SetLabel(str);
}
void IC2Frame::SetSystemID(const char *str)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    systemID->SetLabel(str);
}
void IC2Frame::SetIEEE(const char *str)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    // TODO
}
void IC2Frame::SetPNP(void *str)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
#pragma pack(push, 1)
    struct pnp_data {
        uint8_t vendor_id_source;
        uint16_t vendor_id;
        uint16_t product_id;
        uint16_t product_version;
    } *pnp_data = (struct pnp_data *) str;
#pragma pack(pop)
    switch (pnp_data->vendor_id_source) {
    case 0x01:
        PNPVendorIDSource->SetLabel("Bluetooth SIG");
        break;
    case 0x02:
        PNPVendorIDSource->SetLabel("USB Implementer's");
        break;
    default:
        PNPVendorIDSource->SetLabel("Unknown");
        break;
    }
    char cmd[20];
    sprintf(cmd, "0x%04x", pnp_data->vendor_id);
    PNPVendorID->SetLabel(cmd);
    sprintf(cmd, "0x%04x", pnp_data->product_id);
    PNPProductID->SetLabel(cmd);
    sprintf(cmd, "0x%04x", pnp_data->product_version);
    PNPProductVersion->SetLabel(cmd);
}



//##################################################################################################
// Cycling power feature page
//##################################################################################################
void IC2Frame::SetCyclingPowerFeature(void *str)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
#pragma pack(push, 1)
    struct cycling_power_feature_data {
        uint32_t pedal_power_balance_supported: 1;
        uint32_t accumulated_torque_supported: 1;
        uint32_t wheel_revolution_data_supported: 1;
        uint32_t crank_revolution_data_supported: 1;
        uint32_t extreme_magnitudes_supported: 1;
        uint32_t extreme_angles_supported: 1;
        uint32_t top_and_bottom_dead_spot_angles_supported: 1;
        uint32_t accumulated_energy_supported: 1;
        uint32_t offset_compensation_indicator_supported: 1;
        uint32_t offset_compensation_supported: 1;
        uint32_t cycling_power_measurement_characteristic_content_masking_supported: 1;
        uint32_t multiple_sensor_locations_supported: 1;
        uint32_t crank_length_adjustment_supported: 1;
        uint32_t chain_length_adjustment_supported: 1;
        uint32_t chain_weight_adjustment_supported: 1;
        uint32_t span_length_adjustment_supported: 1;
        uint32_t sensor_measurement_context: 1;
        uint32_t instantaneous_mesurement_direction_supported: 1;
        uint32_t factory_calibration_date_supported: 1;
        uint32_t enhanced_offset_compensation_procedure_supported: 1;
        uint32_t distributed_system_support: 2;
    } *feature = (struct cycling_power_feature_data *) str;
#pragma pack(pop)
    balanceFeature->SetValue(feature->pedal_power_balance_supported);
    torqueFeature->SetValue(feature->accumulated_torque_supported);
    wheelRevolutionFeature->SetValue(feature->wheel_revolution_data_supported);
    crankRevolutionFeature->SetValue(feature->crank_revolution_data_supported);
    magnitudesFeature->SetValue(feature->extreme_magnitudes_supported);
    anglesFeature->SetValue(feature->extreme_angles_supported);
    deadSpotsFeature->SetValue(feature->top_and_bottom_dead_spot_angles_supported);
    energyFeature->SetValue(feature->accumulated_energy_supported);
    compensationIndicatorFeature->SetValue(feature->offset_compensation_indicator_supported);
    compensationFeature->SetValue(feature->offset_compensation_supported);
    maskingFeature->SetValue(feature->cycling_power_measurement_characteristic_content_masking_supported);
    locationsFeature->SetValue(feature->multiple_sensor_locations_supported);
    crankLengthFeature->SetValue(feature->crank_length_adjustment_supported);
    chainLengthFeature->SetValue(feature->chain_length_adjustment_supported);
    chainWeightFeature->SetValue(feature->chain_weight_adjustment_supported);
    spanFeature->SetValue(feature->span_length_adjustment_supported);
    contextFeature->SetLabel(feature->sensor_measurement_context ? "Torque based" : "Force based");
    directionFeature->SetValue(feature->instantaneous_mesurement_direction_supported);
    dateFeature->SetValue(feature->factory_calibration_date_supported);
    enhancedCompensationFeature->SetValue(feature->enhanced_offset_compensation_procedure_supported);
    char dis[40];
    switch (feature->distributed_system_support) {
    case 0x00:
        strcpy(dis, "Unspecified");
        ;
        break;
    case 0x01:
        strcpy(dis, "Not for use in a distributed system");
        ;
        break;
    case 0x02:
        strcpy(dis, "For use in a distributed system");
        ;
        break;
    default:
        strcpy(dis, "RFU");
        ;
        break;
    }
    distributedFeature->SetLabel(dis);
}



//##################################################################################################
// Cycling power measurement page
//##################################################################################################
//void IC2Frame::NotifyCyclingPowerMeasurement(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[32] = "Notify measurement ";
//    switch (evt.GetInt()) {
//    case TRUE:
//        ((wxToggleButton *) evt.GetEventObject())->SetLabel("Stop");
//        strcat(cmd, "on\n");
//        SetStatusText("Notify measurement", 0);
//        break;
//    case FALSE:
//        ((wxToggleButton *) evt.GetEventObject())->SetLabel("Notify");
//        strcat(cmd, "off\n");
//        SetStatusText("", 0);
//        break;
//    }
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

void IC2Frame::SetCyclingPowerMeasurement(void *str, int length)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    if (logMeasurement.IsOpened()) {
        logMeasurement.Write(str, length);
    }

    uint8_t index = 2;

#pragma pack(push, 1)
    struct cycling_power_measurement_flags {
        uint16_t pedal_power_balance_present: 1;
        uint16_t pedal_power_balance_reference: 1;
        uint16_t accumulated_torque_present: 1;
        uint16_t accumulated_torque_source: 1;
        uint16_t wheel_revolution_data_present: 1;
        uint16_t crank_revolution_data_present: 1;
        uint16_t extreme_force_magnitudes_present: 1;
        uint16_t extreme_torque_magnitudes_present: 1;
        uint16_t extreme_angles_present: 1;
        uint16_t top_dead_spot_angle_present: 1;
        uint16_t bottom_dead_spot_angle_present: 1;
        uint16_t accumulated_energy_present: 1;
        uint16_t offset_compensation_indicator: 1;
    } *flags = (struct cycling_power_measurement_flags *) str;
#pragma pack(pop)

    int16_t *instantaneous_power = (int16_t *)(((uint8_t *) str) + index);

    instantaneousPower->SetLabel(wxString().Format("%hd W", *instantaneous_power));
    index += sizeof(int16_t);

    if (flags->pedal_power_balance_present) {
        pedalPowerBalancePresent->SetValue(true);
        uint8_t *pedal_power_balance = ((uint8_t *) str) + index;
        pedalPowerBalance->SetLabel(wxString().Format("%0.1f %%", *pedal_power_balance * 0.5f));
        index += sizeof(uint8_t);
    } else {
        pedalPowerBalancePresent->SetValue(false);
    }

    if (flags->pedal_power_balance_reference) {
        pedalPowerBalancePresent->SetLabel("Balance: Left");
    } else {
        pedalPowerBalancePresent->SetLabel("Balance: Unknown");
    }

    if (flags->accumulated_torque_present) {
        accumulatedTorquePresent->SetValue(true);
        uint16_t *accumulated_torque = (uint16_t *)(((uint8_t *) str) + index);
        accumulatedTorque->SetLabel(wxString().Format("%0.2f", *accumulated_torque / 32.0f));
        static uint16_t previous_accumulated_torque = 0;
        torque->SetLabel(wxString().Format("%0.2f N.m", ((int16_t) (*accumulated_torque - previous_accumulated_torque)) / 32.0f));
        previous_accumulated_torque = *accumulated_torque;
        index += sizeof(uint16_t);
    } else {
        accumulatedTorquePresent->SetValue(false);
    }

    if (flags->accumulated_torque_source) {
        accumulatedTorquePresent->SetLabel("Accumulated torque: Crank based");
    } else {
        accumulatedTorquePresent->SetLabel("Accumulated torque: Wheel based");
    }

    if (flags->wheel_revolution_data_present) {
        wheelRevolutionDataPresent->SetValue(true);
#pragma pack(push,1)
        struct wheel_revolution_data_s {
            uint32_t cumulative_wheel_revolutions;
            uint16_t last_wheel_event_time;
        } *wheel_revolution_data = (struct wheel_revolution_data_s *)(((uint8_t *) str) + index);
#pragma pack(pop)
        cumulativeWheelRevolutions->SetLabel(wxString().Format("%u", wheel_revolution_data->cumulative_wheel_revolutions));
        lastWheelEventTime->SetLabel(wxString().Format("%hu s/2048", wheel_revolution_data->last_wheel_event_time));
        static uint32_t previous_cumulative_wheel_revolutions = 0;
        static uint16_t previous_last_wheel_event_time = 0;
        wheelSpeed->SetLabel(wxString().Format("%0.1lf RPM", 60.0 * ((uint32_t)(wheel_revolution_data->cumulative_wheel_revolutions - previous_cumulative_wheel_revolutions)) / (((uint16_t)(wheel_revolution_data->last_wheel_event_time - previous_last_wheel_event_time)) / 2048.0)));
        previous_cumulative_wheel_revolutions = wheel_revolution_data->cumulative_wheel_revolutions;
        previous_last_wheel_event_time = wheel_revolution_data->last_wheel_event_time;
        index += sizeof(struct wheel_revolution_data_s);
    } else {
        wheelRevolutionDataPresent->SetValue(false);
    }

    if (flags->crank_revolution_data_present) {
        crankRevolutionDataPresent->SetValue(true);
#pragma pack(push,1)
        struct crank_revolution_data_s {
            uint16_t cumulative_crank_revolutions;
            uint16_t last_crank_event_time;
        } *crank_revolution_data = (struct crank_revolution_data_s *)(((uint8_t *) str) + index);
#pragma pack(pop)
        cumulativeCrankRevolutions->SetLabel(wxString().Format("%hu", crank_revolution_data->cumulative_crank_revolutions));
        lastCrankEventTime->SetLabel(wxString().Format("%hu s/1024", crank_revolution_data->last_crank_event_time));
        static uint16_t previous_cumulative_crank_revolutions = 0;
        static uint16_t previous_last_crank_event_time = 0;
        cadence->SetLabel(wxString().Format("%0.1lf RPM", 60.0 * ((uint16_t)(crank_revolution_data->cumulative_crank_revolutions - previous_cumulative_crank_revolutions)) / (((uint16_t)(crank_revolution_data->last_crank_event_time - previous_last_crank_event_time)) / 1024.0)));
        previous_cumulative_crank_revolutions = crank_revolution_data->cumulative_crank_revolutions;
        previous_last_crank_event_time = crank_revolution_data->last_crank_event_time;
        index += sizeof(struct crank_revolution_data_s);
    } else {
        crankRevolutionDataPresent->SetValue(false);
    }

    if (flags->extreme_force_magnitudes_present) {
        extremeForceMagnitudesPresent->SetValue(true);
#pragma pack(push,1)
        struct extreme_force_magnitudes_s {
            int16_t maximum_force_magnitude;
            int16_t minimum_force_magnitude;
        } *extreme_force_magnitudes = (struct extreme_force_magnitudes_s *)(((uint8_t *) str) + index);
#pragma pack(pop)
        maximumForceMagnitude->SetLabel(wxString().Format("%hd N", extreme_force_magnitudes->maximum_force_magnitude));
        minimumForceMagnitude->SetLabel(wxString().Format("%hd N", extreme_force_magnitudes->minimum_force_magnitude));
        index += sizeof(struct extreme_force_magnitudes_s);
    } else {
        extremeForceMagnitudesPresent->SetValue(false);
    }

    if (flags->extreme_torque_magnitudes_present) {
        extremeTorqueMagnitudesPresent->SetValue(true);
#pragma pack(push,1)
        struct extreme_torque_magnitudes_s {
            int16_t maximum_torque_magnitude;
            int16_t minimum_torque_magnitude;
        } *extreme_torque_magnitudes = (struct extreme_torque_magnitudes_s *)(((uint8_t *) str) + index);
#pragma pack(pop)
        maximumTorqueMagnitude->SetLabel(wxString().Format("%0.2f N.m", extreme_torque_magnitudes->maximum_torque_magnitude / 32.0f));
        minimumTorqueMagnitude->SetLabel(wxString().Format("%0.2f N.m", extreme_torque_magnitudes->minimum_torque_magnitude / 32.0f));
        index += sizeof(struct extreme_torque_magnitudes_s);
    } else {
        extremeTorqueMagnitudesPresent->SetValue(false);
    }

    printf("about to extreme angles\n");
    if (flags->extreme_angles_present) {
        extremeAnglesPresent->SetValue(true);
#pragma pack(push,1)
        struct extreme_angles_s {
            uint32_t maximum_angle: 12;
            uint32_t minimum_angle: 12;
        } *extreme_angles = (struct extreme_angles_s *)(((uint8_t *) str) + index);
#pragma pack(pop)
        printf("sizeof(struct extreme_angles_s) %lu\n", sizeof(struct extreme_angles_s));
        maximumAngle->SetLabel(wxString().Format(L"%hu°", extreme_angles->maximum_angle));
        minimumAngle->SetLabel(wxString().Format(L"%hu°", extreme_angles->minimum_angle));
        index += sizeof(struct extreme_angles_s);
    } else {
        extremeAnglesPresent->SetValue(false);
    }

    printf("about to top dead spot angle\n");
    if (flags->top_dead_spot_angle_present) {
        topDeadSpotAnglePresent->SetValue(true);
        uint16_t *top_dead_spot_angle = (uint16_t *)(((uint8_t *) str) + index);
        topDeadSpotAngle->SetLabel(wxString().Format(L"%hu°", *top_dead_spot_angle));
        index += sizeof(uint16_t);
    } else {
        topDeadSpotAnglePresent->SetValue(false);
    }

    if (flags->bottom_dead_spot_angle_present) {
        bottomDeadSpotAnglePresent->SetValue(true);
        uint16_t *bottom_dead_spot_angle = (uint16_t *)(((uint8_t *) str) + index);
        bottomDeadSpotAngle->SetLabel(wxString().Format(L"%hu°", *bottom_dead_spot_angle));
        index += sizeof(uint16_t);
    } else {
        bottomDeadSpotAnglePresent->SetValue(false);
    }

    if (flags->accumulated_energy_present) {
        accumulatedEnergyPresent->SetValue(true);
        uint16_t *accumulated_energy = (uint16_t *)(((uint8_t *) str) + index);
        accumulatedEnergy->SetLabel(wxString().Format("%hd KJ", *accumulated_energy));
        index += sizeof(uint16_t);
    } else {
        accumulatedEnergyPresent->SetValue(false);
    }

    if (flags->offset_compensation_indicator) {
        offsetCompensationIndicator->SetValue(true);
    } else {
        offsetCompensationIndicator->SetValue(false);
    }
    measurement->PostSizeEvent();
}


//##################################################################################################
// Cycling power control point page
//##################################################################################################
struct IC2Frame::sensorLocations_s IC2Frame::sensorLocations[17] = {
    {0, wxString("Other")},
    {1, wxString("Top of shoe")},
    {2, wxString("In shoe")},
    {3, wxString("Hip")},
    {4, wxString("Front Wheel")},
    {5, wxString("Left Crank")},
    {6, wxString("Right Crank")},
    {7, wxString("Left Pedal")},
    {8, wxString("Right Pedal")},
    {9, wxString("Front Hub")},
    {10, wxString("Rear Dropout")},
    {11, wxString("Chainstay")},
    {12, wxString("Rear Wheel")},
    {13, wxString("Rear Hub")},
    {14, wxString("Chest")},
    {15, wxString("Spider")},
    {16, wxString("Chain Ring")},
};

void IC2Frame::SetCyclingPowerControlPoint(void *str, int length)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
#pragma pack(push, 1)
    struct control_point_data {
        uint8_t op_code;
        uint8_t request_code;
        uint8_t response_code;
        uint8_t data[];
    } *cp_data = (struct control_point_data *) str;
#pragma pack(pop)

    enum op_codes {
        SET_CUMULATIVE_VALUE = 1,
        UPDATE_SENSOR_LOCATION,
        REQUEST_SUPPORTED_SENSOR_LOCATIONS,
        SET_CRANK_LENGTH,
        REQUEST_CRANK_LENGTH,
        SET_CHAIN_LENGTH,
        REQUEST_CHAIN_LENGTH,
        SET_CHAIN_WEIGHT,
        REQUEST_CHAIN_WEIGHT,
        SET_SPAN_LENGTH,
        REQUEST_SPAN_LENGTH,
        START_OFFSET_COMPENSATION,
        MAST_CYCLING_POWER_MEASUREMENT_CHARACTERISTIC_CONTENT,
        REQUEST_SAMPLING_RATE,
        REQUEST_FACTORY_CALIBRATION_DATE,
        START_ENHANCED_OFFSET_COMPENSATION,
        RESPONSE_CODE = 0x20,
    };

    enum response_codes {
        SUCCESS = 1,
        NOT_SUPPORTED,
        INVALID_OPERAND,
        OPERATION_FAILED,
    };
    if (cp_data->op_code != RESPONSE_CODE) {
        SetStatusText("Failed - Unknown response", 1);
        return;
    }
    switch (cp_data->response_code) {
    case SUCCESS:
        SetStatusText("Success", 1);
        break;
    case NOT_SUPPORTED:
        SetStatusText("Failed - Not supported", 1);
        return;
    case INVALID_OPERAND:
        SetStatusText("Failed - Invalid operand", 1);
        return;
    case OPERATION_FAILED:
        SetStatusText("Failed - Operation failed", 1);
        return;
    default:
        SetStatusText("Failed - Unknown response", 1);
        return;
    }
    wxString fmt;
    switch (cp_data->request_code) {
    case REQUEST_SUPPORTED_SENSOR_LOCATIONS:
        location->Clear();
        for (int i = 3; i < length; i++) {
            location->Append(sensorLocations[cp_data->data[i - 3]].location, &sensorLocations[cp_data->data[i - 3]].index);
        }
        break;
    case REQUEST_CRANK_LENGTH:
        crankLength->SetValue(wxString() << (*((uint16_t *) cp_data->data)) / 2.0);
        break;
    case REQUEST_CHAIN_LENGTH:
        chainLength->SetValue(wxString() << (*((uint16_t *) cp_data->data)));
        break;
    case REQUEST_CHAIN_WEIGHT:
        chainWeight->SetValue(wxString() << (*((uint16_t *) cp_data->data)));
        break;
    case REQUEST_SPAN_LENGTH:
        span->SetValue(wxString() << (*((uint16_t *) cp_data->data)));
        break;
    case START_OFFSET_COMPENSATION:
        offsetCompensationValue->SetLabel(wxString().Format("%0.2f N.m", (*((int16_t *) cp_data->data)) / 32.0f));
        break;
    case REQUEST_SAMPLING_RATE:
        samplingRate->SetLabel(wxString().Format("%hhu Hz", *((uint8_t *) cp_data->data)));
        break;
    case REQUEST_FACTORY_CALIBRATION_DATE:
        fmt.Printf("%04hu-%02hhu-%02hhu %02hhu:%02hhu:%02hhu", *((uint16_t *) cp_data->data), cp_data->data[2], cp_data->data[3], cp_data->data[4], cp_data->data[5], cp_data->data[6]);
        calibrationDate->SetLabel(fmt);
        break;
    case START_ENHANCED_OFFSET_COMPENSATION:
        enhancedOffsetCompensationValue->SetLabel(wxString().Format("%0.2f N.m", (*((int16_t *) cp_data->data)) / 32.0f));
        break;
    }


}

//void IC2Frame::SetCumulativeValue(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[32];
//    sprintf(cmd, "Set cumulative value %s\n", evt.GetString().c_str().AsChar());
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

void IC2Frame::SetSensorLocation(wxCommandEvent &evt)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    char cmd[32];
    sprintf(cmd, "Set sensor location %d\n", *((int *) evt.GetClientData()));
    SetStatusText(cmd, 0);
    SetStatusText("", 1);
    SendCommand(cmd);
}

//void IC2Frame::GetSupportedSensorLocations(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Get supported sensor locations\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//void IC2Frame::SetCrankLength(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[32];
//    sprintf(cmd, "Set crank length %s\n", evt.GetString().c_str().AsChar());
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//void IC2Frame::GetCrankLength(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Get crank length\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//void IC2Frame::SetChainLength(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[32];
//    sprintf(cmd, "Set chain length %s\n", evt.GetString().c_str().AsChar());
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//void IC2Frame::GetChainLength(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Get chain length\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//void IC2Frame::SetChainWeight(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[32];
//    sprintf(cmd, "Set chain weight %s\n", evt.GetString().c_str().AsChar());
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//void IC2Frame::GetChainWeight(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Get chain weight\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//void IC2Frame::SetSpan(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[32];
//    sprintf(cmd, "Set span %s\n", evt.GetString().c_str().AsChar());
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//void IC2Frame::GetSpan(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Get span\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//void IC2Frame::StartOffsetCompensation(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Start offset compensation\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

void IC2Frame::MaskMeasurement(wxCommandEvent &evt)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    char cmd[32];
    sprintf(
        cmd, "Mask measurement %d\n",
        pedalPowerBalanceMask  ->IsChecked() |
        accumulatedTorqueMask  ->IsChecked() << 1 |
        wheelRevolutionDataMask->IsChecked() << 2 |
        crankRevolutionDataMask->IsChecked() << 3 |
        extremeMagnitudesMask  ->IsChecked() << 4 |
        extremeAnglesMask      ->IsChecked() << 5 |
        topDeadSpotAngleMask   ->IsChecked() << 6 |
        bottomDeadSpotAngleMask->IsChecked() << 7 |
        accumulatedEnergyMask  ->IsChecked() << 8
    );
    SetStatusText(cmd, 0);
    SetStatusText("", 1);
    SendCommand(cmd);
}

//void IC2Frame::GetSamplingRate(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Get sampling rate\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//void IC2Frame::GetFactoryCalibrationDate(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Get factory calibration date\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//void IC2Frame::StartEnhancedOffsetCompensation(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Start enhanced offset compensation\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}


//##################################################################################################
// Sensor location page
//##################################################################################################


//void IC2Frame::GetSensorLocation(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[] = "Get sensor location\n";
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

void IC2Frame::SetSensorLocation(uint8_t idx)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    sensorLocation->SetLabel(sensorLocations[idx].location);
}



//##################################################################################################
// Cycling power vector page
//##################################################################################################
//void IC2Frame::NotifyCyclingPowerVector(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[32] = "Notify vector ";
//    switch (evt.GetInt()) {
//    case TRUE:
//        ((wxToggleButton *) evt.GetEventObject())->SetLabel("Stop");
//        strcat(cmd, "on\n");
//        SetStatusText("Notify vector", 0);
//        break;
//    case FALSE:
//        ((wxToggleButton *) evt.GetEventObject())->SetLabel("Notify");
//        strcat(cmd, "off\n");
//        SetStatusText("", 0);
//        break;
//    }
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}



void IC2Frame::SetCyclingPowerVector(void *str, int length)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    if (logVector.IsOpened()) {
        logVector.Write(str, length);
    }

    uint8_t index = 1;

#pragma pack(push, 1)
    struct cycling_power_vector_flags {
        uint8_t crank_revolution_data_present: 1;
        uint8_t first_crank_measurement_angle_present: 1;
        uint8_t instantaneous_force_magnitude_array_present: 1;
        uint8_t instantaneous_torque_magnitude_array_present: 1;
        uint8_t instantaneous_measurement_direction: 2;
    } *flags = (struct cycling_power_vector_flags *) str;
#pragma pack(pop)

    char direction[][21] = {"Unknown", "Tangential Component", "Radial Component", "Lateral Component"};

    instantaneousMeasurementDirection->SetLabel(direction[flags->instantaneous_measurement_direction]);

    if (flags->crank_revolution_data_present) {
        crankRevolutionDataVectorPresent->SetValue(true);
#pragma pack(push,1)
        struct crank_revolution_data_s {
            uint16_t cumulative_crank_revolutions;
            uint16_t last_crank_event_time;
        } *crank_revolution_data = (struct crank_revolution_data_s *)(((uint8_t *) str) + index);
#pragma pack(pop)
        cumulativeCrankVectorRevolutions->SetLabel(wxString().Format("%hu", crank_revolution_data->cumulative_crank_revolutions));
        lastCrankEventVectorTime->SetLabel(wxString().Format("%hu s/1024", crank_revolution_data->last_crank_event_time));
        index += sizeof(struct crank_revolution_data_s);
    } else {
        crankRevolutionDataVectorPresent->SetValue(false);
    }

    if (flags->first_crank_measurement_angle_present) {
        firstCrankMeasurementAnglePresent->SetValue(true);
        uint16_t *first_crank_angle_measurement = (uint16_t *)(((uint8_t *) str) + index);
        firstCrankMeasurementAngle->SetLabel(wxString().Format(L"%hu°", *first_crank_angle_measurement));
        index += sizeof(uint16_t);
    } else {
        firstCrankMeasurementAnglePresent->SetValue(false);
    }

    wxSizerFlags fieldFlags;
    fieldFlags.Border(wxLEFT | wxRIGHT, 10);

    if (flags->instantaneous_force_magnitude_array_present) {
        instantaneousForceMagnitudeArrayPresent->SetValue(true);
        int16_t *instantaneous_force_magnitude_array = (int16_t *)(((uint8_t *) str) + index);
        forceArraySizer->Clear(true);
        for (int i = 0; index < length; i ++) {
            forceArraySizer->Add(new wxStaticText(vector, wxID_ANY, wxString().Format("%hd", instantaneous_force_magnitude_array[i])), fieldFlags);
            index += sizeof(uint16_t);
        }
        vector->PostSizeEvent();        // wxWrapSizer needs a resize to layout correctly
    } else {
        instantaneousForceMagnitudeArrayPresent->SetValue(false);
    }

    if (flags->instantaneous_torque_magnitude_array_present) {
        instantaneousTorqueMagnitudeArrayPresent->SetValue(true);
        int16_t *instantaneous_torque_magnitude_array = (int16_t *)(((uint8_t *) str) + index);
        torqueArraySizer->Clear(true);
        for (int i = 0; index < length; i ++) {
            torqueArraySizer->Add(new wxStaticText(vector, wxID_ANY, wxString().Format("%0.2f", instantaneous_torque_magnitude_array[i] / 32.0f)), fieldFlags);
            index += sizeof(uint16_t);
        }
        vector->PostSizeEvent();        // wxWrapSizer needs a resize to layout correctly
    } else {
        instantaneousTorqueMagnitudeArrayPresent->SetValue(false);
    }


}


//##################################################################################################
// InfoCrank control point page
//##################################################################################################
void IC2Frame::SetInfoCrankControlPoint(void *str, int length)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
#pragma pack(push, 1)
    struct control_point_data {
        uint8_t op_code;
        uint8_t request_code;
        uint8_t response_code;
        uint8_t data[];
    } *cp_data = (struct control_point_data *) str;
#pragma pack(pop)

    enum op_codes {
        SET_SERIAL_NUMBER = 1,
        SET_FACTORY_CALIBRATION_DATE,
        SET_CALIBRATION_PARAMETERS,
        REQUEST_CALIBRATION_PARAMETERS,
        SET_ACCEL1_TRANSFORM,
        REQUEST_ACCEL1_TRANSFORM,
        SET_ACCEL2_TRANSFORM,
        REQUEST_ACCEL2_TRANSFORM,
        SET_KF_PARAMETERS,
        REQUEST_KF_PARAMETERS,
        SET_PARTNER_ADDRESS,
        REQUEST_PARTNER_ADDRESS,
        DELETE_PARTNER_ADDRESS,
        SET_CYCLING_POWER_VECTOR_PARAMETERS,
        REQUEST_CYCLING_POWER_VECTOR_PARAMETERS,
        RESPONSE_CODE = 0x20,
    };

    enum response_codes {
        SUCCESS = 1,
        NOT_SUPPORTED,
        INVALID_OPERAND,
        OPERATION_FAILED,
    };
    if (cp_data->op_code != RESPONSE_CODE) {
        SetStatusText("Failed - Unknown response", 1);
        return;
    }
    switch (cp_data->response_code) {
    case SUCCESS:
        SetStatusText("Success", 1);
        break;
    case NOT_SUPPORTED:
        SetStatusText("Failed - Not supported", 1);
        return;
    case INVALID_OPERAND:
        SetStatusText("Failed - Invalid operand", 1);
        return;
    case OPERATION_FAILED:
        SetStatusText("Failed - Operation failed", 1);
        return;
    default:
        SetStatusText("Failed - Unknown response", 1);
        return;
    }
    switch (cp_data->request_code) {
    case REQUEST_CALIBRATION_PARAMETERS:
        k1->SetValue(wxString().Format("%f", *((float *) &cp_data->data[0])));
        k2->SetValue(wxString().Format("%f", *((float *) &cp_data->data[4])));
        k3->SetValue(wxString().Format("%f", *((float *) &cp_data->data[8])));
        k4->SetValue(wxString().Format("%f", *((float *) &cp_data->data[12])));
        k5->SetValue(wxString().Format("%f", *((float *) &cp_data->data[16])));
        k6->SetValue(wxString().Format("%f", *((float *) &cp_data->data[20])));
        break;
    case REQUEST_ACCEL1_TRANSFORM:
        a1[0]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[0])));
        a1[1]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[2])));
        a1[2]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[4])));
        a1[3]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[6])));
        a1[4]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[8])));
        a1[5]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[10])));
        a1[6]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[12])));
        a1[7]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[14])));
        a1[8]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[16])));
        a1[9]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[18])));
        a1[10]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[20])));
        a1[11]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[22])));
        break;
    case REQUEST_ACCEL2_TRANSFORM:
        a2[0]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[0])));
        a2[1]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[2])));
        a2[2]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[4])));
        a2[3]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[6])));
        a2[4]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[8])));
        a2[5]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[10])));
        a2[6]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[12])));
        a2[7]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[14])));
        a2[8]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[16])));
        a2[9]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[18])));
        a2[10]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[20])));
        a2[11]->SetValue(wxString().Format("%hd", *((int16_t *) &cp_data->data[22])));
        break;
    case REQUEST_KF_PARAMETERS:
        s2alpha->SetValue(wxString().Format("%0.4f", *((float *) &cp_data->data[0])));
        s2accel->SetValue(wxString().Format("%0.4f", *((float *) &cp_data->data[4])));
        driveRatio->SetValue(wxString().Format("%0.2f", *((float *) &cp_data->data[8])));
        r1->SetValue(wxString().Format("%0.4f", *((float *) &cp_data->data[12])));
        r2->SetValue(wxString().Format("%0.4f", *((float *) &cp_data->data[16])));
        break;
    case REQUEST_PARTNER_ADDRESS:
        ble_addr[0]->SetValue(wxString().Format("%02hhX", cp_data->data[0]));
        ble_addr[1]->SetValue(wxString().Format("%02hhX", cp_data->data[1]));
        ble_addr[2]->SetValue(wxString().Format("%02hhX", cp_data->data[2]));
        ble_addr[3]->SetValue(wxString().Format("%02hhX", cp_data->data[3]));
        ble_addr[4]->SetValue(wxString().Format("%02hhX", cp_data->data[4]));
        ble_addr[5]->SetValue(wxString().Format("%02hhX", cp_data->data[5]));
        break;
    case REQUEST_CYCLING_POWER_VECTOR_PARAMETERS:
        cpvSize->SetValue(wxString().Format("%hhu", cp_data->data[0]));
        cpvDownsample->SetValue(wxString().Format("%hhu", cp_data->data[1]));
        break;
    }
}


//void IC2Frame::SetSerialNumber(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//
//    wxString cmd = wxString().Format("%s %s\n", *((wxString *) evt.GetEventUserData()), ((wxTextCtrl *) evt.GetEventObject())->GetValue());
////    char cmd[64];
////    sprintf(
////        cmd, "Set factory calibration date %s\n", ((wxTextCtrl *) evt.GetEventObject())->GetValue().c_str().AsChar());
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
////    SendCommand(cmd.c_str().AsChar());
//}

//void IC2Frame::SetFactoryCalibrationDate(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//
//    char cmd[64];
//    sprintf(
//        cmd, "Set factory calibration date %s\n", ((wxTextCtrlObject *) evt.GetEventObject())->m_textCtrl->GetValue().c_str().AsChar());
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}

//void IC2Frame::DateTimeNow(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//
//    wxString now = wxDateTime().Now().Format("%FT%T", wxDateTime::UTC);
//
//    ((wxTextCtrlObject *) evt.GetEventUserData())->m_textCtrl->SetValue(now);
//
//    wxString cmd = wxString().Format("Set factory calibration date %s\n", now);
//    SetStatusText(cmd, 0);
//    SetStatusText("", 1);
//    SendCommand(cmd.c_str().AsChar());
//}


//##################################################################################################
// InfoCrank raw data page
//##################################################################################################
void IC2Frame::SetInfoCrankRawData(void *str, int length)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    if (logRaw.IsOpened()) {
        logRaw.Write(str, length);
    }
#pragma pack(push, 1)
    struct raw_data {
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
            uint16_t voltage;
            struct {
                int32_t position;
                int32_t velocity;
                int32_t acceleration;
            } state;
            float vector4[4];
            float matrix33[3][3];
        } data;
    } *raw_data = (struct raw_data *) str;
    uint8_t *byte_pointer_to_raw_data = (uint8_t *) raw_data;
#pragma pack(pop)

    enum op_codes {
        STRAIN_DATA = 0,
        ACCELERATION_DATA,
        TEMPERATURE_DATA,
        BATTERY_DATA,
        STATE_DATA,
        VECTOR4,
        MATRIX33,
    };
    while (length) {
        switch (raw_data->op_code) {
        case STRAIN_DATA:
            strain_num ++;
            strain_sum += raw_data->data.strain.strain;
            strain_sum2 += (long)(raw_data->data.strain.strain) * (long)(raw_data->data.strain.strain);
            {
                double mean = (double) strain_sum / (double) strain_num;
                double sd = pow((double) strain_sum2 / (double) strain_num - mean*mean, 0.5);
                strain->SetValue(wxString().Format("%d", raw_data->data.strain.strain));
                strain_avg->SetValue(wxString().Format("%0.3lf", mean));
                strain_sd->SetValue(wxString().Format("%0.3lf", sd));
            }
            length -= 4;
            byte_pointer_to_raw_data += 4;
            raw_data = (struct raw_data *) byte_pointer_to_raw_data;
            break;
        case ACCELERATION_DATA:
            accel1_num ++;
            accel1X_sum += raw_data->data.acceleration.accel1_x;
            accel1X_sum2 += (raw_data->data.acceleration.accel1_x) * (raw_data->data.acceleration.accel1_x);
            accel1Y_sum += raw_data->data.acceleration.accel1_y;
            accel1Y_sum2 += (raw_data->data.acceleration.accel1_y) * (raw_data->data.acceleration.accel1_y);
            accel1Z_sum += raw_data->data.acceleration.accel1_z;
            accel1Z_sum2 += (raw_data->data.acceleration.accel1_z) * (raw_data->data.acceleration.accel1_z);

            accel2_num ++;
            accel2X_sum += raw_data->data.acceleration.accel2_x;
            accel2X_sum2 += (raw_data->data.acceleration.accel2_x) * (raw_data->data.acceleration.accel2_x);
            accel2Y_sum += raw_data->data.acceleration.accel2_y;
            accel2Y_sum2 += (raw_data->data.acceleration.accel2_y) * (raw_data->data.acceleration.accel2_y);
            accel2Z_sum += raw_data->data.acceleration.accel2_z;
            accel2Z_sum2 += (raw_data->data.acceleration.accel2_z) * (raw_data->data.acceleration.accel2_z);

            {
                double mean = accel1X_sum / accel1_num;
                double sd = pow(accel1X_sum2 / accel1_num - (accel1X_sum / accel1_num) * (accel1X_sum / accel1_num), 0.5);
                // Convert to g. ±8g full scale, 14 bit resolution
                accel1[0]->SetValue(wxString().Format("%0.3f", (raw_data->data.acceleration.accel1_x) * 8.0 / 32768.0));
                accel1_avg[0]->SetValue(wxString().Format("%0.3f", mean * 8.0 / 32768.0));
                accel1_sd[0]->SetValue(wxString().Format("%0.3f", sd * 8.0 / 32768.0));
            }

            {
                double mean = accel1Y_sum / accel1_num;
                double sd = pow(accel1Y_sum2 / accel1_num - (accel1Y_sum / accel1_num) * (accel1Y_sum / accel1_num), 0.5);
                // Convert to g. ±8g full scale, 14 bit resolution
                accel1[1]->SetValue(wxString().Format("%0.3f", (raw_data->data.acceleration.accel1_y) * 8.0 / 32768.0));
                accel1_avg[1]->SetValue(wxString().Format("%0.3f", mean * 8.0 / 32768.0));
                accel1_sd[1]->SetValue(wxString().Format("%0.3f", sd * 8.0 / 32768.0));
            }

            {
                double mean = accel1Z_sum / accel1_num;
                double sd = pow(accel1Z_sum2 / accel1_num - (accel1Z_sum / accel1_num) * (accel1Z_sum / accel1_num), 0.5);
                // Convert to g. ±8g full scale, 14 bit resolution
                accel1[2]->SetValue(wxString().Format("%0.3f", (raw_data->data.acceleration.accel1_z) * 8.0 / 32768.0));
                accel1_avg[2]->SetValue(wxString().Format("%0.3f", mean * 8.0 / 32768.0));
                accel1_sd[2]->SetValue(wxString().Format("%0.3f", sd * 8.0 / 32768.0));
            }

            {
                double mean = accel2X_sum / accel2_num;
                double sd = pow(accel2X_sum2 / accel2_num - (accel2X_sum / accel2_num) * (accel2X_sum / accel2_num), 0.5);
                // Convert to g. ±8g full scale, 14 bit resolution
                accel2[0]->SetValue(wxString().Format("%0.3f", (raw_data->data.acceleration.accel2_x) * 8.0 / 32768.0));
                accel2_avg[0]->SetValue(wxString().Format("%0.3f", mean * 8.0 / 32768.0));
                accel2_sd[0]->SetValue(wxString().Format("%0.3f", sd * 8.0 / 32768.0));
            }

            {
                double mean = accel2Y_sum / accel2_num;
                double sd = pow(accel2Y_sum2 / accel2_num - (accel2Y_sum / accel2_num) * (accel2Y_sum / accel2_num), 0.5);
                // Convert to g. ±8g full scale, 14 bit resolution
                accel2[1]->SetValue(wxString().Format("%0.3f", (raw_data->data.acceleration.accel2_y) * 8.0 / 32768.0));
                accel2_avg[1]->SetValue(wxString().Format("%0.3f", mean * 8.0 / 32768.0));
                accel2_sd[1]->SetValue(wxString().Format("%0.3f", sd * 8.0 / 32768.0));
            }

            {
                double mean = accel2Z_sum / accel2_num;
                double sd = pow(accel2Z_sum2 / accel2_num - (accel2Z_sum / accel2_num) * (accel2Z_sum / accel2_num), 0.5);
                // Convert to g. ±8g full scale, 14 bit resolution
                accel2[2]->SetValue(wxString().Format("%0.3f", (raw_data->data.acceleration.accel2_z) * 8.0 / 32768.0));
                accel2_avg[2]->SetValue(wxString().Format("%0.3f", mean * 8.0 / 32768.0));
                accel2_sd[2]->SetValue(wxString().Format("%0.3f", sd * 8.0 / 32768.0));
            }
            length -= 13;
            byte_pointer_to_raw_data += 13;
            raw_data = (struct raw_data *) byte_pointer_to_raw_data;
            break;
        case TEMPERATURE_DATA:
            temp = raw_data->data.temperature.integral + 1.0e-6 * (raw_data->data.temperature.fractional);
            {
                temperature->SetValue(wxString().Format("%0.2lf", temp));
            }
            length -= 7;
            byte_pointer_to_raw_data += 7;
            raw_data = (struct raw_data *) byte_pointer_to_raw_data;
            break;
        case BATTERY_DATA:
            volts = 0.6f*6.0f*raw_data->data.voltage*0x01p-12;
            {
                batteryVoltage->SetValue(wxString().Format("%0.2lf", volts));
            }
            length -= 3;
            byte_pointer_to_raw_data += 3;
            raw_data = (struct raw_data *) byte_pointer_to_raw_data;
            break;
        case STATE_DATA:
            x->SetValue(wxString().Format(L"%+10.1f°", raw_data->data.state.position * 360.0 * 0x01p-13));
            x_dot->SetValue(wxString().Format(L"%+10.1f°/sec", raw_data->data.state.velocity * 360.0 * 128.0 * 0x01p-35));
            x_ddot->SetValue(wxString().Format(L"%+10.1f°/sec²", raw_data->data.state.acceleration * 360.0 * 16384.0 * 0x01p-40));
            crank_graphics->angle = raw_data->data.state.position * 360.0 / 8192.0;
            crank_graphics->newAngle = true;
            length -= 13;
            byte_pointer_to_raw_data += 13;
            raw_data = (struct raw_data *) byte_pointer_to_raw_data;
            break;
        case VECTOR4:
            printf("Vector: %0.4g, %0.4g, %0.4g, %0.4g\n",
                   raw_data->data.vector4[0],
                   raw_data->data.vector4[1],
                   raw_data->data.vector4[2],
                   raw_data->data.vector4[3]);
            length -= 17;
            byte_pointer_to_raw_data += 17;
            raw_data = (struct raw_data *) byte_pointer_to_raw_data;
            break;
        case MATRIX33:
            printf("Matrix: %0.4g, %0.4g, %0.4g\n"
                   "        %0.4g, %0.4g, %0.4g\n"
                   "        %0.4g, %0.4g, %0.4g\n",
                   raw_data->data.matrix33[0][0], raw_data->data.matrix33[0][1], raw_data->data.matrix33[0][2],
                   raw_data->data.matrix33[1][0], raw_data->data.matrix33[1][1], raw_data->data.matrix33[1][2],
                   raw_data->data.matrix33[1][0], raw_data->data.matrix33[2][1], raw_data->data.matrix33[2][2]);
            length -= 37;
            byte_pointer_to_raw_data += 37;
            raw_data = (struct raw_data *) byte_pointer_to_raw_data;
            break;
        default :
            printf("WARNING: Undefined Op Code %d\n", raw_data->op_code);
            length = 0;
            break;
        }
    }

//    crank_graphics->Refresh();
//    crank_graphics->Update();

}
//void IC2Frame::NotifyInfoCrankRaw(wxCommandEvent &evt)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//    char cmd[32] = "Notify raw ";
//    switch (evt.GetInt()) {
//    case TRUE:
//        ((wxToggleButton *) evt.GetEventObject())->SetLabel("Stop");
//        strcat(cmd, "on\n");
//        SetStatusText("Notify raw", 0);
//        strain_num = 0.001;
//        strain_sum = 0.0;
//        strain_sum2 = 0.0;
//        accel1_num = 0.001;
//        accel1X_sum = 0.0;
//        accel1X_sum2 = 0.0;
//        accel1Y_sum = 0.0;
//        accel1Y_sum2 = 0.0;
//        accel1Z_sum = 0.0;
//        accel1Z_sum2 = 0.0;
//        accel2_num = 0.001;
//        accel2X_sum = 0.0;
//        accel2X_sum2 = 0.0;
//        accel2Y_sum = 0.0;
//        accel2Y_sum2 = 0.0;
//        accel2Z_sum = 0.0;
//        accel2Z_sum2 = 0.0;
//        break;
//    case FALSE:
//        ((wxToggleButton *) evt.GetEventObject())->SetLabel("Notify");
//        strcat(cmd, "off\n");
//        SetStatusText("", 0);
//        break;
//    }
//    SetStatusText("", 1);
//    SendCommand(cmd);
//}


//void IC2Frame::Strain(void *data)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//#pragma pack(push, 1)
//    struct raw_data {
//        int32_t unused0: 6;
//        int32_t strain: 18;
//    } *raw_data = (struct raw_data *) data;
//#pragma pack(pop)
//    if (logRaw.IsOpened()) {
//        logRaw.Write(wxString().Format("%d\n", raw_data->strain));
//    }
//    strain_num ++;
//    strain_sum += raw_data->strain;
//    strain_sum2 += (raw_data->strain) * (raw_data->strain);
//
//    {
//        double mean = strain_sum / strain_num;
//        double sd = pow(strain_sum2 / strain_num - (strain_sum / strain_num) * (strain_sum / strain_num), 0.5);
//        strain->SetValue(wxString().Format("%d", raw_data->strain));
//        strain_avg->SetValue(wxString().Format("%0.3f", mean));
//        strain_sd->SetValue(wxString().Format("%0.3f", sd));
//    }
//}


//void IC2Frame::Temperature(void *data)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//#pragma pack(push, 1)
//    struct raw_data {
//        int16_t integral;
//        int32_t fractional;
//    } *raw_data = (struct raw_data *) data;
//#pragma pack(pop)
//    if (logRaw.IsOpened()) {
//    }
//
//    temp = raw_data->integral + 1.0e-6 * (raw_data->fractional);
//    {
//        temperature->SetValue(wxString().Format("%0.2lf", temp));
//    }
//}


//void IC2Frame::Acceleration(void *data)
//{
//    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
//#pragma pack(push, 1)
//    // Accelerometers are 14 bit resolution with lowest two bits set to zero.
//    struct raw_data {
//        int16_t accel1_x;
//        int16_t accel1_y;
//        int16_t accel1_z;
//        int16_t accel2_x;
//        int16_t accel2_y;
//        int16_t accel2_z;
//    } *raw_data = (struct raw_data *) data;
//#pragma pack(pop)
//    if (logRaw.IsOpened()) {
//    }
//
//    accel1_num ++;
//    accel1X_sum += raw_data->accel1_x;
//    accel1X_sum2 += (raw_data->accel1_x) * (raw_data->accel1_x);
//    accel1Y_sum += raw_data->accel1_y;
//    accel1Y_sum2 += (raw_data->accel1_y) * (raw_data->accel1_y);
//    accel1Z_sum += raw_data->accel1_z;
//    accel1Z_sum2 += (raw_data->accel1_z) * (raw_data->accel1_z);
//
//    accel2_num ++;
//    accel2X_sum += raw_data->accel2_x;
//    accel2X_sum2 += (raw_data->accel2_x) * (raw_data->accel2_x);
//    accel2Y_sum += raw_data->accel2_y;
//    accel2Y_sum2 += (raw_data->accel2_y) * (raw_data->accel2_y);
//    accel2Z_sum += raw_data->accel2_z;
//    accel2Z_sum2 += (raw_data->accel2_z) * (raw_data->accel2_z);
//
//    {
//        double mean = accel1X_sum / accel1_num;
//        double sd = pow(accel1X_sum2 / accel1_num - (accel1X_sum / accel1_num) * (accel1X_sum / accel1_num), 0.5);
//        // Convert to g. ±8g full scale, 14 bit resolution
//        accel1[0]->SetValue(wxString().Format("%0.3f", (raw_data->accel1_x) * 8.0 / 32768.0));
//        accel1_avg[0]->SetValue(wxString().Format("%0.3f", mean * 8.0 / 32768.0));
//        accel1_sd[0]->SetValue(wxString().Format("%0.3f", sd * 8.0 / 32768.0));
//    }
//
//    {
//        double mean = accel1Y_sum / accel1_num;
//        double sd = pow(accel1Y_sum2 / accel1_num - (accel1Y_sum / accel1_num) * (accel1Y_sum / accel1_num), 0.5);
//        // Convert to g. ±8g full scale, 14 bit resolution
//        accel1[1]->SetValue(wxString().Format("%0.3f", (raw_data->accel1_y) * 8.0 / 32768.0));
//        accel1_avg[1]->SetValue(wxString().Format("%0.3f", mean * 8.0 / 32768.0));
//        accel1_sd[1]->SetValue(wxString().Format("%0.3f", sd * 8.0 / 32768.0));
//    }
//
//    {
//        double mean = accel1Z_sum / accel1_num;
//        double sd = pow(accel1Z_sum2 / accel1_num - (accel1Z_sum / accel1_num) * (accel1Z_sum / accel1_num), 0.5);
//        // Convert to g. ±8g full scale, 14 bit resolution
//        accel1[2]->SetValue(wxString().Format("%0.3f", (raw_data->accel1_z) * 8.0 / 32768.0));
//        accel1_avg[2]->SetValue(wxString().Format("%0.3f", mean * 8.0 / 32768.0));
//        accel1_sd[2]->SetValue(wxString().Format("%0.3f", sd * 8.0 / 32768.0));
//    }
//
//    {
//        double mean = accel2X_sum / accel2_num;
//        double sd = pow(accel2X_sum2 / accel2_num - (accel2X_sum / accel2_num) * (accel2X_sum / accel2_num), 0.5);
//        // Convert to g. ±8g full scale, 14 bit resolution
//        accel2[0]->SetValue(wxString().Format("%0.3f", (raw_data->accel2_x) * 8.0 / 32768.0));
//        accel2_avg[0]->SetValue(wxString().Format("%0.3f", mean * 8.0 / 32768.0));
//        accel2_sd[0]->SetValue(wxString().Format("%0.3f", sd * 8.0 / 32768.0));
//    }
//
//    {
//        double mean = accel2Y_sum / accel2_num;
//        double sd = pow(accel2Y_sum2 / accel2_num - (accel2Y_sum / accel2_num) * (accel2Y_sum / accel2_num), 0.5);
//        // Convert to g. ±8g full scale, 14 bit resolution
//        accel2[1]->SetValue(wxString().Format("%0.3f", (raw_data->accel2_y) * 8.0 / 32768.0));
//        accel2_avg[1]->SetValue(wxString().Format("%0.3f", mean * 8.0 / 32768.0));
//        accel2_sd[1]->SetValue(wxString().Format("%0.3f", sd * 8.0 / 32768.0));
//    }
//
//    {
//        double mean = accel2Z_sum / accel2_num;
//        double sd = pow(accel2Z_sum2 / accel2_num - (accel2Z_sum / accel2_num) * (accel2Z_sum / accel2_num), 0.5);
//        // Convert to g. ±8g full scale, 14 bit resolution
//        accel2[2]->SetValue(wxString().Format("%0.3f", (raw_data->accel2_z) * 8.0 / 32768.0));
//        accel2_avg[2]->SetValue(wxString().Format("%0.3f", mean * 8.0 / 32768.0));
//        accel2_sd[2]->SetValue(wxString().Format("%0.3f", sd * 8.0 / 32768.0));
//    }
//
//}

//##################################################################################################
// Bluetooth device window
//##################################################################################################
BLEDevice::BLEDevice(IC2Frame *context, wxWindow *parent, const wxString &name, const wxString &address) : wxStaticBoxSizer(wxVERTICAL, parent, name)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    wxStaticText *addr = new wxStaticText(parent, wxID_ANY, address);
    wxToggleButton *button = new wxToggleButton(parent, wxID_ANY, "Connect");
    Add(addr, 1, wxALL, 10);
    Add(button, 1, wxALL | wxEXPAND, 10);
//    userData = new UserData(address);
    button->Bind(wxEVT_TOGGLEBUTTON, &IC2Frame::OnConnect, context, wxID_ANY, wxID_ANY,  new UserData(address));
}

BLEDevice::~BLEDevice()
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    printf("BLEDevice::~BLEDevice()\n");
}





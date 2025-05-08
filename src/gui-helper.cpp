
// -------------------------------------------------------------------------------------------------
// Helper functions to make the main program a bit more simple
// Created by: Anna Pedersen
// Date: 27/04/2025
// -------------------------------------------------------------------------------------------------

#include "gui-helper.h"
#include "main.h"

#include <wx/menu.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>

// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void CreateMenuBar(IC2Frame* frame) {
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
    frame->Bind(wxEVT_MENU, [&](wxCommandEvent & evt) {
        frame->SendCommand("Disconnect all\n");
    }, DISCONNECT);
    frame->Bind(wxEVT_MENU, [&](wxCommandEvent & evt) {
        frame->SendCommand("Quit\n");
    }, wxID_EXIT);

    wxMenu *help_menu = new wxMenu;
    help_menu->Append(wxID_ABOUT, "&About", "About layout demo...");

    wxMenuBar *menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, "&File");
    menu_bar->Append(help_menu, "&Help");
    frame->SetMenuBar(menu_bar);

    // The status bar
    frame->CreateStatusBar(2);
    //    SetStatusText("Status Bar Section 0", 0);
    //    SetStatusText("Status Bar Section 1", 1);
}

// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void CreateNotebookPages(IC2Frame* frame) {
    // Create the notebook
    frame->notebook = new wxNotebook(frame, wxID_ANY);

    // Create all the pages
    frame->devices = new wxPanel(frame->notebook, wxID_ANY);
    frame->devInfo = new wxPanel(frame->notebook, wxID_ANY);
    frame->battery = new wxPanel(frame->notebook, wxID_ANY);
    frame->features = new wxPanel(frame->notebook, wxID_ANY);
    frame->measurement = new wxPanel(frame->notebook, wxID_ANY);
    frame->sensor_location = new wxPanel(frame->notebook, wxID_ANY);
    frame->control = new wxPanel(frame->notebook, wxID_ANY);
    frame->vector = new wxPanel(frame->notebook, wxID_ANY);
    frame->infoCrank_control = new wxPanel(frame->notebook, wxID_ANY);
    frame->infoCrank_raw = new wxPanel(frame->notebook, wxID_ANY);
    frame->crank_graphics = new CrankCanvas(frame->notebook, wxID_ANY); // special canvas
    frame->page12 = new wxPanel(frame->notebook, wxID_ANY);

    // Add pages to notebook
    frame->notebook->AddPage(frame->devices, "Devices");
    frame->notebook->AddPage(frame->devInfo, "Device Information");
    frame->notebook->AddPage(frame->battery, "Battery Information");
    frame->notebook->AddPage(frame->features, "Features");
    frame->notebook->AddPage(frame->measurement, "Measurement");
    frame->notebook->AddPage(frame->sensor_location, "Sensor Location");
    frame->notebook->AddPage(frame->control, "Control Point");
    frame->notebook->AddPage(frame->vector, "Vector");
    frame->notebook->AddPage(frame->infoCrank_control, "InfoCrank Control Point");
    frame->notebook->AddPage(frame->infoCrank_raw, "InfoCrank Raw Data");
    frame->notebook->AddPage(frame->crank_graphics, "Graphics");
    frame->notebook->AddPage(frame->page12, "Page 12");

    // Layout: Create a sizer for the frame to hold the notebook
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(frame->notebook, 1, wxEXPAND | wxALL, 5);

    frame->SetSizer(mainSizer);

}

void InitializeSizerFlags(
    wxSizerFlags& fieldFlags,
    wxSizerFlags& groupBoxFlags,
    wxSizerFlags& groupBoxInnerFlags,
    wxSizerFlags& rightFlags,
    wxSizerFlags& bottomRightFlags,
    wxSizerFlags& centreFlags,
    wxSizerFlags& gridFlags)
{
    fieldFlags.Border(wxLEFT | wxRIGHT, 10);
    groupBoxFlags = wxSizerFlags(0).Expand().Border(wxTOP | wxLEFT | wxRIGHT, 10);
    groupBoxInnerFlags = wxSizerFlags(0).Expand().Border(wxBOTTOM, 10);
    rightFlags = wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxRIGHT, 10);
    bottomRightFlags = wxSizerFlags().Align(wxALIGN_RIGHT | wxALIGN_BOTTOM).Border(wxBOTTOM | wxRIGHT, 10);
    centreFlags = wxSizerFlags().Align(wxALIGN_CENTRE | wxALIGN_BOTTOM).Border(wxALL, 5);
    gridFlags = wxSizerFlags().Expand().Border(wxALL, 0);
}


// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void SetupDevicePanels(IC2Frame* frame)
{
    // Devices panel
    frame->devicesSizer = new wxWrapSizer(wxHORIZONTAL);
    frame->devices->SetSizerAndFit(frame->devicesSizer);

    // Device information panel
    frame->manufacturerName = new wxStaticText(frame->devInfo, wxID_ANY, "-");
    frame->modelNumber = new wxStaticText(frame->devInfo, wxID_ANY, "-");
    frame->serialNumber = new wxStaticText(frame->devInfo, wxID_ANY, "-");
    frame->hardwareRevisionNumber = new wxStaticText(frame->devInfo, wxID_ANY, "-");
    frame->firmwareRevisionNumber = new wxStaticText(frame->devInfo, wxID_ANY, "-");
    frame->softwareRevisionNumber = new wxStaticText(frame->devInfo, wxID_ANY, "-");
    frame->systemID = new wxStaticText(frame->devInfo, wxID_ANY, "-");
    frame->PNPVendorIDSource = new wxStaticText(frame->devInfo, wxID_ANY, "-");
    frame->PNPVendorID = new wxStaticText(frame->devInfo, wxID_ANY, "-");
    frame->PNPProductID = new wxStaticText(frame->devInfo, wxID_ANY, "-");
    frame->PNPProductVersion = new wxStaticText(frame->devInfo, wxID_ANY, "-");

    // Refresh Button for Device Info
    frame->refreshDeviceInformation = new wxButton(frame->devInfo, wxID_ANY, "Refresh");



}



// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void SetupDeviceInfoPage(IC2Frame* frame, wxSizerFlags& fieldFlags, wxSizerFlags& bottomRightFlags)
{
    frame->refreshDeviceInformation->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Refresh device information\n");
    });

    wxFlexGridSizer* sizer = new wxFlexGridSizer(2, 0, 0);
    sizer->AddGrowableCol(1);
    sizer->AddGrowableRow(11);
    sizer->Add(new wxStaticText(frame->devInfo, wxID_ANY, "Manufacturer name"), fieldFlags);
    sizer->Add(frame->manufacturerName, fieldFlags);
    sizer->Add(new wxStaticText(frame->devInfo, wxID_ANY, "Model number"), fieldFlags);
    sizer->Add(frame->modelNumber, fieldFlags);
    sizer->Add(new wxStaticText(frame->devInfo, wxID_ANY, "Serial number"), fieldFlags);
    sizer->Add(frame->serialNumber, fieldFlags);
    sizer->Add(new wxStaticText(frame->devInfo, wxID_ANY, "Hardware revision number"), fieldFlags);
    sizer->Add(frame->hardwareRevisionNumber, fieldFlags);
    sizer->Add(new wxStaticText(frame->devInfo, wxID_ANY, "Firmware revision number"), fieldFlags);
    sizer->Add(frame->firmwareRevisionNumber, fieldFlags);
    sizer->Add(new wxStaticText(frame->devInfo, wxID_ANY, "Software revision"), fieldFlags);
    sizer->Add(frame->softwareRevisionNumber, fieldFlags);
    sizer->Add(new wxStaticText(frame->devInfo, wxID_ANY, "System ID"), fieldFlags);
    sizer->Add(frame->systemID, fieldFlags);
    sizer->Add(new wxStaticText(frame->devInfo, wxID_ANY, "PNP vendor ID source"), fieldFlags);
    sizer->Add(frame->PNPVendorIDSource, fieldFlags);
    sizer->Add(new wxStaticText(frame->devInfo, wxID_ANY, "PNP vendor ID"), fieldFlags);
    sizer->Add(frame->PNPVendorID, fieldFlags);
    sizer->Add(new wxStaticText(frame->devInfo, wxID_ANY, "PNP product ID"), fieldFlags);
    sizer->Add(frame->PNPProductID, fieldFlags);
    sizer->Add(new wxStaticText(frame->devInfo, wxID_ANY, "PNP product version"), fieldFlags);
    sizer->Add(frame->PNPProductVersion, fieldFlags);
    sizer->AddStretchSpacer();
    sizer->Add(frame->refreshDeviceInformation, bottomRightFlags);

    frame->devInfo->SetSizerAndFit(sizer);
}

// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
// --- Function: Setup Battery Page ---
void SetupBatteryPage(IC2Frame* frame, wxSizerFlags& fieldFlags, wxSizerFlags& groupBoxFlags, wxSizerFlags& groupBoxInnerFlags, wxSizerFlags& rightFlags)
{
    frame->batteryLevel = new wxStaticText(frame->battery, wxID_ANY, "-");
    frame->refreshBatteryInformation = new wxButton(frame->battery, wxID_ANY, "Refresh");
    frame->loggingBattery = new wxCheckBox(frame->battery, wxID_ANY, "/dev/null");
    frame->logFileBattery = new wxButton(frame->battery, wxID_ANY, "...", wxDefaultPosition, wxSize(50, 20));

    // Bind
    frame->refreshBatteryInformation->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Refresh battery information\n");
    });

    frame->logFileBattery->Bind(wxEVT_BUTTON, &IC2Frame::LogFileName, frame, wxID_ANY, wxID_ANY, new IC2Frame::FileDialogParameters("battery.log", frame->loggingBattery));
    frame->loggingBattery->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent & evt) {
        wxCheckBox* checkBox = (wxCheckBox*) evt.GetEventObject();
        if (checkBox->IsChecked()) {
            frame->logBattery.Open(checkBox->GetLabel(), wxFile::write);
        } else {
            frame->logBattery.Close();
        }
    });

    // Layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    {
        wxFlexGridSizer* flexGridSizer = new wxFlexGridSizer(2, 0, 0);
        flexGridSizer->AddGrowableCol(1);

        flexGridSizer->Add(new wxStaticText(frame->battery, wxID_ANY, "Battery level"), fieldFlags);
        flexGridSizer->Add(frame->batteryLevel, fieldFlags);

        for (int i = 0; i < 10; ++i) {
            flexGridSizer->Add(new wxStaticText(frame->battery, wxID_ANY, "Info Line"), fieldFlags);
            flexGridSizer->Add(new wxStaticText(frame->battery, wxID_ANY, "-"), fieldFlags);
        }

        sizer->Add(flexGridSizer, groupBoxFlags);
    }

    sizer->AddStretchSpacer();

    {
        wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
        boxSizer->Add(frame->loggingBattery, fieldFlags);
        boxSizer->Add(frame->logFileBattery, fieldFlags);
        boxSizer->AddStretchSpacer();
        boxSizer->Add(frame->refreshBatteryInformation, rightFlags);
        sizer->Add(boxSizer, groupBoxInnerFlags);
    }

    frame->battery->SetSizerAndFit(sizer);
}

void SetupFeaturesPage(IC2Frame* frame, wxSizerFlags& fieldFlags, wxSizerFlags& bottomRightFlags)
{
    frame->balanceFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 0");
    frame->torqueFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 1");
    frame->wheelRevolutionFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 2");
    frame->crankRevolutionFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 3");
    frame->magnitudesFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 4");
    frame->anglesFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 5");
    frame->deadSpotsFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 6");
    frame->energyFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 7");
    frame->compensationIndicatorFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 8");
    frame->compensationFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 9");
    frame->maskingFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 10");
    frame->locationsFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 11");
    frame->crankLengthFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 12");
    frame->chainLengthFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 13");
    frame->chainWeightFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 14");
    frame->spanFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 15");
    frame->contextFeature = new wxStaticText(frame->features, wxID_ANY, "Bit 16");
    frame->directionFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 17");
    frame->dateFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 18");
    frame->enhancedCompensationFeature = new wxCheckBox(frame->features, wxID_ANY, "Bit 19");
    frame->distributedFeature = new wxStaticText(frame->features, wxID_ANY, "Bit 20-21");

    frame->refreshFeatures = new wxButton(frame->features, wxID_ANY, "Refresh");

    frame->refreshFeatures->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        frame->SendCommand("Refresh features\n");
    });

    wxFlexGridSizer* sizer = new wxFlexGridSizer(1, 0, 0); // only ONE column needed now!
    sizer->AddGrowableCol(0);

    // Just add the control directly
    sizer->Add(frame->balanceFeature, fieldFlags);
    sizer->Add(frame->torqueFeature, fieldFlags);
    sizer->Add(frame->wheelRevolutionFeature, fieldFlags);
    sizer->Add(frame->crankRevolutionFeature, fieldFlags);
    sizer->Add(frame->magnitudesFeature, fieldFlags);
    sizer->Add(frame->anglesFeature, fieldFlags);
    sizer->Add(frame->deadSpotsFeature, fieldFlags);
    sizer->Add(frame->energyFeature, fieldFlags);
    sizer->Add(frame->compensationIndicatorFeature, fieldFlags);
    sizer->Add(frame->compensationFeature, fieldFlags);
    sizer->Add(frame->maskingFeature, fieldFlags);
    sizer->Add(frame->locationsFeature, fieldFlags);
    sizer->Add(frame->crankLengthFeature, fieldFlags);
    sizer->Add(frame->chainLengthFeature, fieldFlags);
    sizer->Add(frame->chainWeightFeature, fieldFlags);
    sizer->Add(frame->spanFeature, fieldFlags);
    sizer->Add(frame->contextFeature, fieldFlags);
    sizer->Add(frame->directionFeature, fieldFlags);
    sizer->Add(frame->dateFeature, fieldFlags);
    sizer->Add(frame->enhancedCompensationFeature, fieldFlags);
    sizer->Add(frame->distributedFeature, fieldFlags);

    sizer->AddStretchSpacer();
    sizer->Add(frame->refreshFeatures, bottomRightFlags);

    frame->features->SetSizerAndFit(sizer);
}

void setupFunction(IC2Frame* frame, wxSizerFlags& fieldFlags, wxSizerFlags& bottomRightFlags) {

    // Bind the controls
    frame->refreshFeatures->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Refresh features\n");
    });

    {
        wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 0, 0);
        sizer->AddGrowableCol(1);
        sizer->AddGrowableRow(20);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Pedal power balance supported"), fieldFlags);
        sizer->Add(frame->balanceFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Accumulated torque supported"), fieldFlags);
        sizer->Add(frame->torqueFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Wheel revolution data supported"), fieldFlags);
        sizer->Add(frame->wheelRevolutionFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Crank revolution data supported"), fieldFlags);
        sizer->Add(frame->crankRevolutionFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Extreme magnitudes supported"), fieldFlags);
        sizer->Add(frame->magnitudesFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Extreme angles supported"), fieldFlags);
        sizer->Add(frame->anglesFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Top and bottom dead spot angles supported"), fieldFlags);
        sizer->Add(frame->deadSpotsFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Accumulated energy supported"), fieldFlags);
        sizer->Add(frame->energyFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Offset compensation indicator supported"), fieldFlags);
        sizer->Add(frame->compensationIndicatorFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Offset compensation supported"), fieldFlags);
        sizer->Add(frame->compensationFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Cycling power measurement characteristic content masking supported"), fieldFlags);
        sizer->Add(frame->maskingFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Multiple sensor locations supported"), fieldFlags);
        sizer->Add(frame->locationsFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Crank length adjustment supported"), fieldFlags);
        sizer->Add(frame->crankLengthFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Chain length adjustment supported"), fieldFlags);
        sizer->Add(frame->chainLengthFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Chain weight adjustment supported"), fieldFlags);
        sizer->Add(frame->chainWeightFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Span length adjustment supported"), fieldFlags);
        sizer->Add(frame->spanFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Sensor measurement context"), fieldFlags);
        sizer->Add(frame->contextFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Instantaneous measurement direction supported"), fieldFlags);
        sizer->Add(frame->directionFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Factory calibration date supported"), fieldFlags);
        sizer->Add(frame->dateFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Enhanced offset compensation procedure supported"), fieldFlags);
        sizer->Add(frame->enhancedCompensationFeature, fieldFlags);
        sizer->Add(new wxStaticText(frame->features, wxID_ANY, "Distributed system support"), fieldFlags);
        sizer->Add(frame->distributedFeature, fieldFlags);
        sizer->AddStretchSpacer();
        sizer->Add(frame->refreshFeatures, bottomRightFlags);
        frame->features->SetSizerAndFit(sizer);
    }
}




// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void SetupMeasurementPage(IC2Frame* frame) {
    // Cycling power measurement
    frame->pedalPowerBalancePresent = new wxCheckBox(frame->measurement, wxID_ANY, "Balance: Unknown");
    //    pedalPowerBalancePresent->Enable(false);
    frame->accumulatedTorquePresent = new wxCheckBox(frame->measurement, wxID_ANY, "Accumulated torque: Wheel based");
    //    accumulatedTorquePresent->Enable(false);
    frame->wheelRevolutionDataPresent = new wxCheckBox(frame->measurement, wxID_ANY, "Wheel revolution data");
    //    wheelRevolutionDataPresent->Enable(false);
    frame->crankRevolutionDataPresent = new wxCheckBox(frame->measurement, wxID_ANY, "Crank revolution data");
    //    crankRevolutionDataPresent->Enable(false);
    frame->extremeForceMagnitudesPresent = new wxCheckBox(frame->measurement, wxID_ANY, "Extreme force magnitudes");
    //    extremeForceMagnitudesPresent->Enable(false);
    frame->extremeTorqueMagnitudesPresent = new wxCheckBox(frame->measurement, wxID_ANY, "Extreme torque magnitudes");
    //    extremeTorqueMagnitudesPresent->Enable(false);
    frame->extremeAnglesPresent = new wxCheckBox(frame->measurement, wxID_ANY, "Extreme angles");
    //    extremeAnglesPresent->Enable(false);
    frame->topDeadSpotAnglePresent = new wxCheckBox(frame->measurement, wxID_ANY, "Top dead spot angle");
    //    topDeadSpotAnglePresent->Enable(false);
    frame->bottomDeadSpotAnglePresent = new wxCheckBox(frame->measurement, wxID_ANY, "Bottom dead spot angle");
    //    bottomDeadSpotAnglePresent->Enable(false);
    frame->accumulatedEnergyPresent = new wxCheckBox(frame->measurement, wxID_ANY, "Accumulated energy");
    //    accumulatedEnergyPresent->Enable(false);
    frame->offsetCompensationIndicator = new wxCheckBox(frame->measurement, wxID_ANY, "Offset compensation indicator");
    //    offsetCompensationIndicator->Enable(false);
    frame->instantaneousPower = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->pedalPowerBalance = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->accumulatedTorque = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->torquePanel = new wxPanel(frame->measurement);
    frame->torquePanel->SetBackgroundColour(wxColour(wxT("YELLOW")));
    frame->torque = new wxStaticText(frame->torquePanel, wxID_ANY, "-");
    frame->torque->SetForegroundColour(wxColour(wxT("RED")));
    frame->cumulativeWheelRevolutions = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->lastWheelEventTime = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->wheelSpeedPanel = new wxPanel(frame->measurement);
    frame->wheelSpeedPanel->SetBackgroundColour(wxColour(wxT("YELLOW")));
    frame->wheelSpeed = new wxStaticText(frame->wheelSpeedPanel, wxID_ANY, "-");
    frame->wheelSpeed->SetForegroundColour(wxColour(wxT("RED")));
    frame->cumulativeCrankRevolutions = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->lastCrankEventTime = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->cadencePanel = new wxPanel(frame->measurement);
    frame->cadencePanel->SetBackgroundColour(wxColour(wxT("YELLOW")));
    frame->cadence = new wxStaticText(frame->cadencePanel, wxID_ANY, "-");
    frame->cadence->SetForegroundColour(wxColour(wxT("RED")));
    frame->maximumForceMagnitude = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->minimumForceMagnitude = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->maximumTorqueMagnitude = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->minimumTorqueMagnitude = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->maximumAngle = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->minimumAngle = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->topDeadSpotAngle = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->bottomDeadSpotAngle = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->accumulatedEnergy = new wxStaticText(frame->measurement, wxID_ANY, "-");
    frame->notifyMeasurement = new wxToggleButton(frame->measurement, wxID_ANY, "Notify");
    frame->broadcastMeasurement = new wxToggleButton(frame->measurement, wxID_ANY, "Broadcast");
    frame->loggingMeasurement = new wxCheckBox(frame->measurement, wxID_ANY, "/dev/null");
    frame->logFileMeasurement = new wxButton(frame->measurement, wxID_ANY, "...", wxDefaultPosition, wxSize(50, 20));
}

void bindControls(IC2Frame* frame) {
    // Bind the controls
    frame->notifyMeasurement->Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent & evt) {
        switch (evt.GetInt()) {
            case TRUE:
                ((wxToggleButton *) evt.GetEventObject())->SetLabel("Stop");
                frame->SendCommand("Notify measurement on\n");
                break;
            case FALSE:
                ((wxToggleButton *) evt.GetEventObject())->SetLabel("Notify");
                frame->SendCommand("Notify measurement off\n");
                break;
        }
    });
    frame->broadcastMeasurement->Bind(wxEVT_TOGGLEBUTTON, [&](wxCommandEvent & evt) {
        switch (evt.GetInt()) {
            case TRUE:
                ((wxToggleButton *) evt.GetEventObject())->SetLabel("Stop");
                frame->SendCommand("Broadcast measurement on\n");
                break;
            case FALSE:
                ((wxToggleButton *) evt.GetEventObject())->SetLabel("Broadcast");
                frame->SendCommand("Broadcast measurement off\n");
                break;
        }
    });
    frame->logFileMeasurement->Bind(wxEVT_BUTTON, &IC2Frame::LogFileName, frame, wxID_ANY, wxID_ANY, new IC2Frame::FileDialogParameters("measurement.log", frame->loggingMeasurement));
    frame->loggingMeasurement->Bind(wxEVT_CHECKBOX,
                             [&](wxCommandEvent & evt) {
                                 wxCheckBox *checkBox = (wxCheckBox *) evt.GetEventObject();
                                 if (checkBox->IsChecked()) {
                                     //            logMeasurement.Create(checkBox->GetLabel(), true);
                                     frame->logMeasurement.Open(checkBox->GetLabel(), wxFile::write);
                                 } else {
                                     frame->logMeasurement.Close();
                                 }
                             });

}

void layoutPage(IC2Frame* frame, wxSizerFlags& fieldFlags, wxSizerFlags& groupBoxFlags, wxSizerFlags& groupBoxInnerFlags, wxSizerFlags& gridFlags, wxSizerFlags& rightFlags) {
    // Layout the page
    {
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

        {
            wxStaticBoxSizer  *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, frame->measurement, "Flags");
            {
                wxWrapSizer *wrapSizer = new wxWrapSizer(wxVERTICAL);
                wrapSizer->Add(frame->pedalPowerBalancePresent, fieldFlags);
                wrapSizer->Add(frame->accumulatedTorquePresent, fieldFlags);
                wrapSizer->Add(frame->wheelRevolutionDataPresent, fieldFlags);
                wrapSizer->Add(frame->crankRevolutionDataPresent, fieldFlags);
                wrapSizer->Add(frame->extremeForceMagnitudesPresent, fieldFlags);
                wrapSizer->Add(frame->extremeTorqueMagnitudesPresent, fieldFlags);
                wrapSizer->Add(frame->extremeAnglesPresent, fieldFlags);
                wrapSizer->Add(frame->topDeadSpotAnglePresent, fieldFlags);
                wrapSizer->Add(frame->bottomDeadSpotAnglePresent, fieldFlags);
                wrapSizer->Add(frame->accumulatedEnergyPresent, fieldFlags);
                wrapSizer->Add(frame->offsetCompensationIndicator, fieldFlags);
                staticBoxSizer->Add(wrapSizer, groupBoxInnerFlags);
            }
            sizer->Add(staticBoxSizer, groupBoxFlags);
        }

        {
            wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
            flexGridSizer->AddGrowableCol(0);
            flexGridSizer->AddGrowableCol(1);

            {
                wxStaticBoxSizer  *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, frame->measurement, "Instantaneous power");
                staticBoxSizer->Add(frame->instantaneousPower, fieldFlags);
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer  *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, frame->measurement, "Pedal power balance");
                staticBoxSizer->Add(frame->pedalPowerBalance, fieldFlags);
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer  *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, frame->measurement, "Accumulated torque");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Accumulated torque"), fieldFlags);
                    flexGridSizer->Add(frame->accumulatedTorque, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Instantaneous torque"), fieldFlags);
                    flexGridSizer->Add(frame->torquePanel, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer  *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, frame->measurement, "Accumulated energy");
                staticBoxSizer->Add(frame->accumulatedEnergy, fieldFlags);
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, frame->measurement, "Wheel revolution data");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Cumulative wheel revolutions"), fieldFlags);
                    flexGridSizer->Add(frame->cumulativeWheelRevolutions, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Last wheel event time"), fieldFlags);
                    flexGridSizer->Add(frame->lastWheelEventTime, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Wheel speed"), fieldFlags);
                    flexGridSizer->Add(frame->wheelSpeedPanel, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, frame->measurement, "Crank revolution data");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Cumulative crank revolutions"), fieldFlags);
                    flexGridSizer->Add(frame->cumulativeCrankRevolutions, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Last crank event time"), fieldFlags);
                    flexGridSizer->Add(frame->lastCrankEventTime, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Cadence"), fieldFlags);
                    //crankRevolutionSizerInnerSizer->Add(cadence, fieldFlags);
                    flexGridSizer->Add(frame->cadencePanel, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, frame->measurement, "Extreme force magnitudes");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Maximum force magnitude"), fieldFlags);
                    flexGridSizer->Add(frame->maximumForceMagnitude, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Minimum force magnitude"), fieldFlags);
                    flexGridSizer->Add(frame->minimumForceMagnitude, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, frame->measurement, "Extreme torque magnitudes");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Maximum torque magnitude"), fieldFlags);
                    flexGridSizer->Add(frame->maximumTorqueMagnitude, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Minimum torque magnitude"), fieldFlags);
                    flexGridSizer->Add(frame->minimumTorqueMagnitude, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, frame->measurement, "Extreme angles");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Maximum angle"), fieldFlags);
                    flexGridSizer->Add(frame->maximumAngle, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Minimum angle"), fieldFlags);
                    flexGridSizer->Add(frame->minimumAngle, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            {
                wxStaticBoxSizer *staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, frame->measurement, "Dead spot angles");
                {
                    wxFlexGridSizer *flexGridSizer = new wxFlexGridSizer(2, 0, 0);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Top dead spot angle"), fieldFlags);
                    flexGridSizer->Add(frame->topDeadSpotAngle, fieldFlags);
                    flexGridSizer->Add(new wxStaticText(frame->measurement, wxID_ANY, "Bottom dead spot angle"), fieldFlags);
                    flexGridSizer->Add(frame->bottomDeadSpotAngle, fieldFlags);
                    staticBoxSizer->Add(flexGridSizer, groupBoxInnerFlags);
                }
                flexGridSizer->Add(staticBoxSizer, groupBoxFlags);
            }

            sizer->Add(flexGridSizer, gridFlags);
        }

        sizer->AddStretchSpacer();

        {
            wxBoxSizer *boxSizer = new wxBoxSizer(wxHORIZONTAL);
            boxSizer->Add(frame->loggingMeasurement, fieldFlags);
            boxSizer->Add(frame->logFileMeasurement, fieldFlags);
            boxSizer->AddStretchSpacer();
            boxSizer->Add(frame->notifyMeasurement, fieldFlags);
            boxSizer->Add(frame->broadcastMeasurement, rightFlags);
            sizer->Add(boxSizer, groupBoxInnerFlags);
        }

        frame->measurement->SetSizerAndFit(sizer);
        frame->measurement->PostSizeEvent();        // wxWrapSizer needs a resize to layout correctly
    }
}



// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void SetupSensorLocationPage(IC2Frame* frame, wxSizerFlags& fieldFlags) {
    // Sensor location page
    wxButton *requestSensorLocation = new wxButton(frame->sensor_location, wxID_ANY, "Get sensor location");
    frame->sensorLocation = new wxStaticText(frame->sensor_location, wxID_ANY, "-");

    // Bind the controls
    requestSensorLocation->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Get sensor location\n");
    });

    // Layout the page
    {
        wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 0, 0);
        sizer->Add(requestSensorLocation, fieldFlags);
        sizer->Add(frame->sensorLocation, fieldFlags);

        frame->sensor_location->SetSizerAndFit(sizer);
    }

}


// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void SetupControlPointPage(IC2Frame* frame) {

    // Cycling power control point
    frame->cumulative = new wxTextCtrl(frame->control, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
    frame->supportedLocations = new wxButton(frame->control, wxID_ANY, "Get supported sensor locations");
    frame->location = new wxComboBox(frame->control, wxID_ANY);
    frame->crankLength = new wxTextCtrl(frame->control, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC));
    frame->requestCrankLength = new wxButton(frame->control, wxID_ANY, "Get crank length");
    frame->chainLength = new wxTextCtrl(frame->control, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
    frame->requestChainLength = new wxButton(frame->control, wxID_ANY, "Get chain length");
    frame->chainWeight = new wxTextCtrl(frame->control, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
    frame->requestChainWeight = new wxButton(frame->control, wxID_ANY, "Get chain weight");
    frame->span = new wxTextCtrl(frame->control, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
    frame->requestSpan = new wxButton(frame->control, wxID_ANY, "Get span");
    frame->offsetCompensation = new wxButton(frame->control, wxID_ANY, "Start offset compensation");
    frame->offsetCompensationValue = new wxStaticText(frame->control, wxID_ANY, "-");
    frame->maskMeasurement = new wxButton(frame->control, wxID_ANY, "Mask cycling power measurement characteristic content");
    frame->pedalPowerBalanceMask   = new wxCheckBox(frame->control, wxID_ANY, "Balance");
    frame->accumulatedTorqueMask   = new wxCheckBox(frame->control, wxID_ANY, "Accumulated torque");
    frame->wheelRevolutionDataMask = new wxCheckBox(frame->control, wxID_ANY, "Wheel revolution");
    frame->crankRevolutionDataMask = new wxCheckBox(frame->control, wxID_ANY, "Crank revolution");
    frame->extremeMagnitudesMask   = new wxCheckBox(frame->control, wxID_ANY, "Extreme magnitudes");
    frame->extremeAnglesMask       = new wxCheckBox(frame->control, wxID_ANY, "Extreme angles");
    frame->topDeadSpotAngleMask    = new wxCheckBox(frame->control, wxID_ANY, "Top dead spot angle");
    frame->bottomDeadSpotAngleMask = new wxCheckBox(frame->control, wxID_ANY, "Bottom dead spot angle");
    frame->accumulatedEnergyMask   = new wxCheckBox(frame->control, wxID_ANY, "Accumulated energy");
    frame->samplingRate = new wxStaticText(frame->control, wxID_ANY, "-");
    frame->requestSamplingRate = new wxButton(frame->control, wxID_ANY, "Get sampling rate");
    frame->calibrationDate = new wxStaticText(frame->control, wxID_ANY, "-");
    frame->requestCalibrationDate = new wxButton(frame->control, wxID_ANY, "Get factory calibration date");
    frame->enhancedOffsetCompensation = new wxButton(frame->control, wxID_ANY, "Start enhanced offset compensation");
    frame->enhancedOffsetCompensationValue = new wxStaticText(frame->control, wxID_ANY, "-");

}

void SetupBindControls(IC2Frame* frame) {

    // Bind the controls
    frame->cumulative->Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent & evt) {
        frame->SendCommand(wxString().Format("Set cumulative value %s\n", frame->cumulative->GetValue()));
    });
    frame->supportedLocations->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Get supported sensor locations\n");
    });
    frame->location->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &IC2Frame::SetSensorLocation, frame);
    frame->requestCrankLength->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Get crank length\n");
    });
    frame->crankLength->Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent & evt) {
        frame->SendCommand(wxString().Format("Set crank length %s\n", frame->crankLength->GetValue()));
    });
    frame->requestChainLength->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Get chain length\n");
    });
    frame->chainLength->Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent & evt) {
        frame->SendCommand(wxString().Format("Set chain length %s\n", frame->chainLength->GetValue()));
    });
    frame->requestChainWeight->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Get chain weight\n");
    });
    frame->chainWeight->Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent & evt) {
        frame->SendCommand(wxString().Format("Set chain weight %s\n", frame->chainWeight->GetValue()));
    });
    frame->requestSpan->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Get span\n");
    });
    frame->span->Bind(wxEVT_TEXT_ENTER, [&](wxCommandEvent & evt) {
        frame->SendCommand(wxString().Format("Set span %s\n", frame->span->GetValue()));
    });
    frame->offsetCompensation->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Start offset compensation\n");
    });
    frame->maskMeasurement->Bind(wxEVT_BUTTON, &IC2Frame::MaskMeasurement, frame);

    frame->requestSamplingRate->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Get sampling rate\n");
    });
    frame->requestCalibrationDate->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Get factory calibration date\n");
    });
    frame->enhancedOffsetCompensation->Bind(wxEVT_BUTTON, [&](wxCommandEvent & evt) {
        frame->SendCommand("Start enhanced offset compensation\n");
    });
}


// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void SetupVectorPage(IC2Frame* frame) {

}


// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void SetupInfoCrankControlPage(IC2Frame* frame) {

}


// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void SetupInfoCrankRawPage(IC2Frame* frame) {

}


// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void SetupCountdownTimer(IC2Frame* frame) {

}

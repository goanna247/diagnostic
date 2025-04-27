
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




// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void SetupMeasurementPage(IC2Frame* frame) {

}


// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void SetupSensorLocationPage(IC2Frame* frame) {

}


// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------
void SetupControlPointPage(IC2Frame* frame) {

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

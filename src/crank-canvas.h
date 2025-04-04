#pragma once
#include <memory>
#include <chrono>

#include "wx/timer.h"
#include "wx/glcanvas.h"

class CrankCanvas : public wxGLCanvas
{
public:
    CrankCanvas(wxWindow *parent, wxWindowID id = wxID_ANY,
                const int *attribList = NULL, const wxPoint &pos = wxDefaultPosition,
                const wxSize &size = wxDefaultSize, long style = 0L,
                const wxString &name = L"GLCanvas", const wxPalette &palette = wxNullPalette);

    virtual ~CrankCanvas();
    float angle = 0.0f;
    bool newAngle = true;

private:
    void OnPaint(wxPaintEvent &event);
//	void Paint();
    void OnIdle(wxIdleEvent &event);
    wxGLContext *m_context;
};

//class CrankTimer : public wxTimer
//{
//private:
//    CrankCanvas *m_canvas;
//
//public:
//    CrankTimer(CrankCanvas *canvas);
//
//    void Notify();
//
//};

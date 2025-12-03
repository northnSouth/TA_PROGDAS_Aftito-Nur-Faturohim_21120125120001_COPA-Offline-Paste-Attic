#include <wx/wx.h>
#include "mainframe.hpp"

class MainApp : public wxApp
{
public:
    virtual bool OnInit() override
    {
        MainFrame *frame = new MainFrame(nullptr);
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MainApp);

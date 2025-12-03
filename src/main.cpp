#include <wx/wx.h>
#include "../include/core.hpp"

class CopaApp : public wxApp
{
public:
    virtual bool OnInit() override
    {
        MainFrame *frame = new MainFrame(nullptr);    
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(CopaApp);

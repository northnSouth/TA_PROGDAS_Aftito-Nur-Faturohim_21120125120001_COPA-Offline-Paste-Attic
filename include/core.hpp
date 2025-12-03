#pragma once

#include <wx/scrolwin.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/timer.h>
#include <map>
#include <string>

class MainFrame : public wxFrame
{
public:
    MainFrame(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "COPA",
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(600, 800),
        long style = wxDEFAULT_FRAME_STYLE, const wxString &name = wxFrameNameStr);

    bool Create(wxWindow *parent, wxWindowID id, const wxString&,
        const wxPoint& pos, const wxSize& size,
        long style, const wxString &name);

private:
    // --- UI Elements ---
    wxPanel*          m_live_clip_page = nullptr; ///< Live page containing panel.
    wxBoxSizer*       m_page_sizer_live = nullptr; ///< Live page sizer.
    wxScrolledWindow* m_scroll_panel = nullptr; ///< Live page scroll panel.
    wxBoxSizer*       m_scroller_panel_sizer = nullptr; ///< Live page scroll panel sizer (clipboard box container).
    
    // --- Logic Variables ---
    wxTimer timer;
    std::string last_clipboard_text = "";
    std::map<std::string, wxBoxSizer*> clip_box_map; ///< Stores clipboard text and panel pair.
    bool empty_message_shown = false;
    const int MAX_CLIP_BOX_HEIGHT = 300; ///< Maximum clipboard box height.

    // Text control height from line helper.
    int CalculateTextHeight(wxTextCtrl* textCtrl);
    // Live page scroll panel create helper.
    void CreateLiveScrollPanel();
    // Live page clipboard empty message.
    void CreateLiveEmptyClipBoxMessage();
    
    // Live page clipboard box creator.
    void CreateLiveClipboardBox(std::string text);
    // Timer event callback, clipboard update trigger.
    void OnClipboardCheck(wxTimerEvent& event);
    // Live page ,ove, create, update clipboard.
    void UpdateLiveClipboardBox(wxString& text);
};
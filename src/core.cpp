#include "../include/core.hpp"
#include "../include/logo_embed.hpp"

#include "wx/clipbrd.h"
#include "wx/event.h"
#include "wx/gdicmn.h"
#include <wx/mstream.h>
#include <wx/generic/statbmpg.h>
#include <wx/notebook.h>
#include <wx/stattext.h>
#include <wx/button.h>

// --- IMAGE CONVERSION -----------------------------------------------------------
#ifdef __cpp_inline_variables
// Convert a data array into a wxImage
inline wxImage wxueImage(const unsigned char* data, size_t size_data)
#else
// Convert a data array into a wxImage
static wxImage wxueImage(const unsigned char* data, size_t size_data)
#endif
{
    wxMemoryInputStream strm(data, size_data);
    wxImage image;
    image.LoadFile(strm);
    return image;
};

namespace wxue_img
{
    extern const unsigned char COPA_png[53665];
}

// --- CONSTRUCTOR ----------------------------------------------------------------
MainFrame::MainFrame(wxWindow* parent, wxWindowID id, const wxString& title,
        const wxPoint& pos, const wxSize& size,
        long style, const wxString &name)
    : timer(this)
{
    Create(parent, id, title, pos, size, style, name);
}

// --- MAIN WINDOW ----------------------------------------------------------------
bool MainFrame::Create(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size, long style, const wxString &name)
{
    if (!wxImage::FindHandler(wxBITMAP_TYPE_PNG))
        wxImage::AddHandler(new wxPNGHandler);

    if (!wxFrame::Create(parent, id, title, pos, size, style, name))
        return false;

    wxNotebook* m_notebook = new wxNotebook(
        this, wxID_ANY, 
        wxDefaultPosition, 
        wxDefaultSize,
        wxBK_LEFT);
    
    // --- LIVE PAGE --------------------------------------------------------------
    m_live_clip_page = new wxPanel(
        m_notebook, wxID_ANY, 
        wxDefaultPosition, wxDefaultSize, 
        wxTAB_TRAVERSAL);
    m_notebook->AddPage(m_live_clip_page, " Live ");

    m_page_sizer_live = new wxBoxSizer(wxVERTICAL);
    m_live_clip_page->SetSizerAndFit(m_page_sizer_live);

    // --- ABOUT PAGE -------------------------------------------------------------
    auto* about = new wxPanel(
        m_notebook, wxID_ANY, 
        wxDefaultPosition, wxDefaultSize, 
        wxTAB_TRAVERSAL);
    m_notebook->AddPage(about, " About ");

    auto* page_sizer_live_box = new wxBoxSizer(wxVERTICAL);

    // Add logo image
    auto* bmp = new wxGenericStaticBitmap(
        about, wxID_ANY, 
        wxBitmapBundle::FromBitmap(
            wxueImage(
                wxue_img::COPA_png, 
                sizeof(wxue_img::COPA_png)
            )
        )
    );
    bmp->SetScaleMode(wxStaticBitmap::Scale_AspectFit);

    // Append logo
    page_sizer_live_box->Add(
        bmp, 
        wxSizerFlags()
            .Center()
            .Border(
                wxALL, 
                FromDIP(wxSize(20, -1)).x)
    );

    // Add text
    wxStaticText* about_text = new wxStaticText(about, wxID_ANY,
        "COPA is abbreviaton of COPA Offline Paste Attic. "
        "This GUI app watches changes in your clipboard"
        " and logs them (on the Live tab) temporarily until the app is closed,"
        " or crash. Enjoy :3",
        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    about_text->Wrap(500);

    // Append text to page
    page_sizer_live_box->Add(
        about_text, 
        wxSizerFlags(1)
            .Center().Border(wxALL)
    );
    about->SetSizerAndFit(page_sizer_live_box);

    // Center the window relative to the screen
    Centre(wxBOTH);
    Bind(wxEVT_TIMER, &MainFrame::OnClipboardCheck, this);
    timer.Start(25);

    return true;
}

// --- MEMBER FUNCTION IMPLEMENTATIONS --------------------------------------------
void MainFrame::OnClipboardCheck(wxTimerEvent& event)
{
    if (!wxTheClipboard->Open()) {
        return;
    }

    if (wxTheClipboard->IsSupported(wxDF_TEXT))
    {
        wxTextDataObject data;
        wxTheClipboard->GetData(data);
        wxString clipText = data.GetText();

        if (!clipText.IsEmpty() && clipText.ToStdString() != last_clipboard_text)
        {
            last_clipboard_text = clipText.ToStdString();

            if (m_scroll_panel == nullptr) {
                m_page_sizer_live->Clear(true); 
                CreateLiveScrollPanel();
            }

            UpdateLiveClipboardBox(clipText);
            empty_message_shown = false;

        } else if (clip_box_map.empty() && !empty_message_shown) {
            if (m_scroll_panel != nullptr) {
                m_scroll_panel->Destroy();
                m_scroll_panel = nullptr;
            }

            CreateLiveEmptyClipBoxMessage();
            m_page_sizer_live->Layout();
            empty_message_shown = true;

        }
    }

    wxTheClipboard->Close();
}

void MainFrame::UpdateLiveClipboardBox(wxString& text) 
{
    auto it = clip_box_map.begin();
    if (!clip_box_map.empty()) {
        while (it != clip_box_map.end() && it->first != text.ToStdString()) ++it;
    }

    if (it != clip_box_map.end())
    {
        m_scroller_panel_sizer->Detach(it->second);
        m_scroller_panel_sizer->Insert(
            1, it->second, 0,
            wxEXPAND | wxALL, 5
        );
    } else { 
        CreateLiveClipboardBox(text.ToStdString());
    }

    m_scroll_panel->Layout();
    m_scroll_panel->FitInside();
    m_scroll_panel->Scroll(0, 0);
    m_live_clip_page->Layout(); 
}

int MainFrame::CalculateTextHeight(wxTextCtrl* textCtrl)
{
    int lineHeight = textCtrl->GetCharHeight();
    int numLines = textCtrl->GetNumberOfLines();
    int height = lineHeight * numLines + 8; // small padding
    return std::min(height, MAX_CLIP_BOX_HEIGHT);
}

void MainFrame::CreateLiveScrollPanel() 
{
    m_scroll_panel = new wxScrolledWindow(
        m_live_clip_page, wxID_ANY, 
        wxDefaultPosition, wxDefaultSize,
        wxHSCROLL | wxVSCROLL);
    m_scroll_panel->SetScrollRate(10, 10);
    m_scroll_panel->SetMinSize(wxSize(500, 800));
    
    m_page_sizer_live->Add(
        m_scroll_panel, 
        wxSizerFlags().Expand().Border(wxALL).Proportion(1)
    );

    m_scroller_panel_sizer = new wxBoxSizer(wxVERTICAL);
    m_scroll_panel->SetSizer(m_scroller_panel_sizer);

    auto* live_btn_clear = new wxButton(
        m_scroll_panel, 
        wxID_ANY, 
        "Clear All"
    );
    m_scroller_panel_sizer->Insert(
        0, live_btn_clear, 
        wxSizerFlags().Expand().Border(wxALL)
    );

    live_btn_clear->Bind(wxEVT_BUTTON, 
        [this](wxCommandEvent&) mutable
    {
        clip_box_map.clear();
    });
};

void MainFrame::CreateLiveEmptyClipBoxMessage()
{
    m_page_sizer_live->AddStretchSpacer(1);
    
    auto* empty_text = new wxStaticText(
        m_live_clip_page, wxID_ANY,
        "Copy something to fill the history."
    );
    m_page_sizer_live->Add(
        empty_text,
        wxSizerFlags().Center().Border(wxALL)
    );
    
    m_page_sizer_live->AddStretchSpacer(1);
}

void MainFrame::CreateLiveClipboardBox(std::string text) 
{
    // --- CLIPBOARD BOX ------------------------------------------------------
    auto* box_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto* live_box_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto* live_clipboard = new wxTextCtrl(
        m_scroll_panel, wxID_ANY, 
        text, wxDefaultPosition, 
        wxSize(450, 50),wxTE_MULTILINE);
    live_clipboard->SetMinSize(
        wxSize(-1, CalculateTextHeight(live_clipboard))
    );
    
    // Timestamp text
    wxDateTime now = wxDateTime::Now();
    wxString timestamp = now.FormatISOCombined(' ');
    
    auto* ts_label = new wxStaticText(
        m_scroll_panel,
        wxID_ANY,
        "Copied: " + timestamp
    );
    
    // Stack textbox + timestamp vertically
    auto* text_with_timestamp = new wxBoxSizer(wxVERTICAL);
    text_with_timestamp->Add(
        live_clipboard,
        wxSizerFlags().Expand().Border(wxBOTTOM, 2)
    );
    text_with_timestamp->Add(
        ts_label,
        wxSizerFlags().Left().Border(wxTOP, 0)
    );

    live_box_sizer->Add(
        text_with_timestamp,
        wxSizerFlags(1).Expand().Border(wxALL, 0)
    );
    
    // --- CLIPBOARD BUTTONS --------------------------------------------------
    auto* live_btn_sizer = new wxBoxSizer(wxVERTICAL);
    
    auto* live_copy = new wxButton(
        m_scroll_panel, 
        wxID_ANY, 
        "Copy"
    );
    live_btn_sizer->Add(
        live_copy, 
        wxSizerFlags(1)
            .Expand().Border(wxALL, 0)
    );

    auto* live_delete = new wxButton(
        m_scroll_panel, 
        wxID_ANY, 
        "Delete"
    );
    live_btn_sizer->Add(
        live_delete, 
        wxSizerFlags(1)
            .Expand().Border(wxALL, 0)
    );

    // Append controls to clipboard sizer
    live_box_sizer->Add(
        live_btn_sizer, 
        wxSizerFlags()
            .Expand().Border(wxALL, 0)
    );
    box_sizer->Add(
        live_box_sizer, 
        wxSizerFlags(1)
            .Expand().Border(wxALL)
    );

    // Copy button binding
    live_copy->Bind(wxEVT_BUTTON, 
        [live_clipboard](wxCommandEvent&)
    {
        if (wxTheClipboard->Open())
        {
            wxTheClipboard->SetData(
                new wxTextDataObject(live_clipboard->GetValue())
            );
            wxTheClipboard->Close();
        }
    });

    // Delete
    live_delete->Bind(wxEVT_BUTTON, 
        [this, box_sizer, text](wxCommandEvent&) mutable
    {
        clip_box_map.erase(text);
        m_scroller_panel_sizer->Detach(box_sizer);
        box_sizer->Clear(true);
        delete box_sizer; box_sizer = nullptr;
        m_scroll_panel->Layout();
    });

    // Prepend clipboard box to live page
    m_scroller_panel_sizer->Insert(
        1, box_sizer, 
        wxSizerFlags()
            .Expand().Border(wxALL)
    );
    // Append clipboard box to the logger map
    clip_box_map[text] = box_sizer;
}
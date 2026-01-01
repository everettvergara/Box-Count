///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class panel_box_count
///////////////////////////////////////////////////////////////////////////////
class panel_box_count : public wxPanel
{
	private:

	protected:
		wxStaticText* m_staticText89;
		wxChoice* choice_doc_type;
		wxStaticText* m_staticText87;
		wxTextCtrl* text_doc_no;
		wxStaticText* m_staticText88;
		wxDatePickerCtrl* picker_doc_date;
		wxStaticText* m_staticText90;
		wxTextCtrl* text_start_time;
		wxStaticText* m_staticText91;
		wxTextCtrl* text_end_time;
		wxStaticText* m_staticText92;
		wxTextCtrl* m_textCtrl74;
		wxStaticBitmap* bitmap_last_box_count;
		wxStaticText* m_staticText93;
		wxTextCtrl* text_reject;
		wxStaticBitmap* bitmap_last_reject_count;
		wxStaticText* m_staticText94;
		wxTextCtrl* text_return;
		wxStaticBitmap* bitmap_last_return_count;
		wxStaticText* m_staticText96;
		wxStaticBitmap* m_bitmap6;
		wxStaticText* m_staticText97;
		wxChoice* choice_camera;
		wxButton* button_calibrate_roi;
		wxCheckBox* check_preview_motion;
		wxCheckBox* check_show_track_history;
		wxCheckBox* check_show_roi;
		wxButton* button_new;
		wxButton* button_start;
		wxButton* button_stop;

		// Virtual event handlers, override them in your derived class
		virtual void on_button_preview_cam_( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_button_new_( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_button_start_( wxCommandEvent& event ) { event.Skip(); }
		virtual void on_button_stop_( wxCommandEvent& event ) { event.Skip(); }


	public:

		panel_box_count( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 900,510 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString );

		~panel_box_count();

};


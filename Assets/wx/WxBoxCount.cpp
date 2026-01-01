///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "WxBoxCount.h"

///////////////////////////////////////////////////////////////////////////

panel_box_count::panel_box_count( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxBoxSizer* sizer_main;
	sizer_main = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* sizer_header;
	sizer_header = new wxBoxSizer( wxHORIZONTAL );

	wxStaticBoxSizer* group_document;
	group_document = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Document") ), wxVERTICAL );

	wxFlexGridSizer* sizer_fields;
	sizer_fields = new wxFlexGridSizer( 0, 1, 0, 0 );
	sizer_fields->SetFlexibleDirection( wxBOTH );
	sizer_fields->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText89 = new wxStaticText( group_document->GetStaticBox(), wxID_ANY, _("Doc Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText89->Wrap( -1 );
	sizer_fields->Add( m_staticText89, 0, wxALL, 5 );

	wxArrayString choice_doc_typeChoices;
	choice_doc_type = new wxChoice( group_document->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( 150,-1 ), choice_doc_typeChoices, 0 );
	choice_doc_type->SetSelection( 0 );
	sizer_fields->Add( choice_doc_type, 0, wxALL, 5 );

	m_staticText87 = new wxStaticText( group_document->GetStaticBox(), wxID_ANY, _("Doc No.:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText87->Wrap( -1 );
	sizer_fields->Add( m_staticText87, 0, wxALL, 5 );

	text_doc_no = new wxTextCtrl( group_document->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 150,-1 ), 0 );
	sizer_fields->Add( text_doc_no, 0, wxALL, 5 );

	m_staticText88 = new wxStaticText( group_document->GetStaticBox(), wxID_ANY, _("Doc Date:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText88->Wrap( -1 );
	sizer_fields->Add( m_staticText88, 0, wxALL, 5 );

	picker_doc_date = new wxDatePickerCtrl( group_document->GetStaticBox(), wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxSize( 150,-1 ), wxDP_DEFAULT );
	sizer_fields->Add( picker_doc_date, 0, wxALL, 5 );

	m_staticText90 = new wxStaticText( group_document->GetStaticBox(), wxID_ANY, _("Start Time:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText90->Wrap( -1 );
	sizer_fields->Add( m_staticText90, 0, wxALL, 5 );

	text_start_time = new wxTextCtrl( group_document->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 150,-1 ), 0 );
	text_start_time->Enable( false );

	sizer_fields->Add( text_start_time, 0, wxALL, 5 );

	m_staticText91 = new wxStaticText( group_document->GetStaticBox(), wxID_ANY, _("End Time:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText91->Wrap( -1 );
	sizer_fields->Add( m_staticText91, 0, wxALL, 5 );

	text_end_time = new wxTextCtrl( group_document->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 150,-1 ), 0 );
	text_end_time->Enable( false );

	sizer_fields->Add( text_end_time, 0, wxALL, 5 );


	group_document->Add( sizer_fields, 0, wxEXPAND, 5 );


	sizer_header->Add( group_document, 0, wxALL|wxEXPAND, 5 );

	wxStaticBoxSizer* group_count;
	group_count = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Counts") ), wxVERTICAL );

	wxFlexGridSizer* sizer_count;
	sizer_count = new wxFlexGridSizer( 0, 1, 0, 0 );
	sizer_count->SetFlexibleDirection( wxBOTH );
	sizer_count->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText92 = new wxStaticText( group_count->GetStaticBox(), wxID_ANY, _("Boxes:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText92->Wrap( -1 );
	sizer_count->Add( m_staticText92, 0, wxALL, 5 );

	wxBoxSizer* sizer_box_count;
	sizer_box_count = new wxBoxSizer( wxHORIZONTAL );

	m_textCtrl74 = new wxTextCtrl( group_count->GetStaticBox(), wxID_ANY, _("0"), wxDefaultPosition, wxSize( 150,100 ), wxTE_CENTER|wxTE_READONLY );
	m_textCtrl74->SetFont( wxFont( 64, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	m_textCtrl74->SetForegroundColour( wxColour( 0, 0, 255 ) );

	sizer_box_count->Add( m_textCtrl74, 0, wxALL, 5 );

	bitmap_last_box_count = new wxStaticBitmap( group_count->GetStaticBox(), wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 160,100 ), 0 );
	sizer_box_count->Add( bitmap_last_box_count, 0, wxALL, 5 );


	sizer_count->Add( sizer_box_count, 1, wxEXPAND, 5 );

	m_staticText93 = new wxStaticText( group_count->GetStaticBox(), wxID_ANY, _("Rejects:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText93->Wrap( -1 );
	sizer_count->Add( m_staticText93, 0, wxALL, 5 );

	wxBoxSizer* sizer_reject_count;
	sizer_reject_count = new wxBoxSizer( wxHORIZONTAL );

	text_reject = new wxTextCtrl( group_count->GetStaticBox(), wxID_ANY, _("0"), wxDefaultPosition, wxSize( 150,100 ), wxTE_CENTER|wxTE_READONLY );
	text_reject->SetFont( wxFont( 64, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	text_reject->SetForegroundColour( wxColour( 255, 0, 0 ) );

	sizer_reject_count->Add( text_reject, 0, wxALL, 5 );

	bitmap_last_reject_count = new wxStaticBitmap( group_count->GetStaticBox(), wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 160,100 ), 0 );
	sizer_reject_count->Add( bitmap_last_reject_count, 0, wxALL, 5 );


	sizer_count->Add( sizer_reject_count, 1, wxEXPAND, 5 );

	m_staticText94 = new wxStaticText( group_count->GetStaticBox(), wxID_ANY, _("Returns:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText94->Wrap( -1 );
	sizer_count->Add( m_staticText94, 0, wxALL, 5 );

	wxBoxSizer* sizer_return_count;
	sizer_return_count = new wxBoxSizer( wxHORIZONTAL );

	text_return = new wxTextCtrl( group_count->GetStaticBox(), wxID_ANY, _("0"), wxDefaultPosition, wxSize( 150,100 ), wxTE_CENTER|wxTE_READONLY );
	text_return->SetFont( wxFont( 64, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	text_return->SetForegroundColour( wxColour( 255, 0, 0 ) );

	sizer_return_count->Add( text_return, 0, wxALL, 5 );

	bitmap_last_return_count = new wxStaticBitmap( group_count->GetStaticBox(), wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 160,100 ), 0 );
	sizer_return_count->Add( bitmap_last_return_count, 0, wxALL, 5 );


	sizer_count->Add( sizer_return_count, 1, wxEXPAND, 5 );


	group_count->Add( sizer_count, 1, wxEXPAND, 5 );


	sizer_header->Add( group_count, 0, wxALL|wxEXPAND, 5 );

	wxStaticBoxSizer* group_live;
	group_live = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Live") ), wxVERTICAL );

	wxFlexGridSizer* sizer_live;
	sizer_live = new wxFlexGridSizer( 0, 1, 0, 0 );
	sizer_live->SetFlexibleDirection( wxBOTH );
	sizer_live->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText96 = new wxStaticText( group_live->GetStaticBox(), wxID_ANY, _("Preview:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText96->Wrap( -1 );
	sizer_live->Add( m_staticText96, 0, wxALL, 5 );

	m_bitmap6 = new wxStaticBitmap( group_live->GetStaticBox(), wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 320,200 ), 0 );
	sizer_live->Add( m_bitmap6, 0, wxALL, 5 );

	m_staticText97 = new wxStaticText( group_live->GetStaticBox(), wxID_ANY, _("Select a Camera:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText97->Wrap( -1 );
	sizer_live->Add( m_staticText97, 0, wxALL, 5 );

	wxArrayString choice_cameraChoices;
	choice_camera = new wxChoice( group_live->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_cameraChoices, 0 );
	choice_camera->SetSelection( 0 );
	sizer_live->Add( choice_camera, 0, wxALL, 5 );

	button_calibrate_roi = new wxButton( group_live->GetStaticBox(), wxID_ANY, _("Calibrate Selected Camera"), wxDefaultPosition, wxDefaultSize, 0 );
	sizer_live->Add( button_calibrate_roi, 0, wxALL, 5 );

	check_preview_motion = new wxCheckBox( group_live->GetStaticBox(), wxID_ANY, _("Open Debug Windows"), wxDefaultPosition, wxDefaultSize, 0 );
	sizer_live->Add( check_preview_motion, 0, wxALL, 5 );

	check_show_track_history = new wxCheckBox( group_live->GetStaticBox(), wxID_ANY, _("Show Track History"), wxDefaultPosition, wxDefaultSize, 0 );
	check_show_track_history->SetValue(true);
	sizer_live->Add( check_show_track_history, 0, wxALL, 5 );

	check_show_roi = new wxCheckBox( group_live->GetStaticBox(), wxID_ANY, _("Show Region of Interest"), wxDefaultPosition, wxDefaultSize, 0 );
	check_show_roi->SetValue(true);
	sizer_live->Add( check_show_roi, 0, wxALL, 5 );


	group_live->Add( sizer_live, 1, wxEXPAND, 5 );


	sizer_header->Add( group_live, 1, wxALL|wxEXPAND, 5 );


	sizer_main->Add( sizer_header, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* group_buttons;
	group_buttons = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxEmptyString ), wxVERTICAL );

	wxBoxSizer* sizer_buttons;
	sizer_buttons = new wxBoxSizer( wxHORIZONTAL );

	button_new = new wxButton( group_buttons->GetStaticBox(), wxID_ANY, _("New"), wxDefaultPosition, wxDefaultSize, 0 );
	sizer_buttons->Add( button_new, 0, wxALL, 5 );


	sizer_buttons->Add( 0, 0, 1, wxEXPAND, 5 );

	button_start = new wxButton( group_buttons->GetStaticBox(), wxID_ANY, _("Start"), wxDefaultPosition, wxDefaultSize, 0 );
	sizer_buttons->Add( button_start, 0, wxALL, 5 );

	button_stop = new wxButton( group_buttons->GetStaticBox(), wxID_ANY, _("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
	sizer_buttons->Add( button_stop, 0, wxALL, 5 );


	group_buttons->Add( sizer_buttons, 1, wxEXPAND, 5 );


	sizer_main->Add( group_buttons, 0, wxALL|wxEXPAND, 5 );


	this->SetSizer( sizer_main );
	this->Layout();

	// Connect Events
	button_calibrate_roi->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( panel_box_count::on_button_preview_cam_ ), NULL, this );
	button_new->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( panel_box_count::on_button_new_ ), NULL, this );
	button_start->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( panel_box_count::on_button_start_ ), NULL, this );
	button_stop->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( panel_box_count::on_button_stop_ ), NULL, this );
}

panel_box_count::~panel_box_count()
{
}

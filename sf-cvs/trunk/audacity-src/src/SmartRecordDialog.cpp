/**********************************************************************

  Audacity: A Digital Audio Editor

  SmartRecordDialog.h

  Copyright 2006 by Vaughan Johnson
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

*******************************************************************//**

\class SmartRecordDialog
\brief Dialog for Smart Record, i.e., timed or long recording, limited GUI.

*//*******************************************************************/

#include "Audacity.h"

#include <wx/defs.h>
#include <wx/datetime.h>
#include <wx/intl.h>
#include <wx/progdlg.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/timer.h>

#include "SmartRecordDialog.h"
#include "Project.h"

#define MAX_PROG 1000
#define TIMER_ID 7000

enum { // control IDs
   ID_DATEPICKER_START = 10000, 
   ID_TIMETEXT_START, 
   ID_DATEPICKER_END, 
   ID_TIMETEXT_END, 
   ID_TIMETEXT_DURATION
};

const int kTimerInterval = 1000; // every 1000 ms -> 1 update per second   

double wxDateTime_to_AudacityTime(wxDateTime& dateTime)
{
   return (dateTime.GetHour() * 3600.0) + (dateTime.GetMinute() * 60.0) + dateTime.GetSecond();
};

BEGIN_EVENT_TABLE(SmartRecordDialog, wxDialog)
   EVT_DATE_CHANGED(ID_DATEPICKER_START, SmartRecordDialog::OnDatePicker_Start)
   EVT_TEXT(ID_TIMETEXT_START, SmartRecordDialog::OnTimeText_Start)
   
   EVT_DATE_CHANGED(ID_DATEPICKER_END, SmartRecordDialog::OnDatePicker_End)
   EVT_TEXT(ID_TIMETEXT_END, SmartRecordDialog::OnTimeText_End)
   
   EVT_TEXT(ID_TIMETEXT_DURATION, SmartRecordDialog::OnTimeText_Duration)

   EVT_BUTTON(wxID_OK, SmartRecordDialog::OnOK)

   EVT_TIMER(TIMER_ID, SmartRecordDialog::OnTimer)
END_EVENT_TABLE()

SmartRecordDialog::SmartRecordDialog(wxWindow* parent)
: wxDialog(parent, -1, _("Audacity Timer Record"), wxDefaultPosition, 
           wxDefaultSize, wxDIALOG_MODAL | wxCAPTION | wxTHICK_FRAME)
{
   m_DateTime_Start = wxDateTime::Now(); //vvv + wxTimeSpan::Minutes(1); 
   m_TimeSpan_Duration = wxTimeSpan::Minutes(5); // default 5 minute duration
   m_DateTime_End = m_DateTime_Start + m_TimeSpan_Duration;

   m_pDatePickerCtrl_Start = NULL;
   m_pTimeTextCtrl_Start = NULL;

   m_pDatePickerCtrl_End = NULL;
   m_pTimeTextCtrl_End = NULL;
   
   m_pTimeTextCtrl_Duration = NULL;

   ShuttleGui S(this, eIsCreating);
   this->PopulateOrExchange(S);

   m_timer.SetOwner(this, TIMER_ID);
   m_timer.Start(kTimerInterval);    // 1 second interval
}

SmartRecordDialog::~SmartRecordDialog()
{
}

void SmartRecordDialog::OnTimer(wxTimerEvent& event)
{
   if (m_DateTime_Start < wxDateTime::Now()) {
      m_DateTime_Start = wxDateTime::Now();
      m_pDatePickerCtrl_Start->SetValue(m_DateTime_Start);
      m_pTimeTextCtrl_Start->SetTimeValue(wxDateTime_to_AudacityTime(m_DateTime_Start));
   }

   this->UpdateEnd(); // Keep Duration constant and update End for changed Start.
}

void SmartRecordDialog::OnDatePicker_Start(wxDateEvent& event)
{
   m_DateTime_Start = m_pDatePickerCtrl_Start->GetValue();
   double dTime = m_pTimeTextCtrl_Start->GetTimeValue();
   unsigned int hr = dTime / 3600.0;
   unsigned int min = (dTime - (hr * 3600.0)) / 60.0;
   unsigned int sec = dTime - (hr * 3600.0) - (min * 60.0);
   m_DateTime_Start.SetHour(hr);
   m_DateTime_Start.SetMinute(min);
   m_DateTime_Start.SetSecond(sec);

   // User might have had the dialog up for a while, or 
   // had a future day, set hour of day less than now's, then changed day to today.
   wxTimerEvent dummyTimerEvent;
   this->OnTimer(dummyTimerEvent);
}

void SmartRecordDialog::OnTimeText_Start(wxCommandEvent& event)
{
   //v TimeTextCtrl doesn't implement upper ranges, i.e., if I tell it "024 h 060 m 060 s", then 
   // user increments the hours past 23, it rolls over to 0 (although if you increment below 0, it stays at 0).
   // So instead, set the max to 99 and just catch hours > 24 and fix the ctrls.
   double dTime = m_pTimeTextCtrl_Start->GetTimeValue();
   unsigned int days = dTime / (24.0 * 3600.0);
   if (days > 0) {
      dTime -= (double)days * 24.0 * 3600.0;
      m_DateTime_Start += wxTimeSpan::Days(days);
      m_pDatePickerCtrl_Start->SetValue(m_DateTime_Start);
      m_pTimeTextCtrl_Start->SetTimeValue(dTime);
   }

   wxDateEvent dummyDateEvent;
   this->OnDatePicker_Start(dummyDateEvent);
}
 
void SmartRecordDialog::OnDatePicker_End(wxDateEvent& event)
{
   m_DateTime_End = m_pDatePickerCtrl_End->GetValue();
   double dTime = m_pTimeTextCtrl_End->GetTimeValue();
   unsigned int hr = dTime / 3600.0;
   unsigned int min = (dTime - (hr * 3600.0)) / 60.0;
   unsigned int sec = dTime - (hr * 3600.0) - (min * 60.0);
   m_DateTime_End.SetHour(hr);
   m_DateTime_End.SetMinute(min);
   m_DateTime_End.SetSecond(sec);

   // DatePickerCtrls use SetRange to make sure End is never less than Start, but 
   // need to implement it for the TimeTextCtrls.
   if (m_DateTime_End < m_DateTime_Start) {
      m_DateTime_End = m_DateTime_Start;
      m_pTimeTextCtrl_End->SetTimeValue(wxDateTime_to_AudacityTime(m_DateTime_End));
   }

   this->UpdateDuration(); // Keep Start constant and update Duration for changed End.
}

void SmartRecordDialog::OnTimeText_End(wxCommandEvent& event)
{
   //v TimeTextCtrl doesn't implement upper ranges, i.e., if I tell it "024 h 060 m 060 s", then 
   // user increments the hours past 23, it rolls over to 0 (although if you increment below 0, it stays at 0).
   // So instead, set the max to 99 and just catch hours > 24 and fix the ctrls.
   double dTime = m_pTimeTextCtrl_End->GetTimeValue();
   unsigned int days = dTime / (24.0 * 3600.0);
   if (days > 0) {
      dTime -= (double)days * 24.0 * 3600.0;
      m_DateTime_End += wxTimeSpan::Days(days);
      m_pDatePickerCtrl_End->SetValue(m_DateTime_End);
      m_pTimeTextCtrl_End->SetTimeValue(dTime);
   }

   wxDateEvent dummyDateEvent;
   this->OnDatePicker_End(dummyDateEvent);
}

void SmartRecordDialog::OnTimeText_Duration(wxCommandEvent& event)
{
   double dTime = m_pTimeTextCtrl_Duration->GetTimeValue();
   unsigned int hr = dTime / 3600.0;
   unsigned int min = (dTime - (hr * 3600.0)) / 60.0;
   unsigned int sec = dTime - (hr * 3600.0) - (min * 60.0);
   m_TimeSpan_Duration = wxTimeSpan(hr, min, sec); //v milliseconds?

   this->UpdateEnd(); // Keep Start constant and update End for changed Duration.
}

void SmartRecordDialog::OnOK(wxCommandEvent& event)
{
   m_timer.Stop();

   this->TransferDataFromWindow();

   bool bDidCancel = false;
   if (m_DateTime_Start > wxDateTime::UNow()) bDidCancel = !this->WaitForStart(); 

   if (!bDidCancel) { // Record for specified time. 
      //v For now, just usual record mechanism.
   	AudacityProject* pProject = GetActiveProject();
      pProject->OnRecord();

      bool bIsRecording = true;

      /* Although inaccurate, use the wxProgressDialog wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME 
         instead of calculating and presenting it.
            wxTimeSpan remaining_TimeSpan;
            wxString strNewMsg;
         */
      wxString strMsg = 
         _("Recording start: ") + m_DateTime_Start.Format() + 
         _("\n\nRecording end:   ") + m_DateTime_End.Format() + 
         _("          Duration: ") + m_TimeSpan_Duration.Format() + wxT("\n"); 

      pProject->ProgressShow(
               _("Audacity Smart Record Progress"), // const wxString& title,
               strMsg); // const wxString& message

      wxTimeSpan done_TimeSpan;
      wxLongLong llProgValue;
      int nProgValue = 0;
      while (bIsRecording && !bDidCancel) {
         wxMilliSleep(kTimerInterval);

         done_TimeSpan = wxDateTime::Now() - m_DateTime_Start;
         // remaining_TimeSpan = m_DateTime_End - wxDateTime::Now();

         llProgValue = 
            (wxLongLong)((done_TimeSpan.GetSeconds() * (double)MAX_PROG) / 
                           m_TimeSpan_Duration.GetSeconds());
         nProgValue = llProgValue.ToLong(); //v ToLong truncates, so may fail.

         // strNewMsg = strMsg + _("\nDone: ") + done_TimeSpan.Format() + _("     Remaining: ") + remaining_TimeSpan.Format();
         bDidCancel = !pProject->ProgressUpdate(nProgValue); // , strNewMsg);
         bIsRecording = (wxDateTime::UNow() < m_DateTime_End);
      }
      pProject->OnStop();
      pProject->ProgressHide();
   }

   this->EndModal(wxID_OK);
}

void SmartRecordDialog::PopulateOrExchange(ShuttleGui& S)
{
   S.SetBorder(5);
   S.StartVerticalLay(true);
   {
      /* //vvv
         S.StartRadioButtonGroup(wxT("Start"));
         {
            S.AddRadioButton(_("Start Time"));
            //vvv TimeTextControl
            S.AddRadioButton(_("Now"));
            }
         S.EndRadioButtonGroup();
         */
      wxString strFormat = wxT("099 h 060 m 060 s");
      S.StartStatic(_("Start Date and Time"), true);
      {
         m_pDatePickerCtrl_Start = 
            new wxDatePickerCtrl(this, // wxWindow *parent, 
                                 ID_DATEPICKER_START, // wxWindowID id, 
                                 m_DateTime_Start); // const wxDateTime& dt = wxDefaultDateTime, 
                                 // const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDP_DEFAULT | wxDP_SHOWCENTURY, const wxValidator& validator = wxDefaultValidator, const wxString& name = "datectrl")
         m_pDatePickerCtrl_Start->SetRange(wxDateTime::Today(), wxInvalidDateTime); // No backdating.
         S.AddWindow(m_pDatePickerCtrl_Start);

         m_pTimeTextCtrl_Start = new TimeTextCtrl(this, ID_TIMETEXT_START, strFormat);
         m_pTimeTextCtrl_Start->SetTimeValue(wxDateTime_to_AudacityTime(m_DateTime_Start));
         S.AddWindow(m_pTimeTextCtrl_Start);
         //v Don't allow them to change time format: m_pTimeTextCtrl_Start->EnableMenu();
      }
      S.EndStatic();

      S.StartStatic(_("End Date and Time"), true);
      {
         m_pDatePickerCtrl_End = 
            new wxDatePickerCtrl(this, // wxWindow *parent, 
                                 ID_DATEPICKER_END, // wxWindowID id, 
                                 m_DateTime_End); // const wxDateTime& dt = wxDefaultDateTime, 
                                 // const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDP_DEFAULT | wxDP_SHOWCENTURY, const wxValidator& validator = wxDefaultValidator, const wxString& name = "datectrl")
         m_pDatePickerCtrl_End->SetRange(m_DateTime_Start, wxInvalidDateTime); // No backdating.
         S.AddWindow(m_pDatePickerCtrl_End);

         m_pTimeTextCtrl_End = new TimeTextCtrl(this, ID_TIMETEXT_END, strFormat);
         m_pTimeTextCtrl_End->SetTimeValue(wxDateTime_to_AudacityTime(m_DateTime_End));
         S.AddWindow(m_pTimeTextCtrl_End);
         //v Don't allow them to change time format:  m_pTimeTextCtrl_End->EnableMenu();
      }
      S.EndStatic();

      S.StartStatic(_("Duration"), true);
      {
         m_pTimeTextCtrl_Duration = new TimeTextCtrl(this, ID_TIMETEXT_DURATION, strFormat);
         m_pTimeTextCtrl_Duration->SetTimeValue(m_TimeSpan_Duration.GetSeconds().ToDouble()); //vvv milliseconds?
         S.AddWindow(m_pTimeTextCtrl_Duration);
         //vvv Maybe allow? //v Don't allow them to change time format:  m_pTimeTextCtrl_Duration->EnableMenu();
      }
      S.EndStatic();
   }
   S.EndVerticalLay();
   
   GetSizer()->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL),
                   0,
                   wxCENTER | wxBOTTOM,
                   10);

   Layout();
   Fit();
   SetMinSize(GetSize());
   Center();
}

bool SmartRecordDialog::TransferDataFromWindow()
{
   double dTime;
   unsigned int hr;
   unsigned int min;
   unsigned int sec;

   m_DateTime_Start = m_pDatePickerCtrl_Start->GetValue();
   dTime = m_pTimeTextCtrl_Start->GetTimeValue();
   hr = dTime / 3600.0;
   min = (dTime - (hr * 3600.0)) / 60.0;
   sec = dTime - (hr * 3600.0) - (min * 60.0);
   m_DateTime_Start.SetHour(hr);
   m_DateTime_Start.SetMinute(min);
   m_DateTime_Start.SetSecond(sec);

   m_DateTime_End = m_pDatePickerCtrl_End->GetValue();
   dTime = m_pTimeTextCtrl_End->GetTimeValue();
   hr = dTime / 3600.0;
   min = (dTime - (hr * 3600.0)) / 60.0;
   sec = dTime - (hr * 3600.0) - (min * 60.0);
   m_DateTime_End.SetHour(hr);
   m_DateTime_End.SetMinute(min);
   m_DateTime_End.SetSecond(sec);

   m_TimeSpan_Duration = m_DateTime_End - m_DateTime_Start;

   return true;
}

// Update m_TimeSpan_Duration and ctrl based on m_DateTime_Start and m_DateTime_End.
void SmartRecordDialog::UpdateDuration() 
{
   //vvvvv A week overflows timeTextCTrl, so implement days ctrl and fix this calculation.
   m_TimeSpan_Duration = m_DateTime_End - m_DateTime_Start;
   m_pTimeTextCtrl_Duration->SetTimeValue(m_TimeSpan_Duration.GetSeconds().ToDouble()); //vvv milliseconds?
}

// Update m_DateTime_End and ctrls based on m_DateTime_Start and m_TimeSpan_Duration.
void SmartRecordDialog::UpdateEnd()
{
   //vvv Use remaining disk -> record time calcs from AudacityProject::OnTimer to set range?
   m_DateTime_End = m_DateTime_Start + m_TimeSpan_Duration;
   m_pDatePickerCtrl_End->SetValue(m_DateTime_End);
   m_pDatePickerCtrl_End->SetRange(m_DateTime_Start, wxInvalidDateTime); // No backdating.
   m_pDatePickerCtrl_End->Refresh();
   m_pTimeTextCtrl_End->SetTimeValue(wxDateTime_to_AudacityTime(m_DateTime_End));
}

bool SmartRecordDialog::WaitForStart()
{
   wxString strMsg = _("Waiting to start recording at ") + m_DateTime_Start.Format() + wxT(".\n"); 
   wxProgressDialog* progDlg =
      new wxProgressDialog(
            _("Audacity Smart Record - Waiting for Start"), // const wxString& title,
            strMsg, // const wxString& message,
            MAX_PROG, // int maximum = 100, 
            NULL, // wxWindow * parent = NULL, 
            wxPD_AUTO_HIDE | wxPD_APP_MODAL | 
               wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME); // int style = wxPD_AUTO_HIDE | wxPD_APP_MODAL
   progDlg->Show();
   wxDateTime startWait_DateTime = wxDateTime::Now();
   wxTimeSpan waitDuration = m_DateTime_Start - startWait_DateTime;

   bool bDidCancel = false;
   bool bIsRecording = false;
   wxTimeSpan done_TimeSpan;
   wxLongLong llProgValue;
   int nProgValue = 0;
   while (!bDidCancel && !bIsRecording) {
      wxMilliSleep(kTimerInterval); //vvv Resolution?

      done_TimeSpan = wxDateTime::Now() - startWait_DateTime;
      llProgValue = 
         (wxLongLong)((done_TimeSpan.GetSeconds() * (double)MAX_PROG) / 
                        waitDuration.GetSeconds());
      nProgValue = llProgValue.ToLong(); //vvv ToLong truncates, so may fail ASSERT.
      bDidCancel = !progDlg->Update(nProgValue);

      bIsRecording = (m_DateTime_Start <= wxDateTime::UNow());
   }
   delete progDlg;
   return !bDidCancel;
}

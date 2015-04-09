#include "dialog_teardrops.h"

DIALOG_TEARDROPS::DIALOG_TEARDROPS(PCB_EDIT_FRAME *aParent, TEARDROPS_SETTINGS *settings):
    DIALOG_TEARDROPS_BASE(aParent)
{
    m_parent = aParent;
    m_settings = settings;
    if (m_settings != NULL) {
    InitDialogSettings();
    }
}

void DIALOG_TEARDROPS::InitDialogSettings()
{
    wxASSERT(m_settings != NULL);
    if (m_modeRemove->GetValue() == true) {
        m_settings->m_mode = TEARDROPS_MODE_REMOVE;
    }
    else {
        m_settings->m_mode = TEARDROPS_MODE_ADD;
    }

    if (m_tracksAll->GetValue() == true) {
        m_settings->m_track = TEARDROPS_TRACKS_ALL;
    }
    else {
        m_settings->m_track = TEARDROPS_TRACKS_SELECTED;
    }

    m_settings->m_type = static_cast<TEARDROPS_TYPE>(m_choiceStyle->GetSelection());
    m_settings->m_scope = TEARDROPS_SCOPE_NONE;
    if (m_scopeVias->IsChecked() == true) {
        m_settings->m_scope = static_cast<TEARDROPS_SCOPE>(m_settings->m_scope | TEARDROPS_SCOPE_VIAS);
    }
    if (m_scopePads->IsChecked() == true) {
        m_settings->m_scope = static_cast<TEARDROPS_SCOPE>(m_settings->m_scope | TEARDROPS_SCOPE_PADS);
    }
    if (m_scopeTracks->IsChecked() == true) {
        m_settings->m_scope = static_cast<TEARDROPS_SCOPE>(m_settings->m_scope | TEARDROPS_SCOPE_TRACKS);
    }
    m_settings->m_clearSelection = m_checkClear->IsChecked();
    m_settings->m_ignoreDrc = m_checkIgnore->IsChecked();
}

void DIALOG_TEARDROPS::OnModeAdd(wxCommandEvent &event)
{
    if (m_settings != NULL) {
        m_settings->m_mode = TEARDROPS_MODE_ADD;
    }
}

void DIALOG_TEARDROPS::OnModeRemove(wxCommandEvent &event)
{
    if (m_settings != NULL) {
        m_settings->m_mode = TEARDROPS_MODE_REMOVE;
    }
}

void DIALOG_TEARDROPS::OnTracksAll(wxCommandEvent &event)
{
    if (m_settings != NULL) {
        m_settings->m_track = TEARDROPS_TRACKS_ALL;
    }
    m_checkClear->Enable(false);
}

void DIALOG_TEARDROPS::OnTracksSelected(wxCommandEvent &event)
{
    if (m_settings != NULL)	 {
        m_settings->m_track = TEARDROPS_TRACKS_SELECTED;
    }
    m_checkClear->Enable(true);
}

void DIALOG_TEARDROPS::OnScopeVias(wxCommandEvent &event)
{
    if (m_settings != NULL) {
        if (m_scopeVias->IsChecked()) {
            m_settings->m_scope = static_cast<TEARDROPS_SCOPE>(m_settings->m_scope | TEARDROPS_SCOPE_VIAS);
        }
        else {
            m_settings->m_scope = static_cast<TEARDROPS_SCOPE>(m_settings->m_scope & (~TEARDROPS_SCOPE_VIAS));
        }
    }
}

void DIALOG_TEARDROPS::OnStyleChanged(wxCommandEvent &event)
{
    if (m_settings != NULL) {
        m_settings->m_type = static_cast<TEARDROPS_TYPE>(m_choiceStyle->GetSelection());
//        if (selection == TEARDROPS_TYPE_STRAIGHT) {
//            m_settings->m_type= TEARDROPS_TYPE_STRAIGHT;
//        }
//        else if (selection == TEARDROPS_TYPE_CURVED) {
//            m_settings->m_type = TEARDROPS_TYPE_CURVED;
//        }
//        else {
//            m_settings->m_type = TEARDROPS_TYPE_NONE;
//        }
    }
}

void DIALOG_TEARDROPS::OnClearSelection(wxCommandEvent &event)
{
    event.Skip();
    if (m_settings != NULL) {
        m_settings->m_clearSelection = m_checkClear->IsChecked();
    }
}

void DIALOG_TEARDROPS::OnIgnoreDrc(wxCommandEvent &event)
{
    event.Skip();
    if (m_settings != NULL) {
        m_settings->m_ignoreDrc = m_checkIgnore->IsChecked();
    }
}

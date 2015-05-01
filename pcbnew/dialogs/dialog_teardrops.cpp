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
    event.Skip();
    if (m_settings != NULL) {
        m_settings->m_mode = TEARDROPS_MODE_ADD;
        LockOptionsControls(false);
        LockTracksControls(false);
        LockScopeControls(false);
    }
}

void DIALOG_TEARDROPS::OnModeRemove(wxCommandEvent &event)
{
    event.Skip();
    if (m_settings != NULL) {
        m_settings->m_mode = TEARDROPS_MODE_REMOVE;
        LockOptionsControls(true);
        LockTracksControls(true);
        LockScopeControls(true);
    }
}

void DIALOG_TEARDROPS::OnTracksAll(wxCommandEvent &event)
{
    event.Skip();
    if (m_settings != NULL) {
        m_settings->m_track = TEARDROPS_TRACKS_ALL;
    }
    m_checkClear->Enable(false);
}

void DIALOG_TEARDROPS::OnTracksSelected(wxCommandEvent &event)
{
    event.Skip();
    if (m_settings != NULL)	 {
        m_settings->m_track = TEARDROPS_TRACKS_SELECTED;
    }
    m_checkClear->Enable(true);
}

void DIALOG_TEARDROPS::OnScopeVias(wxCommandEvent &event)
{
    event.Skip();
    if (m_settings != NULL) {
        if (m_scopeVias->IsChecked()) {
            m_settings->m_scope = static_cast<TEARDROPS_SCOPE>(m_settings->m_scope | TEARDROPS_SCOPE_VIAS);
        }
        else {
            m_settings->m_scope = static_cast<TEARDROPS_SCOPE>(m_settings->m_scope & (~TEARDROPS_SCOPE_VIAS));
        }
    }
}

void DIALOG_TEARDROPS::OnScopePads(wxCommandEvent &event)
{
    event.Skip();
    if (m_settings != NULL) {
        if (m_scopePads->IsChecked()) {
            m_settings->m_scope = static_cast<TEARDROPS_SCOPE>(m_settings->m_scope | TEARDROPS_SCOPE_PADS);
        }
        else {
            m_settings->m_scope = static_cast<TEARDROPS_SCOPE>(m_settings->m_scope & (~TEARDROPS_SCOPE_PADS));
        }
    }
}

void DIALOG_TEARDROPS::OnStyleChanged(wxCommandEvent &event)
{
    event.Skip();
    if (m_settings != NULL) {
        m_settings->m_type = static_cast<TEARDROPS_TYPE>(m_choiceStyle->GetSelection());
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

void DIALOG_TEARDROPS::LockOptionsControls(bool state)
{
    if (state == true) {
        if (m_tracksSelected->GetValue() == false) {
            m_checkClear->Enable(false);
        }
        m_checkIgnore->Enable(false);
        m_choiceStyle->Enable(false);
    }
    else {
        if (m_tracksSelected->GetValue() == true) {
            m_checkClear->Enable(true);
        }
        m_checkIgnore->Enable(true);
        m_choiceStyle->Enable(true);
    }
}

void DIALOG_TEARDROPS::LockTracksControls(bool state)
{
    if (state == true) {
        m_tracksAll->Enable(false);
        m_tracksSelected->Enable(false);
    }
    else {
        m_tracksAll->Enable(true);
        m_tracksSelected->Enable(true);
    }
}

void DIALOG_TEARDROPS::LockScopeControls(bool state)
{
    if (state == true) {
        m_scopePads->Enable(false);
        m_scopeVias->Enable(false);
    }
    else {
        m_scopePads->Enable(true);
        m_scopeVias->Enable(true);
    }
}

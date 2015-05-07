/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Elphel, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dialog_teardrops.h"

DIALOG_TEARDROPS::DIALOG_TEARDROPS(PCB_EDIT_FRAME *aParent, TEARDROPS_SETTINGS *aSettings = NULL ):
    DIALOG_TEARDROPS_BASE( aParent )
{
    m_parent = aParent;
    m_settings = aSettings;
    if( m_settings != NULL )
    {
        initDialogSettings();
    }
}

void DIALOG_TEARDROPS::initDialogSettings()
{
    assert(m_settings != NULL);

    if( m_modeRemove->GetValue() == true )
    {
        m_settings->m_mode = TEARDROPS_MODE_REMOVE;
    }
    else
    {
        m_settings->m_mode = TEARDROPS_MODE_ADD;
    }

    if( m_tracksAll->GetValue() == true )
    {
        m_settings->m_track = TEARDROPS_TRACKS_ALL;
    }
    else
    {
        m_settings->m_track = TEARDROPS_TRACKS_SELECTED;
    }

    m_settings->m_type = static_cast<TEARDROPS_TYPE>( m_choiceStyle->GetSelection() );
    m_settings->m_scope = TEARDROPS_SCOPE_NONE;
    if( m_scopeVias->IsChecked() == true )
    {
        m_settings->m_scope = static_cast<TEARDROPS_SCOPE>( m_settings->m_scope | TEARDROPS_SCOPE_VIAS );
    }
    if( m_scopePads->IsChecked() == true )
    {
        m_settings->m_scope = static_cast<TEARDROPS_SCOPE>( m_settings->m_scope | TEARDROPS_SCOPE_PADS );
    }
    if( m_scopeTracks->IsChecked() == true )
    {
        m_settings->m_scope = static_cast<TEARDROPS_SCOPE>( m_settings->m_scope | TEARDROPS_SCOPE_TRACKS );
    }
    m_settings->m_clearSelection = m_checkClear->IsChecked();
    m_settings->m_ignoreDrc = m_checkIgnore->IsChecked();
}

void DIALOG_TEARDROPS::OnModeAdd( wxCommandEvent &aEvent )
{
    aEvent.Skip();
    if( m_settings != NULL )
    {
        m_settings->m_mode = TEARDROPS_MODE_ADD;
        lockOptionsControls( false );
        lockTracksControls( false );
        lockScopeControls( false );
    }
}

void DIALOG_TEARDROPS::OnModeRemove( wxCommandEvent &aEvent )
{
    aEvent.Skip();
    if( m_settings != NULL )
    {
        m_settings->m_mode = TEARDROPS_MODE_REMOVE;
        lockOptionsControls( true );
        lockTracksControls( true );
        lockScopeControls( true );
    }
}

void DIALOG_TEARDROPS::OnTracksAll( wxCommandEvent &aEvent )
{
    aEvent.Skip();
    if( m_settings != NULL )
    {
        m_settings->m_track = TEARDROPS_TRACKS_ALL;
    }
    m_checkClear->Enable( false );
}

void DIALOG_TEARDROPS::OnTracksSelected( wxCommandEvent &aEvent )
{
    aEvent.Skip();
    if( m_settings != NULL )
    {
        m_settings->m_track = TEARDROPS_TRACKS_SELECTED;
    }
    m_checkClear->Enable( true );
}

void DIALOG_TEARDROPS::OnScopeVias( wxCommandEvent &aEvent )
{
    aEvent.Skip();
    if( m_settings != NULL )
    {
        if( m_scopeVias->IsChecked() )
        {
            m_settings->m_scope = static_cast<TEARDROPS_SCOPE>( m_settings->m_scope | TEARDROPS_SCOPE_VIAS );
        }
        else
        {
            m_settings->m_scope = static_cast<TEARDROPS_SCOPE>( m_settings->m_scope & (~TEARDROPS_SCOPE_VIAS) );
        }
    }
}

void DIALOG_TEARDROPS::OnScopePads( wxCommandEvent &aEvent )
{
    aEvent.Skip();
    if( m_settings != NULL )
    {
        if( m_scopePads->IsChecked() )
        {
            m_settings->m_scope = static_cast<TEARDROPS_SCOPE>( m_settings->m_scope | TEARDROPS_SCOPE_PADS );
        }
        else
        {
            m_settings->m_scope = static_cast<TEARDROPS_SCOPE>( m_settings->m_scope & (~TEARDROPS_SCOPE_PADS) );
        }
    }
}

void DIALOG_TEARDROPS::OnStyleChanged( wxCommandEvent &aEvent )
{
    aEvent.Skip();
    if( m_settings != NULL )
    {
        m_settings->m_type = static_cast<TEARDROPS_TYPE>( m_choiceStyle->GetSelection() );
    }
}

void DIALOG_TEARDROPS::OnClearSelection( wxCommandEvent &aEvent )
{
    aEvent.Skip();
    if( m_settings != NULL )
    {
        m_settings->m_clearSelection = m_checkClear->IsChecked();
    }
}

void DIALOG_TEARDROPS::OnIgnoreDrc( wxCommandEvent &aEvent )
{
    aEvent.Skip();
    if( m_settings != NULL )
    {
        m_settings->m_ignoreDrc = m_checkIgnore->IsChecked();
    }
}

void DIALOG_TEARDROPS::lockOptionsControls( bool state )
{
    if( state == true )
    {
        if( m_tracksSelected->GetValue() == false )
        {
            m_checkClear->Enable( false );
        }
        m_checkIgnore->Enable( false );
        m_choiceStyle->Enable( false );
    }
    else
    {
        if( m_tracksSelected->GetValue() == true )
        {
            m_checkClear->Enable( true );
        }
        m_checkIgnore->Enable( true );
        m_choiceStyle->Enable( true );
    }
}

void DIALOG_TEARDROPS::lockTracksControls( bool state )
{
    if( state == true )
    {
        m_tracksAll->Enable( false );
        m_tracksSelected->Enable( false );
    }
    else
    {
        m_tracksAll->Enable( true );
        m_tracksSelected->Enable( true );
    }
}

void DIALOG_TEARDROPS::lockScopeControls( bool state )
{
    if( state == true )
    {
        m_scopePads->Enable( false );
        m_scopeVias->Enable( false );
    }
    else
    {
        m_scopePads->Enable( true );
        m_scopeVias->Enable( true );
    }
}

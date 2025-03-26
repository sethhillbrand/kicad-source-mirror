/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2025 Ethan Chien <liangtie.qian@gmail.com>
 * Copyright (C) 2025 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef PCB_COPILOT_UI_H
#define PCB_COPILOT_UI_H

#include <pcbnew_settings.h>
#include <pcb_edit_frame.h>
#include <tool/action_toolbar.h>
#include <widgets/wx_aui_utils.h>
#include <copilot_panel_name.h>
#include <assistant_interface.h>
#include <tool/actions.h>
#include <copilot_aui_info.h>

void PCB_EDIT_FRAME::InitCopilotPanel()
{
    if( ASSISTANT_INTERFACE::get_instance().is_assistant_available() )
    {
        m_copilotPanel = ASSISTANT_INTERFACE::get_instance().create_assistant_panel(
                this, m_copilotGlobalContextHdl );
    }
}

void PCB_EDIT_FRAME::InitCopilotAui()
{
    if( m_copilotPanel )
    {
        m_auimgr.AddPane( m_copilotPanel, defaultCopilotPaneInfo( this ).Layer( 5 ) );
    }
}

void PCB_EDIT_FRAME::RecreateCopilotToolBar()
{
    if( m_copilotPanel )
    {
        m_mainToolBar->AddScaledSeparator( this );
        m_mainToolBar->Add( ACTIONS::toggleCopilotPanel );
    }
}

void PCB_EDIT_FRAME::CopilotPanelShowChangedLanguage()
{
    if( m_copilotPanel )
    {
        wxAuiPaneInfo& copilot_panel_info = m_auimgr.GetPane( m_copilotPanel );
        bool           is_shown = copilot_panel_info.IsShown();
        copilot_panel_info.Caption( _( "Copilot" ) );
        copilot_panel_info.Show( is_shown );
    }
}

void PCB_EDIT_FRAME::ToggleCopilot()
{
    wxAuiPaneInfo& copilot_pane = m_auimgr.GetPane( CopilotPanelName() );
    ShowCopilot( !copilot_pane.IsShown() );
}

void PCB_EDIT_FRAME::ShowCopilot( bool show )
{
    PCBNEW_SETTINGS* cfg = GetPcbNewSettings();

    wxCHECK( cfg, /* void */ );

    wxAuiPaneInfo& copilot_pane = m_auimgr.GetPane( CopilotPanelName() );

    bool now_shown = copilot_pane.IsShown();

    if( now_shown == show )
        return;

    copilot_pane.Show( show );

    if( copilot_pane.IsShown() )
    {
        if( copilot_pane.IsFloating() )
        {
            copilot_pane.FloatingSize( cfg->m_AuiPanels.copilot_panel_float_width,
                                       cfg->m_AuiPanels.copilot_panel_float_height );
            m_auimgr.Update();
        }
        else
        {
            // SetAuiPaneSize also updates m_auimgr
            SetAuiPaneSize( m_auimgr, copilot_pane,
                            cfg->m_AuiPanels.copilot_panel_docked_width > 0
                                    ? cfg->m_AuiPanels.copilot_panel_docked_width
                                    : -1,
                            cfg->m_AuiPanels.copilot_panel_docked_height );
        }
    }
    else
    {
        if( copilot_pane.IsFloating() )
        {
            cfg->m_AuiPanels.copilot_panel_float_width = copilot_pane.floating_size.x;
            cfg->m_AuiPanels.copilot_panel_float_height = copilot_pane.floating_size.y;
        }
        else
        {
            cfg->m_AuiPanels.copilot_panel_docked_width = m_copilotPanel->GetSize().x;
            cfg->m_AuiPanels.copilot_panel_docked_height = m_copilotPanel->GetSize().y;
        }

        m_auimgr.Update();
    }
}

void PCB_EDIT_FRAME::SaveCopilotCnf()
{
    PCBNEW_SETTINGS* cfg = GetPcbNewSettings();

    wxCHECK( cfg, /* void */ );

    if( m_copilotPanel )
    {
        wxAuiPaneInfo& copilotPane = m_auimgr.GetPane( CopilotPanelName() );
        cfg->m_AuiPanels.copilot_panel_show = copilotPane.IsShown();

        if( copilotPane.IsDocked() )
        {
            cfg->m_AuiPanels.copilot_panel_docked_width = m_copilotPanel->GetSize().x;
            cfg->m_AuiPanels.copilot_panel_docked_height = m_copilotPanel->GetSize().y;
        }
        else
        {
            cfg->m_AuiPanels.copilot_panel_float_height = copilotPane.floating_size.y;
            cfg->m_AuiPanels.copilot_panel_float_width = copilotPane.floating_size.x;
        }
    }
}

void PCB_EDIT_FRAME::LoadCopilotCnf()
{
    PCBNEW_SETTINGS* cfg = GetPcbNewSettings();

    wxCHECK( cfg, /* void */ );

    if( m_copilotPanel )
    {
        wxAuiPaneInfo& copilotPane = m_auimgr.GetPane( CopilotPanelName() );
        copilotPane.Show( cfg->m_AuiPanels.copilot_panel_show );

        if( cfg->m_AuiPanels.copilot_panel_show )
        {
            SetAuiPaneSize( m_auimgr, copilotPane, cfg->m_AuiPanels.copilot_panel_docked_width,
                            cfg->m_AuiPanels.copilot_panel_docked_height );
        }
    }
}

#endif

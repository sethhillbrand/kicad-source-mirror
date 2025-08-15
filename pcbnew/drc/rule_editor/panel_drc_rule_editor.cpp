/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2024 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <widgets/std_bitmap_button.h>
#include <widgets/wx_html_report_box.h>
#include <widgets/paged_dialog.h>

#include <pgm_base.h>
#include <settings/settings_manager.h>
#include <footprint_editor_settings.h>
#include <template_fieldnames.h>
#include <grid_tricks.h>
#include <eda_text.h>
#include <grid_layer_box_helpers.h>
#include <bitmaps.h>
#include <confirm.h>
#include <kidialog.h>
#include <layer_ids.h>
#include <layer_range.h>
#include <board.h>
#include <idf_parser.h>
#include <scintilla_tricks.h>
#include <wx/stc/stc.h>
#include <dialogs/html_message_box.h>
#include <tools/drc_tool.h>
#include <pcbexpr_evaluator.h>

#include <drc/drc_rule_parser.h>
#include <drc/rule_editor/panel_drc_rule_editor.h>
#include <drc/rule_editor/drc_re_numeric_input_panel.h>
#include <drc/rule_editor/drc_re_bool_input_panel.h>
#include <drc/rule_editor/drc_re_via_style_panel.h>
#include <drc/rule_editor/drc_re_abs_length_two_panel.h>
#include <drc/rule_editor/drc_re_min_txt_ht_th_panel.h>
#include <drc/rule_editor/drc_re_rtg_diff_pair_panel.h>
#include <drc/rule_editor/drc_re_routing_width_panel.h>
#include <drc/rule_editor/drc_re_parallel_limit_panel.h>
#include <drc/rule_editor/drc_re_permitted_layers_panel.h>
#include <drc/rule_editor/drc_re_allowed_orientation_panel.h>
#include <drc/rule_editor/drc_re_corner_style_panel.h>
#include <drc/rule_editor/drc_re_smd_entry_panel.h>
#include <drc/rule_editor/drc_re_custom_rule_panel.h>
#include <drc/rule_editor/drc_re_object_selector_panel.h>
#include "drc_re_numeric_input_constraint_data.h"
#include "drc_re_bool_input_constraint_data.h"
#include "drc_re_via_style_constraint_data.h"
#include "drc_re_abs_length_two_constraint_data.h"
#include "drc_re_min_txt_ht_th_constraint_data.h"
#include "drc_re_rtg_diff_pair_constraint_data.h"
#include "drc_re_routing_width_constraint_data.h"
#include "drc_re_parallel_limit_constraint_data.h"
#include "drc_re_permitted_layers_constraint_data.h"
#include "drc_re_allowed_orientation_constraint_data.h"
#include "drc_re_corner_style_constraint_data.h"
#include "drc_re_smd_entry_constraint_data.h"
#include "drc_re_custom_rule_constraint_data.h"

static bool constraintNeedsTwoObjects( DRC_RULE_EDITOR_CONSTRAINT_NAME aConstraintType )
{
    switch( aConstraintType )
    {
    case BASIC_CLEARANCE:
    case BOARD_OUTLINE_CLEARANCE:
    case MINIMUM_CLEARANCE:
    case MINIMUM_ITEM_CLEARANCE:
    case CREEPAGE_DISTANCE:
    case COPPER_TO_HOLE_CLEARANCE:
    case HOLE_TO_HOLE_CLEARANCE:
    case PHYSICAL_CLEARANCE:
    case HOLE_TO_HOLE_DISTANCE:
    case PARALLEL_LIMIT:
    case ABSOLUTE_LENGTH_2:
    case DAISY_CHAIN_STUB_2: return true;
    default: return false;
    }
}

PANEL_DRC_RULE_EDITOR::PANEL_DRC_RULE_EDITOR( wxWindow* aParent, BOARD* aBoard,
                                              DRC_RULE_EDITOR_CONSTRAINT_NAME              aConstraintType,
                                              wxString*                                    aConstraintTitle,
                                              std::shared_ptr<DRC_RE_BASE_CONSTRAINT_DATA> aConstraintData ) :
        PANEL_DRC_RULE_EDITOR_BASE( aParent ),
        m_board( aBoard ),
        m_constraintTitle( aConstraintTitle ),
        m_validationSucceeded( false ),
        m_constraintData( aConstraintData ),
        m_helpWindow( nullptr )
{
    m_constraintPanel = getConstraintPanel( this, aConstraintType );
    m_constraintContentSizer->Add( dynamic_cast<wxPanel*>( m_constraintPanel ), 0, wxEXPAND | wxTOP, 5 );
    m_layerList = m_board->GetEnabledLayers().UIOrder();
    m_constraintHeaderTitle->SetLabelText( *aConstraintTitle + " Constraint" );

    std::vector<PCB_LAYER_ID> layerIDs = m_layerList;

    auto layerNameGetter = [this]( PCB_LAYER_ID layer )
    {
        return m_board->GetLayerName( layer );
    };

    m_layerListCmbCtrl = new DRC_RE_LAYER_SELECTION_COMBO( this, layerIDs, layerNameGetter );
    m_LayersComboBoxSizer->Add( m_layerListCmbCtrl, 0, wxALL | wxEXPAND, 5 ); // Remove wxEXPAND


    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );
    m_btnShowMatches = new wxButton( this, wxID_ANY, "Show Matches" );
    buttonSizer->Add( m_btnShowMatches, 0, wxALL, 5 );

    bContentSizer->Add( buttonSizer, 0, wxALIGN_RIGHT | wxALL, 2 );

    m_btnShowMatches->Bind( wxEVT_BUTTON, &PANEL_DRC_RULE_EDITOR::onShowMatchesButtonClicked, this );

    m_btnShowMatches->Enable( !m_constraintData->IsNew() );

    m_checkSyntaxBtnCtrl->SetBitmap( KiBitmapBundle( BITMAPS::drc ) );

    m_scintillaTricks = std::make_unique<SCINTILLA_TRICKS>(
            m_textConditionCtrl, wxT( "()" ), false,
            // onAcceptFn
            [this]( wxKeyEvent& aEvent )
            {
                wxPostEvent( PAGED_DIALOG::GetDialog( this ), wxCommandEvent( wxEVT_COMMAND_BUTTON_CLICKED, wxID_OK ) );
            },
            // onCharFn
            [this]( wxStyledTextEvent& aEvent )
            {
                onScintillaCharAdded( aEvent );
            } );

    m_textConditionCtrl->AutoCompSetSeparator( '|' );

    // Build condition selection panels
    m_conditionControlsSizer->Detach( m_textConditionCtrl );

    bool twoObjects = constraintNeedsTwoObjects( aConstraintType );

    wxString labelA = twoObjects ? _( "Where object A matches" ) : _( "Where object matches" );
    m_objectPanelA = new DRC_RE_OBJECT_SELECTOR_PANEL( this, m_board, labelA );
    m_objectPanelA->SetCustomQueryCtrl( m_textConditionCtrl );
    m_conditionControlsSizer->Insert( 0, m_objectPanelA, 0, wxEXPAND | wxBOTTOM, 5 );

    m_textConditionCtrlB =
            new wxStyledTextCtrl( this, wxID_ANY, wxDefaultPosition, wxSize( -1, 60 ), 0, wxEmptyString );
    m_objectPanelB = new DRC_RE_OBJECT_SELECTOR_PANEL( this, m_board, _( "Where object B matches" ) );
    m_objectPanelB->SetCustomQueryCtrl( m_textConditionCtrlB );
    m_conditionControlsSizer->Insert( 1, m_objectPanelB, 0, wxEXPAND | wxBOTTOM, 5 );
    m_objectPanelB->Show( twoObjects );
    if( !twoObjects )
        m_objectPanelB->Hide();

    m_netClassRegex.Compile( "^NetClass\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_netNameRegex.Compile( "^NetName\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_typeRegex.Compile( "^Type\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_viaTypeRegex.Compile( "^Via_Type\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_padTypeRegex.Compile( "^Pad_Type\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_pinTypeRegex.Compile( "^Pin_Type\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_fabPropRegex.Compile( "^Fabrication_Property\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_shapeRegex.Compile( "^Shape\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_padShapeRegex.Compile( "^Pad_Shape\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_padConnectionsRegex.Compile( "^Pad_Connections\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_zoneConnStyleRegex.Compile( "^Zone_Connection_Style\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_lineStyleRegex.Compile( "^Line_Style\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_hJustRegex.Compile( "^Horizontal_Justification\\s*[!=]=\\s*$", wxRE_ADVANCED );
    m_vJustRegex.Compile( "^Vertical_Justification\\s*[!=]=\\s*$", wxRE_ADVANCED );
}


PANEL_DRC_RULE_EDITOR::~PANEL_DRC_RULE_EDITOR()
{
    m_board = nullptr;
    m_constraintTitle = nullptr;

    m_btnShowMatches->Unbind( wxEVT_BUTTON, &PANEL_DRC_RULE_EDITOR::onShowMatchesButtonClicked, this );
}


bool PANEL_DRC_RULE_EDITOR::TransferDataToWindow()
{
    if( m_constraintData )
    {
        m_nameCtrl->SetValue( m_constraintData->GetRuleName() );
        m_commentCtrl->SetValue( m_constraintData->GetComment() );
        m_layerListCmbCtrl->SetItemsSelected( m_constraintData->GetLayers() );
        wxString cond = m_constraintData->GetRuleCondition();

        // Reset object selector panels
        m_objectPanelA->ParseCondition( wxEmptyString, "A" );
        m_objectPanelB->ParseCondition( wxEmptyString, "B" );

        // Split condition into A-only and B-only expressions
        wxString aExpr, bExpr;
        bool     complex = false;

        if( !cond.IsEmpty() )
        {
            wxString remaining = cond;

            while( true )
            {
                int idx = remaining.Find( "&&" );

                wxString part = idx == wxNOT_FOUND ? remaining : remaining.Left( idx );
                part.Trim( true ).Trim( false );

                bool hasA = part.Contains( "A." );
                bool hasB = part.Contains( "B." );

                if( hasA && !hasB )
                {
                    if( !aExpr.IsEmpty() )
                    {
                        complex = true;
                        break;
                    }

                    aExpr = part;
                }
                else if( hasB && !hasA )
                {
                    if( !bExpr.IsEmpty() )
                    {
                        complex = true;
                        break;
                    }

                    bExpr = part;
                }
                else if( !part.IsEmpty() )
                {
                    complex = true;
                    break;
                }

                if( idx == wxNOT_FOUND )
                    break;

                remaining = remaining.Mid( idx + 2 );
            }
        }

        if( complex )
        {
            m_objectPanelA->ParseCondition( cond );
            m_objectPanelB->ParseCondition( wxEmptyString, "B" );
        }
        else
        {
            m_objectPanelA->ParseCondition( aExpr, "A" );
            m_objectPanelB->ParseCondition( bExpr, "B" );
        }
    }

    return dynamic_cast<wxPanel*>( m_constraintPanel )->TransferDataToWindow();
}


bool PANEL_DRC_RULE_EDITOR::TransferDataFromWindow()
{
    dynamic_cast<wxPanel*>( m_constraintPanel )->TransferDataFromWindow();

    m_constraintData->SetRuleName( m_nameCtrl->GetValue() );
    m_constraintData->SetComment( m_commentCtrl->GetValue() );
    m_constraintData->SetLayers( m_layerListCmbCtrl->GetSelectedLayers() );
    wxString aCond = m_objectPanelA->BuildCondition( "A" );
    wxString bCond = m_objectPanelB->BuildCondition( "B" );

    wxString combined = aCond;

    if( !bCond.IsEmpty() )
    {
        if( !combined.IsEmpty() )
            combined += " && ";

        combined += bCond;
    }

    m_constraintData->SetRuleCondition( combined );

    return true;
}


DRC_RULE_EDITOR_CONTENT_PANEL_BASE*
PANEL_DRC_RULE_EDITOR::getConstraintPanel( wxWindow* aParent, const DRC_RULE_EDITOR_CONSTRAINT_NAME& aConstraintType )
{
    switch( aConstraintType )
    {
    case VIA_STYLE:
        return new DRC_RE_VIA_STYLE_PANEL( aParent, m_constraintTitle,
                                           dynamic_pointer_cast<DRC_RE_VIA_STYLE_CONSTRAINT_DATA>( m_constraintData ) );
    case ABSOLUTE_LENGTH_2:
        return new DRC_RE_ABSOLUTE_LENGTH_TWO_PANEL(
                aParent, m_constraintTitle,
                dynamic_pointer_cast<DRC_RE_ABSOLUTE_LENGTH_TWO_CONSTRAINT_DATA>( m_constraintData ) );
    case MINIMUM_TEXT_HEIGHT_AND_THICKNESS:
        return new DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_PANEL(
                aParent, m_constraintTitle,
                dynamic_pointer_cast<DRC_RE_MINIMUM_TEXT_HEIGHT_THICKNESS_CONSTRAINT_DATA>( m_constraintData ) );
    case ROUTING_DIFF_PAIR:
        return new DRC_RE_ROUTING_DIFF_PAIR_PANEL(
                aParent, m_constraintTitle,
                dynamic_pointer_cast<DRC_RE_ROUTING_DIFF_PAIR_CONSTRAINT_DATA>( m_constraintData ) );
    case ROUTING_WIDTH:
        return new DRC_RE_ROUTING_WIDTH_PANEL(
                aParent, m_constraintTitle,
                dynamic_pointer_cast<DRC_RE_ROUTING_WIDTH_CONSTRAINT_DATA>( m_constraintData ) );
    case PARALLEL_LIMIT:
        return new DRC_RE_PARALLEL_LIMIT_PANEL(
                aParent, m_constraintTitle,
                dynamic_pointer_cast<DRC_RE_PARALLEL_LIMIT_CONSTRAINT_DATA>( m_constraintData ) );
    case PERMITTED_LAYERS:
        return new DRC_RE_PERMITTED_LAYERS_PANEL(
                aParent, m_constraintTitle,
                dynamic_pointer_cast<DRC_RE_PERMITTED_LAYERS_CONSTRAINT_DATA>( m_constraintData ) );
    case ALLOWED_ORIENTATION:
        return new DRC_RE_ALLOWED_ORIENTATION_PANEL(
                aParent, m_constraintTitle,
                dynamic_pointer_cast<DRC_RE_ALLOWED_ORIENTATION_CONSTRAINT_DATA>( m_constraintData ) );
    case CORNER_STYLE:
        return new DRC_RE_CORNER_STYLE_PANEL(
                aParent, m_constraintTitle,
                dynamic_pointer_cast<DRC_RE_CORNER_STYLE_CONSTRAINT_DATA>( m_constraintData ) );
    case SMD_ENTRY:
        return new DRC_RE_SMD_ENTRY_PANEL( aParent, m_constraintTitle,
                                           dynamic_pointer_cast<DRC_RE_SMD_ENTRY_CONSTRAINT_DATA>( m_constraintData ) );
    case CUSTOM_RULE:
        return new DRC_RE_CUSTOM_RULE_PANEL(
                aParent, dynamic_pointer_cast<DRC_RE_CUSTOM_RULE_CONSTRAINT_DATA>( m_constraintData ) );
    default:
        if( DRC_RULE_EDITOR_UTILS::IsNumericInputType( aConstraintType ) )
        {
            wxString customLabel;

            switch( aConstraintType )
            {
            case NET_ANTENNA: customLabel = "Net Antenna Tolerance"; break;
            case DAISY_CHAIN_STUB: customLabel = "Maximum Stub Length"; break;
            case ABSOLUTE_LENGTH: customLabel = "Net Length"; break;
            case SMD_CORNER: customLabel = "Minimum Permissible Distance"; break;
            case SMD_TO_PLANE_PLUS: customLabel = "Maximum Permissible Distance"; break;
            default: customLabel = *m_constraintTitle;
            }

            DRC_RE_NUMERIC_INPUT_CONSTRAINT_PANEL_PARAMS numericInputParams(
                    *m_constraintTitle, dynamic_pointer_cast<DRC_RE_NUMERIC_INPUT_CONSTRAINT_DATA>( m_constraintData ),
                    aConstraintType, customLabel );

            if( aConstraintType == MINIMUM_THERMAL_RELIEF_SPOKE_COUNT || aConstraintType == MAXIMUM_VIA_COUNT )
                numericInputParams.SetInputIsCount( true );

            return new DRC_RE_NUMERIC_INPUT_PANEL( aParent, numericInputParams );
        }
        else if( DRC_RULE_EDITOR_UTILS::IsBoolInputType( aConstraintType ) )
        {
            wxString customLabel;

            switch( aConstraintType )
            {
            case SHORT_CIRCUIT: customLabel = "Allow Short Circuit"; break;
            case UNROUTED: customLabel = "Check for incomplete connections"; break;
            case VIAS_UNDER_SMD: customLabel = "Allow Vias under SMD Pads"; break;
            default: customLabel = *m_constraintTitle;
            }

            DRC_RE_BOOL_INPUT_CONSTRAINT_PANEL_PARAMS boolInputParams(
                    *m_constraintTitle, dynamic_pointer_cast<DRC_RE_BOOL_INPUT_CONSTRAINT_DATA>( m_constraintData ),
                    aConstraintType, customLabel );

            return new DRC_RE_BOOL_INPUT_PANEL( aParent, boolInputParams );
        }
        else
        {
            return nullptr;
        };
    }
}


bool PANEL_DRC_RULE_EDITOR::ValidateInputs( int* aErrorCount, std::string* aValidationMessage )
{
    if( !m_callBackRuleNameValidation( m_constraintData->GetId(), m_nameCtrl->GetValue() ) )
    {
        m_validationSucceeded = false;
        ( *aErrorCount )++;
        m_validationMessage +=
                DRC_RULE_EDITOR_UTILS::FormatErrorMessage( *aErrorCount, "Rule Name should be unique !!" );
    }

    if( m_layerListCmbCtrl->GetSelectedItemsString() == wxEmptyString )
    {
        m_validationSucceeded = false;
        ( *aErrorCount )++;
        m_validationMessage +=
                DRC_RULE_EDITOR_UTILS::FormatErrorMessage( *aErrorCount, "Layers selection should not be empty !!" );
    }

    return m_validationSucceeded;
}


void PANEL_DRC_RULE_EDITOR::onSaveButtonClicked( wxCommandEvent& aEvent )
{
    m_validationSucceeded = true;
    int errorCount = 0;
    m_validationMessage = "";

    ValidateInputs( &errorCount, &m_validationMessage );

    if( !m_constraintPanel->ValidateInputs( &errorCount, &m_validationMessage ) )
    {
        m_validationSucceeded = false;
    }

    if( m_callBackSave )
    {
        m_callBackSave( m_constraintData->GetId() );
    }

    if( m_validationSucceeded )
        m_btnShowMatches->Enable( true );
}


void PANEL_DRC_RULE_EDITOR::Save( wxCommandEvent& aEvent )
{
    onSaveButtonClicked( aEvent );
}


void PANEL_DRC_RULE_EDITOR::onRemoveButtonClicked( wxCommandEvent& aEvent )
{
    if( m_callBackRemove && m_constraintData )
        m_callBackRemove( m_constraintData->GetId() );
}


void PANEL_DRC_RULE_EDITOR::Cancel( wxCommandEvent& aEvent )
{
    if( m_constraintData && m_constraintData->IsNew() )
        onRemoveButtonClicked( aEvent );
    else
        onCloseButtonClicked( aEvent );
}


void PANEL_DRC_RULE_EDITOR::onCloseButtonClicked( wxCommandEvent& aEvent )
{
    if( m_callBackClose && m_constraintData )
        m_callBackClose( m_constraintData->GetId() );
}


void PANEL_DRC_RULE_EDITOR::onScintillaCharAdded( wxStyledTextEvent& aEvent )
{
    if( aEvent.GetModifiers() == wxMOD_CONTROL && aEvent.GetKey() == ' ' )
    {
        // This is just a short-cut for do-auto-complete
    }

    m_textConditionCtrl->SearchAnchor();

    wxString rules = m_textConditionCtrl->GetText();
    int      currentPos = m_textConditionCtrl->GetCurrentPos();
    int      startPos = 0;

    for( int line = m_textConditionCtrl->LineFromPosition( currentPos ); line > 0; line-- )
    {
        int      lineStart = m_textConditionCtrl->PositionFromLine( line );
        wxString beginning = m_textConditionCtrl->GetTextRange( lineStart, lineStart + 10 );

        if( beginning.StartsWith( wxT( "(condition " ) ) )
        {
            startPos = lineStart;
            break;
        }
    }

    enum class EXPR_CONTEXT_T : int
    {
        NONE,
        STRING,
        SEXPR_OPEN,
        SEXPR_TOKEN,
        SEXPR_STRING,
        STRUCT_REF
    };

    std::stack<wxString> sexprs;
    wxString             partial;
    wxString             last;
    wxString             constraintType;
    EXPR_CONTEXT_T       context = EXPR_CONTEXT_T::NONE;
    EXPR_CONTEXT_T       expr_context = EXPR_CONTEXT_T::NONE;

    for( int i = startPos; i < currentPos; ++i )
    {
        wxChar c = m_textConditionCtrl->GetCharAt( i );

        if( c == '\\' )
        {
            i++; // skip escaped char
        }
        else if( context == EXPR_CONTEXT_T::STRING )
        {
            if( c == '"' )
            {
                context = EXPR_CONTEXT_T::NONE;
            }
            else
            {
                if( expr_context == EXPR_CONTEXT_T::STRING )
                {
                    if( c == '\'' )
                        expr_context = EXPR_CONTEXT_T::NONE;
                    else
                        partial += c;
                }
                else if( c == '\'' )
                {
                    last = partial;
                    partial = wxEmptyString;
                    expr_context = EXPR_CONTEXT_T::STRING;
                }
                else if( c == '.' )
                {
                    partial = wxEmptyString;
                    expr_context = EXPR_CONTEXT_T::STRUCT_REF;
                }
                else
                {
                    partial += c;
                }
            }
        }
        else if( c == '"' )
        {
            last = partial;
            partial = wxEmptyString;
            context = EXPR_CONTEXT_T::STRING;
        }
        else if( c == '(' )
        {
            if( context == EXPR_CONTEXT_T::SEXPR_OPEN && !partial.IsEmpty() )
            {
                m_textConditionCtrl->AutoCompCancel();
                sexprs.push( partial );
            }

            partial = wxEmptyString;
            context = EXPR_CONTEXT_T::SEXPR_OPEN;
        }
        else if( c == ')' )
        {
            context = EXPR_CONTEXT_T::NONE;
        }
        else if( c == ' ' )
        {
            if( context == EXPR_CONTEXT_T::SEXPR_OPEN && !partial.IsEmpty() )
            {
                m_textConditionCtrl->AutoCompCancel();
                sexprs.push( partial );

                if( partial == wxT( "condition" ) )
                {
                    context = EXPR_CONTEXT_T::SEXPR_STRING;
                }
                else
                {
                    context = EXPR_CONTEXT_T::NONE;
                }

                partial = wxEmptyString;
                continue;
            }

            context = EXPR_CONTEXT_T::NONE;
        }
        else
        {
            partial += c;
        }
    }

    wxString tokens;

    if( context == EXPR_CONTEXT_T::SEXPR_OPEN )
    {
        if( sexprs.empty() )
        {
            tokens = wxT( "condition" );
        }
    }
    else if( context == EXPR_CONTEXT_T::SEXPR_TOKEN )
    {
        if( sexprs.empty() )
        {
            /* badly formed grammar */
        }
    }
    else if( context == EXPR_CONTEXT_T::SEXPR_STRING && !sexprs.empty() && sexprs.top() == wxT( "condition" ) )
    {
        m_textConditionCtrl->AddText( wxT( "\"" ) );
    }
    else if( context == EXPR_CONTEXT_T::STRING && !sexprs.empty() && sexprs.top() == wxT( "condition" ) )
    {
        if( expr_context == EXPR_CONTEXT_T::STRUCT_REF )
        {
            PROPERTY_MANAGER&  propMgr = PROPERTY_MANAGER::Instance();
            std::set<wxString> propNames;

            for( const PROPERTY_MANAGER::CLASS_INFO& cls : propMgr.GetAllClasses() )
            {
                const std::vector<PROPERTY_BASE*>& props = propMgr.GetProperties( cls.type );

                for( PROPERTY_BASE* prop : props )
                {
                    // TODO: It would be nice to replace IsHiddenFromRulesEditor with a nickname
                    // system, so that two different properies don't need to be created.  This is
                    // a bigger change than I want to make right now, though.
                    if( prop->IsHiddenFromRulesEditor() )
                        continue;

                    wxString ref( prop->Name() );
                    ref.Replace( wxT( " " ), wxT( "_" ) );
                    propNames.insert( ref );
                }
            }

            for( const wxString& propName : propNames )
                tokens += wxT( "|" ) + propName;

            PCBEXPR_BUILTIN_FUNCTIONS& functions = PCBEXPR_BUILTIN_FUNCTIONS::Instance();

            for( const wxString& funcSig : functions.GetSignatures() )
            {
                if( !funcSig.Contains( "DEPRECATED" ) )
                    tokens += wxT( "|" ) + funcSig;
            }
        }
        else if( expr_context == EXPR_CONTEXT_T::STRING )
        {
            if( m_netClassRegex.Matches( last ) )
            {
                BOARD_DESIGN_SETTINGS&         bds = m_board->GetDesignSettings();
                std::shared_ptr<NET_SETTINGS>& netSettings = bds.m_NetSettings;

                for( const auto& [name, netclass] : netSettings->GetNetclasses() )
                    tokens += wxT( "|" ) + name;
            }
            else if( m_netNameRegex.Matches( last ) )
            {
                for( const wxString& netnameCandidate : m_board->GetNetClassAssignmentCandidates() )
                    tokens += wxT( "|" ) + netnameCandidate;
            }
            else if( m_typeRegex.Matches( last ) )
            {
                tokens = wxT( "Bitmap|"
                              "Dimension|"
                              "Footprint|"
                              "Graphic|"
                              "Group|"
                              "Leader|"
                              "Pad|"
                              "Target|"
                              "Text|"
                              "Text Box|"
                              "Track|"
                              "Via|"
                              "Zone" );
            }
            else if( m_viaTypeRegex.Matches( last ) )
            {
                tokens = wxT( "Through|"
                              "Blind/buried|"
                              "Micro" );
            }
            else if( m_padTypeRegex.Matches( last ) )
            {
                tokens = wxT( "Through-hole|"
                              "SMD|"
                              "Edge connector|"
                              "NPTH, mechanical" );
            }
            else if( m_pinTypeRegex.Matches( last ) )
            {
                tokens = wxT( "Input|"
                              "Output|"
                              "Bidirectional|"
                              "Tri-state|"
                              "Passive|"
                              "Free|"
                              "Unspecified|"
                              "Power input|"
                              "Power output|"
                              "Open collector|"
                              "Open emitter|"
                              "Unconnected" );
            }
            else if( m_fabPropRegex.Matches( last ) )
            {
                tokens = wxT( "None|"
                              "BGA pad|"
                              "Fiducial, global to board|"
                              "Fiducial, local to footprint|"
                              "Test point pad|"
                              "Heatsink pad|"
                              "Castellated pad" );
            }
            else if( m_shapeRegex.Matches( last ) )
            {
                tokens = wxT( "Segment|"
                              "Rectangle|"
                              "Arc|"
                              "Circle|"
                              "Polygon|"
                              "Bezier" );
            }
            else if( m_padShapeRegex.Matches( last ) )
            {
                tokens = wxT( "Circle|"
                              "Rectangle|"
                              "Oval|"
                              "Trapezoid|"
                              "Rounded rectangle|"
                              "Chamfered rectangle|"
                              "Custom" );
            }
            else if( m_padConnectionsRegex.Matches( last ) )
            {
                tokens = wxT( "Inherited|"
                              "None|"
                              "Solid|"
                              "Thermal reliefs|"
                              "Thermal reliefs for PTH" );
            }
            else if( m_zoneConnStyleRegex.Matches( last ) )
            {
                tokens = wxT( "Inherited|"
                              "None|"
                              "Solid|"
                              "Thermal reliefs" );
            }
            else if( m_lineStyleRegex.Matches( last ) )
            {
                tokens = wxT( "Default|"
                              "Solid|"
                              "Dashed|"
                              "Dotted|"
                              "Dash-Dot|"
                              "Dash-Dot-Dot" );
            }
            else if( m_hJustRegex.Matches( last ) )
            {
                tokens = wxT( "Left|"
                              "Center|"
                              "Right" );
            }
            else if( m_vJustRegex.Matches( last ) )
            {
                tokens = wxT( "Top|"
                              "Center|"
                              "Bottom" );
            }
        }
    }

    if( !tokens.IsEmpty() )
        m_scintillaTricks->DoAutocomplete( partial, wxSplit( tokens, '|' ) );
}


void PANEL_DRC_RULE_EDITOR::onSyntaxHelp( wxHyperlinkEvent& aEvent )
{
    if( m_helpWindow )
    {
        m_helpWindow->ShowModeless();
        return;
    }

    std::vector<wxString> msg;
    msg.clear();

    wxString t =
#include <panel_setup_condition_help_1clauses.h>
            ;
    msg.emplace_back( t );

    t =
#include <panel_setup_rules_help_3items.h>
            ;
    msg.emplace_back( t );

    t =
#include "dialogs/panel_setup_rules_help_5examples.h"
            ;
    msg.emplace_back( t );

    t =
#include "dialogs/panel_setup_rules_help_6notes.h"
            ;
    msg.emplace_back( t );

    t =
#include "dialogs/panel_setup_rules_help_7properties.h"
            ;
    msg.emplace_back( t );

    t =
#include "dialogs/panel_setup_rules_help_8expression_functions.h"
            ;
    msg.emplace_back( t );

    t =
#include "dialogs/panel_setup_rules_help_9more_examples.h"
            ;
    msg.emplace_back( t );

    t =
#include "dialogs/panel_setup_rules_help_10documentation.h"
            ;
    msg.emplace_back( t );


    wxString msg_txt = wxEmptyString;

    for( wxString i : msg )
        msg_txt << wxGetTranslation( i );

#ifdef __WXMAC__
    msg_txt.Replace( wxT( "Ctrl+" ), wxT( "Cmd+" ) );
#endif
    const wxString& msGg_txt = msg_txt;

    m_helpWindow = new HTML_MESSAGE_BOX( this, _( "Syntax Help" ) );
    m_helpWindow->SetDialogSizeInDU( 420, 320 );

    wxString html_txt = wxEmptyString;
    ConvertMarkdown2Html( msGg_txt, html_txt );

    html_txt.Replace( wxS( "<td" ), wxS( "<td valign=top" ) );
    m_helpWindow->AddHTML_Text( html_txt );

    m_helpWindow->ShowModeless();
}


void PANEL_DRC_RULE_EDITOR::onCheckSyntax( wxCommandEvent& event )
{
    m_syntaxErrorReport->Clear();

    if( m_textConditionCtrl->GetText().IsEmpty() )
    {
        wxString msg = _( "ERROR: No text provided for validation." );
        m_syntaxErrorReport->Report( msg, RPT_SEVERITY_ERROR );
        m_syntaxErrorReport->Flush();
        return;
    }


    try
    {
        std::vector<std::shared_ptr<DRC_RULE>> dummyRules;

        DRC_RULES_PARSER conditionParser( m_textConditionCtrl->GetText(), _( "DRC rule condition" ) );

        // if( conditionParser.VerifyParseCondition( m_syntaxErrorReport ) )
        {
            wxString         ruleTemplate = L"(version 1)\n(rule default\n   %s\n)";
            wxString         formattedRule = wxString::Format( ruleTemplate, m_textConditionCtrl->GetText() );
            DRC_RULES_PARSER ruleParser( formattedRule, _( "DRC rule" ) );
            ruleParser.Parse( dummyRules, m_syntaxErrorReport );
        }
    }
    catch( PARSE_ERROR& pe )
    {
        wxString msg = wxString::Format( wxT( "%s <a href='%d:%d'>%s</a>%s" ), _( "ERROR:" ), pe.lineNumber,
                                         pe.byteIndex, pe.ParseProblem(), wxEmptyString );

        m_syntaxErrorReport->Report( msg, RPT_SEVERITY_ERROR );
    }

    m_syntaxErrorReport->Flush();
}


void PANEL_DRC_RULE_EDITOR::onErrorLinkClicked( wxHtmlLinkEvent& event )
{
    wxString      link = event.GetLinkInfo().GetHref();
    wxArrayString parts;
    long          line = 0, offset = 0;

    wxStringSplit( link, parts, ':' );

    if( parts.size() > 1 )
    {
        parts[0].ToLong( &line );
        parts[1].ToLong( &offset );
    }

    int pos = m_textConditionCtrl->PositionFromLine( line - 1 ) + ( offset - 1 );

    m_textConditionCtrl->GotoPos( pos );

    m_textConditionCtrl->SetFocus();
}


void PANEL_DRC_RULE_EDITOR::onContextMenu( wxMouseEvent& event )
{
    wxMenu menu;

    menu.Append( wxID_UNDO, _( "Undo" ) );
    menu.Append( wxID_REDO, _( "Redo" ) );

    menu.AppendSeparator();

    menu.Append( 1, _( "Cut" ) );  // Don't use wxID_CUT, wxID_COPY, etc.  On Mac (at least),
    menu.Append( 2, _( "Copy" ) ); // wxWidgets never delivers them to us.
    menu.Append( 3, _( "Paste" ) );
    menu.Append( 4, _( "Delete" ) );

    menu.AppendSeparator();

    menu.Append( 5, _( "Select All" ) );

    menu.AppendSeparator();

    menu.Append( wxID_ZOOM_IN, _( "Zoom In" ) );
    menu.Append( wxID_ZOOM_OUT, _( "Zoom Out" ) );


    switch( GetPopupMenuSelectionFromUser( menu ) )
    {
    case wxID_UNDO: m_textConditionCtrl->Undo(); break;
    case wxID_REDO: m_textConditionCtrl->Redo(); break;

    case 1: m_textConditionCtrl->Cut(); break;
    case 2: m_textConditionCtrl->Copy(); break;
    case 3: m_textConditionCtrl->Paste(); break;
    case 4:
    {
        long from, to;
        m_textConditionCtrl->GetSelection( &from, &to );

        if( to > from )
            m_textConditionCtrl->DeleteRange( from, to );

        break;
    }

    case 5: m_textConditionCtrl->SelectAll(); break;

    case wxID_ZOOM_IN: m_textConditionCtrl->ZoomIn(); break;
    case wxID_ZOOM_OUT: m_textConditionCtrl->ZoomOut(); break;
    }
}


void PANEL_DRC_RULE_EDITOR::onShowMatchesButtonClicked( wxCommandEvent& event )
{
    if( m_callBackShowMatches )
    {
        m_callBackShowMatches( m_constraintData->GetId() );
    }
}
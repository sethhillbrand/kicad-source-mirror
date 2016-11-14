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

#ifndef TEARDROPS_EDITOR_H
#define TEARDROPS_EDITOR_H

#include "wxPcbStruct.h"
#include "tools/selection_tool.h"
#include "dialogs/dialog_teardrops.h"
#include "class_teardrop.h"
#include "import_export.h"

/**
 * @brief The TEARDROPS_EDITOR class
 * creates actual tracks on the board in accordance with the preferences provided by
 * the DIALOG_TEARDROPS class.
 */
class APIEXPORT TEARDROPS_EDITOR : public TOOL_BASE
{
public:
    TEARDROPS_EDITOR();
    ~TEARDROPS_EDITOR();

    /**
     * @brief Function \a EditTeardrops
     * is invoked for any manupulation with the teardrops on current board.
     * @param [in] aSettings contains user defined settings provided by teadrops editor dialog window
     * @return bool - \a true in case teardrops were successfully created and \a false otherwise
     */
    bool EditTeardrops( const DIALOG_TEARDROPS::TEARDROPS_SETTINGS& aSettings );

    /// @copydoc TOOL_INTERACTIVE::Reset
    void Reset( RESET_REASON aReason ) override;

private:
    /**
     * The DRC_STRATEGY
     * defines the strategy when DRC violation is detected during teardop creation.
     */
    typedef enum
    {
        /// Do not violate DRC and quit teardrop building
        DRC_COMPLY,
        /// Ignore DRC and finish teardop
        DRC_IGNORE,
        /// Try to adjust the outline or size of a teardop (not implemented)
        DRC_ADJUST
    } DRC_STRATEGY;

    PCB_EDIT_FRAME* m_frame;
    KIGFX::VIEW* m_view;
    TEARDROP::TEARDROP_TYPE m_type;
    PICKED_ITEMS_LIST m_undoListPicker;
    DRC_STRATEGY m_strategy;

    /**
     * @brief Function \a filterSelection
     * filters selected objects and removes all objects which can not be processed.
     * @param [in,out] aSelection contains the list of currently selected objects on input and
     * a list of valid for processing objects on output
     */
    void filterSelection( SELECTION& aSelection );

    /**
     * @brief Function \a iterateTracks
     * creates teardrop(s) for all tracks connected to \a aObject.
     * @param [in] aObject is a board object at which teardrops should be created. Currently such an object can
     * be via or circular pad.
     * @return \a true if at least one teardrop was successfully added and \a false otherwise
     */
    bool iterateTracks( const BOARD_CONNECTED_ITEM* aObject );

    /**
     * @brief Function \a addToAll
     * adds teardrops to all tracks on the board.
     * @param [in] aSettings contains user defined settings
     * @return bool - \a true in case teardops were successfully added and \a false otherwise
     */
    bool addToAll( const DIALOG_TEARDROPS::TEARDROPS_SETTINGS& aSettings );

    /**
     * @brief Function \a addToSelected
     * adds teardrops to selected tracks.
     * @param [in] aSelection contains a filtered list of selected tracks
     * @param [in] aSettings contains user defined settings
     * @return bool - \a true in case teardops were successfully added and \a false otherwise
     */
    bool addToSelected( SELECTION& aSelection, const DIALOG_TEARDROPS::TEARDROPS_SETTINGS& aSettings );

    /**
     * @brief Function \a RemoveAll
     * removes all teardrops form current board.
     */
    void removeAll();

    /**
     * @brief Function \a drawSegments
     * adds tracks composing a teardop to the board.
     * @param [in] aTeardrop is a teardrop which should be created on the board
     * @param [in] aTrack is a parent track, some of its parameters are copied to newly created
     * segments
     * @return bool - \a true in case all the tracks were successfully added and \a false
     * otherwise
     */
    bool drawSegments( TEARDROP& aTeardrop, TRACK& aTrack );
};

#endif    // TEARDROPS_EDITOR_H

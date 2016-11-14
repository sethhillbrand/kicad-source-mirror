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

#ifndef DIALOG_TEARDROPS_H
#define DIALOG_TEARDROPS_H

#include "dialog_shim.h"
#include "dialog_teardrops_base.h"
#include "wxPcbStruct.h"

/**
 * @brief The DIALOG_TEARDROPS class
 * implements teardrop management dialog for current board.
 */
class DIALOG_TEARDROPS : public DIALOG_TEARDROPS_BASE
{
public:
    /**
     * @brief The TEARDROPS_MODE
     * defines an action to be performed on teardrops.
     */
    typedef enum
    {
        /// Teardrops addition mode
        TEARDROPS_MODE_ADD,
        /// Teardrops removal mode
        TEARDROPS_MODE_REMOVE
    } TEARDROPS_MODE;

    /**
     * @brief The TEARDROPS_TRACKS
     * determines selection processing.
     */
    typedef enum
    {
        /// Process all tracks
        TEARDROPS_TRACKS_ALL,
        /// Process selected tracks only
        TEARDROPS_TRACKS_SELECTED
    } TEARDROPS_TRACKS;

    /**
     * @brief The TEARDROPS_TYPE
     * defines the shape of teardrops.
     */
    typedef enum
    {
        /// The shape is not defined
        TEARDROPS_TYPE_NONE = -1,
        /// The teardops have straight outlines
        TEARDROPS_TYPE_STRAIGHT,
        /// The teardrops have curved outlines
        TEARDROPS_TYPE_CURVED
    } TEARDROPS_TYPE;

    /**
     * @brief The TEARDROPS_SCOPE
     * defines the types of objects for which teardrops should be created. This is a bit field, each
     * bit correcponds to an object type.
     */
    typedef enum
    {
        /// No objects are specified
        TEARDROPS_SCOPE_NONE,
        /// Create teardrops for vias
        TEARDROPS_SCOPE_VIAS = 1,
        /// Create teardrops for pads
        TEARDROPS_SCOPE_PADS = 2,
        /// Create teardrops for tracks (not implemented yet)
        TEARDROPS_SCOPE_TRACKS = 4
    } TEARDROPS_SCOPE;

    /**
     * @brief The TEARDROPS_SETTINGS
     * class is a container for all the settings specified by the user.
     */
    typedef struct
    {
        /// The action to be performed (addition, deletion)
        TEARDROPS_MODE m_mode;
        /// Process selection
        TEARDROPS_TRACKS m_track;
        /// Objects scope
        TEARDROPS_SCOPE m_scope;
        /// Teardrops type
        TEARDROPS_TYPE m_type;
        /// Clear selection after the processing has finished
        bool m_clearSelection;
        /// Ignore DRC during processing
        bool m_ignoreDrc;
    } TEARDROPS_SETTINGS;

    DIALOG_TEARDROPS( PCB_EDIT_FRAME *aParent, TEARDROPS_SETTINGS *aSettings );

    void OnModeAdd( wxCommandEvent &aEvent ) override;
    void OnModeRemove( wxCommandEvent &aEvent ) override;
    void OnTracksAll( wxCommandEvent &aEvent ) override;
    void OnTracksSelected( wxCommandEvent &aEvent ) override;
    void OnStyleChanged( wxCommandEvent &aEvent ) override;
    void OnClearSelection( wxCommandEvent &aEvent ) override;
    void OnIgnoreDrc( wxCommandEvent &aEvent ) override;
    void OnScopeVias( wxCommandEvent &aEvent ) override;
    void OnScopePads( wxCommandEvent &aEvent ) override;

private:
    PCB_EDIT_FRAME *m_parent;
    TEARDROPS_SETTINGS *m_settings;

    void initDialogSettings();
    void lockOptionsControls( bool aState );
    void lockTracksControls( bool aState );
    void lockScopeControls( bool aState );
};

#endif // DIALOG_TEARDROPS_H

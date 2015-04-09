#ifndef DIALOG_TEARDROPS_H
#define DIALOG_TEARDROPS_H

#include "dialog_shim.h"
#include "dialog_teardrops_base.h"
#include "wxPcbStruct.h"

class DIALOG_TEARDROPS : public DIALOG_TEARDROPS_BASE
{
public:
    typedef enum {
        TEARDROPS_MODE_ADD,
        TEARDROPS_MODE_REMOVE
    } TEARDROPS_MODE;
    typedef enum {
        TEARDROPS_TRACKS_ALL,
        TEARDROPS_TRACKS_SELECTED
    } TEARDROPS_TRACKS;
    typedef enum {
        TEARDROPS_TYPE_NONE = -1,
        TEARDROPS_TYPE_STRAIGHT,
        TEARDROPS_TYPE_CURVED
    } TEARDROPS_TYPE;
    typedef enum {
        TEARDROPS_SCOPE_NONE,
        TEARDROPS_SCOPE_VIAS = 1,
        TEARDROPS_SCOPE_PADS = 2,
        TEARDROPS_SCOPE_TRACKS = 4
    } TEARDROPS_SCOPE;
    typedef struct {
        TEARDROPS_MODE m_mode;
        TEARDROPS_TRACKS m_track;
        TEARDROPS_SCOPE m_scope;
        TEARDROPS_TYPE m_type;
        bool m_clearSelection;
    } TEARDROPS_SETTINGS;
    DIALOG_TEARDROPS(PCB_EDIT_FRAME *aParent, TEARDROPS_SETTINGS *settings);

    void OnModeAdd(wxCommandEvent &event);
    void OnModeRemove(wxCommandEvent &event);
    void OnTracksAll(wxCommandEvent &event);
    void OnTracksSelected(wxCommandEvent &event);
    void OnStyleChanged(wxCommandEvent &event);
    void OnClearSelection(wxCommandEvent &event);
    void OnScopeVias(wxCommandEvent &event);

private:
    PCB_EDIT_FRAME *m_parent;
    TEARDROPS_SETTINGS *m_settings;

    void InitDialogSettings();
};

#endif // DIALOG_TEARDROPS_H

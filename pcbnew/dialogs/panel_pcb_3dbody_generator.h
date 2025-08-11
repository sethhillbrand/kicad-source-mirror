#ifndef PANEL_PCB_3DBODY_GENERATOR_H
#define PANEL_PCB_3DBODY_GENERATOR_H

#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/spinbutt.h>
#include <vector>

#include <pcb_3dbody.h>
#include <panel_preview_3d_model.h>

class PCB_BASE_EDIT_FRAME;
class FOOTPRINT;

class PANEL_PCB_3DBODY_GENERATOR : public wxPanel
{
public:
    PANEL_PCB_3DBODY_GENERATOR( PCB_BASE_EDIT_FRAME* aFrame, FOOTPRINT* aFootprint,
                                wxWindow* aParent, wxWindowID id = wxID_ANY );

private:
    void onShapeChanged( wxCommandEvent& );
    void onParamText( wxCommandEvent& );
    void onRadiusSpinUp( wxSpinEvent& );
    void onRadiusSpinDown( wxSpinEvent& );
    void onLengthSpinUp( wxSpinEvent& );
    void onLengthSpinDown( wxSpinEvent& );
    void onWidthSpinUp( wxSpinEvent& );
    void onWidthSpinDown( wxSpinEvent& );
    void onHeightSpinUp( wxSpinEvent& );
    void onHeightSpinDown( wxSpinEvent& );

    void updateModel();
    void updateVisibility();

    PCB_3DBODY                m_body;
    std::vector<FP_3DMODEL>   m_models;
    PANEL_PREVIEW_3D_MODEL*   m_preview;
    wxString                  m_tempFile;

    wxChoice*     m_shapeChoice;
    wxTextCtrl*   m_radiusCtrl;
    wxSpinButton* m_spinRadius;
    wxTextCtrl*   m_lengthCtrl;
    wxSpinButton* m_spinLength;
    wxTextCtrl*   m_widthCtrl;
    wxSpinButton* m_spinWidth;
    wxTextCtrl*   m_heightCtrl;
    wxSpinButton* m_spinHeight;
};

#endif // PANEL_PCB_3DBODY_GENERATOR_H

#ifndef DIALOG_SIM_AXIS_PROPERTIES_H
#define DIALOG_SIM_AXIS_PROPERTIES_H

#include <wx/dialog.h>
#include <wx/string.h>

#include <utility>
#include <vector>

class wxCheckBox;
class wxTextCtrl;
class wxChoice;

struct SIM_AXIS_DIALOG_SETTINGS
{
    wxString axisName;
    bool     isHorizontal;
    double   minValue;
    double   maxValue;
    bool     boundsLocked;
    bool     logSupported;
    bool     isLogarithmic;
    bool     showUnits;
    std::vector<std::pair<int, wxString>> alignments;
    int currentAlignment;
};

struct SIM_AXIS_DIALOG_RESULTS
{
    double minValue;
    double maxValue;
    bool   boundsLocked;
    bool   isLogarithmic;
    bool   showUnits;
    int    alignment;
};

class DIALOG_SIM_AXIS_PROPERTIES : public wxDialog
{
public:
    DIALOG_SIM_AXIS_PROPERTIES( wxWindow* aParent, const SIM_AXIS_DIALOG_SETTINGS& aSettings );

    void GetResults( SIM_AXIS_DIALOG_RESULTS& aResults ) const;

private:
    void onAutoBounds( wxCommandEvent& aEvent );
    bool TransferDataFromWindow() override;

private:
    wxCheckBox* m_autoBounds;
    wxTextCtrl* m_minValue;
    wxTextCtrl* m_maxValue;
    wxCheckBox* m_logScale;
    wxCheckBox* m_showUnits;
    wxChoice*   m_alignment;
    std::vector<int> m_alignmentValues;
    SIM_AXIS_DIALOG_RESULTS m_results;
};

#endif

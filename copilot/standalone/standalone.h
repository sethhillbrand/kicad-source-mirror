#include <wx/app.h>
#include <wx/dialog.h>



class MyApp : public wxApp
{
public:
    virtual bool OnInit() wxOVERRIDE;
};

class MyDialog: public wxDialog
{
public:
    MyDialog(const wxString& title);
    virtual ~MyDialog();

protected:
    void OnAbout(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);

    wxDECLARE_EVENT_TABLE();
};

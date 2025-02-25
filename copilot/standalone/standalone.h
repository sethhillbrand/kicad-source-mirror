#include <wx/app.h>
#include <wx/dialog.h>
#include <wx/frame.h>
#include <wx/window.h>



class MyApp : public wxApp
{
public:
    virtual bool OnInit() wxOVERRIDE;
};

class MyDialog: public wxFrame
{
public:
    MyDialog(const wxString& title);
    virtual ~MyDialog();



};

#include <boost/test/unit_test.hpp>
#include <project.h>
#include <schematic.h>
#include <sch_sheet.h>
#include <sch_sheet_path.h>
#include <sch_screen.h>
#include <sch_sheet_path.h>
#include <sch_reference_list.h>
#include <project/project_file.h>
#include <lib_symbol.h>
#include <sch_symbol.h>
#include <wx/filename.h>
#include <wx/filefn.h>

BOOST_AUTO_TEST_SUITE( MultipleTopSheets )

BOOST_AUTO_TEST_CASE( SheetListSkipsSyntheticRoot )
{
    PROJECT prj;
    SCHEMATIC sch( &prj );
    SCH_SHEET* root = new SCH_SHEET( &sch );
    root->SetSyntheticRoot( true );
    SCH_SCREEN* rootScreen = new SCH_SCREEN( &sch );
    root->SetScreen( rootScreen );
    sch.SetRoot( root );

    SCH_SHEET* a = new SCH_SHEET( root );
    SCH_SCREEN* aScreen = new SCH_SCREEN( &sch );
    a->SetScreen( aScreen );
    rootScreen->Append( a );

    SCH_SHEET* b = new SCH_SHEET( root );
    SCH_SCREEN* bScreen = new SCH_SCREEN( &sch );
    b->SetScreen( bScreen );
    rootScreen->Append( b );

    SCH_SHEET_LIST sheets( root );
    BOOST_CHECK_EQUAL( sheets.size(), 2 );

    for( const SCH_SHEET_PATH& path : sheets )
        BOOST_CHECK_EQUAL( path.size(), 1 );
}

BOOST_AUTO_TEST_CASE( ProjectFileRoundTrip )
{
    wxFileName tmp = wxFileName::CreateTempFileName( wxS("topsheets") );
    wxString path = tmp.GetFullPath();
    wxRemoveFile( path );

    PROJECT_FILE pf( path );
    pf.GetTopSheets().emplace_back( wxS( "a.kicad_sch" ) );
    pf.GetTopSheets().emplace_back( wxS( "b.kicad_sch" ) );
    BOOST_REQUIRE( pf.SaveToFile() );

    PROJECT_FILE pf2( path );
    BOOST_REQUIRE( pf2.LoadFromFile() );

    std::vector<wxString>& list = pf2.GetTopSheets();
    BOOST_CHECK_EQUAL( list.size(), 2 );
    BOOST_CHECK( list[0] == wxS( "a.kicad_sch" ) );
    BOOST_CHECK( list[1] == wxS( "b.kicad_sch" ) );

    wxRemoveFile( path );
}

BOOST_AUTO_TEST_CASE( NetlistCollectsComponentsFromTopSheets )
{
    PROJECT prj;
    SCHEMATIC sch( &prj );
    SCH_SHEET* root = new SCH_SHEET( &sch );
    root->SetSyntheticRoot( true );
    SCH_SCREEN* rootScreen = new SCH_SCREEN( &sch );
    root->SetScreen( rootScreen );
    sch.SetRoot( root );

    SCH_SHEET* a = new SCH_SHEET( root );
    SCH_SCREEN* aScreen = new SCH_SCREEN( &sch );
    a->SetScreen( aScreen );
    rootScreen->Append( a );

    SCH_SHEET* b = new SCH_SHEET( root );
    SCH_SCREEN* bScreen = new SCH_SCREEN( &sch );
    b->SetScreen( bScreen );
    rootScreen->Append( b );

    LIB_SYMBOL lib( wxS( "R" ), nullptr );

    SCH_SHEET_PATH pathA;
    pathA.push_back( a );
    SCH_SYMBOL* symA = new SCH_SYMBOL( lib, lib.GetLibId(), &pathA, 0, 0, VECTOR2I( 0, 0 ) );
    symA->SetRef( &pathA, wxS( "R1" ) );
    aScreen->Append( symA );

    SCH_SHEET_PATH pathB;
    pathB.push_back( b );
    SCH_SYMBOL* symB = new SCH_SYMBOL( lib, lib.GetLibId(), &pathB, 0, 0, VECTOR2I( 0, 0 ) );
    symB->SetRef( &pathB, wxS( "R2" ) );
    bScreen->Append( symB );

    SCH_SHEET_LIST sheets( root );
    SCH_REFERENCE_LIST refs;
    sheets.GetSymbols( refs );
    BOOST_CHECK_EQUAL( refs.GetCount(), 2 );
}
BOOST_AUTO_TEST_SUITE_END()

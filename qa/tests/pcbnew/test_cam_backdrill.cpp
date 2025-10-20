#include <boost/test/unit_test.hpp>

#include <board.h>
#include <pcbnew/exporters/gendrill_Excellon_writer.h>
#include <pcbnew/exporters/gendrill_gerber_writer.h>
#include <pcbnew/pcb_io/odbpp/pcb_io_odbpp.h>
#include <pcbnew/pcb_track.h>
#include <base_units.h>

#include <map>

#include <core/utf8.h>

#include <wx/dir.h>
#include <wx/ffile.h>
#include <wx/filename.h>
#include <wx/utils.h>


namespace
{
wxFileName MakeTempDir()
{
    wxFileName tempDir( wxFileName::GetTempDir(), wxEmptyString );
    tempDir.AppendDir( wxString::Format( "kicad-backdrill-%llu",
                                         static_cast<unsigned long long>( wxGetUTCTime() ) ) );
    BOOST_REQUIRE( tempDir.Mkdir( wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL ) );

    return tempDir;
}
} // anonymous namespace


BOOST_AUTO_TEST_CASE( BackdrillCamOutputs )
{
    wxFileName tempDir = MakeTempDir();
    wxFileName boardFile( tempDir.GetFullPath(), wxT( "backdrill_board.kicad_pcb" ) );

    BOARD board;
    board.SetCopperLayerCount( 6 );
    board.SetFileName( boardFile.GetFullPath() );

    auto via = new PCB_VIA( &board );
    via->SetPosition( VECTOR2I( 0, 0 ) );
    via->SetLayerPair( F_Cu, B_Cu );
    via->SetDrill( pcbIUScale.mmToIU( 0.30 ) );
    via->SetWidth( pcbIUScale.mmToIU( 0.60 ) );
    via->SetSecondaryDrillSize( pcbIUScale.mmToIU( 0.20 ) );
    via->SetSecondaryDrillStartLayer( F_Cu );
    via->SetSecondaryDrillEndLayer( In3_Cu );
    via->SetSecondaryDrillPostMachiningFlag( true );
    board.Add( via );

    EXCELLON_WRITER excellon( &board );
    excellon.SetOptions( false, false, VECTOR2I( 0, 0 ), false );
    excellon.SetFormat( true );
    BOOST_REQUIRE( excellon.CreateDrillandMapFilesSet( tempDir.GetFullPath(), true, false, nullptr ) );

    wxFileName excellonFile( tempDir.GetFullPath(), wxT( "backdrill_board_Backdrills_Drill_1_4.drl" ) );
    BOOST_REQUIRE( excellonFile.FileExists() );

    wxFFile excellonStream( excellonFile.GetFullPath(), wxT( "rb" ) );
    wxString excellonContents;
    BOOST_REQUIRE( excellonStream.ReadAll( &excellonContents ) );
    BOOST_CHECK( excellonContents.Contains( wxT( "TF.FileFunction,NonPlated,1,4,Blind" ) ) );
    BOOST_CHECK( excellonContents.Contains( wxT( "; Backdrill" ) ) );
    BOOST_CHECK( excellonContents.Contains( wxT( "post-machining" ) ) );

    GERBER_WRITER gerber( &board );
    gerber.SetOptions( VECTOR2I( 0, 0 ) );
    gerber.SetFormat( 6 );
    BOOST_REQUIRE( gerber.CreateDrillandMapFilesSet( tempDir.GetFullPath(), true, false, false, nullptr ) );

    wxFileName gerberFile( tempDir.GetFullPath(), wxT( "backdrill_board_Backdrills_Drill_1_4.gbr" ) );
    BOOST_REQUIRE( gerberFile.FileExists() );

    wxFFile gerberStream( gerberFile.GetFullPath(), wxT( "rb" ) );
    wxString gerberContents;
    BOOST_REQUIRE( gerberStream.ReadAll( &gerberContents ) );
    BOOST_CHECK( gerberContents.Contains( wxT( "%TA.AperFunction,BackDrill*%" ) ) );
    BOOST_CHECK( gerberContents.Contains( wxT( "%TF.FileFunction,NonPlated,1,4,Blind,Drill*%" ) ) );

    wxFileName odbRoot( tempDir.GetFullPath(), wxEmptyString );
    odbRoot.AppendDir( wxT( "odb_out" ) );
    BOOST_REQUIRE( odbRoot.Mkdir( wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL ) );

    PCB_IO_ODBPP odbExporter;
    std::map<std::string, UTF8> props;
    props["units"] = "mm";
    props["sigfig"] = "4";
    BOOST_REQUIRE_NO_THROW( odbExporter.SaveBoard( odbRoot.GetFullPath(), &board, &props ) );

    wxFileName drill1Dir( odbRoot.GetFullPath(), wxEmptyString );
    drill1Dir.AppendDir( wxT( "steps" ) );
    drill1Dir.AppendDir( wxT( "pcb" ) );
    drill1Dir.AppendDir( wxT( "layers" ) );
    drill1Dir.AppendDir( wxT( "drill1" ) );
    BOOST_REQUIRE( drill1Dir.DirExists() );

    wxFileName toolsFile( drill1Dir.GetFullPath(), wxT( "tools" ) );
    BOOST_REQUIRE( toolsFile.FileExists() );

    wxFFile toolsStream( toolsFile.GetFullPath(), wxT( "rb" ) );
    wxString toolsContents;
    BOOST_REQUIRE( toolsStream.ReadAll( &toolsContents ) );
    BOOST_CHECK( toolsContents.Contains( wxT( "TYPE,NON_PLATED" ) ) );
    BOOST_CHECK( toolsContents.Contains( wxT( "TYPE2,BLIND" ) ) );

    toolsStream.Close();
    gerberStream.Close();
    excellonStream.Close();

    wxFileName::Rmdir( odbRoot.GetFullPath(), wxPATH_RMDIR_RECURSIVE );
    wxFileName::Rmdir( tempDir.GetFullPath(), wxPATH_RMDIR_RECURSIVE );
}

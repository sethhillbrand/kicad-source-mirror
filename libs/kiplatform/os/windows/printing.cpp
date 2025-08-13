/*
* This program source code file is part of KiCad, a free EDA CAD application.
*
* Copyright The KiCad Developers, see AUTHORS.txt for contributors.
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


#include <windows.h>
#include <roapi.h>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Printing.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Printing.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Data.Pdf.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#include <windows.graphics.printing.printmanagerinterop.h>

#include <printing.h>

#pragma comment(lib, "windowsapp.lib")

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Printing;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Printing;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Data::Pdf;


static inline std::pair<uint32_t, uint32_t> DpToPixels( Windows::Data::Pdf::PdfPage const& page, double dpi )
{
    const auto   s = page.Size(); // DIPs (1 DIP = 1/96 inch)
    const double scale = dpi / 96.0;
    uint32_t     w = static_cast<uint32_t>( std::max( 1.0, std::floor( s.Width * scale + 0.5 ) ) );
    uint32_t     h = static_cast<uint32_t>( std::max( 1.0, std::floor( s.Height * scale + 0.5 ) ) );
    return { w, h };
}

// Render one page to a XAML Image using RenderToStreamAsync
// dpi: e.g., 300 for preview; 600 for print
static Image RenderPdfPageToImage( PdfDocument const& aPdf,
    InMemoryRandomAccessStream& aStream, uint32_t aPageIndex, double aDpi )
{
    auto page = aPdf.GetPage( aPageIndex );

    if( !page )
        return nullptr;

    auto [pxW, pxH] = DpToPixels( page, aDpi );

    PdfPageRenderOptions opts;
    opts.DestinationWidth( pxW );
    opts.DestinationHeight( pxH );

    page.RenderToStreamAsync( aStream, opts ).get(); // sync for simplicity

    // Use a BitmapImage that sources directly from the stream (efficient; no extra copies)
    Windows::UI::Xaml::Media::Imaging::BitmapImage bmp;
    bmp.SetSourceAsync( aStream ).get();

    Image img;
    img.Source( bmp );
    img.Stretch( Windows::UI::Xaml::Media::Stretch::Uniform );
    return img;
}


namespace KIPLATFORM {
namespace PRINTING {

class WIN_PDF_PRINTER
{
public:
    WIN_PDF_PRINTER( HWND hwndOwner, PdfDocument const& pdf ) :
            m_hwnd( hwndOwner ),
            m_pdf( pdf )
    {
    }

    PRINT_RESULT Run()
    {
        if( !m_pdf )
            return PRINT_RESULT::FAILED_TO_LOAD;

        // Create hidden XAML Island host
        m_xamlSource = Windows::UI::Xaml::Hosting::DesktopWindowXamlSource();
        auto native = m_xamlSource.as<IDesktopWindowXamlSourceNative>();
        RECT rc{ 0, 0, 1, 1 };
        m_host = ::CreateWindowExW( 0, L"STATIC", L"", WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, rc.left, rc.top,
                                    rc.right - rc.left, rc.bottom - rc.top, m_hwnd, nullptr,
                                    ::GetModuleHandleW( nullptr ), nullptr );
        check_hresult( native->AttachToWindow( m_host ) );

        m_root = Grid();
        m_xamlSource.Content( m_root );

        m_printDoc = PrintDocument();
        m_docSrc = m_printDoc.DocumentSource();
        m_pageCount = std::max<uint32_t>( 1, m_pdf.PageCount() );

    InMemoryRandomAccessStream stream;
        m_paginateRevoker = m_printDoc.Paginate( auto_revoke,
                [this]( auto const&, PaginateEventArgs const& )
                {
                    m_printDoc.SetPreviewPageCount( m_pageCount, PreviewPageCountType::Final );
                } );

        m_getPreviewRevoker = m_printDoc.GetPreviewPage( auto_revoke,
                [this]( auto const&, GetPreviewPageEventArgs const& e )
                {
                    const uint32_t index = e.PageNumber() - 1; // 1-based from system
                    auto visual = RenderPdfPageToImage( m_pdf, m_preview_stream, index, /*dpi*/ 300.0 );
                    m_printDoc.SetPreviewPage( e.PageNumber(), visual );
                } );

        m_addPagesRevoker = m_printDoc.AddPages( auto_revoke,
                [this]( auto const&, AddPagesEventArgs const& )
                {
                    for( uint32_t i = 0; i < m_pageCount; ++i )
                    {
                        auto visual = RenderPdfPageToImage( m_pdf, m_print_stream, i, /*dpi*/ 600.0 );
                        if( visual )
                            m_printDoc.AddPage( visual );
                    }
                    m_printDoc.AddPagesComplete();
                } );

        com_ptr<IPrintManagerInterop> pmInterop;
        check_hresult( RoGetActivationFactory(
                HStringReference( RuntimeClass_Windows_Graphics_Printing_PrintManager ).Get(), pmInterop.put() ) );

        com_ptr<ABI::Windows::Graphics::Printing::IPrintManager> pmAbi;
        check_hresult( pmInterop->GetForWindow( m_hwnd, __uuidof( pmAbi ), pmAbi.put_void() ) );

        // Bridge back ABI->WinRT
        m_rtPM = PrintManager( pmAbi.as<IInspectable>() );
        m_taskRequestedToken = m_rtPM.PrintTaskRequested(
                [this]( auto const&, PrintTaskRequestedEventArgs const& e )
                {
                    auto task = e.Request().CreatePrintTask( L"KiCad PDF Print",
                            [this]( PrintTask const& t )
                            {
                                // Supply document source for preview
                                t.Source( [this]{ return m_docSrc; } );
                            } );
                } );

        com_ptr<IAsyncOperation<bool>> op;

        // Immediately wait for results to keep this in thread
        check_hresult( pmInterop->ShowPrintUIForWindowAsync( m_hwnd, op.put() ) );
        bool shown = false;

        try
        {
            shown = op->GetResults();
        }
        catch( ... )
        {
            Cleanup();
            return PRINT_RESULT::FAILED_TO_PRINT;
        }

        Cleanup();
        return shown ? PRINT_RESULT::OK : PRINT_RESULT::CANCELLED;
    }

private:
    void Cleanup()
    {
        if( m_rtPM )
        {
            m_rtPM.PrintTaskRequested( m_taskRequestedToken );
            m_rtPM = nullptr;
        }

        m_addPagesRevoker.revoke();
        m_getPreviewRevoker.revoke();
        m_paginateRevoker.revoke();

        m_docSrc = nullptr;
        m_printDoc = nullptr;
        m_root = nullptr;

        if( m_host )
        {
            ::DestroyWindow( m_host );
            m_host = nullptr;
        }

        m_xamlSource = nullptr;
    }

private:
    HWND        m_hwnd{};
    PdfDocument m_pdf{ nullptr };

    Windows::UI::Xaml::Hosting::DesktopWindowXamlSource m_xamlSource{ nullptr };
    Grid                                                m_root{ nullptr };
    PrintDocument                                       m_printDoc{ nullptr };
    IPrintDocumentSource                                m_docSrc{ nullptr };

    uint32_t           m_pageCount{ 0 };
    PrintManager       m_rtPM{ nullptr };
    winrt::event_token m_taskRequestedToken{};

    InMemoryRandomAccessStream m_preview_stream;
    InMemoryRandomAccessStream m_page_stream;

    Windows::Foundation::EventRevoker<PrintDocument> m_paginateRevoker;
    Windows::Foundation::EventRevoker<PrintDocument> m_getPreviewRevoker;
    Windows::Foundation::EventRevoker<PrintDocument> m_addPagesRevoker;

    HWND m_host{ nullptr };
};


    static std::wstring Utf8ToWide( std::string const& s )
    {
        if( s.empty() ) return {};

        int          len = MultiByteToWideChar( CP_UTF8, 0, s.data(), (int) s.size(), nullptr, 0 );
        std::wstring out( len, L'\0' );

        MultiByteToWideChar( CP_UTF8, 0, s.data(), (int) s.size(), out.data(), len );
        return out;
    }

PRINT_RESULT PrintPDF(std::string const& aFile )
{
    // Validate path
    DWORD attrs = GetFileAttributesA( aFile.c_str() );

    if( attrs == INVALID_FILE_ATTRIBUTES )
        return PRINT_RESULT::FILE_NOT_FOUND;

    // Load PDF via Windows.Data.Pdf
    PdfDocument pdf{ nullptr };

    try
    {
        auto path = Utf8ToWide( aFile );
        auto file = StorageFile::GetFileFromPathAsync( winrt::hstring( path ) ).get();
        pdf = PdfDocument::LoadFromFileAsync( file ).get();
    }
    catch( ... )
    {
        return PRINT_RESULT::FAILED_TO_LOAD;
    }

    if( !pdf || pdf.PageCount() == 0 ) return PRINT_RESULT::FAILED_TO_LOAD;

    HWND hwndOwner = ::GetActiveWindow();
    if( !hwndOwner ) hwndOwner = ::GetForegroundWindow();
    if( !hwndOwner ) return PRINT_RESULT::FAILED_TO_PRINT;

    try
    {
        WIN_PDF_PRINTER printer( hwndOwner, pdf );
        return printer.Run();
    }
    catch( ... )
    {
        return PRINT_RESULT::FAILED_TO_PRINT;
    }
}

} // namespace PRINTING
} // namespace KIPLATFORM
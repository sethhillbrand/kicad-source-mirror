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
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Data.Pdf.h>
#include <winrt/Windows.Graphics.Imaging.h>
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

// Manual declaration of IPrintManagerInterop to avoid missing header
// Copied directly from https://github.com/tpn/winsdk-10/blob/master/Include/10.0.16299.0/um/PrintManagerInterop.h
MIDL_INTERFACE("C5435A42-8D43-4E7B-A68A-EF311E392087")
IPrintManagerInterop : public IInspectable
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetForWindow(
        /* [in] */ HWND appWindow,
        /* [in] */ REFIID riid,
        /* [iid_is][retval][out] */ void **printManager) = 0;

    virtual HRESULT STDMETHODCALLTYPE ShowPrintUIForWindowAsync(
        /* [in] */ HWND appWindow,
        /* [retval][out] */ ABI::Windows::Foundation::IAsyncOperation<bool> **operation) = 0;
};

// Manual declaration of IDesktopWindowXamlSourceNative to avoid missing header
MIDL_INTERFACE("3cbcf1bf-2f76-4e9c-96ab-e84b37972554")
IDesktopWindowXamlSourceNative : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE AttachToWindow(
        /* [in] */ HWND parentWnd) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_WindowHandle(
        /* [retval][out] */ HWND *hWnd) = 0;
};

static inline std::pair<uint32_t, uint32_t> DpToPixels( Windows::Data::Pdf::PdfPage const& page, double dpi )
{
    const auto   s = page.Size(); // DIPs (1 DIP = 1/96 inch)
    const double scale = dpi / 96.0;
    uint32_t     w = static_cast<uint32_t>( std::max( 1.0, std::floor( s.Width * scale + 0.5 ) ) );
    uint32_t     h = static_cast<uint32_t>( std::max( 1.0, std::floor( s.Height * scale + 0.5 ) ) );
    return { w, h };
}

// Helper class to manage image with its associated stream
struct ManagedImage
{
    Image image;
    InMemoryRandomAccessStream stream;

    ManagedImage() = default;
    ManagedImage(Image img, InMemoryRandomAccessStream str) : image(img), stream(str) {}
};

// Render one page to a XAML Image using RenderToStreamAsync
// dpi: e.g., 300 for preview; 600 for print
// Returns a ManagedImage that keeps the stream alive
static ManagedImage RenderPdfPageToImage( PdfDocument const& pdf, uint32_t pageIndex, double dpi )
{
    auto page = pdf.GetPage( pageIndex );

    if( !page )
        return {};

    auto [pxW, pxH] = DpToPixels( page, dpi );

    PdfPageRenderOptions opts;
    opts.DestinationWidth( pxW );
    opts.DestinationHeight( pxH );

    InMemoryRandomAccessStream stream;

    try
    {
        page.RenderToStreamAsync( stream, opts ).get(); // sync for simplicity
    }
    catch( ... )
    {
        return {};
    }

    // Use a BitmapImage that sources directly from the stream (efficient; no extra copies)
    Windows::UI::Xaml::Media::Imaging::BitmapImage bmp;

    try
    {
        bmp.SetSourceAsync( stream ).get();
    }
    catch( ... )
    {
        return {};
    }

    Image img;
    img.Source( bmp );
    img.Stretch( Windows::UI::Xaml::Media::Stretch::Uniform );
    return ManagedImage{ img, stream };
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

        if( !native )
            return PRINT_RESULT::FAILED_TO_PRINT;

        RECT rc{ 0, 0, 100, 100 }; // Set minimum size to avoid 1x1 computation problems
        m_host = ::CreateWindowExW( 0, L"STATIC", L"", WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                    rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, m_hwnd, nullptr,
                                    ::GetModuleHandleW( nullptr ), nullptr );

        if( !m_host )
        {
            Cleanup();
            return PRINT_RESULT::FAILED_TO_PRINT;
        }

        HRESULT hr = native->AttachToWindow( m_host );
        if( FAILED(hr) )
        {
            Cleanup();
            return PRINT_RESULT::FAILED_TO_PRINT;
        }

        m_root = Grid();
        m_xamlSource.Content( m_root );

        m_printDoc = PrintDocument();
        m_docSrc = m_printDoc.DocumentSource();
        m_pageCount = std::max<uint32_t>( 1, m_pdf.PageCount() );

        m_paginateRevoker = m_printDoc.Paginate( auto_revoke,
                [this]( auto const&, PaginateEventArgs const& )
                {
                    m_printDoc.SetPreviewPageCount( m_pageCount, PreviewPageCountType::Final );
                } );

        m_getPreviewRevoker = m_printDoc.GetPreviewPage( auto_revoke,
                [this]( auto const&, GetPreviewPageEventArgs const& e )
                {
                    const uint32_t index = e.PageNumber() - 1; // 1-based from system
                    auto managedImg = RenderPdfPageToImage( m_pdf, index, /*dpi*/ 300.0 );

                    if( managedImg.image )
                    {
                        m_previewImages[index] = std::move(managedImg);
                        m_printDoc.SetPreviewPage( e.PageNumber(), m_previewImages[index].image );
                    }
                } );

        m_addPagesRevoker = m_printDoc.AddPages( auto_revoke,
                [this]( auto const&, AddPagesEventArgs const& )
                {
                    for( uint32_t i = 0; i < m_pageCount; ++i )
                    {
                        auto managedImg = RenderPdfPageToImage( m_pdf, i, /*dpi*/ 600.0 );

                        if( managedImg.image )
                        {
                            m_printImages[i] = std::move(managedImg);
                            m_printDoc.AddPage( m_printImages[i].image );
                        }
                    }
                    m_printDoc.AddPagesComplete();
                } );

        com_ptr<IPrintManagerInterop> pmInterop;
        HRESULT hrActivation = RoGetActivationFactory(
                HStringReference( RuntimeClass_Windows_Graphics_Printing_PrintManager ).Get(),
                __uuidof(IPrintManagerInterop), pmInterop.put_void() );

        if( FAILED(hrActivation) )
        {
            Cleanup();
            return PRINT_RESULT::FAILED_TO_PRINT;
        }

        com_ptr<ABI::Windows::Graphics::Printing::IPrintManager> pmAbi;
        HRESULT hrGetForWindow = pmInterop->GetForWindow( m_hwnd, __uuidof( pmAbi ), pmAbi.put_void() );

        if( FAILED(hrGetForWindow) )
        {
            Cleanup();
            return PRINT_RESULT::FAILED_TO_PRINT;
        }

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

        com_ptr<ABI::Windows::Foundation::IAsyncOperation<bool>> op;

        // Immediately wait for results to keep this in thread
        HRESULT hrShowPrint = pmInterop->ShowPrintUIForWindowAsync( m_hwnd, op.put() );

        if( FAILED(hrShowPrint) )
        {
            Cleanup();
            return PRINT_RESULT::FAILED_TO_PRINT;
        }

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
        // This needs to happen first to release the streams
        m_previewImages.clear();
        m_printImages.clear();

        if( m_rtPM )
        {
            m_rtPM.PrintTaskRequested({});
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

    Windows::Foundation::EventRevoker<PrintDocument> m_paginateRevoker;
    Windows::Foundation::EventRevoker<PrintDocument> m_getPreviewRevoker;
    Windows::Foundation::EventRevoker<PrintDocument> m_addPagesRevoker;

    HWND m_host{ nullptr };

    // Holds images for each page, keeping the bitmap lifetime with the printer
    std::map<uint32_t, ManagedImage> m_previewImages;
    std::map<uint32_t, ManagedImage> m_printImages;
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
    DWORD attrs = GetFileAttributesA( aFile.c_str() );

    if( attrs == INVALID_FILE_ATTRIBUTES )
        return PRINT_RESULT::FILE_NOT_FOUND;

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
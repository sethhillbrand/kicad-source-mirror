/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/gpl-3.0.html
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "allegro_file.h"

#include <wx/log.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
#endif

MAPPED_FILE_REGION::MAPPED_FILE_REGION() :
    m_data( nullptr ),
    m_size( 0 ),
    m_isValid( false ),
    m_ownsBuffer( false )
#ifdef _WIN32
    ,m_fileHandle( INVALID_HANDLE_VALUE ),
    m_mapHandle( nullptr )
#else
    ,m_fileDescriptor( -1 )
#endif
{
}

MAPPED_FILE_REGION::MAPPED_FILE_REGION( const wxString& aFilePath ) :
    MAPPED_FILE_REGION()
{
    MapFile( aFilePath );
}

MAPPED_FILE_REGION::MAPPED_FILE_REGION( const void* aBuffer, size_t aSize, bool aOwnsBuffer ) :
    MAPPED_FILE_REGION()
{
    SetBuffer( aBuffer, aSize, aOwnsBuffer );
}

MAPPED_FILE_REGION::MAPPED_FILE_REGION( MAPPED_FILE_REGION&& aOther ) noexcept :
    m_data( aOther.m_data ),
    m_size( aOther.m_size ),
    m_isValid( aOther.m_isValid ),
    m_ownsBuffer( aOther.m_ownsBuffer ),
    m_filePath( std::move( aOther.m_filePath ) )
#ifdef _WIN32
    ,m_fileHandle( aOther.m_fileHandle ),
    m_mapHandle( aOther.m_mapHandle )
#else
    ,m_fileDescriptor( aOther.m_fileDescriptor )
#endif
{
    // Reset other object
    aOther.m_data = nullptr;
    aOther.m_size = 0;
    aOther.m_isValid = false;
    aOther.m_ownsBuffer = false;
#ifdef _WIN32
    aOther.m_fileHandle = INVALID_HANDLE_VALUE;
    aOther.m_mapHandle = nullptr;
#else
    aOther.m_fileDescriptor = -1;
#endif
}

MAPPED_FILE_REGION& MAPPED_FILE_REGION::operator=( MAPPED_FILE_REGION&& aOther ) noexcept
{
    if( this != &aOther )
    {
        Unmap();

        m_data = aOther.m_data;
        m_size = aOther.m_size;
        m_isValid = aOther.m_isValid;
        m_ownsBuffer = aOther.m_ownsBuffer;
        m_filePath = std::move( aOther.m_filePath );
#ifdef _WIN32
        m_fileHandle = aOther.m_fileHandle;
        m_mapHandle = aOther.m_mapHandle;
#else
        m_fileDescriptor = aOther.m_fileDescriptor;
#endif

        // Reset other object
        aOther.m_data = nullptr;
        aOther.m_size = 0;
        aOther.m_isValid = false;
        aOther.m_ownsBuffer = false;
#ifdef _WIN32
        aOther.m_fileHandle = INVALID_HANDLE_VALUE;
        aOther.m_mapHandle = nullptr;
#else
        aOther.m_fileDescriptor = -1;
#endif
    }
    return *this;
}

MAPPED_FILE_REGION::~MAPPED_FILE_REGION()
{
    Unmap();
}

bool MAPPED_FILE_REGION::MapFile( const wxString& aFilePath )
{
    Unmap();

    m_filePath = aFilePath;

#ifdef _WIN32
    m_fileHandle = CreateFileW(
        aFilePath.wc_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if( m_fileHandle == INVALID_HANDLE_VALUE )
    {
        wxLogError( wxT( "Failed to open file: %s" ), aFilePath );
        return false;
    }

    LARGE_INTEGER fileSize;
    if( !GetFileSizeEx( m_fileHandle, &fileSize ) )
    {
        wxLogError( wxT( "Failed to get file size: %s" ), aFilePath );
        CloseHandle( m_fileHandle );
        m_fileHandle = INVALID_HANDLE_VALUE;
        return false;
    }

    m_size = static_cast<size_t>( fileSize.QuadPart );

    if( m_size == 0 )
    {
        wxLogError( wxT( "File is empty: %s" ), aFilePath );
        CloseHandle( m_fileHandle );
        m_fileHandle = INVALID_HANDLE_VALUE;
        return false;
    }

    m_mapHandle = CreateFileMappingW( m_fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr );
    if( m_mapHandle == nullptr )
    {
        wxLogError( wxT( "Failed to create file mapping: %s" ), aFilePath );
        CloseHandle( m_fileHandle );
        m_fileHandle = INVALID_HANDLE_VALUE;
        return false;
    }

    m_data = MapViewOfFile( m_mapHandle, FILE_MAP_READ, 0, 0, 0 );
    if( m_data == nullptr )
    {
        wxLogError( wxT( "Failed to map view of file: %s" ), aFilePath );
        CloseHandle( m_mapHandle );
        CloseHandle( m_fileHandle );
        m_mapHandle = nullptr;
        m_fileHandle = INVALID_HANDLE_VALUE;
        return false;
    }

#else
    m_fileDescriptor = open( aFilePath.mb_str(), O_RDONLY );
    if( m_fileDescriptor == -1 )
    {
        wxLogError( wxT( "Failed to open file: %s" ), aFilePath );
        return false;
    }

    struct stat fileStat;
    if( fstat( m_fileDescriptor, &fileStat ) == -1 )
    {
        wxLogError( wxT( "Failed to get file size: %s" ), aFilePath );
        close( m_fileDescriptor );
        m_fileDescriptor = -1;
        return false;
    }

    m_size = static_cast<size_t>( fileStat.st_size );

    if( m_size == 0 )
    {
        wxLogError( wxT( "File is empty: %s" ), aFilePath );
        close( m_fileDescriptor );
        m_fileDescriptor = -1;
        return false;
    }

    m_data = mmap( nullptr, m_size, PROT_READ, MAP_PRIVATE, m_fileDescriptor, 0 );
    if( m_data == MAP_FAILED )
    {
        wxLogError( wxT( "Failed to map file: %s" ), aFilePath );
        close( m_fileDescriptor );
        m_fileDescriptor = -1;
        m_data = nullptr;
        return false;
    }
#endif

    m_isValid = true;
    m_ownsBuffer = false;  // Memory mapping handles cleanup
    return true;
}

void MAPPED_FILE_REGION::SetBuffer( const void* aBuffer, size_t aSize, bool aOwnsBuffer )
{
    Unmap();

    m_data = aBuffer;
    m_size = aSize;
    m_isValid = ( aBuffer != nullptr && aSize > 0 );
    m_ownsBuffer = aOwnsBuffer;
    m_filePath.clear();
}

void MAPPED_FILE_REGION::Unmap()
{
    if( m_data )
    {
        if( m_ownsBuffer )
        {
            delete[] static_cast<const uint8_t*>( m_data );
        }
        else if( !m_filePath.empty() )
        {
#ifdef _WIN32
            UnmapViewOfFile( m_data );
#else
            munmap( const_cast<void*>( m_data ), m_size );
#endif
        }
        m_data = nullptr;
    }

#ifdef _WIN32
    if( m_mapHandle )
    {
        CloseHandle( m_mapHandle );
        m_mapHandle = nullptr;
    }

    if( m_fileHandle != INVALID_HANDLE_VALUE )
    {
        CloseHandle( m_fileHandle );
        m_fileHandle = INVALID_HANDLE_VALUE;
    }
#else
    if( m_fileDescriptor != -1 )
    {
        close( m_fileDescriptor );
        m_fileDescriptor = -1;
    }
#endif

    m_size = 0;
    m_isValid = false;
    m_ownsBuffer = false;
    m_filePath.clear();
}

const void* MAPPED_FILE_REGION::get_address() const
{
    return m_data;
}

size_t MAPPED_FILE_REGION::get_size() const
{
    return m_size;
}

bool MAPPED_FILE_REGION::is_valid() const
{
    return m_isValid;
}
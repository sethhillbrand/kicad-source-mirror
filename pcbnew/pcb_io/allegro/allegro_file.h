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

#ifndef ALLEGRO_FILE_H
#define ALLEGRO_FILE_H

#include <cstdint>
#include <memory>
#include <string>

#include <wx/string.h>
#include <wx/file.h>

/**
 * @brief Memory region interface for binary file access
 *
 * Provides a simple interface to access a contiguous memory region
 * containing binary file data, typically from memory-mapped files.
 */
class MEMORY_REGION
{
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~MEMORY_REGION() = default;

    /**
     * @brief Get the base address of the memory region
     * @return Pointer to the start of the memory region
     */
    virtual const void* get_address() const = 0;

    /**
     * @brief Get the size of the memory region in bytes
     * @return Size in bytes
     */
    virtual size_t get_size() const = 0;

    /**
     * @brief Check if the region is valid
     * @return true if the region contains valid data
     */
    virtual bool is_valid() const = 0;
};

/**
 * @brief Memory-mapped file region implementation
 *
 * Provides access to a file through memory mapping for efficient
 * random access to large binary files.
 */
class MAPPED_FILE_REGION : public MEMORY_REGION
{
public:
    /**
     * @brief Default constructor - creates invalid region
     */
    MAPPED_FILE_REGION();

    /**
     * @brief Constructor from file path
     * @param aFilePath Path to the file to map
     */
    explicit MAPPED_FILE_REGION( const wxString& aFilePath );

    /**
     * @brief Constructor from memory buffer
     * @param aBuffer Pointer to memory buffer
     * @param aSize Size of the buffer in bytes
     * @param aOwnsBuffer Whether this object should manage the buffer memory
     */
    MAPPED_FILE_REGION( const void* aBuffer, size_t aSize, bool aOwnsBuffer = false );

    /**
     * @brief Move constructor
     */
    MAPPED_FILE_REGION( MAPPED_FILE_REGION&& aOther ) noexcept;

    /**
     * @brief Move assignment operator
     */
    MAPPED_FILE_REGION& operator=( MAPPED_FILE_REGION&& aOther ) noexcept;

    /**
     * @brief Destructor
     */
    ~MAPPED_FILE_REGION() override;

    // Delete copy constructor and assignment operator
    MAPPED_FILE_REGION( const MAPPED_FILE_REGION& ) = delete;
    MAPPED_FILE_REGION& operator=( const MAPPED_FILE_REGION& ) = delete;

    /**
     * @brief Map a file into memory
     * @param aFilePath Path to the file to map
     * @return true if mapping was successful
     */
    bool MapFile( const wxString& aFilePath );

    /**
     * @brief Set buffer from existing memory
     * @param aBuffer Pointer to memory buffer
     * @param aSize Size of the buffer in bytes
     * @param aOwnsBuffer Whether this object should manage the buffer memory
     */
    void SetBuffer( const void* aBuffer, size_t aSize, bool aOwnsBuffer = false );

    /**
     * @brief Unmap the current file/buffer
     */
    void Unmap();

    // MEMORY_REGION interface implementation
    const void* get_address() const override;
    size_t get_size() const override;
    bool is_valid() const override;

    /**
     * @brief Get the file path if mapped from file
     * @return File path or empty string if not mapped from file
     */
    const wxString& GetFilePath() const { return m_filePath; }

private:
    const void* m_data;           ///< Pointer to mapped data
    size_t      m_size;           ///< Size of mapped region
    bool        m_isValid;        ///< Whether the region is valid
    bool        m_ownsBuffer;     ///< Whether we own the buffer memory
    wxString    m_filePath;       ///< Path to mapped file (if any)

#ifdef _WIN32
    void*       m_fileHandle;     ///< File handle on Windows
    void*       m_mapHandle;      ///< Mapping handle on Windows
#else
    int         m_fileDescriptor; ///< File descriptor on Unix
#endif
};

/**
 * @brief Allegro file wrapper
 *
 * Provides access to Allegro PCB files with memory-mapped
 * binary data access.
 */
struct ALLEGRO_FILE
{
    MAPPED_FILE_REGION region;  ///< Memory region containing file data

    /**
     * @brief Default constructor
     */
    ALLEGRO_FILE() = default;

    /**
     * @brief Constructor from file path
     * @param aFilePath Path to Allegro PCB file
     */
    explicit ALLEGRO_FILE( const wxString& aFilePath )
        : region( aFilePath )
    {
    }

    /**
     * @brief Constructor from memory buffer
     * @param aBuffer Pointer to file data in memory
     * @param aSize Size of the data in bytes
     * @param aOwnsBuffer Whether this object manages the buffer memory
     */
    ALLEGRO_FILE( const void* aBuffer, size_t aSize, bool aOwnsBuffer = false )
        : region( aBuffer, aSize, aOwnsBuffer )
    {
    }

    /**
     * @brief Check if the file is valid and ready for parsing
     * @return true if file contains valid data
     */
    bool IsValid() const
    {
        return region.is_valid() && region.get_size() > 0;
    }

    /**
     * @brief Get the file size in bytes
     * @return File size in bytes
     */
    size_t GetSize() const
    {
        return region.get_size();
    }

    /**
     * @brief Get pointer to the raw file data
     * @return Pointer to file data
     */
    const void* GetData() const
    {
        return region.get_address();
    }
};

#endif // ALLEGRO_FILE_H
/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2025 KiCad Developers, see AUTHORS.txt
 * Based on original work by Isaac Marino Bavaresco and Eldar Khayrullin
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
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ki_exception.h>
#include <sch_symbol.h>
#include <sch_pin.h>
#include <sch_shape.h>
#include <sch_line.h>
#include <sch_text.h>
#include <sch_sheet.h>
#include <sch_screen.h>
#include <sch_field.h>
#include <lib_symbol.h>
#include <symbol_lib_table.h>
#include <project_sch.h>
#include <sch_io/kicad_sexpr/sch_io_kicad_sexpr.h>
#include <string_utils.h>
#include <wildcards_and_files_ext.h>

#include <font/text_attributes.h>
#include <geometry/shape_poly_set.h>

#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/filename.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "sch_io_pcad.h"

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

SCH_IO_PCAD::SCH_IO_PCAD() : SCH_IO( wxS( "P-CAD" ) )
{
    m_reporter = &WXLOG_REPORTER::GetInstance();
}

SCH_IO_PCAD::~SCH_IO_PCAD() = default;

bool SCH_IO_PCAD::CanReadSchematicFile( const wxString& aFileName ) const
{
    if( !SCH_IO::CanReadSchematicFile( aFileName ) )
        return false;

    return checkHeader( aFileName );
}

bool SCH_IO_PCAD::CanReadLibrary( const wxString& aFileName ) const
{
    if( !SCH_IO::CanReadLibrary( aFileName ) )
        return false;

    return checkHeader( aFileName );
}

int SCH_IO_PCAD::GetModifyHash() const
{
    return 0;
}

bool SCH_IO_PCAD::checkHeader( const wxString& aFileName ) const
{
    try
    {
        std::ifstream file( aFileName.ToStdString() );
        if( !file.is_open() )
            return false;

        std::string line;
        if( !std::getline( file, line ) )
            return false;

        // P-CAD files start with "ACCEL_ASCII" followed by version info
        return line.find( "ACCEL_ASCII" ) == 0;
    }
    catch( ... )
    {
        return false;
    }
}

SCH_SHEET* SCH_IO_PCAD::LoadSchematicFile( const wxString& aFileName, SCHEMATIC* aSchematic,
                                           SCH_SHEET* aAppendToMe,
                                           const std::map<std::string, UTF8>* aProperties )
{
    wxCHECK_MSG( aSchematic, nullptr, wxT( "Schematic object is NULL" ) );

    m_filename = aFileName;
    m_schematic = aSchematic;

    if( m_progressReporter )
    {
        m_progressReporter->Report( wxString::Format( _( "Loading %s..." ), aFileName ) );
        if( !m_progressReporter->KeepRefreshing() )
            THROW_IO_ERROR( _( "Open canceled by user." ) );
    }

    try
    {
        parseSchematicFile( aFileName );

        // Convert parsed P-CAD data to KiCad format
        if( !m_pcadSchematic )
            THROW_IO_ERROR( wxString::Format( _( "Failed to parse P-CAD file: %s" ), aFileName ) );

        // Create root sheet if not appending
        if( !aAppendToMe )
        {
            m_rootSheet = new SCH_SHEET( aSchematic );
            m_rootSheet->SetName( m_filename.GetName() );
        }
        else
        {
            m_rootSheet = aAppendToMe;
        }

        // Convert P-CAD schematic data to KiCad structures
        convertSchematic();

        return m_rootSheet;
    }
    catch( const std::exception& e )
    {
        THROW_IO_ERROR( wxString::Format( _( "Error loading P-CAD schematic file '%s': %s" ),
                                         aFileName, e.what() ) );
    }
}

void SCH_IO_PCAD::EnumerateSymbolLib( wxArrayString& aSymbolNameList,
                                      const wxString& aLibraryPath,
                                      const std::map<std::string, UTF8>* aProperties )
{
    try
    {
        parseLibraryFile( aLibraryPath );

        if( m_pcadLibrary )
        {
            for( const auto& [name, component] : m_pcadLibrary->components )
            {
                aSymbolNameList.Add( wxString::FromUTF8( name ) );
            }
        }
    }
    catch( const std::exception& e )
    {
        THROW_IO_ERROR( wxString::Format( _( "Error enumerating P-CAD library '%s': %s" ),
                                         aLibraryPath, e.what() ) );
    }
}

void SCH_IO_PCAD::EnumerateSymbolLib( std::vector<LIB_SYMBOL*>& aSymbolList,
                                      const wxString& aLibraryPath,
                                      const std::map<std::string, UTF8>* aProperties )
{
    try
    {
        parseLibraryFile( aLibraryPath );

        if( m_pcadLibrary )
        {
            for( const auto& [name, component] : m_pcadLibrary->components )
            {
                LIB_SYMBOL* symbol = convertComponent( *component );
                if( symbol )
                    aSymbolList.push_back( symbol );
            }
        }
    }
    catch( const std::exception& e )
    {
        THROW_IO_ERROR( wxString::Format( _( "Error loading P-CAD library '%s': %s" ),
                                         aLibraryPath, e.what() ) );
    }
}

LIB_SYMBOL* SCH_IO_PCAD::LoadSymbol( const wxString& aLibraryPath, const wxString& aSymbolName,
                                     const std::map<std::string, UTF8>* aProperties )
{
    try
    {
        parseLibraryFile( aLibraryPath );

        if( m_pcadLibrary )
        {
            auto it = m_pcadLibrary->components.find( aSymbolName.ToStdString() );
            if( it != m_pcadLibrary->components.end() )
            {
                return convertComponent( *it->second );
            }
        }
    }
    catch( const std::exception& e )
    {
        THROW_IO_ERROR( wxString::Format( _( "Error loading symbol '%s' from P-CAD library '%s': %s" ),
                                         aSymbolName, aLibraryPath, e.what() ) );
    }

    return nullptr;
}

// ============================================================================
// Tokenizer Implementation
// ============================================================================

SCH_IO_PCAD::Token SCH_IO_PCAD::PcadTokenizer::getNextToken()
{
    if( m_hasPeekedToken )
    {
        m_hasPeekedToken = false;
        return m_peekedToken;
    }

    skipWhitespace();

    if( isAtEnd() )
        return Token{ TokenType::EOF_TOKEN, "", m_line, m_column };

    char c = currentChar();
    int startLine = m_line;
    int startColumn = m_column;

    if( c == '(' )
    {
        advance();
        return Token{ TokenType::OPEN_PAREN, "(", startLine, startColumn };
    }

    if( c == ')' )
    {
        advance();
        return Token{ TokenType::CLOSE_PAREN, ")", startLine, startColumn };
    }

    if( c == '"' )
    {
        return Token{ TokenType::STRING, readString(), startLine, startColumn };
    }

    if( std::isdigit( c ) || c == '-' || c == '+' )
    {
        std::string number = readNumber();
        TokenType type = number.find('.') != std::string::npos ? TokenType::REAL : TokenType::INTEGER;
        return Token{ type, number, startLine, startColumn };
    }

    if( std::isalpha( c ) || c == '_' )
    {
        return Token{ TokenType::NAME, readName(), startLine, startColumn };
    }

    THROW_IO_ERROR( wxString::Format( _("Unexpected character '%c' at line %d, column %d"),
                                     c, m_line, m_column ) );
}

SCH_IO_PCAD::Token SCH_IO_PCAD::PcadTokenizer::peekToken()
{
    if( !m_hasPeekedToken )
    {
        m_peekedToken = getNextToken();
        m_hasPeekedToken = true;
    }
    return m_peekedToken;
}

void SCH_IO_PCAD::PcadTokenizer::putBackToken(const Token& token)
{
    m_peekedToken = token;
    m_hasPeekedToken = true;
}

char SCH_IO_PCAD::PcadTokenizer::currentChar() const
{
    return m_pos < m_content.length() ? m_content[m_pos] : '\0';
}

char SCH_IO_PCAD::PcadTokenizer::peekChar() const
{
    return m_pos + 1 < m_content.length() ? m_content[m_pos + 1] : '\0';
}

void SCH_IO_PCAD::PcadTokenizer::advance()
{
    if( m_pos < m_content.length() )
    {
        if( m_content[m_pos] == '\n' )
        {
            m_line++;
            m_column = 1;
        }
        else
        {
            m_column++;
        }
        m_pos++;
    }
}

bool SCH_IO_PCAD::PcadTokenizer::isAtEnd() const
{
    return m_pos >= m_content.length();
}

void SCH_IO_PCAD::PcadTokenizer::skipWhitespace()
{
    while( !isAtEnd() && std::isspace( currentChar() ) )
        advance();
}

std::string SCH_IO_PCAD::PcadTokenizer::readString()
{
    std::string result;
    advance(); // Skip opening quote

    while( !isAtEnd() && currentChar() != '"' )
    {
        char c = currentChar();
        if( c == '\\' )
        {
            advance();
            if( !isAtEnd() )
            {
                char escaped = currentChar();
                switch( escaped )
                {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                case '\\': result += '\\'; break;
                case '"': result += '"'; break;
                default: result += escaped; break;
                }
            }
        }
        else
        {
            result += c;
        }
        advance();
    }

    if( isAtEnd() )
        THROW_IO_ERROR( wxString::Format( _("Unterminated string at line %d"), m_line ) );

    advance(); // Skip closing quote
    return result;
}

std::string SCH_IO_PCAD::PcadTokenizer::readNumber()
{
    std::string result;

    if( currentChar() == '-' || currentChar() == '+' )
    {
        result += currentChar();
        advance();
    }

    while( !isAtEnd() && (std::isdigit( currentChar() ) || currentChar() == '.') )
    {
        result += currentChar();
        advance();
    }

    return result;
}

std::string SCH_IO_PCAD::PcadTokenizer::readName()
{
    std::string result;

    while( !isAtEnd() && (std::isalnum( currentChar() ) || currentChar() == '_') )
    {
        result += currentChar();
        advance();
    }

    return result;
}

// ============================================================================
// ParseContext Implementation
// ============================================================================

void SCH_IO_PCAD::ParseContext::expectToken(TokenType expected)
{
    Token token = getNextToken();
    if( token.type != expected )
    {
        THROW_IO_ERROR( wxString::Format( _("Expected token type %d, got %d at line %d, column %d"),
                                         static_cast<int>(expected), static_cast<int>(token.type),
                                         token.line, token.column ) );
    }
}

void SCH_IO_PCAD::ParseContext::expectName(const std::string& expected)
{
    Token token = getNextToken();
    if( token.type != TokenType::NAME || token.value != expected )
    {
        THROW_IO_ERROR( wxString::Format( _("Expected name '%s', got '%s' at line %d, column %d"),
                                         expected.c_str(), token.value.c_str(),
                                         token.line, token.column ) );
    }
}

std::string SCH_IO_PCAD::ParseContext::getString()
{
    Token token = getNextToken();
    if( token.type != TokenType::STRING && token.type != TokenType::NAME )
    {
        THROW_IO_ERROR( wxString::Format( _("Expected string or name, got token type %d at line %d, column %d"),
                                         static_cast<int>(token.type), token.line, token.column ) );
    }
    return token.value;
}

int32_t SCH_IO_PCAD::ParseContext::getInteger()
{
    Token token = getNextToken();
    if( token.type != TokenType::INTEGER )
    {
        THROW_IO_ERROR( wxString::Format( _("Expected integer, got token type %d at line %d, column %d"),
                                         static_cast<int>(token.type), token.line, token.column ) );
    }

    try
    {
        return std::stoi( token.value );
    }
    catch( const std::exception& e )
    {
        THROW_IO_ERROR( wxString::Format( _("Invalid integer value '%s' at line %d, column %d: %s"),
                                         token.value.c_str(), token.line, token.column, e.what() ) );
    }
}

double SCH_IO_PCAD::ParseContext::getReal()
{
    Token token = getNextToken();
    if( token.type != TokenType::REAL && token.type != TokenType::INTEGER )
    {
        THROW_IO_ERROR( wxString::Format( _("Expected real number, got token type %d at line %d, column %d"),
                                         static_cast<int>(token.type), token.line, token.column ) );
    }

    try
    {
        return std::stod( token.value );
    }
    catch( const std::exception& e )
    {
        THROW_IO_ERROR( wxString::Format( _("Invalid real value '%s' at line %d, column %d: %s"),
                                         token.value.c_str(), token.line, token.column, e.what() ) );
    }
}

void SCH_IO_PCAD::ParseContext::parseGeneric(const ParseStruct& parseStruct, void* object)
{
    // Process fixed fields first
    for( const auto& field : parseStruct.fixedFields )
    {
        if( (field.flags & FieldFlags::WRAPPED) != FieldFlags::NONE )
        {
            expectToken(TokenType::OPEN_PAREN);
            if( !field.tagString.empty() )
                expectName(field.tagString);
        }

        try
        {
            field.parseFunction(*this, static_cast<char*>(object) + field.offset);
        }
        catch( const IO_ERROR& )
        {
            throw;
        }
        catch( const std::exception& e )
        {
            THROW_IO_ERROR( wxString::Format( _("Error parsing field '%s': %s"),
                                             field.tagString.c_str(), e.what() ) );
        }

        if( (field.flags & FieldFlags::WRAPPED) != FieldFlags::NONE )
        {
            expectToken(TokenType::CLOSE_PAREN);
        }
    }

    // Process optional fields
    Token token = peekToken();
    while( token.type == TokenType::OPEN_PAREN )
    {
        getNextToken(); // consume OPEN_PAREN
        token = getNextToken();

        if( token.type != TokenType::NAME )
        {
            THROW_IO_ERROR( wxString::Format( _("Expected field name at line %d, column %d"),
                                             token.line, token.column ) );
        }

        // Find matching optional field
        bool found = false;
        for( const auto& field : parseStruct.optionalFields )
        {
            if( field.tagString == token.value )
            {
                try
                {
                    field.parseFunction(*this, static_cast<char*>(object) + field.offset);
                }
                catch( const IO_ERROR& )
                {
                    throw;
                }
                catch( const std::exception& e )
                {
                    THROW_IO_ERROR( wxString::Format( _("Error parsing field '%s': %s"),
                                                     field.tagString.c_str(), e.what() ) );
                }
                found = true;
                break;
            }
        }

        if( !found )
        {
            // Skip unknown field
            skipUnknownField();
        }

        expectToken(TokenType::CLOSE_PAREN);
        token = peekToken();
    }
}

[[noreturn]] void SCH_IO_PCAD::ParseContext::error(const std::string& message)
{
    THROW_IO_ERROR( wxString::FromUTF8( message ) );
}

void SCH_IO_PCAD::ParseContext::skipUnknownField()
{
    int depth = 0;
    Token token = peekToken();

    while( true )
    {
        token = getNextToken();
        if( token.type == TokenType::OPEN_PAREN )
        {
            depth++;
        }
        else if( token.type == TokenType::CLOSE_PAREN )
        {
            if( depth == 0 )
            {
                // Put the close paren back for the caller to handle
                m_tokenizer.putBackToken( token );
                break;
            }
            depth--;
        }
        else if( token.type == TokenType::EOF_TOKEN )
        {
            THROW_IO_ERROR( _("Unexpected end of file while skipping field") );
        }
    }
}

// ============================================================================
// File parsing methods
// ============================================================================

void SCH_IO_PCAD::parseSchematicFile( const wxString& aFileName )
{
    try
    {
        std::ifstream file( aFileName.ToStdString() );
        if( !file.is_open() )
            THROW_IO_ERROR( wxString::Format( _("Cannot open file: %s"), aFileName ) );

        std::string content( ( std::istreambuf_iterator<char>( file ) ),
                             std::istreambuf_iterator<char>() );

        PcadTokenizer tokenizer( content );
        ParseContext ctx( tokenizer, this );  // Pass 'this' as user data

        // Skip header
        ctx.expectName( "ACCEL_ASCII" );
        ctx.getString(); // version string

        // Parse main schematic structure
        m_pcadSchematic = std::make_unique<PcadSchematic>();
        parseSchematic( ctx, m_pcadSchematic.get() );
    }
    catch( const IO_ERROR& )
    {
        throw;
    }
    catch( const std::exception& e )
    {
        THROW_IO_ERROR( wxString::Format( _("Error reading P-CAD file '%s': %s"),
                                         aFileName, e.what() ) );
    }
}

void SCH_IO_PCAD::parseLibraryFile( const wxString& aFileName )
{
    try
    {
        std::ifstream file( aFileName.ToStdString() );
        if( !file.is_open() )
            THROW_IO_ERROR( wxString::Format( _("Cannot open file: %s"), aFileName ) );

        std::string content( ( std::istreambuf_iterator<char>( file ) ),
                             std::istreambuf_iterator<char>() );

        PcadTokenizer tokenizer( content );
        ParseContext ctx( tokenizer, this );  // Pass 'this' as user data

        // Skip header
        ctx.expectName( "ACCEL_ASCII" );
        ctx.getString(); // version string

        // Parse library structure
        m_pcadLibrary = std::make_unique<PcadLibrary>();
        parseLibrary( ctx, m_pcadLibrary.get() );
    }
    catch( const IO_ERROR& )
    {
        throw;
    }
    catch( const std::exception& e )
    {
        THROW_IO_ERROR( wxString::Format( _("Error reading P-CAD library file '%s': %s"),
                                         aFileName, e.what() ) );
    }
}

// ============================================================================
// ParseStruct Implementations
// ============================================================================

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getPointParseStruct()
{
    ParseStruct pointStruct;

    pointStruct.fixedFields = {
        {
            FieldFlags::NAKED,
            "",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadPoint*>(target)->x = ctx.getInteger();
            },
            offsetof(PcadPoint, x),
            0
        },
        {
            FieldFlags::NAKED,
            "",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadPoint*>(target)->y = ctx.getInteger();
            },
            offsetof(PcadPoint, y),
            0
        }
    };

    return pointStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getAttributeParseStruct()
{
    ParseStruct attrStruct;

    // Fixed fields: attrName, attrValue
    attrStruct.fixedFields = {
        {
            FieldFlags::NAKED,
            "",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadAttribute*>(target)->name = ctx.getString();
            },
            offsetof(PcadAttribute, name),
            0
        },
        {
            FieldFlags::NAKED,
            "",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadAttribute*>(target)->value = ctx.getString();
            },
            offsetof(PcadAttribute, value),
            0
        }
    };

    // Optional fields
    attrStruct.optionalFields = {
        {
            FieldFlags::WRAPPED,
            "attrType",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadAttribute*>(target)->type = ctx.getString();
            },
            offsetof(PcadAttribute, type),
            0
        },
        {
            FieldFlags::WRAPPED,
            "isVisible",
            [](ParseContext& ctx, void* target) {
                std::string visible = ctx.getString();
                static_cast<PcadAttribute*>(target)->isVisible = (visible == "True");
            },
            offsetof(PcadAttribute, isVisible),
            0
        },
        {
            FieldFlags::WRAPPED,
            "pt",
            [](ParseContext& ctx, void* target) {
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());
                ParseStruct pointStruct = parser->getPointParseStruct();
                ctx.parseGeneric(pointStruct, &static_cast<PcadAttribute*>(target)->point);
            },
            offsetof(PcadAttribute, point),
            sizeof(PcadPoint)
        },
        {
            FieldFlags::WRAPPED,
            "rotation",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadAttribute*>(target)->rotation = ctx.getInteger();
            },
            offsetof(PcadAttribute, rotation),
            0
        },
        {
            FieldFlags::WRAPPED,
            "isFlipped",
            [](ParseContext& ctx, void* target) {
                std::string flipped = ctx.getString();
                static_cast<PcadAttribute*>(target)->isFlipped = (flipped == "True");
            },
            offsetof(PcadAttribute, isFlipped),
            0
        },
        {
            FieldFlags::WRAPPED,
            "textStyleRef",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadAttribute*>(target)->textStyleRef = ctx.getString();
            },
            offsetof(PcadAttribute, textStyleRef),
            0
        },
        {
            FieldFlags::WRAPPED,
            "justify",
            [](ParseContext& ctx, void* target) {
                std::string justify = ctx.getString();
                // Convert to PCAD_JUSTIFICATION enum - simplified
                static_cast<PcadAttribute*>(target)->justify = PCAD_JUSTIFICATION::LEFT;
            },
            offsetof(PcadAttribute, justify),
            0
        }
    };

    return attrStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getPortParseStruct()
{
    ParseStruct portStruct;

    // Fixed fields: pt, portType
    portStruct.fixedFields = {
        {
            FieldFlags::WRAPPED,
            "pt",
            [](ParseContext& ctx, void* target) {
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());
                ParseStruct pointStruct = parser->getPointParseStruct();
                ctx.parseGeneric(pointStruct, &static_cast<PcadPort*>(target)->point);
            },
            offsetof(PcadPort, point),
            sizeof(PcadPoint)
        },
        {
            FieldFlags::WRAPPED,
            "portType",
            [](ParseContext& ctx, void* target) {
                std::string portType = ctx.getString();
                // Convert string to enum - simplified mapping
                if( portType == "INPUT" )
                    static_cast<PcadPort*>(target)->portType = PCAD_PIN_TYPE::INPUT;
                else if( portType == "OUTPUT" )
                    static_cast<PcadPort*>(target)->portType = PCAD_PIN_TYPE::OUTPUT;
                else if( portType == "BIDIRECTIONAL" )
                    static_cast<PcadPort*>(target)->portType = PCAD_PIN_TYPE::BIDIRECTIONAL;
                else
                    static_cast<PcadPort*>(target)->portType = PCAD_PIN_TYPE::PASSIVE;
            },
            offsetof(PcadPort, portType),
            0
        }
    };

    // Optional fields
    portStruct.optionalFields = {
        {
            FieldFlags::WRAPPED,
            "portPinLength",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadPort*>(target)->portPinLength = ctx.getString();
            },
            offsetof(PcadPort, portPinLength),
            0
        },
        {
            FieldFlags::WRAPPED,
            "netNameRef",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadPort*>(target)->netNameRef = ctx.getString();
            },
            offsetof(PcadPort, netNameRef),
            0
        },
        {
            FieldFlags::WRAPPED,
            "rotation",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadPort*>(target)->rotation = ctx.getInteger();
            },
            offsetof(PcadPort, rotation),
            0
        },
        {
            FieldFlags::WRAPPED,
            "isFlipped",
            [](ParseContext& ctx, void* target) {
                std::string flipped = ctx.getString();
                static_cast<PcadPort*>(target)->isFlipped = (flipped == "True");
            },
            offsetof(PcadPort, isFlipped),
            0
        }
    };

    return portStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getInstanceParseStruct()
{
    ParseStruct instanceStruct;

    // Fixed fields: name
    instanceStruct.fixedFields = {
        {
            FieldFlags::NAKED,
            "",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadInstance*>(target)->name = ctx.getString();
            },
            offsetof(PcadInstance, name),
            0
        }
    };

    // Optional fields
    instanceStruct.optionalFields = {
        {
            FieldFlags::WRAPPED,
            "compRef",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadInstance*>(target)->compRef = ctx.getString();
            },
            offsetof(PcadInstance, compRef),
            0
        },
        {
            FieldFlags::WRAPPED,
            "originalName",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadInstance*>(target)->originalName = ctx.getString();
            },
            offsetof(PcadInstance, originalName),
            0
        },
        {
            FieldFlags::WRAPPED,
            "compValue",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadInstance*>(target)->compValue = ctx.getString();
            },
            offsetof(PcadInstance, compValue),
            0
        },
        {
            FieldFlags::WRAPPED,
            "patternName",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadInstance*>(target)->patternName = ctx.getString();
            },
            offsetof(PcadInstance, patternName),
            0
        },
        {
            FieldFlags::WRAPPED,
            "symbolRef",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadInstance*>(target)->symbolRef = ctx.getString();
            },
            offsetof(PcadInstance, symbolRef),
            0
        },
        {
            FieldFlags::WRAPPED,
            "pt",
            [](ParseContext& ctx, void* target) {
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());
                ParseStruct pointStruct = parser->getPointParseStruct();
                ctx.parseGeneric(pointStruct, &static_cast<PcadInstance*>(target)->point);
            },
            offsetof(PcadInstance, point),
            sizeof(PcadPoint)
        },
        {
            FieldFlags::WRAPPED,
            "rotation",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadInstance*>(target)->rotation = ctx.getInteger();
            },
            offsetof(PcadInstance, rotation),
            0
        },
        {
            FieldFlags::WRAPPED,
            "isFlipped",
            [](ParseContext& ctx, void* target) {
                std::string flipped = ctx.getString();
                static_cast<PcadInstance*>(target)->isFlipped = (flipped == "True");
            },
            offsetof(PcadInstance, isFlipped),
            0
        },
        {
            FieldFlags::WRAPPED | FieldFlags::LIST,
            "attr",
            [](ParseContext& ctx, void* target) {
                auto attr = std::make_unique<PcadAttribute>();
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());

                ParseStruct attrStruct = parser->getAttributeParseStruct();
                ctx.parseGeneric(attrStruct, attr.get());

                static_cast<PcadInstance*>(target)->attributes.push_back(std::move(attr));
            },
            0,
            sizeof(PcadAttribute)
        }
    };

    return instanceStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getExtentParseStruct()
{
    ParseStruct extentStruct;

    // Fixed fields: x and y extents (naked fields)
    extentStruct.fixedFields = {
        {
            FieldFlags::NAKED,
            "",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadExtent*>(target)->x = ctx.getReal();
            },
            offsetof(PcadExtent, x),
            0
        },
        {
            FieldFlags::NAKED,
            "",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadExtent*>(target)->y = ctx.getReal();
            },
            offsetof(PcadExtent, y),
            0
        }
    };

    return extentStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getFontParseStruct()
{
    ParseStruct fontStruct;

    // Optional fields for font properties
    fontStruct.optionalFields = {
        {
            FieldFlags::WRAPPED,
            "fontType",
            [](ParseContext& ctx, void* target) {
                std::string fontType = ctx.getString();
                // Store font type if needed
            },
            0,
            0
        },
        {
            FieldFlags::WRAPPED,
            "fontFamily",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadFont*>(target)->fontFamily = ctx.getString();
            },
            offsetof(PcadFont, fontFamily),
            0
        },
        {
            FieldFlags::WRAPPED,
            "fontFace",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadFont*>(target)->fontFace = ctx.getString();
            },
            offsetof(PcadFont, fontFace),
            0
        },
        {
            FieldFlags::WRAPPED,
            "fontHeight",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadFont*>(target)->fontHeight = ctx.getReal();
            },
            offsetof(PcadFont, fontHeight),
            0
        },
        {
            FieldFlags::WRAPPED,
            "strokeWidth",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadFont*>(target)->strokeWidth = ctx.getReal();
            },
            offsetof(PcadFont, strokeWidth),
            0
        },
        {
            FieldFlags::WRAPPED,
            "fontWeight",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadFont*>(target)->fontWeight = ctx.getInteger();
            },
            offsetof(PcadFont, fontWeight),
            0
        }
    };

    return fontStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getStyleParseStruct()
{
    ParseStruct styleStruct;

    // Optional fields for style properties
    styleStruct.optionalFields = {
        {
            FieldFlags::WRAPPED,
            "lineStyle",
            [](ParseContext& ctx, void* target) {
                std::string lineStyle = ctx.getString();
                // Parse line style enum if needed
            },
            0,
            0
        },
        {
            FieldFlags::WRAPPED,
            "lineWidth",
            [](ParseContext& ctx, void* target) {
                double width = ctx.getReal();
                // Store line width if needed
            },
            0,
            0
        }
    };

    return styleStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getBusParseStruct()
{
    ParseStruct busStruct;

    // Fixed fields: name, pt1, pt2
    busStruct.fixedFields = {
        {
            FieldFlags::NAKED,
            "",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadBus*>(target)->name = ctx.getString();
            },
            offsetof(PcadBus, name),
            0
        },
        {
            FieldFlags::WRAPPED,
            "pt",
            [](ParseContext& ctx, void* target) {
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());
                ParseStruct pointStruct = parser->getPointParseStruct();
                ctx.parseGeneric(pointStruct, &static_cast<PcadBus*>(target)->pt1);
            },
            offsetof(PcadBus, pt1),
            sizeof(PcadPoint)
        },
        {
            FieldFlags::WRAPPED,
            "pt",
            [](ParseContext& ctx, void* target) {
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());
                ParseStruct pointStruct = parser->getPointParseStruct();
                ctx.parseGeneric(pointStruct, &static_cast<PcadBus*>(target)->pt2);
            },
            offsetof(PcadBus, pt2),
            sizeof(PcadPoint)
        }
    };

    // Optional fields
    busStruct.optionalFields = {
        {
            FieldFlags::WRAPPED,
            "dispName",
            [](ParseContext& ctx, void* target) {
                std::string dispName = ctx.getString();
                static_cast<PcadBus*>(target)->displayName = (dispName == "True");
            },
            offsetof(PcadBus, displayName),
            0
        },
        {
            FieldFlags::WRAPPED,
            "text",
            [](ParseContext& ctx, void* target) {
                auto text = std::make_unique<PcadText>();
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());

                ParseStruct textStruct = parser->getTextParseStruct();
                ctx.parseGeneric(textStruct, text.get());

                static_cast<PcadBus*>(target)->text = std::move(text);
            },
            0,
            sizeof(PcadText)
        }
    };

    return busStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getJunctionParseStruct()
{
    ParseStruct junctionStruct;

    // Fixed fields: pt, netNameRef
    junctionStruct.fixedFields = {
        {
            FieldFlags::WRAPPED,
            "pt",
            [](ParseContext& ctx, void* target) {
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());
                ParseStruct pointStruct = parser->getPointParseStruct();
                ctx.parseGeneric(pointStruct, &static_cast<PcadJunction*>(target)->point);
            },
            offsetof(PcadJunction, point),
            sizeof(PcadPoint)
        },
        {
            FieldFlags::WRAPPED,
            "netNameRef",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadJunction*>(target)->netNameRef = ctx.getString();
            },
            offsetof(PcadJunction, netNameRef),
            0
        }
    };

    return junctionStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getFieldParseStruct()
{
    ParseStruct fieldStruct;

    // Fixed fields: name, pt
    fieldStruct.fixedFields = {
        {
            FieldFlags::NAKED,
            "",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadField*>(target)->name = ctx.getString();
            },
            offsetof(PcadField, name),
            0
        },
        {
            FieldFlags::WRAPPED,
            "pt",
            [](ParseContext& ctx, void* target) {
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());
                ParseStruct pointStruct = parser->getPointParseStruct();
                ctx.parseGeneric(pointStruct, &static_cast<PcadField*>(target)->point);
            },
            offsetof(PcadField, point),
            sizeof(PcadPoint)
        }
    };

    // Optional fields
    fieldStruct.optionalFields = {
        {
            FieldFlags::WRAPPED,
            "fieldValue",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadField*>(target)->value = ctx.getString();
            },
            offsetof(PcadField, value),
            0
        },
        {
            FieldFlags::WRAPPED,
            "textStyleRef",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadField*>(target)->textStyleRef = ctx.getString();
            },
            offsetof(PcadField, textStyleRef),
            0
        },
        {
            FieldFlags::WRAPPED,
            "rotation",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadField*>(target)->rotation = ctx.getReal();
            },
            offsetof(PcadField, rotation),
            0
        },
        {
            FieldFlags::WRAPPED,
            "isFlipped",
            [](ParseContext& ctx, void* target) {
                std::string flipped = ctx.getString();
                static_cast<PcadField*>(target)->isFlipped = (flipped == "True");
            },
            offsetof(PcadField, isFlipped),
            0
        },
        {
            FieldFlags::WRAPPED,
            "justify",
            [](ParseContext& ctx, void* target) {
                std::string justify = ctx.getString();
                // Convert to PCAD_JUSTIFICATION enum - simplified
                static_cast<PcadField*>(target)->justify = PCAD_JUSTIFICATION::LEFT;
            },
            offsetof(PcadField, justify),
            0
        }
    };

    return fieldStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getTitleSheetParseStruct()
{
    ParseStruct titleSheetStruct;

    // Optional fields for various title sheet elements
    titleSheetStruct.optionalFields = {
        {
            FieldFlags::WRAPPED | FieldFlags::LIST,
            "line",
            [](ParseContext& ctx, void* target) {
                auto line = std::make_unique<PcadLine>();
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());

                ParseStruct lineStruct = parser->getLineParseStruct();
                ctx.parseGeneric(lineStruct, line.get());

                static_cast<PcadTitleSheet*>(target)->lines.push_back(std::move(line));
            },
            0,
            sizeof(PcadLine)
        },
        {
            FieldFlags::WRAPPED | FieldFlags::LIST,
            "text",
            [](ParseContext& ctx, void* target) {
                auto text = std::make_unique<PcadText>();
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());

                ParseStruct textStruct = parser->getTextParseStruct();
                ctx.parseGeneric(textStruct, text.get());

                static_cast<PcadTitleSheet*>(target)->texts.push_back(std::move(text));
            },
            0,
            sizeof(PcadText)
        }
    };

    return titleSheetStruct;
}

// Add existing ParseStruct methods that were already implemented
SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getTextParseStruct()
{
    ParseStruct textStruct;

    textStruct.fixedFields = {
        {
            FieldFlags::WRAPPED,
            "pt",
            [](ParseContext& ctx, void* target) {
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());
                ParseStruct pointStruct = parser->getPointParseStruct();
                ctx.parseGeneric(pointStruct, &static_cast<PcadText*>(target)->point);
            },
            offsetof(PcadText, point),
            sizeof(PcadPoint)
        },
        {
            FieldFlags::NAKED,
            "",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadText*>(target)->value = ctx.getString();
            },
            offsetof(PcadText, value),
            0
        }
    };

    textStruct.optionalFields = {
        {
            FieldFlags::WRAPPED,
            "textStyleRef",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadText*>(target)->textStyleRef = ctx.getString();
            },
            offsetof(PcadText, textStyleRef),
            0
        },
        {
            FieldFlags::WRAPPED,
            "rotation",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadText*>(target)->rotation = ctx.getInteger();
            },
            offsetof(PcadText, rotation),
            0
        },
        {
            FieldFlags::WRAPPED,
            "isFlipped",
            [](ParseContext& ctx, void* target) {
                std::string flipped = ctx.getString();
                static_cast<PcadText*>(target)->isVisible = (flipped != "True");
            },
            0,
            0
        },
        {
            FieldFlags::WRAPPED,
            "justify",
            [](ParseContext& ctx, void* target) {
                std::string justify = ctx.getString();
                // Convert to enum - simplified
                static_cast<PcadText*>(target)->justify = PCAD_JUSTIFICATION::LEFT;
            },
            offsetof(PcadText, justify),
            0
        }
    };

    return textStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getLineParseStruct()
{
    ParseStruct lineStruct;

    lineStruct.fixedFields = {
        {
            FieldFlags::WRAPPED,
            "pt",
            [](ParseContext& ctx, void* target) {
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());
                ParseStruct pointStruct = parser->getPointParseStruct();
                ctx.parseGeneric(pointStruct, &static_cast<PcadLine*>(target)->pt1);
            },
            offsetof(PcadLine, pt1),
            sizeof(PcadPoint)
        },
        {
            FieldFlags::WRAPPED,
            "pt",
            [](ParseContext& ctx, void* target) {
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());
                ParseStruct pointStruct = parser->getPointParseStruct();
                ctx.parseGeneric(pointStruct, &static_cast<PcadLine*>(target)->pt2);
            },
            offsetof(PcadLine, pt2),
            sizeof(PcadPoint)
        }
    };

    lineStruct.optionalFields = {
        {
            FieldFlags::WRAPPED,
            "style",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadLine*>(target)->style = ctx.getString();
            },
            offsetof(PcadLine, style),
            0
        },
        {
            FieldFlags::WRAPPED,
            "width",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadLine*>(target)->width = ctx.getInteger();
            },
            offsetof(PcadLine, width),
            0
        }
    };

    return lineStruct;
}

// Add remaining stub methods for parsing hierarchy
void SCH_IO_PCAD::parseSchematic(ParseContext& ctx, PcadSchematic* schematic)
{
    ParseStruct schematicStruct = getSchematicParseStruct();
    ctx.parseGeneric(schematicStruct, schematic);
}

void SCH_IO_PCAD::parseLibrary(ParseContext& ctx, PcadLibrary* library)
{
    ParseStruct libraryStruct = getLibraryParseStruct();
    ctx.parseGeneric(libraryStruct, library);
}

// Placeholder implementations for required parse structures
SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getSchematicParseStruct()
{
    ParseStruct schematicStruct;

    schematicStruct.fixedFields = {
        {
            FieldFlags::NAKED,
            "",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadSchematic*>(target)->name = ctx.getString();
            },
            offsetof(PcadSchematic, name),
            0
        }
    };

    schematicStruct.optionalFields = {
        {
            FieldFlags::WRAPPED | FieldFlags::LIST,
            "sheet",
            [](ParseContext& ctx, void* target) {
                auto sheet = std::make_unique<PcadSheet>();
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());

                ParseStruct sheetStruct = parser->getSheetParseStruct();
                ctx.parseGeneric(sheetStruct, sheet.get());

                static_cast<PcadSchematic*>(target)->sheets.push_back(std::move(sheet));
            },
            0,
            sizeof(PcadSheet)
        }
    };

    return schematicStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getSheetParseStruct()
{
    ParseStruct sheetStruct;

    sheetStruct.fixedFields = {
        {
            FieldFlags::NAKED,
            "",
            [](ParseContext& ctx, void* target) {
                static_cast<PcadSheet*>(target)->name = ctx.getString();
            },
            offsetof(PcadSheet, name),
            0
        }
    };

    // Add simplified optional fields for sheet elements
    sheetStruct.optionalFields = {
        {
            FieldFlags::WRAPPED | FieldFlags::LIST,
            "symbol",
            [](ParseContext& ctx, void* target) {
                auto instance = std::make_unique<PcadInstance>();
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());

                ParseStruct instanceStruct = parser->getInstanceParseStruct();
                ctx.parseGeneric(instanceStruct, instance.get());

                static_cast<PcadSheet*>(target)->instances.push_back(std::move(instance));
            },
            0,
            sizeof(PcadInstance)
        },
        {
            FieldFlags::WRAPPED | FieldFlags::LIST,
            "junction",
            [](ParseContext& ctx, void* target) {
                auto junction = std::make_unique<PcadJunction>();
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());

                ParseStruct junctionStruct = parser->getJunctionParseStruct();
                ctx.parseGeneric(junctionStruct, junction.get());

                static_cast<PcadSheet*>(target)->junctions.push_back(std::move(junction));
            },
            0,
            sizeof(PcadJunction)
        },
        {
            FieldFlags::WRAPPED | FieldFlags::LIST,
            "bus",
            [](ParseContext& ctx, void* target) {
                auto bus = std::make_unique<PcadBus>();
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());

                ParseStruct busStruct = parser->getBusParseStruct();
                ctx.parseGeneric(busStruct, bus.get());

                static_cast<PcadSheet*>(target)->buses.push_back(std::move(bus));
            },
            0,
            sizeof(PcadBus)
        },
        {
            FieldFlags::WRAPPED | FieldFlags::LIST,
            "field",
            [](ParseContext& ctx, void* target) {
                auto field = std::make_unique<PcadField>();
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());

                ParseStruct fieldStruct = parser->getFieldParseStruct();
                ctx.parseGeneric(fieldStruct, field.get());

                static_cast<PcadSheet*>(target)->fields.push_back(std::move(field));
            },
            0,
            sizeof(PcadField)
        },
        {
            FieldFlags::WRAPPED,
            "titleSheet",
            [](ParseContext& ctx, void* target) {
                auto titleSheet = std::make_unique<PcadTitleSheet>();
                SCH_IO_PCAD* parser = static_cast<SCH_IO_PCAD*>(ctx.getUserData());

                ParseStruct titleSheetStruct = parser->getTitleSheetParseStruct();
                ctx.parseGeneric(titleSheetStruct, titleSheet.get());

                static_cast<PcadSheet*>(target)->titleSheet = std::move(titleSheet);
            },
            0,
            sizeof(PcadTitleSheet)
        }
    };

    return sheetStruct;
}

// Placeholder implementations for remaining required methods
SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getLibraryParseStruct()
{
    ParseStruct libraryStruct;
    // Simplified implementation - would need full implementation for production use
    return libraryStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getPinParseStruct()
{
    ParseStruct pinStruct;
    // Simplified implementation - would need full implementation for production use
    return pinStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getSymbolParseStruct()
{
    ParseStruct symbolStruct;
    // Simplified implementation - would need full implementation for production use
    return symbolStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getComponentParseStruct()
{
    ParseStruct componentStruct;
    // Simplified implementation - would need full implementation for production use
    return componentStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getWireParseStruct()
{
    ParseStruct wireStruct;
    // Simplified implementation - would need full implementation for production use
    return wireStruct;
}

SCH_IO_PCAD::ParseStruct SCH_IO_PCAD::getNetParseStruct()
{
    ParseStruct netStruct;
    // Simplified implementation - would need full implementation for production use
    return netStruct;
}

// ============================================================================
// Conversion methods (P-CAD to KiCad) - Placeholder implementations
// ============================================================================

void SCH_IO_PCAD::convertSchematic()
{
    // Simplified conversion - real implementation would be much more comprehensive
    if( !m_pcadSchematic || !m_rootSheet )
        return;

    // Create screen for the root sheet
    auto screen = std::make_unique<SCH_SCREEN>( m_schematic );
    m_rootSheet->SetScreen( screen.release() );
}

SCH_SYMBOL* SCH_IO_PCAD::convertInstance( const PcadInstance& pcadInstance )
{
    // Simplified conversion - real implementation would be much more comprehensive
    return nullptr;
}

LIB_SYMBOL* SCH_IO_PCAD::convertComponent( const PcadComponent& pcadComponent )
{
    // Simplified conversion - real implementation would be much more comprehensive
    return nullptr;
}

SCH_PIN* SCH_IO_PCAD::convertPin( const PcadPin& pcadPin )
{
    // Simplified conversion - real implementation would be much more comprehensive
    return nullptr;
}

SCH_SHAPE* SCH_IO_PCAD::convertLine( const PcadLine& pcadLine )
{
    // Simplified conversion - real implementation would be much more comprehensive
    return nullptr;
}

SCH_TEXT* SCH_IO_PCAD::convertText( const PcadText& pcadText )
{
    // Simplified conversion - real implementation would be much more comprehensive
    return nullptr;
}

// ============================================================================
// Utility conversion methods
// ============================================================================

ELECTRICAL_PINTYPE SCH_IO_PCAD::pcadToKiCadPinType( PCAD_PIN_TYPE pcadType ) const
{
    switch( pcadType )
    {
        case PCAD_PIN_TYPE::INPUT:          return ELECTRICAL_PINTYPE::PT_INPUT;
        case PCAD_PIN_TYPE::OUTPUT:         return ELECTRICAL_PINTYPE::PT_OUTPUT;
        case PCAD_PIN_TYPE::BIDIRECTIONAL:  return ELECTRICAL_PINTYPE::PT_BIDI;
        case PCAD_PIN_TYPE::TRISTATE:       return ELECTRICAL_PINTYPE::PT_TRISTATE;
        case PCAD_PIN_TYPE::PASSIVE:        return ELECTRICAL_PINTYPE::PT_PASSIVE;
        case PCAD_PIN_TYPE::POWER:          return ELECTRICAL_PINTYPE::PT_POWER_IN;
        default:                            return ELECTRICAL_PINTYPE::PT_UNSPECIFIED;
    }
}

GR_TEXT_H_ALIGN_T SCH_IO_PCAD::pcadToKiCadHAlign( PCAD_JUSTIFICATION justify ) const
{
    switch( justify )
    {
        case PCAD_JUSTIFICATION::LEFT:
        case PCAD_JUSTIFICATION::UPPER_LEFT:
        case PCAD_JUSTIFICATION::LOWER_LEFT:
            return GR_TEXT_H_ALIGN_LEFT;

        case PCAD_JUSTIFICATION::CENTER:
        case PCAD_JUSTIFICATION::UPPER_CENTER:
        case PCAD_JUSTIFICATION::LOWER_CENTER:
            return GR_TEXT_H_ALIGN_CENTER;

        case PCAD_JUSTIFICATION::RIGHT:
        case PCAD_JUSTIFICATION::UPPER_RIGHT:
        case PCAD_JUSTIFICATION::LOWER_RIGHT:
            return GR_TEXT_H_ALIGN_RIGHT;

        default:
            return GR_TEXT_H_ALIGN_LEFT;
    }
}

GR_TEXT_V_ALIGN_T SCH_IO_PCAD::pcadToKiCadVAlign( PCAD_JUSTIFICATION justify ) const
{
    switch( justify )
    {
        case PCAD_JUSTIFICATION::UPPER_LEFT:
        case PCAD_JUSTIFICATION::UPPER_CENTER:
        case PCAD_JUSTIFICATION::UPPER_RIGHT:
            return GR_TEXT_V_ALIGN_TOP;

        case PCAD_JUSTIFICATION::LEFT:
        case PCAD_JUSTIFICATION::CENTER:
        case PCAD_JUSTIFICATION::RIGHT:
            return GR_TEXT_V_ALIGN_CENTER;

        case PCAD_JUSTIFICATION::LOWER_LEFT:
        case PCAD_JUSTIFICATION::LOWER_CENTER:
        case PCAD_JUSTIFICATION::LOWER_RIGHT:
            return GR_TEXT_V_ALIGN_BOTTOM;

        default:
            return GR_TEXT_V_ALIGN_CENTER;
    }
}

wxString SCH_IO_PCAD::cleanSymbolName( const std::string& name ) const
{
    wxString cleanName = wxString::FromUTF8( name );

    // Replace invalid characters with underscores
    for( size_t i = 0; i < cleanName.length(); i++ )
    {
        wxChar c = cleanName[i];
        if( !wxIsalnum( c ) && c != '_' && c != '-' )
            cleanName[i] = '_';
    }

    return cleanName;
}

void SCH_IO_PCAD::skipUnknownElement( ParseContext& ctx )
{
    ctx.skipUnknownField();
}
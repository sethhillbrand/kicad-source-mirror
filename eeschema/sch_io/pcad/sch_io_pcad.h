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

#ifndef SCH_IO_PCAD_H_
#define SCH_IO_PCAD_H_

#include <sch_io/sch_io.h>
#include <sch_io/sch_io_mgr.h>
#include <pin_type.h>
#include <vector2d.h>
#include <wx/xml/xml.h>

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include <functional>
#include <stack>

class SCH_SHEET;
class SCH_SCREEN;
class SCH_SYMBOL;
class SCH_PIN;
class SCH_SHAPE;
class SCH_LINE;
class SCH_TEXT;
class LIB_SYMBOL;
class SCH_FIELD;

/**
 * PCAD schematic units and coordinate system definitions
 */
enum class PCAD_UNITS
{
    NONE = 0,
    MM = 1,
    MIL = 2,
    IN = 3
};

/**
 * PCAD pin and port types
 */
enum class PCAD_PIN_TYPE
{
    NONE = 0,
    UNKNOWN,
    PASSIVE,
    INPUT,
    OUTPUT,
    BIDIRECTIONAL,
    OPEN_H,
    OPEN_L,
    PASSIVE_H,
    PASSIVE_L,
    TRISTATE,
    POWER
};

/**
 * PCAD text justification
 */
enum class PCAD_JUSTIFICATION
{
    LOWER_LEFT = 0,
    LOWER_CENTER,
    LOWER_RIGHT,
    LEFT,
    CENTER,
    RIGHT,
    UPPER_LEFT,
    UPPER_CENTER,
    UPPER_RIGHT
};

/**
 * Forward declarations for PCAD data structures
 */
struct PcadPoint
{
    int32_t x{0};
    int32_t y{0};

    PcadPoint() = default;
    PcadPoint(int32_t x_, int32_t y_) : x(x_), y(y_) {}

    VECTOR2I ToKiCadUnits(double scale = 1.0) const
    {
        return VECTOR2I(KiROUND(x * scale), KiROUND(y * scale));
    }
};

struct PcadAttribute
{
    std::string name;
    std::string value;
    std::string type;       // Optional attribute type
    bool isVisible{true};   // Visibility flag
    PcadPoint point;
    int32_t rotation{0};
    PCAD_JUSTIFICATION justify{PCAD_JUSTIFICATION::LOWER_LEFT};
    bool isFlipped{false};
    std::string textStyleRef;

    PcadAttribute() = default;
    PcadAttribute(const std::string& n, const std::string& v) : name(n), value(v) {}
};

struct PcadExtent
{
    double x{0.0};
    double y{0.0};

    PcadExtent() = default;
    PcadExtent(double x_, double y_) : x(x_), y(y_) {}
};

struct PcadFont
{
    std::string fontFamily;
    std::string fontFace;
    double fontHeight{0.0};
    double strokeWidth{0.0};
    int fontWeight{400};

    PcadFont() = default;
};

struct PcadPin
{
    std::string pinNumber;
    std::string pinName;
    int32_t partNum{0};
    uint32_t symPinNum{0};
    int32_t gateEq{0};
    int32_t pinEq{0};
    PCAD_PIN_TYPE pinType{PCAD_PIN_TYPE::PASSIVE};
    PcadPoint point;
    int32_t rotation{0};
    std::string length;
    std::string visible;
    std::string function;
};

struct PcadLine
{
    PcadPoint pt1;
    PcadPoint pt2;
    int32_t width{0};
    std::string style;
    std::string netNameRef;
};

struct PcadText
{
    std::string value;
    PcadPoint point;
    int32_t rotation{0};
    PCAD_JUSTIFICATION justify{PCAD_JUSTIFICATION::LOWER_LEFT};
    int32_t size{0};
    bool isVisible{true};
    std::string textStyleRef;
};

struct PcadBus
{
    std::string name;
    PcadPoint pt1;
    PcadPoint pt2;
    bool displayName{false};
    std::unique_ptr<PcadText> text;

    PcadBus() = default;
};

struct PcadJunction
{
    PcadPoint point;
    std::string netNameRef;

    PcadJunction() = default;
};

struct PcadField
{
    std::string name;
    PcadPoint point;
    std::string value;
    std::string textStyleRef;
    double rotation{0.0};
    bool isFlipped{false};
    PCAD_JUSTIFICATION justify{PCAD_JUSTIFICATION::LEFT};

    PcadField() = default;
};

struct PcadTitleSheet
{
    std::vector<std::unique_ptr<PcadLine>> lines;
    std::vector<std::unique_ptr<PcadText>> texts;

    PcadTitleSheet() = default;
};

struct PcadSymbol
{
    std::string name;
    std::string originalName;
    std::vector<std::unique_ptr<PcadPin>> pins;
    std::vector<std::unique_ptr<PcadLine>> lines;
    std::vector<std::unique_ptr<PcadText>> texts;
    std::vector<std::unique_ptr<PcadAttribute>> attributes;
};

struct PcadComponent
{
    std::string name;
    std::string originalName;
    std::string sourceLibrary;
    uint32_t numPins{0};
    uint32_t numParts{0};
    std::string refDesPrefix;
    std::vector<std::unique_ptr<PcadSymbol>> symbols;
    std::string patternName;
};

struct PcadLibrary
{
    std::string name;
    std::unordered_map<std::string, std::unique_ptr<PcadSymbol>> symbols;
    std::unordered_map<std::string, std::unique_ptr<PcadComponent>> components;
};

struct PcadInstance
{
    std::string name;         // Basic name field
    std::string part;         // Legacy field name for compatibility
    std::string compRef;
    std::string originalName;
    std::string compValue;
    std::string symbolRef;
    std::string patternName;
    PcadPoint point;
    int32_t rotation{0};
    bool isFlipped{false};
    std::vector<std::unique_ptr<PcadAttribute>> attributes;
};

struct PcadWire
{
    PcadPoint pt1;
    PcadPoint pt2;
    int32_t width{0};
    std::string netNameRef;
};

struct PcadPort
{
    PcadPoint point;
    PCAD_PIN_TYPE portType{PCAD_PIN_TYPE::NONE};
    std::string netNameRef;
    int32_t rotation{0};
    bool isFlipped{false};
    std::string portPinLength;

    PcadPort() = default;
};

struct PcadNet
{
    std::string name;
    std::vector<std::unique_ptr<PcadWire>> wires;
    std::vector<std::unique_ptr<PcadPort>> ports;
};

struct PcadSheet
{
    std::string name;
    std::vector<std::unique_ptr<PcadInstance>> instances;
    std::vector<std::unique_ptr<PcadNet>> nets;
    std::vector<std::unique_ptr<PcadLine>> lines;
    std::vector<std::unique_ptr<PcadText>> texts;
    std::vector<std::unique_ptr<PcadField>> fields;
    std::vector<std::unique_ptr<PcadJunction>> junctions;
    std::vector<std::unique_ptr<PcadBus>> buses;
    std::unique_ptr<PcadTitleSheet> titleSheet;
};

struct PcadSchematic
{
    std::string name;
    std::vector<std::unique_ptr<PcadSheet>> sheets;
};

/**
 * A SCH_IO derivation for loading P-CAD 2006 ASCII schematic files.
 */
class SCH_IO_PCAD : public SCH_IO
{
public:
    SCH_IO_PCAD();
    ~SCH_IO_PCAD() override;

    const IO_BASE::IO_FILE_DESC GetSchematicFileDesc() const override
    {
        return IO_BASE::IO_FILE_DESC( _HKI( "P-CAD 2006 ASCII schematic files" ), { "sch" } );
    }

    const IO_BASE::IO_FILE_DESC GetLibraryDesc() const override
    {
        return IO_BASE::IO_FILE_DESC( _HKI( "P-CAD 2006 ASCII library files" ), { "lib" } );
    }

    bool CanReadSchematicFile( const wxString& aFileName ) const override;
    bool CanReadLibrary( const wxString& aFileName ) const override;

    int GetModifyHash() const override;

    SCH_SHEET* LoadSchematicFile( const wxString& aFileName, SCHEMATIC* aSchematic,
                                  SCH_SHEET* aAppendToMe = nullptr,
                                  const std::map<std::string, UTF8>* aProperties = nullptr ) override;

    void EnumerateSymbolLib( wxArrayString& aSymbolNameList, const wxString& aLibraryPath,
                             const std::map<std::string, UTF8>* aProperties ) override;

    void EnumerateSymbolLib( std::vector<LIB_SYMBOL*>& aSymbolList,
                             const wxString& aLibraryPath,
                             const std::map<std::string, UTF8>* aProperties ) override;

    LIB_SYMBOL* LoadSymbol( const wxString& aLibraryPath, const wxString& aSymbolName,
                            const std::map<std::string, UTF8>* aProperties ) override;

    bool IsLibraryWritable( const wxString& aLibraryPath ) override { return false; }

private:
    // Token parsing
    enum class TokenType
    {
        NONE,
        OPEN_PAREN,
        CLOSE_PAREN,
        NAME,
        STRING,
        INTEGER,
        REAL,
        EOF_TOKEN
    };

    struct Token
    {
        TokenType type{TokenType::NONE};
        std::string value;
        int line{0};
        int column{0};
    };

    // Tokenizer class
    class PcadTokenizer
    {
    private:
        std::string m_content;
        size_t m_pos{0};
        int m_line{1};
        int m_column{1};
        bool m_hasPeekedToken{false};
        Token m_peekedToken;

        char currentChar() const;
        char peekChar() const;
        void advance();
        bool isAtEnd() const;
        void skipWhitespace();
        std::string readString();
        std::string readNumber();
        std::string readName();

    public:
        explicit PcadTokenizer(const std::string& content) : m_content(content) {}

        Token getNextToken();
        Token peekToken();
        void putBackToken(const Token& token);
    };

    // Parser flags and field structures
    enum class FieldFlags : int
    {
        NONE = 0,
        WRAPPED = 1,    // Field wrapped in parentheses
        NAKED = 2,      // Field not wrapped
        LIST = 4        // Field is a list/array
    };

    struct ParseField;
    struct ParseStruct;

    using ParseFunction = std::function<void(class ParseContext&, void*)>;

    struct ParseField
    {
        FieldFlags flags{FieldFlags::NONE};
        std::string tagString;
        ParseFunction parseFunction;
        size_t offset{0};
        size_t size{0};
    };

    struct ParseStruct
    {
        std::vector<ParseField> fixedFields;
        std::vector<ParseField> optionalFields;
    };

    // Parser context class
    class ParseContext
    {
    private:
        PcadTokenizer& m_tokenizer;
        std::stack<std::string> m_contextStack;
        PCAD_UNITS m_units{PCAD_UNITS::MIL};
        void* m_userData{nullptr};

    public:
        ParseContext(PcadTokenizer& tokenizer, void* userData = nullptr)
            : m_tokenizer(tokenizer), m_userData(userData) {}

        Token getNextToken() { return m_tokenizer.getNextToken(); }
        Token peekToken() { return m_tokenizer.peekToken(); }

        // Context management
        void pushContext(const std::string& context) { m_contextStack.push(context); }
        void popContext() { if (!m_contextStack.empty()) m_contextStack.pop(); }
        std::string getCurrentContext() const { return m_contextStack.empty() ? "" : m_contextStack.top(); }

        // Unit handling
        void setUnits(PCAD_UNITS units) { m_units = units; }
        PCAD_UNITS getUnits() const { return m_units; }

        // User data access
        void* getUserData() const { return m_userData; }
        void setUserData(void* data) { m_userData = data; }

        // Parsing utilities
        void expectToken(TokenType expected);
        void expectName(const std::string& expected);
        std::string getString();
        int32_t getInteger();
        double getReal();

        // Generic parsing
        void parseGeneric(const ParseStruct& parseStruct, void* object);

        // Error handling
        [[noreturn]] void error(const std::string& message);

        // Utility methods
        void skipUnknownField();
    };

    // Core parsing methods
    bool checkHeader( const wxString& aFileName ) const;
    void parseSchematicFile( const wxString& aFileName );
    void parseLibraryFile( const wxString& aFileName );

    // Parse structure definitions
    static ParseStruct getPointParseStruct();
    static ParseStruct getAttributeParseStruct();
    static ParseStruct getPinParseStruct();
    static ParseStruct getLineParseStruct();
    static ParseStruct getTextParseStruct();
    static ParseStruct getSymbolParseStruct();
    static ParseStruct getComponentParseStruct();
    static ParseStruct getLibraryParseStruct();
    static ParseStruct getInstanceParseStruct();
    static ParseStruct getWireParseStruct();
    static ParseStruct getPortParseStruct();
    static ParseStruct getNetParseStruct();
    static ParseStruct getSheetParseStruct();
    static ParseStruct getSchematicParseStruct();
    static ParseStruct getExtentParseStruct();
    static ParseStruct getFontParseStruct();
    static ParseStruct getStyleParseStruct();
    static ParseStruct getBusParseStruct();
    static ParseStruct getJunctionParseStruct();
    static ParseStruct getFieldParseStruct();
    static ParseStruct getTitleSheetParseStruct();

    // Parsing functions for basic types
    static void parseString(ParseContext& ctx, void* target);
    static void parseInt(ParseContext& ctx, void* target);
    static void parseReal(ParseContext& ctx, void* target);
    static void parseBoolean(ParseContext& ctx, void* target);

    // Higher-level parsing methods
    void parseSchematic(ParseContext& ctx, PcadSchematic* schematic);
    void parseLibrary(ParseContext& ctx, PcadLibrary* library);
    void parseSheet(ParseContext& ctx, PcadSheet* sheet);
    void parseSymbol(ParseContext& ctx, PcadSymbol* symbol);
    void parseComponent(ParseContext& ctx, PcadComponent* component);
    void parsePoint(ParseContext& ctx, PcadPoint* point);
    void parseText(ParseContext& ctx, PcadText* text);
    void parseLine(ParseContext& ctx, PcadLine* line);
    void parsePin(ParseContext& ctx, PcadPin* pin);
    void parseAttribute(ParseContext& ctx, PcadAttribute* attribute);
    void parseWire(ParseContext& ctx, PcadWire* wire);
    void parseNet(ParseContext& ctx, PcadNet* net);
    void parseInstance(ParseContext& ctx, PcadInstance* instance);

    // Conversion utilities
    SCH_SYMBOL* convertInstance( const PcadInstance& pcadInstance );
    LIB_SYMBOL* convertComponent( const PcadComponent& pcadComponent );
    SCH_PIN* convertPin( const PcadPin& pcadPin );
    SCH_SHAPE* convertLine( const PcadLine& pcadLine );
    SCH_TEXT* convertText( const PcadText& pcadText );
    ELECTRICAL_PINTYPE pcadToKiCadPinType(PCAD_PIN_TYPE pcadType) const;
    GR_TEXT_H_ALIGN_T pcadToKiCadHAlign(PCAD_JUSTIFICATION justify) const;
    GR_TEXT_V_ALIGN_T pcadToKiCadVAlign(PCAD_JUSTIFICATION justify) const;
    wxString cleanSymbolName(const std::string& name) const;

    // Parser utility methods
    void skipUnknownElement(ParseContext& ctx);

    // Member variables
    wxFileName m_filename;
    SCHEMATIC* m_schematic{nullptr};
    SCH_SHEET* m_rootSheet{nullptr};
    SCH_SHEET_PATH m_sheetPath;

    // Parsed data
    std::unique_ptr<PcadSchematic> m_pcadSchematic;
    std::unique_ptr<PcadLibrary> m_pcadLibrary;

    // Conversion state
    PCAD_UNITS m_units{PCAD_UNITS::MIL};
    double m_scaleFactor{1.0};
    int m_sheetIndex{0};

    // Symbol library management
    wxString m_libName;
    std::unordered_map<std::string, std::unique_ptr<LIB_SYMBOL>> m_convertedSymbols;
    IO_RELEASER<SCH_IO> m_pi;  ///< Plugin interface for creating KiCad symbol library

    // Constants
    static constexpr double MIL_TO_IU = 25400.0;  // nanometers per mil
    static constexpr double MM_TO_IU = 1000000.0;  // nanometers per mm
    static constexpr double IN_TO_IU = 25400000.0; // nanometers per inch
};

// Bitwise operations for FieldFlags
inline SCH_IO_PCAD::FieldFlags operator|(SCH_IO_PCAD::FieldFlags a, SCH_IO_PCAD::FieldFlags b)
{
    return static_cast<SCH_IO_PCAD::FieldFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline SCH_IO_PCAD::FieldFlags operator&(SCH_IO_PCAD::FieldFlags a, SCH_IO_PCAD::FieldFlags b)
{
    return static_cast<SCH_IO_PCAD::FieldFlags>(static_cast<int>(a) & static_cast<int>(b));
}

inline bool operator!=(SCH_IO_PCAD::FieldFlags a, SCH_IO_PCAD::FieldFlags b)
{
    return static_cast<int>(a) != static_cast<int>(b);
}

#endif // SCH_IO_PCAD_H_
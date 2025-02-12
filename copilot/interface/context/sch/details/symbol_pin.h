#ifndef FC544252_F0B7_4E2C_AAA6_80E4C77DE94A
#define FC544252_F0B7_4E2C_AAA6_80E4C77DE94A

#include <nlohmann/json.hpp>
#include <string>

struct SYMBOL_PIN
{
    std::string number;
    std::string name;
    std::string shape;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SYMBOL_PIN, number, name, shape)
};


#endif /* FC544252_F0B7_4E2C_AAA6_80E4C77DE94A */

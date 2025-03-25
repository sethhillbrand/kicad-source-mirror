#ifndef A19A1F34_6E11_4B7B_AB98_2468C9850261
#define A19A1F34_6E11_4B7B_AB98_2468C9850261

#include <nlohmann/json.hpp>
#include "symbol_pin.h"

struct SYMBOL_PROPERTIES{
    std::string name;
    std::string description;
    std::string footprint;
    std::vector<SYMBOL_PIN> pins;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SYMBOL_PROPERTIES, name, description, footprint, pins)
};


#endif /* A19A1F34_6E11_4B7B_AB98_2468C9850261 */

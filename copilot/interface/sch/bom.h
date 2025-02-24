#ifndef F0AB5A2D_1CD0_45D0_ADFB_7A77B6EC6294
#define F0AB5A2D_1CD0_45D0_ADFB_7A77B6EC6294


#include <nlohmann/json.hpp>

#include "symbol_properties.h"


struct BOM_ITEM : SYMBOL_PROPERTIES{
    std::vector<std::string> designators;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BOM_ITEM, designators)
};

struct BOM{
    std::vector<BOM_ITEM> items;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BOM, items)
};



#endif /* F0AB5A2D_1CD0_45D0_ADFB_7A77B6EC6294 */

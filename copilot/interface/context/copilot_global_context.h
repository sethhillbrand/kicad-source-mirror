/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2025 Ethan Chien <liangtie.qian@gmail.com>
 * Copyright (C) 2025 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef COPILOT_GLOBAL_CONTEXT_H
#define COPILOT_GLOBAL_CONTEXT_H

#include <nlohmann/json.hpp>
#include <string>
#include <kicad_version_info.h>
#include <vector>


namespace copilot
{

struct PROJECT_FILE
{
    std::string name;
    std::string ext;
    std::string path;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( PROJECT_FILE, path, name, ext )
};


struct PROJECT_CONTEXT
{
    std::string               project_name;
    std::string               project_path;
    std::vector<PROJECT_FILE> files;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( PROJECT_CONTEXT, project_name, project_path )
};


}; // namespace copilot


struct COPILOT_GLOBAL_CONTEXT
{
    std::string              uuid;
    KICAD_VERSION_INFO       kicad_version_info;
    copilot::PROJECT_CONTEXT project_context;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( COPILOT_GLOBAL_CONTEXT, uuid, kicad_version_info,
                                    project_context )
};


#endif

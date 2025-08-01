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

#ifndef PCB_FAB_SETTINGS_CONTEXT_H
#define PCB_FAB_SETTINGS_CONTEXT_H

#include "pcb_fab_distance_settings.h"


struct PCB_FAB_SETTINGS
{
    PCB_FAB_DISTANCE_SETTINGS distance_settings;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( PCB_FAB_SETTINGS, distance_settings )
};


struct PCB_FAB_SETTINGS_CONTEXT
{
    PCB_FAB_SETTINGS fab_settings;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE( PCB_FAB_SETTINGS_CONTEXT, fab_settings )

    virtual ~PCB_FAB_SETTINGS_CONTEXT() = default;
};

#endif

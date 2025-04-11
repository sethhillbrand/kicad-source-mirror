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

#ifndef COPILOT_UTILS_H
#define COPILOT_UTILS_H

#include <algorithm>
#include <string>

#include <random>
#include <sstream>
#include <iomanip>

// Helper function to generate a UUID
std::string generate_uuid() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(8) << dis(gen) << "-"
        << std::setw(4) << (dis(gen) & 0xFFFF) << "-"
        << std::setw(4) << ((dis(gen) & 0x0FFF) | 0x4000) << "-" // Version 4
        << std::setw(4) << ((dis(gen) & 0x3FFF) | 0x8000) << "-" // Variant 1
        << std::setw(12) << dis(gen) << dis(gen);

    return oss.str();
}

// Modify the session_id assignment

inline auto convert_to_upper( std::string& str )
{
    return std::transform( str.begin(), str.end(), str.begin(), ::toupper );
}

#endif

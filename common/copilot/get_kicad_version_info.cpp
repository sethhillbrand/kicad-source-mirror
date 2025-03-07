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

#include <wx/wx.h>
#include <config.h>
#include <boost/version.hpp>
#include <kiplatform/app.h>
#include <font/version_info.h>
#include <build_version.h>

#include "get_kicad_version_info.h"
KICAD_VERSION_INFO get_kicad_version_info()
{
    wxString indent4 = "\t";

    const auto version =
            ( KIPLATFORM::APP::IsOperatingSystemUnsupported() ? wxString( wxS( "(UNSUPPORTED)" ) )
                                                              : GetSemanticVersion() );

    wxPlatformInfo platform;
    wxString       osDescription;

#if __LINUX__
    osDescription = wxGetLinuxDistributionInfo().Description;
#endif

    // Linux uses the lsb-release program to get the description of the OS, if lsb-release
    // isn't installed, then the string will be empty and we fallback to the method used on
    // the other platforms (to at least get the kernel/uname info).
    if( osDescription.empty() )
        osDescription = wxGetOsDescription();

    return { version.ToStdString(),
             osDescription.ToStdString(),
             GetPlatformGetBitnessName().ToStdString(),
             platform.GetEndiannessName().ToStdString(),
             platform.GetPortIdName().ToStdString(),
             wxGetLibraryVersionInfo().GetVersionString().ToStdString() };
}

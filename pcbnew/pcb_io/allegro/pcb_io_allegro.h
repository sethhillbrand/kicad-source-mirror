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

#ifndef PCB_IO_ALLEGRO_H_
#define PCB_IO_ALLEGRO_H_

#include <pcb_io/pcb_io.h>
#include "allegro_parser.h"

class PCB_IO_ALLEGRO : public PCB_IO
{
public:
    PCB_IO_ALLEGRO();
    ~PCB_IO_ALLEGRO() override;


    const IO_FILE_DESC GetBoardFileDesc() const override
    {
        return IO_FILE_DESC( _HKI("Cadence Allegro layout file"), { "brd" } );
    }

    BOARD* LoadBoard( const wxString& aFileName, BOARD* aAppendToMe,
                      const std::map<std::string, UTF8>* aProperties,
                      PROJECT* aProject,
                      PROGRESS_REPORTER* aReporter );

    const IO_FILE_DESC GetLibraryDesc() const override { return IO_FILE_DESC(); }
    long long GetLibraryTimestamp( const wxString& ) const override { return 0; }
};

#endif // PCB_IO_ALLEGRO_H_

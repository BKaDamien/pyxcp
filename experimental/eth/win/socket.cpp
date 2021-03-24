/*
 * pyXCP
 *
 * (C) 2021 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */
#include <cstdio>
#include "exceptions.hpp"
#include "eth.hpp"
#include "socket.hpp"
#include "periodata.hpp"

#include <MSWSock.h>


class WinSockBoilerplate {
public:
    WinSockBoilerplate() {
        WSAData data;

        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
            throw OSException();
        }
    }

    ~WinSockBoilerplate()
    {
        WSACleanup();
    }
};


static WinSockBoilerplate boilerplate; // Ensure WinSock startup/shutdown.

Socket::Pool_t Socket::m_iod_pool;  // Initialize static member.


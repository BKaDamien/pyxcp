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

#if !defined(__ISOCKET_HPP)
#define __ISOCKET_HPP

#include "eth.hpp"

struct CAddress {
    int length;
    struct sockaddr address;
};


class ISocket {
public:
    ~ISocket() = default;

    virtual void getOption(int option, char * optval, int * optlen) = 0;
    virtual void setOption(int option, const char * optval, int optlen) = 0;
    virtual bool getaddrinfo(int family, int socktype, int protocol, const char * hostname, int port, CAddress & address, int flags = AI_PASSIVE) = 0;
    virtual bool connect(CAddress & address) = 0;
    virtual bool disconnect() = 0;
    virtual bool bind(CAddress & address) = 0;
    virtual bool listen(int backlog = 10) = 0;
    virtual bool accept(CAddress & peerAddress) = 0;

private:
    int m_family;
    int m_socktype;
    int m_protocol;
    bool m_connected;
};

#endif  // __ISOCKET_HPP


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

Socket::Socket(int family, int socktype, int protocol, int options)
{
//    WSAPROTOCOL_INFO protocolInfo;
//    bool connection_oriented = (socktype == SOCK_STREAM);
    m_family = family;
    m_socktype = socktype;
    m_protocol = protocol;
    m_connected = false;
    m_addr = NULL;
    loadFunctions();

#if 0
    ZeroOut(&protocolInfo, sizeof(WSAPROTOCOL_INFO));
    protocolInfo.dwServiceFlags1  = XP1_GUARANTEED_DELIVERY | XP1_GUARANTEED_ORDER | XP1_GRACEFUL_CLOSE |
        XP1_MESSAGE_ORIENTED | (connection_oriented ? 0: XP1_CONNECTIONLESS);
#endif

    m_socket = ::WSASocket(family, socktype, protocol, NULL /*&protocolInfo*/, 0, WSA_FLAG_OVERLAPPED | options);
    if (m_socket == INVALID_SOCKET) {
        SocketErrorExit("Socket::Socket()");
    }
    printf("m_socket: %x %d\n", m_socket, m_socket == INVALID_SOCKET);
    ZeroOut(&m_peerAddress, sizeof(SOCKADDR_STORAGE));
}

Socket::~Socket()
{
    ::closesocket(m_socket);
}

void Socket::loadFunctions()
{
    int res;
    DWORD dwBytes;
    GUID guid = WSAID_CONNECTEX;
    SOCKET dummy_socket;

    dummy_socket = ::socket(AF_INET, SOCK_STREAM, 0);
    res = ::WSAIoctl(
        dummy_socket,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guid, sizeof(guid),
        &connectEx, sizeof(connectEx),
        &dwBytes, NULL, NULL
    );
    ::closesocket(dummy_socket);
}

HANDLE Socket::getHandle() const
{
    return reinterpret_cast<HANDLE>(m_socket);
}

void Socket::setOption(int option, const char * optval, int optlen)
{
    ::setsockopt(m_socket, SOL_SOCKET, option, optval, optlen);
}

void Socket::getOption(int option, char * optval, int * optlen)
{
    ::getsockopt(m_socket, SOL_SOCKET, option, optval, optlen);
}

bool Socket::bind(CAddress & address)
{

    if (::bind(m_socket, &address.address, address.length) == SOCKET_ERROR) {
        SocketErrorExit("Socket::bind()");
        return false;
    }
    return true;
}

bool Socket::connect(CAddress & address)
{
    if (::connect(m_socket, &address.address, address.length) == SOCKET_ERROR) {
        SocketErrorExit("Socket::connect()");
        return false;
    }
    PerHandleData handleData(HandleType::HANDLE_SOCKET, getHandle());
    //m_iocp->registerHandle(&handleData);
    m_connected = true;
    return true;
}

bool Socket::disconnect()
{
    m_connected == false; //::disconnect();
    return true;
}

bool Socket::listen(int backlog)
{
    if (::listen(m_socket, backlog) == SOCKET_ERROR) {
        SocketErrorExit("Socket::listen()");
        return false;
    }
    return true;
}

bool Socket::accept(CAddress & peerAddress)
{
    SOCKET sock;

    peerAddress.length = sizeof peerAddress.address;
    sock = ::accept(m_socket, (sockaddr *)&peerAddress.address, &peerAddress.length);

    if (sock  == INVALID_SOCKET) {
        SocketErrorExit("Socket::accept()");
        return false;
    }
    return true;
}

bool Socket::getaddrinfo(int family, int socktype, int protocol, const char * hostname, int port, CAddress & address, int flags)
{
    int err;
    ADDRINFO hints;
    ADDRINFO * t_addr;
    char port_str[16] = {0};

    ZeroOut(&hints, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = socktype;
    hints.ai_protocol = protocol;
    hints.ai_flags = flags;

    ::sprintf(port_str, "%d", port);
    err = ::getaddrinfo(hostname, port_str, &hints, &t_addr);
    if (err != 0) {
        gai_strerror(err);
        ::freeaddrinfo(t_addr);
        return false;
    }

    address.length = t_addr->ai_addrlen;
    ::CopyMemory(&address.address, t_addr->ai_addr, sizeof(struct sockaddr));

    ::freeaddrinfo(t_addr);
    return true;
}


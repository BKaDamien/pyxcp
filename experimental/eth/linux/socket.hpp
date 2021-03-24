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

#if !defined(__SOCKET_HPP)
#define __SOCKET_HPP

#include <array>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "isocket.hpp"
#include "utils.hpp"

#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (-1)

class Socket : public ISocket {
public:

    Socket(int family = PF_INET, int socktype = SOCK_STREAM, int protocol = IPPROTO_TCP) :
        m_family(family), m_socktype(socktype), m_protocol(protocol), m_connected(false),
        m_addr(nullptr) {
        m_socket = ::socket(m_family, m_socktype, m_protocol);
        if (m_socket == INVALID_SOCKET) {
            SocketErrorExit("Socket::Socket()");
        }
        ZeroOut(&m_peerAddress, sizeof(sockaddr_storage));
    }

    ~Socket() {
        ::close(m_socket);
    }

    void option(int optname, int level, int * value) {
        socklen_t len;

        len = sizeof(*value);
        if (*value == 0) {
            ::getsockopt(m_socket, level, optname, (char*) value, &len);
        } else {
            ::setsockopt(m_socket, level, optname, (const char*) value, len);
        }
    }

    bool getaddrinfo(int family, int socktype, int protocol, const char * hostname, int port, CAddress & address, int flags = AI_PASSIVE) {
        int err;
        addrinfo hints;
        addrinfo * t_addr;
        char port_str[16] = {0};

        ZeroOut(&hints, sizeof(hints));
        hints.ai_family = family;
        hints.ai_socktype = socktype;
        hints.ai_protocol = protocol;
        hints.ai_flags = flags;

        ::sprintf(port_str, "%d", port);
        err = ::getaddrinfo(hostname, port_str, &hints, &t_addr);
        if (err != 0) {
            printf("%s\n", gai_strerror(err));
            ::freeaddrinfo(t_addr);
            SocketErrorExit("getaddrinfo()");
            return false;
        }

        address.length = t_addr->ai_addrlen;
        ::memcpy(&address.address, t_addr->ai_addr, sizeof(struct sockaddr));

        ::freeaddrinfo(t_addr);
        return true;
    }

    void connect(CAddress & address) {
        if (::connect(m_socket, &address.address, address.length) == SOCKET_ERROR) {
            SocketErrorExit("Socket::connect()");
        }
    }

    void bind(CAddress & address) {
        if (::bind(m_socket, &address.address, address.length) == SOCKET_ERROR) {
            SocketErrorExit("Socket::bind()");
        }
    }

    void listen(int backlog = 5) {
        if (::listen(m_socket, backlog) == SOCKET_ERROR) {
            SocketErrorExit("Socket::listen()");
        }
    }

    void accept(CAddress & peerAddress) {
        int sock;

        peerAddress.length = sizeof peerAddress.address;
        sock = ::accept(m_socket, (sockaddr *)&peerAddress.address, (socklen_t*)&peerAddress.length);

        if (sock  == INVALID_SOCKET) {
            SocketErrorExit("Socket::accept()");
        }
    }

#if 0
    template <typename T, size_t N>
    void write(std::array<T, N>& arr, bool alloc = true) {
        DWORD bytesWritten = 0;
        int addrLen;
        //PerIoData * iod = new PerIoData(128);
        PerIoData * iod;

        if (alloc == true) {
            iod = m_pool_mgr.get_iod().acquire();
            //iod = m_iod_pool.acquire();
        }
        iod->reset();
        iod->set_buffer(arr);
        iod->set_opcode(IoType::IO_WRITE);
        iod->set_transfer_length(arr.size());
        if (m_socktype == SOCK_DGRAM) {
            addrLen = sizeof(SOCKADDR_STORAGE);
            if (::WSASendTo(m_socket,
                iod->get_buffer(),
                1,
                &bytesWritten,
                0,
                (LPSOCKADDR)&m_peerAddress,
                addrLen,
                (LPWSAOVERLAPPED)iod,
                nullptr
            ) == SOCKET_ERROR) {
                // WSA_IO_PENDING
                SocketErrorExit("Socket::send()");
            }
        } else if (m_socktype == SOCK_STREAM) {
            if (::WSASend(
                m_socket,
                iod->get_buffer(),
                1,
                &bytesWritten,
                0,
                (LPWSAOVERLAPPED)iod,
                nullptr) == SOCKET_ERROR) {
                    SocketErrorExit("Socket::send()");
                closesocket(m_socket);
            }
        }
        printf("Status: %d bytes_written: %d\n", WSAGetLastError(), bytesWritten);
    }
#endif
    void triggerRead(unsigned int len);

#if 0
    HANDLE getHandle() const {
        return reinterpret_cast<HANDLE>(m_socket);
    }
#endif

private:
    int m_family;
    int m_socktype;
    int m_protocol;
    bool m_connected;
//    PoolManager m_pool_mgr;
    addrinfo * m_addr;
    int m_socket;
    //CAddress ourAddress;
    sockaddr_storage m_peerAddress;
};

#endif  // __SOCKET_HPP


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
#if !defined(__IOCP_HPP)
#define __IOCP_HPP

//#include "eth.hpp"
#include "socket.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

//#include "utils.hpp"

#if !defined(__GNUC__)
#pragma comment(lib,"ws2_32.lib") // MSVC only.
#endif


enum class HandleType {
    HANDLE_SOCKET,
    HANDLE_FILE,
    HANDLE_NAMED_PIPE,
    HANDLE_USER,
};

enum class IoType {
    IO_ACCEPT,
    IO_CONNECT,
    IO_READ,
    IO_WRITE
};

struct PerPortData {
    HANDLE handle;
};


struct PerHandleData {
    HandleType m_handleType;
    Socket * m_socket;
    DWORD m_seqNoSend;
    DWORD m_seqNoRecv;

    PerHandleData(HandleType handleType, Socket * socket) : m_handleType(handleType),
        m_socket(socket), m_seqNoSend(0), m_seqNoRecv(0) {}

};

struct ThreadType {
    HANDLE handle;
    DWORD id;
};

class IOCP {
public:
    IOCP(DWORD numProcessors = 0);
    ~IOCP();
    bool registerHandle(PerHandleData * object);
    void postUserMessage() const;
    void postQuitMessage() const;
    HANDLE getHandle() const;
private:
    PerPortData m_port;
    DWORD m_numWorkerThreads;
    std::vector<HANDLE> m_threads;
};

#endif // __IOCP_HPP


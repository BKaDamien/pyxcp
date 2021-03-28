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

#include "iocp.hpp"
#include "socket.hpp"
#include "exceptions.hpp"

#include <iostream>
#include <cstdio>
#include <cstddef>


/*
 *
 *
 *
 */


static DWORD WINAPI WorkerThread(LPVOID lpParameter);

IOCP::IOCP(size_t numProcessors, size_t multiplier)
{
    m_numWorkerThreads = numProcessors * multiplier;
    m_port.handle  = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, static_cast<ULONG_PTR>(0), m_numWorkerThreads);
    if (m_port.handle == nullptr) {
        OsErrorExit("IOCP::IOCP()");
    }

    m_threads.reserve(m_numWorkerThreads);

    HANDLE hThread;

    for (DWORD idx = 0; idx < m_numWorkerThreads; ++idx) {
        hThread = ::CreateThread(nullptr, 0, WorkerThread, reinterpret_cast<LPVOID>(this), 0, nullptr);
        ::SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
        m_threads.push_back(hThread);
    }
}

IOCP::~IOCP()
{
    DWORD numThreads = static_cast<DWORD>(m_threads.size());
    std::ldiv_t divres = std::ldiv(numThreads, MAXIMUM_WAIT_OBJECTS);
    DWORD rounds = static_cast<DWORD>(divres.quot);
    DWORD remaining = static_cast<DWORD>(divres.rem);
    HANDLE * thrArray = nullptr;
    DWORD offset = 0;
    DWORD idx = 0;

    postQuitMessage();

    thrArray = new HANDLE[MAXIMUM_WAIT_OBJECTS];
    for (DWORD r = 0; r < rounds; ++r) {
        for (idx = 0; idx < MAXIMUM_WAIT_OBJECTS; ++idx) {
            thrArray[idx] = m_threads.at(idx + offset);
        }
        ::WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS, thrArray, TRUE, INFINITE);
        for (idx = 0; idx < MAXIMUM_WAIT_OBJECTS; ++idx) {
            ::CloseHandle(thrArray[idx]);
        }
        offset += MAXIMUM_WAIT_OBJECTS;
    }

    if (remaining > 0) {
        for (idx = 0; idx < remaining; ++idx) {
            thrArray[idx] = m_threads.at(idx + offset);
        }
        ::WaitForMultipleObjects(remaining, thrArray, TRUE, INFINITE);
        for (idx = 0; idx < remaining; ++idx) {
            ::CloseHandle(thrArray[idx]);
        }
    }
    delete[] thrArray;
    ::CloseHandle(m_port.handle);
}

void IOCP::registerHandle(const PerHandleData& object) const
{
    HANDLE handle;
    bool ok;

    handle = ::CreateIoCompletionPort(object.m_handle, m_port.handle, reinterpret_cast<ULONG_PTR>(&object), 0);
    ok = (handle == m_port.handle);
    if ((handle == nullptr) || (!ok)) {
        OsErrorExit("IOCP::registerHandle()");
    }
}


void IOCP::registerSocket(const Socket& socket) const
{
    auto handleData = PerHandleData(HandleType::HANDLE_SOCKET, socket.getHandle());

    registerHandle(handleData);

}

void IOCP::postQuitMessage() const
{
    if (!::PostQueuedCompletionStatus(m_port.handle, 0, static_cast<ULONG_PTR>(NULL), nullptr)) {
        OsErrorExit("IOCP::postQuitMessage()");
    }
}

HANDLE IOCP::getHandle() const
{
    return m_port.handle;
}

void IOCP::postUserMessage() const
{
    ::PostQueuedCompletionStatus(m_port.handle, 0, static_cast<ULONG_PTR>(NULL), nullptr);
}


static DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
    IOCP const * const iocp = reinterpret_cast<IOCP const * const>(lpParameter);
    DWORD bytesTransfered = 0;
    ULONG_PTR completionKey;
    PerIoData * iod = nullptr;
    PerHandleData * phd = nullptr;
    OVERLAPPED * olap = nullptr;
    bool exitLoop = false;
    DWORD error;

    printf("Entering worker thread %d.\n", ::GetCurrentThreadId());
    while (!exitLoop) {
        if (::GetQueuedCompletionStatus(iocp->getHandle(), &bytesTransfered, &completionKey, (LPOVERLAPPED*)&olap, INFINITE)) {
            if ((bytesTransfered == 0) &&  (completionKey == static_cast<ULONG_PTR>(NULL))) {
                iocp->postQuitMessage();    // "Broadcast"
                exitLoop = true;
            } else {
                phd = reinterpret_cast<PerHandleData *>(completionKey);
                iod = reinterpret_cast<PerIoData* >(olap);
                printf("\tOPCODE: %d bytes: %d\n", iod->get_opcode(), bytesTransfered);
                switch (iod->get_opcode()) {
                    case IoType::IO_WRITE:
                        iod->decr_bytes_to_xfer(bytesTransfered);
//                        phd->m_socket->triggerRead(1024);
                        if (iod->xfer_finished()) {
                            delete iod;
                        } else {
                            //iod->m_wsabuf.buf = iod->m_wsabuf.buf + (iod->get_bytes_to_xfer() - iod->m_bytesRemaining);
                            //iod->m_wsabuf.len = iod->m_bytesRemaining;
                            iod->reset();
                        }
                        break;
                    case IoType::IO_READ:
                        printf("IO_READ() numBytes: %d\n", bytesTransfered);
                        break;
                    case IoType::IO_ACCEPT:
                        break;
                }
            }
        } else {
            error = ::GetLastError();
            if (olap == nullptr) {

            } else {
                // Failed I/O operation.
                // The function stores information in the variables pointed to by lpNumberOfBytes, lpCompletionKey.
            }
            //Win_ErrorMsg("IOWorkerThread::GetQueuedCompletionStatus()", error);
        }
    }
    printf("Exiting worker thread %d\n", ::GetCurrentThreadId());
    ::ExitThread(0);
}

#if 0


void Socket::triggerRead(unsigned int len)
{
    DWORD numReceived = (DWORD)0;
    DWORD flags = (DWORD)0;
    DWORD err = 0;
    int addrLen;
    static char buf[1024];

    PerIoData * iod = new PerIoData(128);

    iod->m_wsabuf.buf = buf;
    iod->m_wsabuf.len = len;
    iod->m_opcode = IoType::IO_READ;
    iod->m_bytesRemaining = iod->m_bytesToXfer = len;
    iod->reset();

    if (m_socktype == SOCK_STREAM) {
        if (WSARecv(m_socket,
                    &iod->m_wsabuf,
                    1,
                    &numReceived,
                    &flags,
                    (LPWSAOVERLAPPED)&iod->m_overlapped,
                    (LPWSAOVERLAPPED_COMPLETION_ROUTINE)nullptr)  == SOCKET_ERROR) {
            err = WSAGetLastError();
            if (err != WSA_IO_PENDING) {

            }
        }
    } else if (m_socktype == SOCK_DGRAM) {
        addrLen = sizeof(SOCKADDR_STORAGE);
        if (WSARecvFrom(m_socket,
                    &iod->m_wsabuf,
                    1,
                    &numReceived,
                    &flags,
                    (LPSOCKADDR)&numReceived,
                    &addrLen,
                    (LPWSAOVERLAPPED)&iod->m_overlapped,
                    (LPWSAOVERLAPPED_COMPLETION_ROUTINE)nullptr)) {
            err = WSAGetLastError();
            if (err != WSA_IO_PENDING) {

            }
        }
    }
}

typedef std::function<void(DWORD transferred)> CompleteHandler_t;

class AsyncIoServiceFactory {

};

#endif


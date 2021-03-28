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
#if !defined(__EPOLL_HPP)
#define __EPOLL_HPP

#include <map>
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>

#include "socket.hpp"
#include "iasyncioservice.hpp"

void *  WorkerThread(void * param);

class Epoll : public IAsyncIoService {
public:
    Epoll(size_t numProcessors = 1, size_t multiplier = 1) {
        int ret;

        m_epoll_fd = ::epoll_create(42);
        ret = pthread_create(&m_worker_thread, nullptr, &WorkerThread, reinterpret_cast<void*>(this));
        if (ret != 0) {
            OsErrorExit("Epoll:Epoll() -- Create worker thread");
        }
    }

    ~Epoll() {
        ::close(m_epoll_fd);
    }

    void registerSocket(Socket& socket) {

        registerHandle(socket.getHandle(), reinterpret_cast<void*>(&socket));
        //registerHandle(socket.getTimeout().getHandle(), reinterpret_cast<void*>(&socket));
        printf("S: %d T: %d\n", socket.getHandle(), socket.getTimeout().getHandle());
    }

    void postUserMessage() const {}
    void postQuitMessage() const {}

    HANDLE getHandle() const {
        return m_epoll_fd;
    }

protected:

    void registerHandle(HANDLE handle, void * data_ptr) {

        struct epoll_event event;

        event.data.ptr = data_ptr;
        event.events = EPOLLIN;
        if (::epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, handle, &event) == -1) {
            OsErrorExit("Epoll::registerHandle()");
        }
    }

private:
    int m_epoll_fd;
    pthread_t m_worker_thread;
    std::map<HANDLE, Socket&> m_timeouts;
};


#endif // __EPOLL_HPP


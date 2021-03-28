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

#include "epoll.hpp"

constexpr size_t MAX_EVENTS = 8;

static struct epoll_event events[MAX_EVENTS];

void * WorkerThread(void * param)
{
    Epoll const * const epoll = reinterpret_cast<Epoll const * const>(param);
    Socket * socket;
    int nfds;
    int idx;
    char buffer[128];
    int evt_mask;

    printf("Entering worker thread...\n");

    for (;;) {
        nfds = epoll_wait(epoll->getHandle() ,events, MAX_EVENTS, 500);
        for (idx = 0; idx < nfds; ++idx) {
            socket = reinterpret_cast<Socket*>(events[idx].data.ptr);
            evt_mask = events[idx].events;
            printf("Evt#%d: %x\n", evt_mask);
            if (evt_mask & EPOLLIN) {
                read(socket->getHandle(), buffer, 128);
                printf("R: %s\n", buffer);
            } else if (evt_mask & EPOLLHUP) {
                printf("HANG-UP\n");
                //SocketErrorExit("HANG-UP");
            } else if (evt_mask & EPOLLERR) {
                SocketErrorExit("WorkerThread::epoll_wait()");
            }
        }
    }

    return nullptr;
}


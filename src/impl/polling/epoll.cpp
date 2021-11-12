/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "epoll.hpp"
#include <dci/poll/descriptor.hpp>
#include <dci/poll/error.hpp>

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

namespace dci::poll::impl::polling
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Epoll::Epoll()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Epoll::~Epoll()
    {
        interrupt();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Epoll::initialize()
    {
        if(-1 != _fd)
        {
            return error::already_initialized;
        }

        _fd = epoll_create1(EPOLL_CLOEXEC);
        if(-1 == _fd)
        {
            return std::error_code{errno, std::generic_category()};
        }

        _interruptEvent = eventfd(1, EFD_NONBLOCK|EFD_CLOEXEC);

        if(0 > _interruptEvent)
        {
            int err = errno;
            deinitialize();
            return std::error_code(err, std::generic_category());
        }

        epoll_event evt{0,{nullptr}};
        evt.data.fd = _interruptEvent;
        evt.events = EPOLLIN | EPOLLOUT | EPOLLET;

        if(epoll_ctl(_fd, EPOLL_CTL_ADD, _interruptEvent, &evt))
        {
            int err = errno;
            deinitialize();
            return std::error_code(err, std::generic_category());
        }

        return std::error_code{};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Epoll::initialized() const
    {
        return -1 != _fd;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Epoll::installDescriptor(Descriptor* d)
    {
        epoll_event evt{0,{nullptr}};
        evt.data.ptr = d;
        evt.events = EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLET;

        if(epoll_ctl(_fd, EPOLL_CTL_ADD, d->fd(), &evt))
        {
            return std::error_code{errno, std::generic_category()};
        }

        return std::error_code{};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Epoll::uninstallDescriptor(Descriptor* d)
    {
        if(epoll_ctl(_fd, EPOLL_CTL_DEL, d->fd(), nullptr))
        {
            return std::error_code{errno, std::generic_category()};
        }

        return std::error_code{};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Epoll::execute(clocking::Duration timeout)
    {
        if(timeout.count() < 0)
        {
            timeout = std::chrono::milliseconds(-1);
        }

        epoll_event* eventsBuffer = reinterpret_cast<epoll_event *>(_eventsBuffer);
        int eventsAmount = epoll_wait(
            _fd,
            eventsBuffer,
            sizeof(_eventsBuffer)/sizeof(epoll_event),
            static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()));

        if(-1 == eventsAmount)
        {
            if(-1 == _fd)
            {
                return error::not_initialized;
            }

            return std::error_code{errno, std::generic_category()};
        }

        for(int idx(0); idx<eventsAmount; ++idx)
        {
            epoll_event& evt = eventsBuffer[idx];

            std::uint_fast32_t readyState =
                    ((evt.events & (EPOLLIN))               ? static_cast<std::uint_fast32_t>(poll::Descriptor::rsf_read ) : 0) |
                    ((evt.events & (EPOLLPRI))              ? static_cast<std::uint_fast32_t>(poll::Descriptor::rsf_pri )  : 0) |
                    ((evt.events & (EPOLLOUT))              ? static_cast<std::uint_fast32_t>(poll::Descriptor::rsf_write) : 0) |
                    ((evt.events & (EPOLLERR))              ? static_cast<std::uint_fast32_t>(poll::Descriptor::rsf_error) : 0) |
                    ((evt.events & (EPOLLHUP|EPOLLRDHUP))   ? static_cast<std::uint_fast32_t>(poll::Descriptor::rsf_eof  ) : 0) |
                    0;

            if(_interruptEvent == evt.data.fd)
            {
                if(poll::Descriptor::rsf_read & readyState)
                {
                    eventfd_t value;
                    eventfd_read(_interruptEvent, &value);
                }
            }
            else
            {
                Descriptor* d = static_cast<Descriptor *>(evt.data.ptr);
                d->setReadyState(readyState);
            }
        }

        return std::error_code{};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Epoll::interrupt()
    {
        if(-1 == _fd)
        {
            return error::not_initialized;
        }

        int res = eventfd_write(_interruptEvent, 1);

        if(0 > res)
        {
            return std::error_code{errno, std::generic_category()};
        }

        return std::error_code{};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Epoll::deinitialize()
    {
        if(-1 == _fd)
        {
            return error::not_initialized;
        }

        {
            int closeRes = ::close(_interruptEvent);
            while(0 != closeRes && EINTR == errno)
            {
                //try again
                closeRes = ::close(_interruptEvent);
            }
            _interruptEvent = -1;
        }

        std::error_code ec;

        int closeRes = ::close(_fd);
        while(0 != closeRes && EINTR == errno)
        {
            //try again
            closeRes = ::close(_fd);
        }

        if(closeRes)
        {
            ec = std::error_code{errno, std::generic_category()};
        }

        _fd = -1;

        return ec;
    }

}

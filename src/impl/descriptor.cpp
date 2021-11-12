/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "descriptor.hpp"
#include "service.hpp"
#include <dci/poll/descriptor.hpp>
#include <dci/logger.hpp>
#include <dci/utils/atScopeExit.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace dci::poll::impl
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Descriptor::Descriptor(int fd, cmt::task::Owner* actOwner, cmt::Raisable* raisable)
        : _fd{fd}
        , _actOwner{(actOwner ? actOwner : &_localActOwner)}
        , _raisable{raisable}
    {
        if(valid())
        {
            std::error_code ec = install();
            if(ec)
            {
                setReadyState(poll::Descriptor::rsf_error);
                close();
            }
        }

        if(_readyState && _raisable)
        {
            _raisable->raise();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Descriptor::~Descriptor()
    {
        _onAct->_owner = {};
        close();
        _localActOwner.stop(false);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    sbs::Signal<void, int /*fd*/, std::uint_fast32_t /*readyState*/> Descriptor::onAct()
    {
        return _onAct->_wire.out();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::emitActIfNeed()
    {
        if(_readyState && _onAct->_wire.connected() && !_onAct->_inProgress)
        {
            std::uint_fast32_t readyState = _readyState;
            _readyState = 0;
            _onAct->_inProgress = true;
            cmt::spawn() += _actOwner * [fd{_fd}, readyState, onAct{_onAct}, cleaner{dci::utils::AtScopeExit{[onAct=_onAct]{onAct->_inProgress=false;}}}]
            {
                onAct->_wire.in(fd, readyState);

                if(onAct->_owner)
                    onAct->_owner->emitActIfNeed();
            };
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::setActOwner(cmt::task::Owner* actOwner)
    {
        if (actOwner != _actOwner)
        {
            _actOwner->stop(false);
        }
        _actOwner = actOwner ? actOwner : &_localActOwner;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::resetActOwner()
    {
        _localActOwner.stop(false);
        _actOwner = &_localActOwner;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::setRaisable(cmt::Raisable* raisable)
    {
        _raisable = raisable;
        dbgAssert(_raisable);

        if(_readyState && _raisable)
        {
            _raisable->raise();
        }
    }

    void Descriptor::resetRaisable()
    {
        _raisable = {};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Descriptor::valid() const
    {
        return 0 <= _fd;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::error()
    {
        int errcode = ENOTSOCK;
        socklen_t errcodelen = sizeof(errcode);
        if(-1 == getsockopt(_fd, SOL_SOCKET, SO_ERROR, &errcode, &errcodelen))
        {
            errcode = errno;
        }

        return std::error_code(errcode, std::generic_category());
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    int Descriptor::fd() const
    {
        return _fd;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::close(bool withUninstall)
    {
        std::error_code ec;
        if(valid())
        {
            if(withUninstall)
            {
                ec = uninstall();
            }

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
            _readyState &= ~(poll::Descriptor::rsf_read | poll::Descriptor::rsf_pri | poll::Descriptor::rsf_write);

            setReadyState(poll::Descriptor::rsf_close | poll::Descriptor::rsf_eof);
        }

        return ec;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::attach(int fd)
    {
        close();

        _fd = fd;
        _readyState = {};

        if(valid())
        {
            std::error_code ec = install();
            if(ec)
            {
                setReadyState(poll::Descriptor::rsf_error);
                close();
                return ec;
            }
        }

        return std::error_code{};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::detach()
    {
        std::error_code ec;

        if(valid())
        {
            ec = uninstall();
            _fd = -1;
            _readyState = {};
        }

        return ec;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::uint_fast32_t Descriptor::readyState() const
    {
        return _readyState;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::resetReadyState(std::uint_fast32_t flags)
    {
        _readyState &= ~flags;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::setReadyState(std::uint_fast32_t flags)
    {
        _readyState |= flags;

        emitActIfNeed();

        if(_readyState && _raisable)
        {
            _raisable->raise();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::install()
    {
        dbgAssert(valid());

        return service.polling().installDescriptor(this);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::uninstall()
    {
        dbgAssert(valid());

        return service.polling().uninstallDescriptor(this);
    }
}

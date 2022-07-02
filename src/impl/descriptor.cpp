/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#if __has_include(<Winsock2.h>)
#   include <Winsock2.h>
#endif

#include "descriptor.hpp"
#include "service.hpp"
#include <dci/poll/descriptor.hpp>
#include <dci/logger.hpp>
#include <dci/cmt/functions.hpp>
#include <dci/utils/atScopeExit.hpp>

#ifdef _WIN32
#   include <dci/utils/win32/error.hpp>
#endif

#include <unistd.h>
#include <sys/types.h>

#if __has_include(<sys/socket.h>)
#   include <sys/socket.h>
#endif

namespace dci::poll::impl
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Descriptor::Descriptor(Native native, cmt::task::Owner* readyOwner, cmt::Raisable* raisable)
        : _native{native}
        , _readyOwner{(readyOwner ? readyOwner : &_localReadyOwner)}
        , _raisable{raisable}
    {
        if(valid())
        {
            std::error_code ec = install();
            if(ec)
            {
                setReadyState(descriptor::rsf_error);
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
        close();
        _ready->_owner = {};
        _ready.reset();
        _localReadyOwner.stop(false);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    sbs::Signal<void, descriptor::Native /*native*/, descriptor::ReadyStateFlags /*readyState*/> Descriptor::ready()
    {
        return _ready->_wire.out();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::emitReadyIfNeed()
    {
        if(_readyState && _ready && _ready->_wire.connected() && !_ready->_inProgress)
        {
            ReadyStateFlags readyState = _readyState;
            _readyState = {};
            _ready->_inProgress = true;
            cmt::spawn() += _readyOwner * [native{_native}, readyState, ready{_ready}]
            {
                {
                    dci::utils::AtScopeExit cleaner{[&]{ready->_inProgress=false;}};
                    ready->_wire.in(native, readyState);
                }

                if(ready->_owner)
                    ready->_owner->emitReadyIfNeed();
            };
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::setReadyOwner(cmt::task::Owner* readyOwner)
    {
        if(readyOwner != _readyOwner)
        {
            _readyOwner->stop(false);
        }
        _readyOwner = readyOwner ? readyOwner : &_localReadyOwner;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::resetReadyOwner()
    {
        _localReadyOwner.stop(false);
        _readyOwner = &_localReadyOwner;
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
#ifdef _WIN32
        return _native._value != _native._bad && !!_native._value;
#else
        return _native._value >= 0;
#endif
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::error()
    {
#ifdef _WIN32
        int errcode = ENOTSOCK;
        int errcodelen = sizeof(errcode);
        if(SOCKET_ERROR == getsockopt(_native, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&errcode), &errcodelen))
        {
            errcode = WSAGetLastError();
        }

        return utils::win32::error::make(errcode);
#else
        int errcode = ENOTSOCK;
        socklen_t errcodelen = sizeof(errcode);
        if(-1 == getsockopt(_native, SOL_SOCKET, SO_ERROR, &errcode, &errcodelen))
        {
            errcode = errno;
        }

        return std::error_code{errcode, std::generic_category()};
#endif
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    descriptor::Native Descriptor::native() const
    {
        return _native;
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

#ifdef _WIN32
            if(int closeRes = closesocket(_native))
            {
                ec = utils::win32::error::make(closeRes);
            }
#else
            int closeRes = ::close(_native);
            while(0 != closeRes && EINTR == errno)
            {
                //try again
                closeRes = ::close(_native);
            }

            if(closeRes)
            {
                ec = std::error_code{errno, std::generic_category()};
            }
#endif
            _native = {};
            _readyState = _readyState & ~(descriptor::rsf_read | descriptor::rsf_pri | descriptor::rsf_write);

            setReadyState(descriptor::rsf_close | descriptor::rsf_eof);
        }

        return ec;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::attach(Native native)
    {
        close();

        _native = native;
        _readyState = {};

        if(valid())
        {
            std::error_code ec = install();
            if(ec)
            {
                setReadyState(descriptor::rsf_error);
                close();
                return ec;
            }
        }

        return {};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::detach()
    {
        std::error_code ec;

        if(valid())
        {
            ec = uninstall();
            _native = {};
            _readyState = {};
        }

        return ec;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    descriptor::ReadyStateFlags Descriptor::readyState() const
    {
        return _readyState;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::resetReadyState(ReadyStateFlags flags)
    {
        _readyState &= ~flags;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::setReadyState(ReadyStateFlags flags)
    {
        _readyState |= flags;

        emitReadyIfNeed();

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

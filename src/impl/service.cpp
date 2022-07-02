/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "service.hpp"
#include <dci/poll/descriptor.hpp>
#include <dci/poll/error.hpp>
#include <dci/cmt.hpp>

namespace dci::poll::impl
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Service::Service()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Service::~Service()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Service::initialize()
    {
        return _polling.initialize();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Service::run(bool emitStartedStopped)
    {
        if(!_stop)
        {
            return error::already_started;
        }

        if(!_polling.initialized())
        {
            return error::not_initialized;
        }

        _stop = false;

        if(emitStartedStopped)
        {
            _started.in();
        }

        std::size_t workWithoutPollingCount{};
        while(!_stop)
        {
            ++workWithoutPollingCount;
            _workPossible.in();

            if(_clocking.fireTicks() && workWithoutPollingCount < 50)
            {
                continue;
            }

            if(_awaking.woken() && workWithoutPollingCount < 50)
            {
                continue;
            }

            if(!_polling.hasPayload() && !_clocking.hasPayload() && !_awaking.hasPayload())
            {
                _stop = true;
                break;
            }

            auto timeout = std::chrono::duration_cast<std::chrono::milliseconds>(_clocking.distance2NextPoint());
            if(!timeout.count() && workWithoutPollingCount < 5)
            {
                continue;
            }
            workWithoutPollingCount = 0;
            auto ec = _polling.execute(timeout);
            if(ec)
            {
                if(ec == std::errc::interrupted)
                {
                    _awaking.woken();
                    continue;
                }

                if(_stop && ec == error::not_initialized)
                {
                    break;
                }

                if(emitStartedStopped)
                {
                    _stopped.in();
                }

                return ec;
            }
        }

        _workPossible.in();

        if(emitStartedStopped)
        {
            _stopped.in();
        }

        return std::error_code{};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    sbs::Signal<> Service::started()
    {
        return _started.out();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    sbs::Signal<> Service::workPossible()
    {
        return _workPossible.out();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Service::stop()
    {
        if(_stop)
        {
            return error::already_stopped;
        }

        _stop = true;
        return _polling.wakeup();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    sbs::Signal<> Service::stopped()
    {
        return _stopped.out();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Service::deinitialize()
    {
        if(!_stop)
        {
            return error::not_stopped;
        }

        return _polling.deinitialize();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Clocking& Service::clocking()
    {
        return _clocking;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Polling& Service::polling()
    {
        return _polling;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Awaking& Service::awaking()
    {
        return _awaking;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    namespace
    {
        std::aligned_storage_t<sizeof(Service), alignof(Service)> servicePlace;
    }

    Service& service{*(new (&servicePlace) Service)};
}

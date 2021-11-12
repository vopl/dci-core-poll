/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
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
    std::error_code Service::run()
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

        while(!_stop)
        {
            _onWorkPossible.in();

            if(_clocking.fireTicks())
            {
                continue;
            }

            if(!_polling.hasPayload() && !_clocking.hasPayload())
            {
                _stop = true;
                break;
            }

            auto timeout = std::chrono::duration_cast<std::chrono::milliseconds>(_clocking.distance2NextPoint());
            auto ec = _polling.execute(timeout);
            if(ec)
            {
                if(ec == std::errc::interrupted)
                {
                    continue;
                }

                if(_stop && ec == error::not_initialized)
                {
                    break;
                }

                return ec;
            }
        }

        _onWorkPossible.in();

        return std::error_code{};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    sbs::Signal<> Service::onWorkPossible()
    {
        return _onWorkPossible.out();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Service::interrupt()
    {
        _polling.interrupt();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Service::stop()
    {
        if(_stop)
        {
            return error::already_stopped;
        }

        _stop = true;
        return _polling.interrupt();
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
    Service service;
}

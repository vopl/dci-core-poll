/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/poll/timer.hpp>
#include "impl/timer.hpp"

namespace dci::poll
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Timer::Timer(std::chrono::milliseconds interval)
        : himpl::FaceLayout<Timer, impl::Timer>{interval, false, nullptr, nullptr}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Timer::Timer(std::chrono::milliseconds interval, bool repeatable)
        : himpl::FaceLayout<Timer, impl::Timer>{interval, repeatable, nullptr, nullptr}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Timer::Timer(std::chrono::milliseconds interval, cmt::task::Owner* tickOwner)
        : himpl::FaceLayout<Timer, impl::Timer>{interval, false, tickOwner, nullptr}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Timer::Timer(std::chrono::milliseconds interval, bool repeatable, cmt::task::Owner* tickOwner)
        : himpl::FaceLayout<Timer, impl::Timer>{interval, repeatable, tickOwner, nullptr}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Timer::Timer(std::chrono::milliseconds interval, cmt::Raisable* raisable)
        : himpl::FaceLayout<Timer, impl::Timer>{interval, false, nullptr, raisable}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Timer::Timer(std::chrono::milliseconds interval, bool repeatable, cmt::Raisable* raisable)
        : himpl::FaceLayout<Timer, impl::Timer>{interval, repeatable, nullptr, raisable}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Timer::~Timer()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    sbs::Signal<> Timer::onTick()
    {
        return impl().onTick();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Timer::setTickOwner(cmt::task::Owner* tickOwner)
    {
        return impl().setTickOwner(tickOwner);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Timer::resetTickOwner()
    {
        return impl().resetTickOwner();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Timer::setRaisable(cmt::Raisable* raisable)
    {
        return impl().setRaisable(raisable);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Timer::resetRaisable()
    {
        return impl().resetRaisable();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::chrono::milliseconds Timer::remaining() const
    {
        return impl().remaining();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::chrono::milliseconds Timer::interval() const
    {
        return impl().interval();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Timer::interval(std::chrono::milliseconds v)
    {
        return impl().interval(v);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Timer::repeatable() const
    {
        return impl().repeatable();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Timer::repeatable(bool v)
    {
        return impl().repeatable(v);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Timer::start()
    {
        return impl().start();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Timer::restart()
    {
        return impl().restart();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Timer::started() const
    {
        return impl().started();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Timer::stop()
    {
        return impl().stop();
    }
}

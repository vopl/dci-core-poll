/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include <dci/poll/api.hpp>
#include <dci/himpl.hpp>
#include <dci/poll/implMetaInfo.hpp>
#include <dci/cmt/task/owner.hpp>
#include <dci/cmt/raisable.hpp>
#include <dci/sbs/signal.hpp>

#include <chrono>
#include <concepts>

namespace dci::poll
{
    class API_DCI_POLL Timer
        : public himpl::FaceLayout<Timer, impl::Timer>
    {
        Timer(const Timer&) = delete;
        void operator=(const Timer&) = delete;

    public:
        Timer(std::chrono::milliseconds interval = std::chrono::seconds{1});
        Timer(std::chrono::milliseconds interval, bool repeatable);

        Timer(std::chrono::milliseconds interval, auto&& onTick, cmt::task::Owner* tickOwner = nullptr) requires(std::invocable<decltype(onTick)&&>);
        Timer(std::chrono::milliseconds interval, bool repeatable, auto&& onTick, cmt::task::Owner* tickOwner = nullptr) requires(std::invocable<decltype(onTick)&&>);

        Timer(std::chrono::milliseconds interval, cmt::task::Owner* tickOwner);
        Timer(std::chrono::milliseconds interval, bool repeatable, cmt::task::Owner* tickOwner);

        Timer(std::chrono::milliseconds interval, cmt::Raisable* raisable);
        Timer(std::chrono::milliseconds interval, bool repeatable, cmt::Raisable* raisable);

        ~Timer();

        sbs::Signal<> tick();

        void setTickOwner(cmt::task::Owner* owner);
        void resetTickOwner();

        void setRaisable(cmt::Raisable* raisable);
        void resetRaisable();

        std::chrono::milliseconds remaining() const;

        std::chrono::milliseconds interval() const;
        void interval(std::chrono::milliseconds v);

        bool repeatable() const;
        void repeatable(bool v);

        void start();
        void restart();
        bool started() const;
        void stop();
    };

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Timer::Timer(std::chrono::milliseconds interval, auto&& onTick, cmt::task::Owner* tickOwner) requires(std::invocable<decltype(onTick)&&>)
        : Timer{interval, tickOwner}
    {
        this->tick() += std::forward<decltype(onTick)>(onTick);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Timer::Timer(std::chrono::milliseconds interval, bool repeatable, auto&& onTick, cmt::task::Owner* tickOwner) requires(std::invocable<decltype(onTick)&&>)
        : Timer{interval, repeatable, tickOwner}
    {
        this->tick() += std::forward<decltype(onTick)>(onTick);
    }
}

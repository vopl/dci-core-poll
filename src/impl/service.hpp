/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "polling.hpp"
#include "clocking.hpp"
#include "awaking.hpp"
#include <system_error>
#include <dci/sbs/wire.hpp>

namespace dci::poll::impl
{
    class Service
    {
    public:
        Service();
        ~Service();

    public:
        std::error_code initialize();
        std::error_code run(bool emitStartedStopped);
        sbs::Signal<>   started();
        sbs::Signal<>   workPossible();
        std::error_code stop();
        sbs::Signal<>   stopped();
        std::error_code deinitialize();

    private:
        friend class Timer;
        Clocking& clocking();

    private:
        friend class Descriptor;
        Polling& polling();

    private:
        friend class Awaker;
        Awaking& awaking();

    private:
        Clocking    _clocking;
        Polling     _polling;
        Awaking     _awaking;

        sbs::Wire<> _started;
        sbs::Wire<> _workPossible;
        sbs::Wire<> _stopped;
        bool        _stop{true};
    };

    extern Service& service;
}

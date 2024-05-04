/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "waitableTimer.hpp"
#include <dci/cmt/event.hpp>

namespace dci::poll
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class Rep, class Period, class RaisableAndWaitable = dci::cmt::Event>
    auto timeout(std::chrono::duration<Rep, Period> interval);
}

namespace dci::poll
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template<class Rep, class Period, class RaisableAndWaitable>
    auto timeout(std::chrono::duration<Rep, Period> interval)
    {
        struct Timeout : WaitableTimer<RaisableAndWaitable>
        {
            Timeout(std::chrono::milliseconds interval)
                : WaitableTimer<RaisableAndWaitable>{interval}
            {
                this->start();
            }
        };
        return Timeout{std::chrono::duration_cast<std::chrono::milliseconds>(interval)};
    }
}

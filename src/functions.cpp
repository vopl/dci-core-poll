/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/poll/functions.hpp>
#include <dci/poll/error.hpp>
#include "impl/service.hpp"

namespace dci::poll
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code initialize()
    {
        return impl::service.initialize();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code run(bool emitStartedStopped)
    {
        return impl::service.run(emitStartedStopped);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    sbs::Signal<> started()
    {
        return impl::service.started();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    sbs::Signal<> workPossible()
    {
        return impl::service.workPossible();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code stop()
    {
        return impl::service.stop();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    sbs::Signal<> stopped()
    {
        return impl::service.stopped();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code deinitialize()
    {
        return impl::service.deinitialize();
    }
}

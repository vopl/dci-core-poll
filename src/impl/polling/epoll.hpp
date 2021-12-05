/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "../descriptor.hpp"
#include "../clocking/config.hpp"

namespace dci::poll::impl::polling
{
    class Epoll
    {
    public:
        Epoll();
        ~Epoll();

        std::error_code initialize();

        bool initialized() const;

        std::error_code installDescriptor(Descriptor* d);
        std::error_code uninstallDescriptor(Descriptor* d);

        std::error_code execute(clocking::Duration timeout);
        std::error_code wakeup();

        std::error_code deinitialize();

    private:
        int _fd {-1};
        int _wakeupEvent {-1};
        char _eventsBuffer[8192];
    };
}

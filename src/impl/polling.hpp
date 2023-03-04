/* This file is part of the the dci project. Copyright (C) 2013-2023 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "descriptor.hpp"

#include <dci/utils/intrusiveDlist.hpp>

#ifdef _WIN32
#   include "polling/asyncSelect.hpp"
#else
#   include "polling/epoll.hpp"
#endif

#include <system_error>
#include <chrono>

namespace dci::poll::impl
{
    class Polling
    {
    public:
        Polling();
        ~Polling();

        std::error_code initialize();

        bool initialized() const;

        std::error_code installDescriptor(Descriptor* d);
        std::error_code uninstallDescriptor(Descriptor* d);

        std::error_code execute(std::chrono::milliseconds timeout);
        std::error_code wakeup();

        std::error_code deinitialize();

    public:
        bool hasPayload() const;

    private:
        utils::IntrusiveDlist<Descriptor> _descriptors;

#ifdef _WIN32
        polling::AsyncSelect    _engine;
#else
        polling::Epoll          _engine;
#endif
    };
}

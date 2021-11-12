/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include "api.hpp"
#include <system_error>

namespace dci::poll
{
    enum class error
    {
        unknown = 1,
        already_initialized,
        already_started,
        already_stopped,
        no_engine_available,
        not_initialized,
        not_stopped,
        bad_descriptor,
    };

    API_DCI_POLL const std::error_category& error_category();
    API_DCI_POLL std::error_code make_error_code(error e);
}

namespace std
{
    template<>
    struct is_error_code_enum<dci::poll::error> : public true_type {};
}

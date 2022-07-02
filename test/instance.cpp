/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/test.hpp>
#include <dci/poll.hpp>
#include <dci/cmt.hpp>

using namespace dci::poll;

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, instance)
{
    std::error_code ec;

    {
        ec = deinitialize();
        EXPECT_TRUE(ec);

        ec = run();
        EXPECT_TRUE(ec);

        ec = stop();
        EXPECT_TRUE(ec);
    }

    {
        ec = initialize();
        EXPECT_FALSE(ec);

        ec = deinitialize();
        EXPECT_FALSE(ec);
    }

    {
        ec = initialize();
        EXPECT_FALSE(ec);

        ec = initialize();
        EXPECT_TRUE(ec);

        ec = deinitialize();
        EXPECT_FALSE(ec);

        ec = deinitialize();
        EXPECT_TRUE(ec);
    }

    {
        ec = run();
        EXPECT_TRUE(ec);

        ec = stop();
        EXPECT_TRUE(ec);

        ec = deinitialize();
        EXPECT_TRUE(ec);
    }

    {
        ec = initialize();
        EXPECT_FALSE(ec);

        dci::cmt::spawn() += []{
            auto ec = stop();
            EXPECT_FALSE(ec);
        };

        {
            dci::sbs::Owner workPossibleOwner;
            dci::poll::workPossible() += workPossibleOwner * [&]
            {
                dci::cmt::executeReadyFibers();
            };
            ec = run();
            EXPECT_FALSE(ec);
        }

        ec = deinitialize();
        EXPECT_FALSE(ec);
    }

    {
        ec = initialize();
        EXPECT_FALSE(ec);

        ec = run();
        EXPECT_FALSE(ec);

        ec = deinitialize();
        EXPECT_FALSE(ec);
    }
}

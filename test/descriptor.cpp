/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/test.hpp>
#include <dci/poll.hpp>
#include <dci/cmt.hpp>
#include "dci/poll/descriptor/native.hpp"
using namespace dci::poll;

#include "utils/complexRun.hpp"
#include "utils/pipe.hpp"

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#if __has_include(<sys/socket.h>)
#   include <sys/socket.h>
#endif

#if __has_include(<Winsock2.h>)
#   include <Winsock2.h>
#endif

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_badFd)
{
    //bad fd
    dci::cmt::spawn() += []
    {
        descriptor::Native fd = ::socket(AF_UNIX, SOCK_STREAM/*|SOCK_NONBLOCK*/, 0);
        EXPECT_GT(fd, descriptor::Native{0});
#ifdef _WIN32
        ::closesocket(fd);
#else
        ::close(fd);
#endif

        Descriptor d{fd};

        EXPECT_FALSE(d.valid());
        EXPECT_TRUE(d.error());

        bool callbackActivated{};

        d.ready() += [&](descriptor::Native /*native*/, descriptor::ReadyStateFlags readyState)
        {
            EXPECT_TRUE(readyState & descriptor::rsf_error);
            EXPECT_TRUE(0 == d.readyState());
            callbackActivated = true;
        };
        d.emitReadyIfNeed();

        dci::cmt::yield();
        EXPECT_TRUE(callbackActivated);
    };

    utils::complexRun();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_fd)
{
    //fd
    dci::cmt::spawn() += []
    {
        {
            Descriptor d{};
            EXPECT_EQ(descriptor::Native{}, d.native());
        }

        {
            descriptor::Native fd = socket(AF_UNIX, SOCK_STREAM/*|SOCK_NONBLOCK*/, 0);

            Descriptor d{fd};
            EXPECT_EQ(fd, d.native());
        }
    };

    utils::complexRun();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_close)
{
    //close
    dci::cmt::spawn() += []
    {
        descriptor::Native fd = socket(AF_UNIX, SOCK_STREAM/*|SOCK_NONBLOCK*/, 0);

        Descriptor d{fd};

        EXPECT_TRUE(d.valid());
        EXPECT_FALSE(d.error());

        bool eofNotified = false;
        d.ready() += [&](descriptor::Native /*native*/, descriptor::ReadyStateFlags readyState)
        {
            EXPECT_TRUE(readyState & descriptor::rsf_eof);
            eofNotified = true;
        };

        EXPECT_TRUE(!eofNotified);
        d.close();
        dci::cmt::yield();
        EXPECT_TRUE(eofNotified);


        Descriptor d2{fd};//already closed
        EXPECT_TRUE(d2.error());

        EXPECT_TRUE(d2.readyState() & descriptor::rsf_error);
        EXPECT_TRUE(d2.readyState() & descriptor::rsf_eof);
    };

    utils::complexRun();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_closeOnDestroy)
{
    //close on destoy, notification
    dci::cmt::spawn() += []
    {
        descriptor::Native fd = socket(AF_UNIX, SOCK_STREAM/*|SOCK_NONBLOCK*/, 0);

        bool eofNotified = false;
        dci::cmt::task::Owner taskOwner;
        {
            Descriptor d
            {
                fd,
                [&](descriptor::Native /*native*/, descriptor::ReadyStateFlags readyState)
                {
                    EXPECT_TRUE(readyState & descriptor::rsf_eof);
                    eofNotified = true;
                },
                &taskOwner
            };

            EXPECT_TRUE(d.valid());
            EXPECT_FALSE(d.error());
        }
        dci::cmt::yield();
        EXPECT_TRUE(eofNotified);

        Descriptor d{fd};//already closed
        EXPECT_TRUE(d.error());

        EXPECT_TRUE(d.readyState() & descriptor::rsf_error);
        EXPECT_TRUE(d.readyState() & descriptor::rsf_eof);
    };

    utils::complexRun();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_detach)
{
    //detach
    dci::cmt::spawn() += []
    {
        descriptor::Native fd = socket(AF_UNIX, SOCK_STREAM/*|SOCK_NONBLOCK*/, 0);

        Descriptor d{fd};
        EXPECT_FALSE(d.error());

        d.detach();
        EXPECT_TRUE(d.error());

        Descriptor d2{fd};
        EXPECT_FALSE(d2.error());

        d2.detach();
        EXPECT_TRUE(d.error());

        ::close(fd);
    };

    utils::complexRun();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_work)
{
    //work
    dci::cmt::spawn() += []
    {
        {
            dci::poll::descriptor::Native::Value pipefd[2];
            int i = utils::pipe(pipefd);
            EXPECT_EQ(i, 0);

            Descriptor d0{descriptor::Native{pipefd[0]}};
            Descriptor d1{descriptor::Native{pipefd[1]}};
            dci::cmt::Event messageDelivered;

            d0.ready() += [&](descriptor::Native /*native*/, descriptor::ReadyStateFlags readyState)
            {
                if(readyState & descriptor::rsf_read)
                {
                    char buf[32];
#ifdef _WIN32
                    EXPECT_EQ(::recv(d0.native(), buf, sizeof(buf), 0), 4);
#else
                    EXPECT_EQ(::read(d0.native(), buf, sizeof(buf)), 4);
#endif
                    EXPECT_TRUE(std::string("msg") == buf);

                    messageDelivered.raise();
                }

            };

#ifdef _WIN32
            EXPECT_EQ(::send(d1.native(), "msg", 4, 0), 4);
#else
            EXPECT_EQ(::write(d1.native(), "msg", 4), 4);
#endif

            messageDelivered.wait();
        }
    };

    utils::complexRun();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_raisable)
{
    //work
    dci::cmt::spawn() += []
    {
        {
            dci::poll::descriptor::Native::Value pipefd[2];
            int i = utils::pipe(pipefd);
            EXPECT_EQ(i, 0);

            Descriptor d0{descriptor::Native{pipefd[0]}};
            Descriptor d1{descriptor::Native{pipefd[1]}};
            dci::cmt::Notifier notifier0;
            dci::cmt::Notifier notifier1;

            d0.setRaisable(&notifier0);
            d1.setRaisable(&notifier1);

            EXPECT_FALSE(notifier0.isRaised());
            EXPECT_FALSE(notifier1.isRaised());

#ifdef _WIN32
            EXPECT_EQ(::send(d1.native(), "msg", 4, 0), 4);
#else
            EXPECT_EQ(::write(d1.native(), "msg", 4), 4);
#endif

            notifier0.wait();
            EXPECT_TRUE(d0.readyState() & descriptor::rsf_read);
        }
    };

    utils::complexRun();
}

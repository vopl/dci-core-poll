/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include <dci/test.hpp>
#include <dci/poll.hpp>
#include <dci/cmt.hpp>
#include "complexRun.hpp"

using namespace dci::poll;

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_badFd)
{
    //bad fd
    dci::cmt::spawn() += []
    {
        int pipefd[2];
        int i = pipe2(pipefd, O_NONBLOCK);
        EXPECT_EQ(i, 0);
        close(pipefd[0]);
        close(pipefd[1]);

        Descriptor d(pipefd[0]);

        EXPECT_FALSE(d.valid());
        EXPECT_TRUE(d.error());

        bool callbackActivated{};

        d.onAct() += [&](int /*fd*/, std::uint_fast32_t readyState)
        {
            EXPECT_TRUE(readyState & Descriptor::rsf_error);
            EXPECT_TRUE(0 == d.readyState());
            callbackActivated = true;
        };
        d.emitActIfNeed();

        dci::cmt::yield();
        EXPECT_TRUE(callbackActivated);
    };

    complexRun();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_fd)
{
    //fd
    dci::cmt::spawn() += []
    {
        {
            Descriptor d(-1);
            EXPECT_EQ(-1, d.fd());
        }

        {
            int fd = socket(AF_LOCAL, SOCK_STREAM|SOCK_NONBLOCK, 0);

            Descriptor d(fd);
            EXPECT_EQ(fd, d.fd());
        }
    };

    complexRun();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_close)
{
    //close
    dci::cmt::spawn() += []
    {
        int fd = socket(AF_LOCAL, SOCK_STREAM|SOCK_NONBLOCK, 0);

        Descriptor d(fd);

        EXPECT_TRUE(d.valid());
        EXPECT_FALSE(d.error());

        bool eofNotified = false;
        d.onAct() += [&](int /*fd*/, std::uint_fast32_t readyState)
        {
            EXPECT_TRUE(readyState & Descriptor::rsf_eof);
            eofNotified = true;
        };

        EXPECT_TRUE(!eofNotified);
        d.close();
        dci::cmt::yield();
        EXPECT_TRUE(eofNotified);


        Descriptor d2(fd);//already closed
        EXPECT_TRUE(d2.error());

        EXPECT_TRUE(d2.readyState() & Descriptor::rsf_error);
        EXPECT_TRUE(d2.readyState() & Descriptor::rsf_eof);
    };

    complexRun();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_closeOnDestroy)
{
    //close on destoy, notification
    dci::cmt::spawn() += []
    {
        int fd = socket(AF_LOCAL, SOCK_STREAM|SOCK_NONBLOCK, 0);

        bool eofNotified = false;
        dci::cmt::task::Owner taskOwner;
        {
            Descriptor d
            {
                fd,
                [&](int /*fd*/, std::uint_fast32_t readyState)
                {
                    EXPECT_TRUE(readyState & Descriptor::rsf_eof);
                    eofNotified = true;
                },
                &taskOwner
            };

            EXPECT_TRUE(d.valid());
            EXPECT_FALSE(d.error());
        }
        dci::cmt::yield();
        EXPECT_TRUE(eofNotified);

        Descriptor d(fd);//already closed
        EXPECT_TRUE(d.error());

        EXPECT_TRUE(d.readyState() & Descriptor::rsf_error);
        EXPECT_TRUE(d.readyState() & Descriptor::rsf_eof);
    };

    complexRun();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_detach)
{
    //detach
    dci::cmt::spawn() += []
    {
        int fd = socket(AF_LOCAL, SOCK_STREAM|SOCK_NONBLOCK, 0);

        Descriptor d(fd);
        EXPECT_FALSE(d.error());

        d.detach();
        EXPECT_TRUE(d.error());

        Descriptor d2(fd);
        EXPECT_FALSE(d2.error());

        d2.detach();
        EXPECT_TRUE(d.error());

        ::close(fd);
    };

    complexRun();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_work)
{
    //work
    dci::cmt::spawn() += []
    {
        {
            int pipefd[2];
            int i = pipe2(pipefd, O_NONBLOCK);
            EXPECT_EQ(i, 0);

            Descriptor d0{pipefd[0]};
            Descriptor d1{pipefd[1]};
            dci::cmt::Event messageDelivered;

            d0.onAct() += [&](int /*fd*/, std::uint_fast32_t readyState)
            {
                if(readyState & Descriptor::rsf_read)
                {
                    char buf[32];
                    EXPECT_EQ(read(d0, buf, sizeof(buf)), 4);

                    EXPECT_TRUE(std::string("msg") == buf);

                    messageDelivered.raise();
                }

            };

            EXPECT_EQ(write(d1, "msg", 4), 4);

            messageDelivered.wait();
        }
    };

    complexRun();
}

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
TEST(poll, descriptor_raisable)
{
    //work
    dci::cmt::spawn() += []
    {
        {
            int pipefd[2];
            int i = pipe2(pipefd, O_NONBLOCK);
            EXPECT_EQ(i, 0);

            Descriptor d0{pipefd[0]};
            Descriptor d1{pipefd[1]};
            dci::cmt::Notifier notifier0;
            dci::cmt::Notifier notifier1;

            d0.setRaisable(&notifier0);
            d1.setRaisable(&notifier1);

            EXPECT_FALSE(notifier0.isRaised());
            EXPECT_FALSE(notifier1.isRaised());

            EXPECT_EQ(write(d1, "msg", 4), 4);

            notifier0.wait();
            EXPECT_TRUE(d0.readyState() & Descriptor::ReadyStateFlags::rsf_read);
        }
    };

    complexRun();
}

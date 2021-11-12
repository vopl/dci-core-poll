/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
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

namespace dci::poll
{
    class API_DCI_POLL Descriptor final
        : public himpl::FaceLayout<Descriptor, impl::Descriptor>
    {
        Descriptor(const Descriptor&) = delete;
        void operator=(const Descriptor&) = delete;

    public:
        enum ReadyStateFlags : std::uint_fast32_t
        {
            rsf_read    = 0x001,
            rsf_pri     = 0x002,
            rsf_write   = 0x004,

            rsf_error   = 0x010,

            rsf_eof     = 0x100,
            rsf_close   = 0x200,
        };

    public:
        Descriptor(int fd=-1);
        Descriptor(int fd, auto&& onAct, cmt::task::Owner* actOwner = nullptr) requires(std::invocable<decltype(onAct)&&, int, std::uint_fast32_t>);
        Descriptor(int fd, cmt::task::Owner* actOwner);
        Descriptor(int fd, cmt::Raisable* raisable);
        ~Descriptor();

        sbs::Signal<void, int /*fd*/, std::uint_fast32_t /*readyState*/> onAct();
        void emitActIfNeed();

        void setActOwner(cmt::task::Owner* actOwner);
        void resetActOwner();

        void setRaisable(cmt::Raisable* raisable);
        void resetRaisable();

        bool valid() const;
        std::error_code error();

        int fd() const;
        operator int() const;

        std::error_code close();
        std::error_code attach(int fd);
        std::error_code detach();

        std::uint_fast32_t readyState() const;
        void resetReadyState(std::uint_fast32_t flags);
    };

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Descriptor::Descriptor(int fd, auto&& onAct, cmt::task::Owner* actOwner) requires(std::invocable<decltype(onAct)&&, int, std::uint_fast32_t>)
        : Descriptor{fd, actOwner}
    {
        this->onAct() += std::forward<decltype(onAct)>(onAct);
        emitActIfNeed();
    }
}

/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#pragma once

#include <dci/cmt/task/owner.hpp>
#include <dci/cmt/raisable.hpp>
#include <dci/sbs/wire.hpp>
#include <system_error>
#include <memory>

namespace dci::poll::impl
{
    class Polling;

    class Descriptor final
    {
        Descriptor(const Descriptor&) = delete;
        void operator=(const Descriptor&) = delete;

    public:
        Descriptor(int fd, cmt::task::Owner* actOwner, cmt::Raisable* raisable);
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

        std::error_code close(bool withUninstall = true);

        std::error_code attach(int fd);
        std::error_code detach();

        std::uint_fast32_t readyState() const;
        void resetReadyState(std::uint_fast32_t flags);

    public:
        void setReadyState(std::uint_fast32_t flags);//from polling engine

    private:
        std::error_code install();
        std::error_code uninstall();

    private:
        friend class Polling;
        Descriptor* _nextInEngine{};
        Descriptor* _prevInEngine{};

    private:
        int                                         _fd;

        std::uint_fast32_t                          _readyState{};

        struct OnAct
        {
            Descriptor* _owner;
            bool _inProgress{};
            sbs::Wire<void, int, std::uint_fast32_t> _wire;

            OnAct(Descriptor* owner) : _owner{owner} {}
        };
        using OnActPtr = std::shared_ptr<OnAct>;
        OnActPtr _onAct{std::make_shared<OnAct>(this)};

        cmt::task::Owner*                           _actOwner {};
        cmt::task::Owner                            _localActOwner{};
        cmt::Raisable*                              _raisable{};
    };
}

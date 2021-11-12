/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "impl/descriptor.hpp"
#include <dci/poll/descriptor.hpp>

namespace dci::poll
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Descriptor::Descriptor(int fd)
        : FaceLayout{fd, nullptr, nullptr}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Descriptor::Descriptor(int fd, cmt::task::Owner* actOwner)
        : FaceLayout{fd, actOwner, nullptr}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Descriptor::Descriptor(int fd, cmt::Raisable* raisable)
        : FaceLayout{fd, nullptr, raisable}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Descriptor::~Descriptor()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    sbs::Signal<void, int /*fd*/, std::uint_fast32_t /*readyState*/> Descriptor::onAct()
    {
        return impl().onAct();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::emitActIfNeed()
    {
        return impl().emitActIfNeed();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::setActOwner(cmt::task::Owner* actOwner)
    {
        return impl().setActOwner(actOwner);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::resetActOwner()
    {
        return impl().resetActOwner();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::setRaisable(cmt::Raisable* raisable)
    {
        return impl().setRaisable(raisable);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::resetRaisable()
    {
        return impl().resetRaisable();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Descriptor::valid() const
    {
        return impl().valid();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::error()
    {
        return impl().error();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    int Descriptor::fd() const
    {
        return impl().fd();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Descriptor::operator int() const
    {
        return impl().fd();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::close()
    {
        return impl().close();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::attach(int fd)
    {
        return impl().attach(fd);
    }

/////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Descriptor::detach()
    {
        return impl().detach();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::uint_fast32_t Descriptor::readyState() const
    {
        return impl().readyState();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Descriptor::resetReadyState(std::uint_fast32_t flags)
    {
        return impl().resetReadyState(flags);
    }
}


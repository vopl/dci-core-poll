/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "awaking.hpp"

namespace dci::poll::impl
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Awaking::Awaking()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Awaking::~Awaking()
    {
        std::scoped_lock lock{_mtx};
        dbgAssert(_awakers.empty());
        dbgAssert(_awakersReady.empty());
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Awaking::install(Awaker* awaker)
    {
        std::scoped_lock lock{_mtx};
        dbgAssert(!_awakers.contains(awaker));
        dbgAssert(!_awakersReady.contains(awaker));
        _awakers.push(awaker);

        if(awaker->keepLoop())
        {
            ++_keepLoop;
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Awaking::uninstall(Awaker* awaker)
    {
        std::scoped_lock lock{_mtx};
        dbgAssert(_awakers.contains(awaker));
        _awakers.remove(awaker);

        if(awaker->ready())
            _awakersReady.remove(awaker);

        if(awaker->keepLoop())
        {
            dbgAssert(0 < _keepLoop);
            --_keepLoop;
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Awaking::ready(Awaker* awaker)
    {
        std::scoped_lock lock{_mtx};
        dbgAssert(_awakers.contains(awaker));
        dbgAssert(!_awakersReady.contains(awaker));
        _awakersReady.push(awaker);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Awaking::unready(Awaker* awaker)
    {
        std::scoped_lock lock{_mtx};
        dbgAssert(_awakers.contains(awaker));
        dbgAssert(_awakersReady.contains(awaker));
        _awakersReady.remove(awaker);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Awaking::woken()
    {
        std::scoped_lock lock{_mtx};
        if(_awakersReady.empty())
        {
            return false;
        }

        _awakersReady.each([](Awaker* awaker)
        {
            awaker->emitWokenIfNeed();
        });
        return true;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Awaking::hasPayload() const
    {
        std::scoped_lock lock{_mtx};
        return !!_keepLoop;
    }
}

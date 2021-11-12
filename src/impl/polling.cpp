/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "polling.hpp"

#include <dci/utils/dbg.hpp>
#include <dci/poll/descriptor.hpp>

namespace dci::poll::impl
{

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Polling::Polling()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Polling::~Polling()
    {
        while(_descriptors)
        {
            _descriptors->close(false);
            _descriptors->setReadyState(poll::Descriptor::rsf_error);
            _descriptors = _descriptors->_nextInEngine;
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Polling::initialize()
    {
        return _engine.initialize();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Polling::initialized() const
    {
        return _engine.initialized();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Polling::installDescriptor(Descriptor* d)
    {
        std::error_code ec = _engine.installDescriptor(d);

        if(ec)
        {
            return ec;
        }

        dbgAssert(!d->_nextInEngine);
        dbgAssert(!d->_prevInEngine);
        if(!_descriptors)
        {
            _descriptors = d;
        }
        else
        {
            dbgAssert(!_descriptors->_prevInEngine);
            _descriptors->_prevInEngine = d;
            d->_nextInEngine = _descriptors;
            _descriptors = d;
        }

        return ec;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Polling::uninstallDescriptor(Descriptor* d)
    {
        std::error_code ec = _engine.uninstallDescriptor(d);

        if(d->_nextInEngine || d->_prevInEngine || d == _descriptors)
        {
            if(d == _descriptors)
            {
                if(d->_prevInEngine)
                {
                    _descriptors = d->_prevInEngine;
                }
                else
                {
                    _descriptors = d->_nextInEngine;
                }
            }

            if(d->_nextInEngine)
            {
                d->_nextInEngine->_prevInEngine = d->_prevInEngine;
            }
            if(d->_prevInEngine)
            {
                d->_prevInEngine->_nextInEngine = d->_nextInEngine;
            }

            d->_nextInEngine = nullptr;
            d->_prevInEngine = nullptr;
        }

        return ec;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Polling::execute(std::chrono::milliseconds timeout)
    {
        return _engine.execute(timeout);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Polling::interrupt()
    {
        return _engine.interrupt();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::error_code Polling::deinitialize()
    {
        return _engine.deinitialize();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Polling::hasPayload() const
    {
        return !!_descriptors;
    }
}

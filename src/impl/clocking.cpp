/* This file is part of the the dci project. Copyright (C) 2013-2021 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "clocking.hpp"

#include <iostream>

namespace dci::poll::impl
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Clocking::Clocking()
        : _now(std::chrono::time_point_cast<Duration>(Clock::now()).time_since_epoch().count())
        , _nearestPoint(_now)
        , _readyBucket(&_readyAmount, _bucketsAmount-1, 0)
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Clocking::~Clocking()
    {
        dbgAssert(!hasPayload());
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Clocking::PointRep Clocking::now() const
    {
        return _now;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Clocking::start(Timer* t)
    {
        dbgAssert(!t->_bucket);

        if(t->nextPoint() <= _now)
        {
            _readyBucket.insert(t);
            return;
        }

        _nearestPoint = std::min(_nearestPoint, t->nextPoint());

        clocking::Bucket* bucket = getOrMakeBucket(bucketIndexFor(t->nextPoint()));
        bucket->insert(t);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Clocking::update(Timer* t)
    {
        stop(t);
        start(t);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Clocking::stop(Timer* t)
    {
        if(t->_bucket)
        {
            t->_bucket->remove(t);
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Clocking::hasPayload() const
    {
        return 0 < _amount || 0 < _readyAmount;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Clocking::fireTicks()
    {
        PointRep now = std::chrono::time_point_cast<Duration>(Clock::now()).time_since_epoch().count();

        _nearestPoint = rollBucketsTo(now);

        dbgAssert(_now == now);

        bool res;
        {
            std::size_t flushAmount = 0;
            clocking::Bucket flushBucket(&flushAmount, _bucketsAmount-1, 0);
            _readyBucket.flushTo(&flushBucket);
            res = !!flushAmount;

            flushBucket.flush([&](clocking::BucketElement* el)
            {
                Timer* t = static_cast<Timer*>(el);
                t->tick(now);
            });
        }

        return res;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Clocking::Duration Clocking::distance2NextPoint() const
    {
        if(_readyAmount)
        {
            return Duration(0);
        }

        dbgAssert(_nearestPoint >= _now);
        return Duration(_nearestPoint - _now);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Clocking::PointRep Clocking::rollBucketsTo(PointRep time)
    {
        dbgAssert(_now <= time);

        PointRep nearestPoint = time >= _nearestPoint ? Point::max().time_since_epoch().count() : _nearestPoint;

        while(_now < time)
        {
            std::size_t boundBucketIndex = bucketIndexFor(time);

            for(std::size_t idx(0); idx<boundBucketIndex; ++idx)
            {
                clocking::BucketPtr& bucket = _buckets[idx];
                if(bucket)
                {
#ifndef NDEBUG
                    bucket->each([&](clocking::BucketElement* el)
                    {
                        dbgAssert(el->_point <= time);
                    });
#endif
                    bucket->flushTo(&_readyBucket);
                    bucket.reset();
                }
            }

            clocking::BucketPtr& boundBucket = _buckets[boundBucketIndex];
            if(boundBucket)
            {
                _now = boundBucket->start();

                boundBucket->separateChildren(&_readyBucket, _buckets, time, nearestPoint);

                boundBucket.reset();
            }
            else
            {
                _now = calculateBucketStart(boundBucketIndex);
            }
        }

        dbgAssert(_now == time);

        if(_amount && Point::max().time_since_epoch().count() == nearestPoint)
        {
            for(std::size_t idx(0); idx<_bucketsAmount; ++idx)
            {
                clocking::BucketPtr& boundBucket = _buckets[idx];
                if(boundBucket && boundBucket->getNearestPoint(nearestPoint))
                {
                    break;
                }
            }
        }

        dbgAssert(nearestPoint > _now);
        return nearestPoint;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::size_t Clocking::bucketIndexFor(PointRep target)
    {
        dbgAssert(target > _now);

        return sizeof(PointRep)*CHAR_BIT - dci::utils::bits::count0Most(target ^ _now) - 1;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Clocking::PointRep Clocking::calculateBucketStart(std::size_t index)
    {
        DurationRep aligned = DurationRep(1) << index;
        PointRep  bucketStart = (_now & ~(aligned-1)) + aligned;

        return bucketStart;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    clocking::Bucket* Clocking::getOrMakeBucket(std::size_t index)
    {
        dbgAssert(_bucketsAmount > index);
        clocking::BucketPtr& bucket = _buckets[index];

        if(!bucket)
        {
            bucket.reset(new clocking::Bucket{&_amount, index, calculateBucketStart(index)});
        }

        return bucket.get();
    }
}

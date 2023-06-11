#pragma once

#include "MathHelpers.h"
#include "TypeHelpers.h"

#include "glog/logging.h"

namespace Jnrlib
{
    class BoundingBox
    {
    public:
        BoundingBox()
        {
            constexpr Float minNum = std::numeric_limits<Float>::lowest();
            constexpr Float maxNum = std::numeric_limits<Float>::max();

            pMin = Position(maxNum, maxNum, maxNum);
            pMax = Position(minNum, minNum, minNum);
        };

        explicit BoundingBox(Position const& pos) :
            pMin(pos), pMax(pos)
        {
        }

        explicit BoundingBox(Position const& min, Position const& max) :
            pMin(min), pMax(max)
        { }

        bool operator == (BoundingBox const& rhs) const
        {
            return pMin == rhs.pMin && pMax == rhs.pMax;
        }

        bool operator != (BoundingBox const& rhs) const
        {
            return pMin != rhs.pMin || pMax != rhs.pMax;
        }

        Position Diagonal() const
        {
            return pMax - pMin;
        }

        Axis MaximumExtent() const
        {
            auto d = Diagonal();
            if (d.x > d.y && d.x > d.z)
                return Axis::X;
            else if (d.y > d.z)
                return Axis::Y;
            else
                return Axis::Z;
        }

        Position const& operator [](int pos) const
        {
            CHECK(pos == 0 || pos == 1) << "Position must be 0 or 1";
            return pos == 0 ? pMin : pMax;
        }

        Position pMin;
        Position pMax;
    };

    inline BoundingBox Union(BoundingBox const& boundingBox, Position const& point)
    {
        BoundingBox bb{};
        bb.pMin = min(boundingBox.pMin, point);
        bb.pMax = max(boundingBox.pMax, point);
        return bb;
    }
    inline BoundingBox Union(BoundingBox const& boundingBox1, BoundingBox const& boundingBox2)
    {
        BoundingBox bb{};
        bb.pMin = min(boundingBox1.pMin, boundingBox2.pMin);
        bb.pMax = max(boundingBox1.pMax, boundingBox2.pMax);
        return bb;
    }

}
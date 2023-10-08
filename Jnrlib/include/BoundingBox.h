#pragma once

#include "MathHelpers.h"
#include "TypeHelpers.h"

#include "glog/logging.h"

namespace Jnrlib
{
    class BoundingBox
    {
    public:
        inline BoundingBox()
        {
            constexpr Float minNum = std::numeric_limits<Float>::lowest();
            constexpr Float maxNum = std::numeric_limits<Float>::max();

            pMin = Position(maxNum, maxNum, maxNum);
            pMax = Position(minNum, minNum, minNum);
        };

        explicit inline BoundingBox(Position const& pos) :
            pMin(pos), pMax(pos)
        {
        }

        explicit inline BoundingBox(Position const& min, Position const& max) :
            pMin(min), pMax(max)
        { }

        inline bool operator == (BoundingBox const& rhs) const
        {
            return pMin == rhs.pMin && pMax == rhs.pMax;
        }

        inline bool operator != (BoundingBox const& rhs) const
        {
            return pMin != rhs.pMin || pMax != rhs.pMax;
        }

        inline Position Diagonal() const
        {
            return pMax - pMin;
        }

        inline Axis MaximumExtent() const
        {
            auto d = Diagonal();
            if (d.x > d.y && d.x > d.z)
                return Axis::X;
            else if (d.y > d.z)
                return Axis::Y;
            else
                return Axis::Z;
        }

        inline Position Offset(Position const& p) const
        {
            Position origin = p - pMin;
            Position length = pMax - pMin;

            /* There are cases in which the length can be 0 */
            if (length.x > 0) origin.x /= length.x;
            if (length.y > 0) origin.y /= length.y;
            if (length.z > 0) origin.z /= length.z;

            return origin;
        }

        inline Float SurfaceArea() const
        {
            auto d = Diagonal();
            return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
        }

        inline Position const& operator [](int pos) const
        {
            CHECK(pos == 0 || pos == 1) << "Index for a bounding box must be 0 or 1";
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
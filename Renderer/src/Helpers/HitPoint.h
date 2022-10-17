#pragma once

#include <Jnrlib.h>

class HitPoint
{
public:
    static constexpr const unsigned int MAX_INTERSECTION_POINTS = 2;

public:
    HitPoint() = default;
    ~HitPoint() = default;

public:
    void SetColor(Jnrlib::Color const& color);
    void SetIntersectionPoint(Jnrlib::Float t);
    void SetNormal(Jnrlib::Direction const& normal);

public:
    Jnrlib::Color const& GetColor() const;
    Jnrlib::Direction const& GetNormal() const;
    Jnrlib::Float GetIntersectionPoint() const;

private:
    Jnrlib::Color mColor;
    Jnrlib::Direction mNormal;
    Jnrlib::Float mIntersectionPoint;

};

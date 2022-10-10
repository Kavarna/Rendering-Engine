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
    void SetIntersectionPoint(Jnrlib::Float t, uint32_t index = 0);

public:
    Jnrlib::Color const& GetColor() const;
    Jnrlib::Float GetIntersectionPoint(uint32_t index = 0) const;

    std::array<Jnrlib::Float, MAX_INTERSECTION_POINTS> const& GetIntersectionPoints() const;

private:
    Jnrlib::Color mColor;
    std::array<Jnrlib::Float, MAX_INTERSECTION_POINTS> mIntersectionPoints;

};

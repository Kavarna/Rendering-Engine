#pragma once


#include "Jnrlib.h"
#include "CreateInfoUtils.h"


class Camera
{
public:
    Camera(CreateInfo::Camera const&);

private:
    CreateInfo::Camera mInfo;
};

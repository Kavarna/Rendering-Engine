#include "Camera.h"

Camera::Camera(CreateInfo::Camera const& info) :
    mInfo(info)
{
    LOG(INFO) << "Creating camera with info: " << info;
}

#pragma once


#include "Jnrlib.h"

enum class RendererType
{
    PathTracing,
    None
};
RendererType GetRendererTypeFromString(std::string const& str);

class Scene
{
public:
    Scene(RendererType type);
    ~Scene();

public:
    RendererType GetRendererType() const;

private:
    RendererType mRendererType;
};



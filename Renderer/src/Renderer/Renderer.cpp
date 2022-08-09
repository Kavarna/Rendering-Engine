#include "Renderer.h"

void RenderScene(std::unique_ptr<Scene> scene)
{
	switch (scene->GetRendererType())
	{
		case RendererType::PathTracing:
			break;
		default:
			LOG(ERROR) << "Invalid renderer specified in scene";
			break;
	}
}

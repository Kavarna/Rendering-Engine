#include "Renderer.h"
#include "PathTracing.h"
#include "PngDumper.h"


void RenderScene(std::unique_ptr<Scene>& scene)
{
	using namespace std::placeholders;

	const auto& imageInfo = scene->GetImageInfo();

	PngDumper dumper((uint32_t)imageInfo.width, (uint32_t)imageInfo.height, scene->GetOutputFile());

	switch (scene->GetRendererType())
	{
		case CreateInfo::RendererType::PathTracing:
		{
			PathTracing(dumper, *scene.get()).Render();
			break;
		}
		default:
			LOG(ERROR) << "Invalid renderer specified in scene";
			break;
	}
	
}

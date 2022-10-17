#include "Renderer.h"
#include "PathTracing.h"
#include "PngDumper.h"



void RenderScene(std::unique_ptr<Scene>& scene, CreateInfo::Renderer const& rendererInfo)
{
	using namespace std::placeholders;

	const auto& imageInfo = scene->GetImageInfo();

	PngDumper dumper((uint32_t)imageInfo.width, (uint32_t)imageInfo.height, scene->GetOutputFile());

	switch (rendererInfo.rendererType)
	{
		case CreateInfo::RendererType::PathTracing:
		{
			PathTracing(dumper, *scene.get(), rendererInfo.numSamples, rendererInfo.maxDepth).Render();
			break;
		}
		default:
			LOG(ERROR) << "Invalid renderer specified in scene";
			break;
	}
	
}

#include "Renderer.h"
#include "PathTracing.h"
#include "SimpleRayTracing.h"
#include "PngDumper.h"



void RayTracing::RenderScene(std::unique_ptr<Common::Scene>& scene, CreateInfo::RayTracing const& rendererInfo)
{
	using namespace std::placeholders;

	const auto& imageInfo = scene->GetImageInfo();

	Common::PngDumper dumper((uint32_t)imageInfo.width, (uint32_t)imageInfo.height, scene->GetOutputFile());

	switch (rendererInfo.rendererType)
	{
		case CreateInfo::RayTracingType::PathTracing:
		{
			PathTracing(dumper, *scene, rendererInfo.numSamples, rendererInfo.maxDepth).Render();
			break;
		}
		case CreateInfo::RayTracingType::SimpleRayTracing:
		{
			SimpleRayTracing(dumper, *scene, rendererInfo.maxDepth).Render();
			break;
		}
		default:
			LOG(ERROR) << "Invalid renderer specified in scene";
			break;
	}
	
}

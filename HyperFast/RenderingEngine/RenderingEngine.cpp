#include "RenderingEngine.h"
#include "../VulkanLoader/VulkanLoader.h"
#include <sstream>

namespace HyperFast
{
	RenderingEngine::RenderingEngine(const std::string_view &appName, const std::string_view &engineName)
	{
		

		/*const VkApplicationInfo appInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = appName.data(),
			.applicationVersion;
			.pEngineName;
			.engineVersion;
			.apiVersion;
		};*/

		__getInstanceVersion();
		__checkInstanceVersionSupport();
	}

	void RenderingEngine::__getInstanceVersion() noexcept
	{
		const VKL::GlobalProcedure &globalGlobalProcedure{ VKL::VulkanLoader::getInstance().getGlobalProcedure() };
		if (!(globalGlobalProcedure.vkEnumerateInstanceVersion))
		{
			__instanceVersion = VK_API_VERSION_1_0;
			return;
		}

		globalGlobalProcedure.vkEnumerateInstanceVersion(&__instanceVersion);
	}

	void RenderingEngine::__checkInstanceVersionSupport() const
	{
		const uint32_t major{ VK_API_VERSION_MAJOR(__instanceVersion) };
		const uint32_t minor{ VK_API_VERSION_MINOR(__instanceVersion) };
		const uint32_t patch{ VK_API_VERSION_PATCH(__instanceVersion) };
		const uint32_t variant{ VK_API_VERSION_VARIANT(__instanceVersion) };

		if (major > 1U)
			return;

		if ((major == 1U) && (minor >= 3U))
			return;

		std::ostringstream oss;
		oss << "Failed to initialize the engine. ";
		oss << "The instance version is too low. ";
		oss << "Expected: >= 1.3, Actual: ";
		oss << VK_API_VERSION_MAJOR(__instanceVersion) << '.';
		oss << VK_API_VERSION_MINOR(__instanceVersion) << '.';
		oss << VK_API_VERSION_PATCH(__instanceVersion) << '.';
		oss << VK_API_VERSION_VARIANT(__instanceVersion);

		throw std::exception{ oss.str().c_str() };
	}
}
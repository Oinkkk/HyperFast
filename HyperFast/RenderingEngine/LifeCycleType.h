#pragma once

#include <cstdint>

namespace HyperFast
{
	enum class LifeCycleType : uint32_t
	{
		START = 0U,

		// buffer, image 업데이트
		DATA_UPDATE,

		// swaphain, framebuffer, renderpass 등 screen 관련 리소스 업데이트
		SCREEN_UPDATE,

		// 디스크립터 업데이트
		DESCRIPTOR_UPDATE,

		// 모든 업데이트 커맨드 빌드
		COMMAND_BUILD,

		// 빌드된 커맨드 enqueue
		COMMAND_ENQUEUE,

		// 모든 submit 요청 commit
		COMMAND_SUBMIT,

		// swapchain present
		PRESENT,

		END
	};

	static constexpr uint32_t NUM_LIFE_CYCLE_TYPES
	{
		uint32_t(LifeCycleType::END) - (uint32_t(LifeCycleType::START) + 1U)
	};
}
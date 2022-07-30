#pragma once

#include <cstdint>

namespace HyperFast
{
	enum class LifeCycleType : uint32_t
	{
		START,

		// buffer, image 등 업데이트 command submit 요청
		DATA_UPDATE,

		// swaphain, framebuffer, renderpass 등 screen과 관련된 리소스 업데이트
		SCREEN_UPDATE,

		// rendering command recording
		RENDER,

		// 모든 submit 요청 commit
		SUBMIT,

		// swapchain present
		PRESENT,

		// 사용 만료된 리소스 삭제
		GARBAGE_COLLECT,

		END
	};

	static constexpr uint32_t NUM_LIFE_CYCLE_TYPES
	{
		uint32_t(LifeCycleType::END) - (uint32_t(LifeCycleType::START) + 1U)
	};
}
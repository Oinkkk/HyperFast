#pragma once

#include <cstdint>

namespace HyperFast
{
	enum class SubmitLayerType : uint32_t
	{
		START = 0U,

		GRAPHICS,
		TRANSFER,

		END
	};

	static constexpr uint32_t NUM_SUBMIT_LAYER_TYPES
	{
		uint32_t(SubmitLayerType::END) - (uint32_t(SubmitLayerType::START) + 1U)
	};
}
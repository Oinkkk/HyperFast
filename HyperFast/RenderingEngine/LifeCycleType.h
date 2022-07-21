#pragma once

namespace HyperFast
{
	enum class LifeCycleType
	{
		START,

		DATA_UPDATE,
		SCREEN_UPDATE,
		RENDER,
		SUBMIT,
		PRESENT,

		END
	};
}
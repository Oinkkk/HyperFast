#include "RenderLooper.h"

enum class RenderLooperMessageType : uint64_t
{
	DRAW = 0ULL
};

RenderLooper::~RenderLooper() noexcept
{
	stop();
}

void RenderLooper::start() noexcept
{
	__looper.start([this] (const uint64_t id, const std::vector<std::any> &arguments)
	{
		const RenderLooperMessageType messageType{ RenderLooperMessageType(id) };

		switch (messageType)
		{
		case RenderLooperMessageType::DRAW:
		{
			HyperFast::Screen *const pScreen{ std::any_cast<HyperFast::Screen *>(arguments[0]) };

			const bool validDraw{ pScreen->draw() };
			if (!validDraw)
				requestDraw(*pScreen);
		}
			break;
		}
	});
}

void RenderLooper::stop() noexcept
{
	__looper.stop();
}

void RenderLooper::requestDraw(HyperFast::Screen &screen) noexcept
{
	__looper.enqueueMessage(uint64_t(RenderLooperMessageType::DRAW), &screen);
}
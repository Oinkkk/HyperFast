#include "RenderLooper.h"

namespace Jin
{
	enum class MessageType : uint64_t
	{
		DRAW = 0ULL
	};

	void RenderLooper::start()
	{
		__looper.start([this](const std::vector<Infra::Message> &messages, const float deltaTime)
		{
			__loop(messages, deltaTime);
		});
	}

	void RenderLooper::stop() noexcept
	{
		__looper.stop();
	}

	void RenderLooper::draw(HyperFast::Screen &screen)
	{
		__looper.enqueueMessage(uint64_t(MessageType::DRAW), &screen);
	}

	void RenderLooper::__loop(const std::vector<Infra::Message> &messages, const float deltaTime)
	{
		for (const Infra::Message &message : messages)
		{
			switch (MessageType(message.id))
			{
			case MessageType::DRAW:
			{
				HyperFast::Screen *const pScreen
				{
					std::any_cast<HyperFast::Screen *>(message.arguments[0])
				};

				__drawRequested.emplace_back(pScreen);
			}
				break;
			}

			// TODO: 扁鸥 皋技瘤 贸府
		}

		// TODO: deltaTime 贸府

		if (__drawRequested.empty())
			return;

		for (HyperFast::Screen *const pScreen : __drawRequested)
			pScreen->draw();

		__drawRequested.clear();
	}
}
#pragma once

#include "../Infrastructure/Looper.h"
#include "../RenderingEngine/Screen.h"

namespace Jin
{
	class RenderLooper : public Infra::Unique
	{
	public:
		void start();
		void stop() noexcept;

		void draw(HyperFast::Screen &screen);

	private:
		Infra::Looper __looper;
		std::vector<HyperFast::Screen *> __drawRequested;

		void __loop(const std::vector<Infra::Message> &messages, const float deltaTime);
	};
}

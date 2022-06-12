#pragma once

#include "../Infrastructure/Looper.h"
#include "../RenderingEngine/Screen.h"

class RenderLooper : public Infra::Unique
{
public:
	~RenderLooper() noexcept;

	void start() noexcept;
	void stop() noexcept;

	void requestDraw(HyperFast::Screen &screen) noexcept;

private:
	Infra::MessageLooper __looper;
};
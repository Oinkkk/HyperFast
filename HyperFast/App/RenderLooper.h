#pragma once

#include "../Infrastructure/Looper.h"
#include "../RenderingEngine/Screen.h"

enum class RenderLooperMessageType : uint64_t
{
	DRAW = 0ULL
};

class RenderLooper : public Infra::Unique
{
public:
	~RenderLooper() noexcept;

	void start() noexcept;
	void stop() noexcept;

	void requestDraw(HyperFast::Screen *const pScreen) noexcept;

private:
	Infra::MessageLooper __looper;
};

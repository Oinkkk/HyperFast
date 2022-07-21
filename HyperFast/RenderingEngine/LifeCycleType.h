#pragma once

namespace HyperFast
{
	enum class LifeCycleType
	{
		START,

		// buffer, image �� screen�� ������ ���ҽ� ������Ʈ/������Ʈ command submit ��û
		DATA_UPDATE,

		// swaphain, framebuffer, renderpass �� screen�� ���õ� ���ҽ� ������Ʈ
		SCREEN_UPDATE,

		// rendering command recording
		RENDER,

		// ��� submit ��û commit
		SUBMIT,

		// swapchain present
		PRESENT,

		// ��� ����� ���ҽ� ����
		GARBAGE_COLLECT,

		END
	};
}
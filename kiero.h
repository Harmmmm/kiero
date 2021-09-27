#pragma once

#include "kiero_userconfig.h"

#include <Windows.h>
#include <stdio.h>
#include <stdint.h>

// Original: https://github.com/Rebzzel/kiero/
// Based on version 1.2.12 with parts of 1.22.0-preview
#define KIERO_VERSION "HarmsFork-1.0.0"

#ifndef KIERO_TEXTA
#define KIERO_TEXTA(_TEXT) _TEXT
#endif

#ifndef KIERO_TEXTW
#define KIERO_TEXTW(_TEXT) L##_TEXT
#endif

#ifdef UNICODE
#define KIERO_TEXT(_TEXT) KIERO_TEXTW(_TEXT)
#else
#define KIERO_TEXT(_TEXT) KIERO_TEXTA(_TEXT)
#endif

#ifndef KIERO_ARRAYSIZE
#define KIERO_ARRAYSIZE(_ARR) ((size_t)(sizeof(_ARR) / sizeof(*(_ARR))))
#endif

namespace kiero
{
	struct Status
	{
		enum Enum
		{
			UnknownError = -1,
			NotSupportedError = -2,
			ModuleNotFoundError = -3,
			AlreadyInitializedError = -4,
			NotInitializedError = -5,
			Success = 0,
		};
	};

	struct RenderType
	{
		enum Enum
		{
			None,
			D3D9,
			D3D10,
			D3D11,
			D3D12,
			OpenGL,
			Vulkan,
			Auto
		};
	};

	Status::Enum init(RenderType::Enum renderType);
	void shutdown();

	Status::Enum bind(uint16_t index, void** original, void* function);
	void unbind(uint16_t index);

	RenderType::Enum getRenderType();
	uintptr_t* getMethodsTable();
}
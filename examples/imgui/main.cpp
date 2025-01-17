#include "kiero.h"

#ifdef KIERO_INCLUDE_D3D9
#include "impl/d3d9_impl.h"
#endif

#ifdef KIERO_INCLUDE_D3D10
#include "impl/d3d10_impl.h"
#endif

#ifdef KIERO_INCLUDE_D3D11
#include "impl/d3d11_impl.h"
#endif

#ifdef KIERO_INCLUDE_D3D12
#endif

#ifdef KIERO_INCLUDE_OPENGL
#include "impl/opengl_impl.h"
#endif

#ifdef KIERO_INCLUDE_VULKAN
#endif

#ifndef KIERO_USE_MINHOOK
#error "The example requires that minhook be enabled!"
#endif

int kieroExampleThread()
{
    if (kiero::init(kiero::RenderType::Auto) == kiero::Status::Success)
    {
        switch (kiero::getRenderType())
        {
#ifdef KIERO_INCLUDE_D3D9
            case kiero::RenderType::D3D9:
                impl::d3d9::init();
                break;
#endif
#ifdef KIERO_INCLUDE_D3D10
            case kiero::RenderType::D3D10:
                impl::d3d10::init();
                break;
#endif
#ifdef KIERO_INCLUDE_D3D11
            case kiero::RenderType::D3D11:
                impl::d3d11::init();
                break;
#endif
#ifdef KIERO_INCLUDE_D3D12
            case kiero::RenderType::D3D12:
                //TODO: D3D12 implementation
                break;
#endif
#ifdef KIERO_INCLUDE_OPENGL
            case kiero::RenderType::OpenGL:
                impl::opengl::init();
                break;
#endif
#ifdef KIERO_INCLUDE_VULKAN
            case kiero::RenderType::Vulkan:
                //TODO: Vulkan implementation
                break;
#endif
        }

        return 1;
    }

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID)
{
    DisableThreadLibraryCalls(hInstance);

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)kieroExampleThread, NULL, 0, NULL);
            break;
    }

    return TRUE;
}
#include "kiero.h"

#ifdef KIERO_INCLUDE_D3D9
#include <d3d9.h>
#endif

#ifdef KIERO_INCLUDE_D3D10
#include <dxgi.h>
#include <d3d10_1.h>
#include <d3d10.h>
#endif

#ifdef KIERO_INCLUDE_D3D11
#include <dxgi.h>
#include <d3d11.h>
#endif

#ifdef KIERO_INCLUDE_D3D12
#include <dxgi.h>
#include <d3d12.h>
#endif

#ifdef KIERO_INCLUDE_OPENGL
#include <gl/GL.h>
#endif

#ifdef KIERO_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#endif

#ifdef KIERO_USE_MINHOOK
#include <MinHook.h>

#if defined _M_X64
#pragma comment(lib, "libMinHook.x64.lib")
#elif defined _M_IX86
#pragma comment(lib, "libMinHook.x86.lib")
#endif

#endif

static kiero::RenderType::Enum g_renderType = kiero::RenderType::None;
static uintptr_t* g_methodsTable = NULL;

kiero::Status::Enum kiero::init(RenderType::Enum _renderType)
{
	if (g_renderType != RenderType::None)
	{
		return Status::AlreadyInitializedError;
	}

	if (_renderType != RenderType::None)
	{
		if (_renderType >= RenderType::D3D9 && _renderType <= RenderType::D3D12)
		{
			WNDCLASSEX windowClass;
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.style = CS_HREDRAW | CS_VREDRAW;
			windowClass.lpfnWndProc = DefWindowProc;
			windowClass.cbClsExtra = 0;
			windowClass.cbWndExtra = 0;
			windowClass.hInstance = GetModuleHandle(NULL);
			windowClass.hIcon = NULL;
			windowClass.hCursor = NULL;
			windowClass.hbrBackground = NULL;
			windowClass.lpszMenuName = NULL;
			windowClass.lpszClassName = KIERO_TEXT("Kiero");
			windowClass.hIconSm = NULL;

			::RegisterClassEx(&windowClass);

			HWND window = ::CreateWindow(windowClass.lpszClassName, KIERO_TEXT("Kiero DirectX Window"), WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, windowClass.hInstance, NULL);

			if (_renderType == RenderType::D3D9)
			{
#ifdef KIERO_INCLUDE_D3D9
				HMODULE libD3D9;
				if ((libD3D9 = ::GetModuleHandle(KIERO_TEXT("d3d9.dll"))) == NULL)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::ModuleNotFoundError;
				}

				void* Direct3DCreate9;
				if ((Direct3DCreate9 = ::GetProcAddress(libD3D9, KIERO_TEXTA("Direct3DCreate9"))) == NULL)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				LPDIRECT3D9 direct3D9;
				if ((direct3D9 = ((LPDIRECT3D9(__stdcall*)(uint32_t))(Direct3DCreate9))(D3D_SDK_VERSION)) == NULL)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				D3DPRESENT_PARAMETERS params;
				params.BackBufferWidth = 0;
				params.BackBufferHeight = 0;
				params.BackBufferFormat = D3DFMT_UNKNOWN;
				params.BackBufferCount = 0;
				params.MultiSampleType = D3DMULTISAMPLE_NONE;
				params.MultiSampleQuality = NULL;
				params.SwapEffect = D3DSWAPEFFECT_DISCARD;
				params.hDeviceWindow = window;
				params.Windowed = 1;
				params.EnableAutoDepthStencil = 0;
				params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
				params.Flags = NULL;
				params.FullScreen_RefreshRateInHz = 0;
				params.PresentationInterval = 0;

				LPDIRECT3DDEVICE9 device;
				if (direct3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &params, &device) < 0)
				{
					direct3D9->Release();
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				g_methodsTable = (uintptr_t*)::calloc(119, sizeof(uintptr_t));
				::memcpy(g_methodsTable, *(uintptr_t**)device, 119 * sizeof(uintptr_t));

#ifdef KIERO_USE_MINHOOK
				MH_Initialize();
#endif

				device->Release();
				device = NULL;

				direct3D9->Release();
				direct3D9 = NULL;

				g_renderType = RenderType::D3D9;

				::DestroyWindow(window);
				::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

				return Status::Success;
#endif
			}
			else if (_renderType == RenderType::D3D10)
			{
#ifdef KIERO_INCLUDE_D3D10
				HMODULE libDXGI;
				HMODULE libD3D10;
				if ((libDXGI = ::GetModuleHandle(KIERO_TEXT("dxgi.dll"))) == NULL || (libD3D10 = ::GetModuleHandle(KIERO_TEXT("d3d10.dll"))) == NULL)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::ModuleNotFoundError;
				}

				void* CreateDXGIFactory;
				if ((CreateDXGIFactory = ::GetProcAddress(libDXGI, KIERO_TEXTA("CreateDXGIFactory"))) == NULL)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				IDXGIFactory* factory;
				if (((long(__stdcall*)(const IID&, void**))(CreateDXGIFactory))(__uuidof(IDXGIFactory), (void**)&factory) < 0)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				IDXGIAdapter* adapter;
				if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				void* D3D10CreateDeviceAndSwapChain;
				if ((D3D10CreateDeviceAndSwapChain = ::GetProcAddress(libD3D10, KIERO_TEXTA("D3D10CreateDeviceAndSwapChain"))) == NULL)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				DXGI_RATIONAL refreshRate;
				refreshRate.Numerator = 60;
				refreshRate.Denominator = 1;

				DXGI_MODE_DESC bufferDesc;
				bufferDesc.Width = 100;
				bufferDesc.Height = 100;
				bufferDesc.RefreshRate = refreshRate;
				bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

				DXGI_SAMPLE_DESC sampleDesc;
				sampleDesc.Count = 1;
				sampleDesc.Quality = 0;

				DXGI_SWAP_CHAIN_DESC swapChainDesc;
				swapChainDesc.BufferDesc = bufferDesc;
				swapChainDesc.SampleDesc = sampleDesc;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.BufferCount = 1;
				swapChainDesc.OutputWindow = window;
				swapChainDesc.Windowed = 1;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
				swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

				IDXGISwapChain* swapChain;
				ID3D10Device* device;

				if (((long(__stdcall*)(
					IDXGIAdapter*,
					D3D10_DRIVER_TYPE,
					HMODULE,
					UINT,
					UINT,
					DXGI_SWAP_CHAIN_DESC*,
					IDXGISwapChain**,
					ID3D10Device**))(D3D10CreateDeviceAndSwapChain))(adapter, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0, D3D10_SDK_VERSION, &swapChainDesc, &swapChain, &device) < 0)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				g_methodsTable = (uintptr_t*)::calloc(116, sizeof(uintptr_t));
				::memcpy(g_methodsTable, *(uintptr_t**)swapChain, 18 * sizeof(uintptr_t));
				::memcpy(g_methodsTable + 18, *(uintptr_t**)device, 98 * sizeof(uintptr_t));

#ifdef KIERO_USE_MINHOOK
				MH_Initialize();
#endif

				swapChain->Release();
				swapChain = NULL;

				device->Release();
				device = NULL;

				::DestroyWindow(window);
				::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

				g_renderType = RenderType::D3D10;

				return Status::Success;
#endif
			}
			else if (_renderType == RenderType::D3D11)
			{
#ifdef KIERO_INCLUDE_D3D11
				HMODULE libD3D11;
				if ((libD3D11 = ::GetModuleHandle(KIERO_TEXT("d3d11.dll"))) == NULL)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::ModuleNotFoundError;
				}

				void* D3D11CreateDeviceAndSwapChain;
				if ((D3D11CreateDeviceAndSwapChain = ::GetProcAddress(libD3D11, KIERO_TEXTA("D3D11CreateDeviceAndSwapChain"))) == NULL)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				D3D_FEATURE_LEVEL featureLevel;
				const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0 };

				DXGI_RATIONAL refreshRate;
				refreshRate.Numerator = 60;
				refreshRate.Denominator = 1;

				DXGI_MODE_DESC bufferDesc;
				bufferDesc.Width = 100;
				bufferDesc.Height = 100;
				bufferDesc.RefreshRate = refreshRate;
				bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

				DXGI_SAMPLE_DESC sampleDesc;
				sampleDesc.Count = 1;
				sampleDesc.Quality = 0;

				DXGI_SWAP_CHAIN_DESC swapChainDesc;
				swapChainDesc.BufferDesc = bufferDesc;
				swapChainDesc.SampleDesc = sampleDesc;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.BufferCount = 1;
				swapChainDesc.OutputWindow = window;
				swapChainDesc.Windowed = 1;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
				swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

				IDXGISwapChain* swapChain;
				ID3D11Device* device;
				ID3D11DeviceContext* context;

				if (((long(__stdcall*)(
					IDXGIAdapter*,
					D3D_DRIVER_TYPE,
					HMODULE,
					UINT,
					const D3D_FEATURE_LEVEL*,
					UINT,
					UINT,
					const DXGI_SWAP_CHAIN_DESC*,
					IDXGISwapChain**,
					ID3D11Device**,
					D3D_FEATURE_LEVEL*,
					ID3D11DeviceContext**))(D3D11CreateDeviceAndSwapChain))(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 2, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, &featureLevel, &context) < 0)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				g_methodsTable = (uintptr_t*)::calloc(205, sizeof(uintptr_t));
				::memcpy(g_methodsTable, *(uintptr_t**)swapChain, 18 * sizeof(uintptr_t));
				::memcpy(g_methodsTable + 18, *(uintptr_t**)device, 43 * sizeof(uintptr_t));
				::memcpy(g_methodsTable + 18 + 43, *(uintptr_t**)context, 144 * sizeof(uintptr_t));

#ifdef KIERO_USE_MINHOOK
				MH_Initialize();
#endif

				swapChain->Release();
				swapChain = NULL;

				device->Release();
				device = NULL;

				context->Release();
				context = NULL;

				::DestroyWindow(window);
				::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

				g_renderType = RenderType::D3D11;

				return Status::Success;
#endif
			}
			else if (_renderType == RenderType::D3D12)
			{
#ifdef KIERO_INCLUDE_D3D12
				HMODULE libDXGI;
				HMODULE libD3D12;
				if ((libDXGI = ::GetModuleHandle(KIERO_TEXT("dxgi.dll"))) == NULL || (libD3D12 = ::GetModuleHandle(KIERO_TEXT("d3d12.dll"))) == NULL)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::ModuleNotFoundError;
				}

				void* CreateDXGIFactory;
				if ((CreateDXGIFactory = ::GetProcAddress(libDXGI, KIERO_TEXTA("CreateDXGIFactory"))) == NULL)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				IDXGIFactory* factory;
				if (((long(__stdcall*)(const IID&, void**))(CreateDXGIFactory))(__uuidof(IDXGIFactory), (void**)&factory) < 0)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				IDXGIAdapter* adapter;
				if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				void* D3D12CreateDevice;
				if ((D3D12CreateDevice = ::GetProcAddress(libD3D12, KIERO_TEXTA("D3D12CreateDevice"))) == NULL)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				ID3D12Device* device;
				if (((long(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, const IID&, void**))(D3D12CreateDevice))(adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&device) < 0)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				D3D12_COMMAND_QUEUE_DESC queueDesc;
				queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
				queueDesc.Priority = 0;
				queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				queueDesc.NodeMask = 0;

				ID3D12CommandQueue* commandQueue;
				if (device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), (void**)&commandQueue) < 0)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				ID3D12CommandAllocator* commandAllocator;
				if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&commandAllocator) < 0)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				ID3D12GraphicsCommandList* commandList;
				if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&commandList) < 0)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				DXGI_RATIONAL refreshRate;
				refreshRate.Numerator = 60;
				refreshRate.Denominator = 1;

				DXGI_MODE_DESC bufferDesc;
				bufferDesc.Width = 100;
				bufferDesc.Height = 100;
				bufferDesc.RefreshRate = refreshRate;
				bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

				DXGI_SAMPLE_DESC sampleDesc;
				sampleDesc.Count = 1;
				sampleDesc.Quality = 0;

				DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
				swapChainDesc.BufferDesc = bufferDesc;
				swapChainDesc.SampleDesc = sampleDesc;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.BufferCount = 2;
				swapChainDesc.OutputWindow = window;
				swapChainDesc.Windowed = 1;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

				IDXGISwapChain* swapChain;
				if (factory->CreateSwapChain(commandQueue, &swapChainDesc, &swapChain) < 0)
				{
					::DestroyWindow(window);
					::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
					return Status::UnknownError;
				}

				g_methodsTable = (uintptr_t*)::calloc(150, sizeof(uintptr_t));
				::memcpy(g_methodsTable, *(uintptr_t**)device, 44 * sizeof(uintptr_t));
				::memcpy(g_methodsTable + 44, *(uintptr_t**)commandQueue, 19 * sizeof(uintptr_t));
				::memcpy(g_methodsTable + 44 + 19, *(uintptr_t**)commandAllocator, 9 * sizeof(uintptr_t));
				::memcpy(g_methodsTable + 44 + 19 + 9, *(uintptr_t**)commandList, 60 * sizeof(uintptr_t));
				::memcpy(g_methodsTable + 44 + 19 + 9 + 60, *(uintptr_t**)swapChain, 18 * sizeof(uintptr_t));

#ifdef KIERO_USE_MINHOOK
				MH_Initialize();
#endif

				device->Release();
				device = NULL;

				commandQueue->Release();
				commandQueue = NULL;

				commandAllocator->Release();
				commandAllocator = NULL;

				commandList->Release();
				commandList = NULL;

				swapChain->Release();
				swapChain = NULL;

				::DestroyWindow(window);
				::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

				g_renderType = RenderType::D3D12;

				return Status::Success;
#endif
			}

			::DestroyWindow(window);
			::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

			return Status::NotSupportedError;
		}
		else if (_renderType != RenderType::Auto)
		{
			if (_renderType == RenderType::OpenGL)
			{
#ifdef KIERO_INCLUDE_OPENGL
				HMODULE libOpenGL32;
				if ((libOpenGL32 = ::GetModuleHandle(KIERO_TEXT("opengl32.dll"))) == NULL)
				{
					return Status::ModuleNotFoundError;
				}

				const char* const methodsNames[] = {
					KIERO_TEXTA("glAccum"), KIERO_TEXTA("glAlphaFunc"), KIERO_TEXTA("glAreTexturesResident"), KIERO_TEXTA("glArrayElement"), KIERO_TEXTA("glBegin"),
					KIERO_TEXTA("glBindTexture"), KIERO_TEXTA("glBitmap"), KIERO_TEXTA("glBlendFunc"), KIERO_TEXTA("glCallList"), KIERO_TEXTA("glCallLists"),
					KIERO_TEXTA("glClear"), KIERO_TEXTA("glClearAccum"), KIERO_TEXTA("glClearColor"), KIERO_TEXTA("glClearDepth"), KIERO_TEXTA("glClearIndex"),
					KIERO_TEXTA("glClearStencil"), KIERO_TEXTA("glClipPlane"), KIERO_TEXTA("glColor3b"), KIERO_TEXTA("glColor3bv"), KIERO_TEXTA("glColor3d"),
					KIERO_TEXTA("glColor3dv"), KIERO_TEXTA("glColor3f"), KIERO_TEXTA("glColor3fv"), KIERO_TEXTA("glColor3i"), KIERO_TEXTA("glColor3iv"),
					KIERO_TEXTA("glColor3s"), KIERO_TEXTA("glColor3sv"), KIERO_TEXTA("glColor3ub"), KIERO_TEXTA("glColor3ubv"), KIERO_TEXTA("glColor3ui"),
					KIERO_TEXTA("glColor3uiv"), KIERO_TEXTA("glColor3us"), KIERO_TEXTA("glColor3usv"), KIERO_TEXTA("glColor4b"), KIERO_TEXTA("glColor4bv"),
					KIERO_TEXTA("glColor4d"), KIERO_TEXTA("glColor4dv"), KIERO_TEXTA("glColor4f"), KIERO_TEXTA("glColor4fv"), KIERO_TEXTA("glColor4i"),
					KIERO_TEXTA("glColor4iv"), KIERO_TEXTA("glColor4s"), KIERO_TEXTA("glColor4sv"), KIERO_TEXTA("glColor4ub"), KIERO_TEXTA("glColor4ubv"),
					KIERO_TEXTA("glColor4ui"), KIERO_TEXTA("glColor4uiv"), KIERO_TEXTA("glColor4us"), KIERO_TEXTA("glColor4usv"), KIERO_TEXTA("glColorMask"),
					KIERO_TEXTA("glColorMaterial"), KIERO_TEXTA("glColorPointer"), KIERO_TEXTA("glCopyPixels"), KIERO_TEXTA("glCopyTexImage1D"),
					KIERO_TEXTA("glCopyTexImage2D"), KIERO_TEXTA("glCopyTexSubImage1D"), KIERO_TEXTA("glCopyTexSubImage2D"), KIERO_TEXTA("glCullFaceglCullFace"),
					KIERO_TEXTA("glDeleteLists"), KIERO_TEXTA("glDeleteTextures"), KIERO_TEXTA("glDepthFunc"), KIERO_TEXTA("glDepthMask"), KIERO_TEXTA("glDepthRange"),
					KIERO_TEXTA("glDisable"), KIERO_TEXTA("glDisableClientState"), KIERO_TEXTA("glDrawArrays"), KIERO_TEXTA("glDrawBuffer"), KIERO_TEXTA("glDrawElements"),
					KIERO_TEXTA("glDrawPixels"), KIERO_TEXTA("glEdgeFlag"), KIERO_TEXTA("glEdgeFlagPointer"), KIERO_TEXTA("glEdgeFlagv"), KIERO_TEXTA("glEnable"),
					KIERO_TEXTA("glEnableClientState"), KIERO_TEXTA("glEnd"), KIERO_TEXTA("glEndList"), KIERO_TEXTA("glEvalCoord1d"), KIERO_TEXTA("glEvalCoord1dv"),
					KIERO_TEXTA("glEvalCoord1f"), KIERO_TEXTA("glEvalCoord1fv"), KIERO_TEXTA("glEvalCoord2d"), KIERO_TEXTA("glEvalCoord2dv"), KIERO_TEXTA("glEvalCoord2f"),
					KIERO_TEXTA("glEvalCoord2fv"), KIERO_TEXTA("glEvalMesh1"), KIERO_TEXTA("glEvalMesh2"), KIERO_TEXTA("glEvalPoint1"), KIERO_TEXTA("glEvalPoint2"),
					KIERO_TEXTA("glFeedbackBuffer"), KIERO_TEXTA("glFinish"), KIERO_TEXTA("glFlush"), KIERO_TEXTA("glFogf"), KIERO_TEXTA("glFogfv"), KIERO_TEXTA("glFogi"),
					KIERO_TEXTA("glFogiv"), KIERO_TEXTA("glFrontFace"), KIERO_TEXTA("glFrustum"), KIERO_TEXTA("glGenLists"), KIERO_TEXTA("glGenTextures"),
					KIERO_TEXTA("glGetBooleanv"), KIERO_TEXTA("glGetClipPlane"), KIERO_TEXTA("glGetDoublev"), KIERO_TEXTA("glGetError"), KIERO_TEXTA("glGetFloatv"),
					KIERO_TEXTA("glGetIntegerv"), KIERO_TEXTA("glGetLightfv"), KIERO_TEXTA("glGetLightiv"), KIERO_TEXTA("glGetMapdv"), KIERO_TEXTA("glGetMapfv"),
					KIERO_TEXTA("glGetMapiv"), KIERO_TEXTA("glGetMaterialfv"), KIERO_TEXTA("glGetMaterialiv"), KIERO_TEXTA("glGetPixelMapfv"),
					KIERO_TEXTA("glGetPixelMapuiv"), KIERO_TEXTA("glGetPixelMapusv"), KIERO_TEXTA("glGetPointerv"), KIERO_TEXTA("glGetPolygonStipple"),
					KIERO_TEXTA("glGetString"), KIERO_TEXTA("glGetTexEnvfv"), KIERO_TEXTA("glGetTexEnviv"), KIERO_TEXTA("glGetTexGendv"), KIERO_TEXTA("glGetTexGenfv"),
					KIERO_TEXTA("glGetTexGeniv"), KIERO_TEXTA("glGetTexImage"), KIERO_TEXTA("glGetTexLevelParameterfv"), KIERO_TEXTA("glGetTexLevelParameteriv"),
					KIERO_TEXTA("glGetTexParameterfv"), KIERO_TEXTA("glGetTexParameteriv"), KIERO_TEXTA("glHint"), KIERO_TEXTA("glIndexMask"), KIERO_TEXTA("glIndexPointer"),
					KIERO_TEXTA("glIndexd"), KIERO_TEXTA("glIndexdv"), KIERO_TEXTA("glIndexf"), KIERO_TEXTA("glIndexfv"), KIERO_TEXTA("glIndexi"), KIERO_TEXTA("glIndexiv"),
					KIERO_TEXTA("glIndexs"), KIERO_TEXTA("glIndexsv"), KIERO_TEXTA("glIndexub"), KIERO_TEXTA("glIndexubv"), KIERO_TEXTA("glInitNames"),
					KIERO_TEXTA("glInterleavedArrays"), KIERO_TEXTA("glIsEnabled"), KIERO_TEXTA("glIsList"), KIERO_TEXTA("glIsTexture"), KIERO_TEXTA("glLightModelf"),
					KIERO_TEXTA("glLightModelfv"), KIERO_TEXTA("glLightModeli"), KIERO_TEXTA("glLightModeliv"), KIERO_TEXTA("glLightf"), KIERO_TEXTA("glLightfv"),
					KIERO_TEXTA("glLighti"), KIERO_TEXTA("glLightiv"), KIERO_TEXTA("glLineStipple"), KIERO_TEXTA("glLineWidth"), KIERO_TEXTA("glListBase"),
					KIERO_TEXTA("glLoadIdentity"), KIERO_TEXTA("glLoadMatrixd"), KIERO_TEXTA("glLoadMatrixf"), KIERO_TEXTA("glLoadName"), KIERO_TEXTA("glLogicOp"),
					KIERO_TEXTA("glMap1d"), KIERO_TEXTA("glMap1f"), KIERO_TEXTA("glMap2d"), KIERO_TEXTA("glMap2f"), KIERO_TEXTA("glMapGrid1d"), KIERO_TEXTA("glMapGrid1f"),
					KIERO_TEXTA("glMapGrid2d"), KIERO_TEXTA("glMapGrid2f"), KIERO_TEXTA("glMaterialf"), KIERO_TEXTA("glMaterialfv"), KIERO_TEXTA("glMateriali"),
					KIERO_TEXTA("glMaterialiv"), KIERO_TEXTA("glMatrixMode"), KIERO_TEXTA("glMultMatrixd"), KIERO_TEXTA("glMultMatrixf"), KIERO_TEXTA("glNewList"),
					KIERO_TEXTA("glNormal3b"), KIERO_TEXTA("glNormal3bv"), KIERO_TEXTA("glNormal3d"), KIERO_TEXTA("glNormal3dv"), KIERO_TEXTA("glNormal3f"),
					KIERO_TEXTA("glNormal3fv"), KIERO_TEXTA("glNormal3i"), KIERO_TEXTA("glNormal3iv"), KIERO_TEXTA("glNormal3s"), KIERO_TEXTA("glNormal3sv"),
					KIERO_TEXTA("glNormalPointer"), KIERO_TEXTA("glOrtho"), KIERO_TEXTA("glPassThrough"), KIERO_TEXTA("glPixelMapfv"), KIERO_TEXTA("glPixelMapuiv"),
					KIERO_TEXTA("glPixelMapusv"), KIERO_TEXTA("glPixelStoref"), KIERO_TEXTA("glPixelStorei"), KIERO_TEXTA("glPixelTransferf"),
					KIERO_TEXTA("glPixelTransferi"), KIERO_TEXTA("glPixelZoom"), KIERO_TEXTA("glPointSize"), KIERO_TEXTA("glPolygonMode"), KIERO_TEXTA("glPolygonOffset"),
					KIERO_TEXTA("glPolygonStipple"), KIERO_TEXTA("glPopAttrib"), KIERO_TEXTA("glPopClientAttrib"), KIERO_TEXTA("glPopMatrix"), KIERO_TEXTA("glPopName"),
					KIERO_TEXTA("glPrioritizeTextures"), KIERO_TEXTA("glPushAttrib"), KIERO_TEXTA("glPushClientAttrib"), KIERO_TEXTA("glPushMatrix"),
					KIERO_TEXTA("glPushName"), KIERO_TEXTA("glRasterPos2d"), KIERO_TEXTA("glRasterPos2dv"), KIERO_TEXTA("glRasterPos2f"), KIERO_TEXTA("glRasterPos2fv"),
					KIERO_TEXTA("glRasterPos2i"), KIERO_TEXTA("glRasterPos2iv"), KIERO_TEXTA("glRasterPos2s"), KIERO_TEXTA("glRasterPos2sv"), KIERO_TEXTA("glRasterPos3d"),
					KIERO_TEXTA("glRasterPos3dv"), KIERO_TEXTA("glRasterPos3f"), KIERO_TEXTA("glRasterPos3fv"), KIERO_TEXTA("glRasterPos3i"), KIERO_TEXTA("glRasterPos3iv"),
					KIERO_TEXTA("glRasterPos3s"), KIERO_TEXTA("glRasterPos3sv"), KIERO_TEXTA("glRasterPos4d"), KIERO_TEXTA("glRasterPos4dv"), KIERO_TEXTA("glRasterPos4f"),
					KIERO_TEXTA("glRasterPos4fv"), KIERO_TEXTA("glRasterPos4i"), KIERO_TEXTA("glRasterPos4iv"), KIERO_TEXTA("glRasterPos4s"), KIERO_TEXTA("glRasterPos4sv"),
					KIERO_TEXTA("glReadBuffer"), KIERO_TEXTA("glReadPixels"), KIERO_TEXTA("glRectd"), KIERO_TEXTA("glRectdv"), KIERO_TEXTA("glRectf"),
					KIERO_TEXTA("glRectfv"), KIERO_TEXTA("glRecti"), KIERO_TEXTA("glRectiv"), KIERO_TEXTA("glRects"), KIERO_TEXTA("glRectsv"), KIERO_TEXTA("glRenderMode"),
					KIERO_TEXTA("glRotated"), KIERO_TEXTA("glRotatef"), KIERO_TEXTA("glScaled"), KIERO_TEXTA("glScalef"), KIERO_TEXTA("glScissor"),
					KIERO_TEXTA("glSelectBuffer"), KIERO_TEXTA("glShadeModel"), KIERO_TEXTA("glStencilFunc"), KIERO_TEXTA("glStencilMask"), KIERO_TEXTA("glStencilOp"),
					KIERO_TEXTA("glTexCoord1d"), KIERO_TEXTA("glTexCoord1dv"), KIERO_TEXTA("glTexCoord1f"), KIERO_TEXTA("glTexCoord1fv"), KIERO_TEXTA("glTexCoord1i"),
					KIERO_TEXTA("glTexCoord1iv"), KIERO_TEXTA("glTexCoord1s"), KIERO_TEXTA("glTexCoord1sv"), KIERO_TEXTA("glTexCoord2d"), KIERO_TEXTA("glTexCoord2dv"),
					KIERO_TEXTA("glTexCoord2f"), KIERO_TEXTA("glTexCoord2fv"), KIERO_TEXTA("glTexCoord2i"), KIERO_TEXTA("glTexCoord2iv"), KIERO_TEXTA("glTexCoord2s"),
					KIERO_TEXTA("glTexCoord2sv"), KIERO_TEXTA("glTexCoord3d"), KIERO_TEXTA("glTexCoord3dv"), KIERO_TEXTA("glTexCoord3f"), KIERO_TEXTA("glTexCoord3fv"),
					KIERO_TEXTA("glTexCoord3i"), KIERO_TEXTA("glTexCoord3iv"), KIERO_TEXTA("glTexCoord3s"), KIERO_TEXTA("glTexCoord3sv"), KIERO_TEXTA("glTexCoord4d"),
					KIERO_TEXTA("glTexCoord4dv"), KIERO_TEXTA("glTexCoord4f"), KIERO_TEXTA("glTexCoord4fv"), KIERO_TEXTA("glTexCoord4i"), KIERO_TEXTA("glTexCoord4iv"),
					KIERO_TEXTA("glTexCoord4s"), KIERO_TEXTA("glTexCoord4sv"), KIERO_TEXTA("glTexCoordPointer"), KIERO_TEXTA("glTexEnvf"), KIERO_TEXTA("glTexEnvfv"),
					KIERO_TEXTA("glTexEnvi"), KIERO_TEXTA("glTexEnviv"), KIERO_TEXTA("glTexGend"), KIERO_TEXTA("glTexGendv"), KIERO_TEXTA("glTexGenf"),
					KIERO_TEXTA("glTexGenfv"), KIERO_TEXTA("glTexGeni"), KIERO_TEXTA("glTexGeniv"), KIERO_TEXTA("glTexImage1D"), KIERO_TEXTA("glTexImage2D"),
					KIERO_TEXTA("glTexParameterf"), KIERO_TEXTA("glTexParameterfv"), KIERO_TEXTA("glTexParameteri"), KIERO_TEXTA("glTexParameteriv"),
					KIERO_TEXTA("glTexSubImage1D"), KIERO_TEXTA("glTexSubImage2D"), KIERO_TEXTA("glTranslated"), KIERO_TEXTA("glTranslatef"), KIERO_TEXTA("glVertex2d"),
					KIERO_TEXTA("glVertex2dv"), KIERO_TEXTA("glVertex2f"), KIERO_TEXTA("glVertex2fv"), KIERO_TEXTA("glVertex2i"), KIERO_TEXTA("glVertex2iv"),
					KIERO_TEXTA("glVertex2s"), KIERO_TEXTA("glVertex2sv"), KIERO_TEXTA("glVertex3d"), KIERO_TEXTA("glVertex3dv"), KIERO_TEXTA("glVertex3f"),
					KIERO_TEXTA("glVertex3fv"), KIERO_TEXTA("glVertex3i"), KIERO_TEXTA("glVertex3iv"), KIERO_TEXTA("glVertex3s"), KIERO_TEXTA("glVertex3sv"),
					KIERO_TEXTA("glVertex4d"), KIERO_TEXTA("glVertex4dv"), KIERO_TEXTA("glVertex4f"), KIERO_TEXTA("glVertex4fv"), KIERO_TEXTA("glVertex4i"),
					KIERO_TEXTA("glVertex4iv"), KIERO_TEXTA("glVertex4s"), KIERO_TEXTA("glVertex4sv"), KIERO_TEXTA("glVertexPointer"), KIERO_TEXTA("glViewport"),
					KIERO_TEXTA("wglSwapBuffers") 
				};

				size_t size = KIERO_ARRAYSIZE(methodsNames);

				g_methodsTable = (uintptr_t*)::calloc(size, sizeof(uintptr_t));

				for (int i = 0; i < size; i++)
				{
					g_methodsTable[i] = (uintptr_t)::GetProcAddress(libOpenGL32, methodsNames[i]);
				}

#ifdef KIERO_USE_MINHOOK
				MH_Initialize();
#endif

				g_renderType = RenderType::OpenGL;

				return Status::Success;
#endif
			}
			else if (_renderType == RenderType::Vulkan)
			{
#ifdef KIERO_INCLUDE_VULKAN
				HMODULE libVulkan;
				if ((libVulkan = GetModuleHandle(KIERO_TEXT("vulkan-1.dll"))) == NULL)
				{
					return Status::ModuleNotFoundError;
				}

				const char* const methodsNames[] = {
					KIERO_TEXTA("vkCreateInstance"), KIERO_TEXTA("vkDestroyInstance"), KIERO_TEXTA("vkEnumeratePhysicalDevices"),
					KIERO_TEXTA("vkGetPhysicalDeviceFeatures"), KIERO_TEXTA("vkGetPhysicalDeviceFormatProperties"),
					KIERO_TEXTA("vkGetPhysicalDeviceImageFormatProperties"), KIERO_TEXTA("vkGetPhysicalDeviceProperties"),
					KIERO_TEXTA("vkGetPhysicalDeviceQueueFamilyProperties"), KIERO_TEXTA("vkGetPhysicalDeviceMemoryProperties"),
					KIERO_TEXTA("vkGetInstanceProcAddr"), KIERO_TEXTA("vkGetDeviceProcAddr"), KIERO_TEXTA("vkCreateDevice"), KIERO_TEXTA("vkDestroyDevice"),
					KIERO_TEXTA("vkEnumerateInstanceExtensionProperties"), KIERO_TEXTA("vkEnumerateDeviceExtensionProperties"),
					KIERO_TEXTA("vkEnumerateDeviceLayerProperties"), KIERO_TEXTA("vkGetDeviceQueue"), KIERO_TEXTA("vkQueueSubmit"), KIERO_TEXTA("vkQueueWaitIdle"),
					KIERO_TEXTA("vkDeviceWaitIdle"), KIERO_TEXTA("vkAllocateMemory"), KIERO_TEXTA("vkFreeMemory"), KIERO_TEXTA("vkMapMemory"), KIERO_TEXTA("vkUnmapMemory"),
					KIERO_TEXTA("vkFlushMappedMemoryRanges"), KIERO_TEXTA("vkInvalidateMappedMemoryRanges"), KIERO_TEXTA("vkGetDeviceMemoryCommitment"),
					KIERO_TEXTA("vkBindBufferMemory"), KIERO_TEXTA("vkBindImageMemory"), KIERO_TEXTA("vkGetBufferMemoryRequirements"),
					KIERO_TEXTA("vkGetImageMemoryRequirements"), KIERO_TEXTA("vkGetImageSparseMemoryRequirements"),
					KIERO_TEXTA("vkGetPhysicalDeviceSparseImageFormatProperties"), KIERO_TEXTA("vkQueueBindSparse"), KIERO_TEXTA("vkCreateFence"),
					KIERO_TEXTA("vkDestroyFence"), KIERO_TEXTA("vkResetFences"), KIERO_TEXTA("vkGetFenceStatus"), KIERO_TEXTA("vkWaitForFences"),
					KIERO_TEXTA("vkCreateSemaphore"), KIERO_TEXTA("vkDestroySemaphore"), KIERO_TEXTA("vkCreateEvent"), KIERO_TEXTA("vkDestroyEvent"),
					KIERO_TEXTA("vkGetEventStatus"), KIERO_TEXTA("vkSetEvent"), KIERO_TEXTA("vkResetEvent"), KIERO_TEXTA("vkCreateQueryPool"),
					KIERO_TEXTA("vkDestroyQueryPool"), KIERO_TEXTA("vkGetQueryPoolResults"), KIERO_TEXTA("vkCreateBuffer"), KIERO_TEXTA("vkDestroyBuffer"),
					KIERO_TEXTA("vkCreateBufferView"), KIERO_TEXTA("vkDestroyBufferView"), KIERO_TEXTA("vkCreateImage"), KIERO_TEXTA("vkDestroyImage"),
					KIERO_TEXTA("vkGetImageSubresourceLayout"), KIERO_TEXTA("vkCreateImageView"), KIERO_TEXTA("vkDestroyImageView"), KIERO_TEXTA("vkCreateShaderModule"),
					KIERO_TEXTA("vkDestroyShaderModule"), KIERO_TEXTA("vkCreatePipelineCache"), KIERO_TEXTA("vkDestroyPipelineCache"),
					KIERO_TEXTA("vkGetPipelineCacheData"), KIERO_TEXTA("vkMergePipelineCaches"), KIERO_TEXTA("vkCreateGraphicsPipelines"),
					KIERO_TEXTA("vkCreateComputePipelines"), KIERO_TEXTA("vkDestroyPipeline"), KIERO_TEXTA("vkCreatePipelineLayout"),
					KIERO_TEXTA("vkDestroyPipelineLayout"), KIERO_TEXTA("vkCreateSampler"), KIERO_TEXTA("vkDestroySampler"), KIERO_TEXTA("vkCreateDescriptorSetLayout"),
					KIERO_TEXTA("vkDestroyDescriptorSetLayout"), KIERO_TEXTA("vkCreateDescriptorPool"), KIERO_TEXTA("vkDestroyDescriptorPool"),
					KIERO_TEXTA("vkResetDescriptorPool"), KIERO_TEXTA("vkAllocateDescriptorSets"), KIERO_TEXTA("vkFreeDescriptorSets"),
					KIERO_TEXTA("vkUpdateDescriptorSets"), KIERO_TEXTA("vkCreateFramebuffer"), KIERO_TEXTA("vkDestroyFramebuffer"), KIERO_TEXTA("vkCreateRenderPass"),
					KIERO_TEXTA("vkDestroyRenderPass"), KIERO_TEXTA("vkGetRenderAreaGranularity"), KIERO_TEXTA("vkCreateCommandPool"), KIERO_TEXTA("vkDestroyCommandPool"),
					KIERO_TEXTA("vkResetCommandPool"), KIERO_TEXTA("vkAllocateCommandBuffers"), KIERO_TEXTA("vkFreeCommandBuffers"), KIERO_TEXTA("vkBeginCommandBuffer"),
					KIERO_TEXTA("vkEndCommandBuffer"), KIERO_TEXTA("vkResetCommandBuffer"), KIERO_TEXTA("vkCmdBindPipeline"), KIERO_TEXTA("vkCmdSetViewport"),
					KIERO_TEXTA("vkCmdSetScissor"), KIERO_TEXTA("vkCmdSetLineWidth"), KIERO_TEXTA("vkCmdSetDepthBias"), KIERO_TEXTA("vkCmdSetBlendConstants"),
					KIERO_TEXTA("vkCmdSetDepthBounds"), KIERO_TEXTA("vkCmdSetStencilCompareMask"), KIERO_TEXTA("vkCmdSetStencilWriteMask"),
					KIERO_TEXTA("vkCmdSetStencilReference"), KIERO_TEXTA("vkCmdBindDescriptorSets"), KIERO_TEXTA("vkCmdBindIndexBuffer"),
					KIERO_TEXTA("vkCmdBindVertexBuffers"), KIERO_TEXTA("vkCmdDraw"), KIERO_TEXTA("vkCmdDrawIndexed"), KIERO_TEXTA("vkCmdDrawIndirect"),
					KIERO_TEXTA("vkCmdDrawIndexedIndirect"), KIERO_TEXTA("vkCmdDispatch"), KIERO_TEXTA("vkCmdDispatchIndirect"), KIERO_TEXTA("vkCmdCopyBuffer"),
					KIERO_TEXTA("vkCmdCopyImage"), KIERO_TEXTA("vkCmdBlitImage"), KIERO_TEXTA("vkCmdCopyBufferToImage"), KIERO_TEXTA("vkCmdCopyImageToBuffer"),
					KIERO_TEXTA("vkCmdUpdateBuffer"), KIERO_TEXTA("vkCmdFillBuffer"), KIERO_TEXTA("vkCmdClearColorImage"), KIERO_TEXTA("vkCmdClearDepthStencilImage"),
					KIERO_TEXTA("vkCmdClearAttachments"), KIERO_TEXTA("vkCmdResolveImage"), KIERO_TEXTA("vkCmdSetEvent"), KIERO_TEXTA("vkCmdResetEvent"),
					KIERO_TEXTA("vkCmdWaitEvents"), KIERO_TEXTA("vkCmdPipelineBarrier"), KIERO_TEXTA("vkCmdBeginQuery"), KIERO_TEXTA("vkCmdEndQuery"),
					KIERO_TEXTA("vkCmdResetQueryPool"), KIERO_TEXTA("vkCmdWriteTimestamp"), KIERO_TEXTA("vkCmdCopyQueryPoolResults"), KIERO_TEXTA("vkCmdPushConstants"),
					KIERO_TEXTA("vkCmdBeginRenderPass"), KIERO_TEXTA("vkCmdNextSubpass"), KIERO_TEXTA("vkCmdEndRenderPass"), KIERO_TEXTA("vkCmdExecuteCommands")
				};

				size_t size = KIERO_ARRAYSIZE(methodsNames);

				g_methodsTable = (uintptr_t*)::calloc(size, sizeof(uintptr_t));

				for (int i = 0; i < size; i++)
				{
					g_methodsTable[i] = (uintptr_t)::GetProcAddress(libVulkan, methodsNames[i]);
				}

#ifdef KIERO_USE_MINHOOK
				MH_Initialize();
#endif

				g_renderType = RenderType::Vulkan;

				return Status::Success;
#endif
			}

			return Status::NotSupportedError;
		}
		else
		{
			RenderType::Enum type = RenderType::None;

			if (::GetModuleHandle(KIERO_TEXT("d3d9.dll")) != NULL)
			{
				type = RenderType::D3D9;
			}
			else if (::GetModuleHandle(KIERO_TEXT("d3d10.dll")) != NULL)
			{
				type = RenderType::D3D10;
			}
			else if (::GetModuleHandle(KIERO_TEXT("d3d11.dll")) != NULL)
			{
				type = RenderType::D3D11;
			}
			else if (::GetModuleHandle(KIERO_TEXT("d3d12.dll")) != NULL)
			{
				type = RenderType::D3D12;
			}
			else if (::GetModuleHandle(KIERO_TEXT("opengl32.dll")) != NULL)
			{
				type = RenderType::OpenGL;
			}
			else if (::GetModuleHandle(KIERO_TEXT("vulkan-1.dll")) != NULL)
			{
				type = RenderType::Vulkan;
			}
			else
			{
				return Status::NotSupportedError;
			}

			return init(type);
		}
	}

	return Status::Success;
}

void kiero::shutdown()
{
	if (g_renderType != RenderType::None)
	{
#ifdef KIERO_USE_MINHOOK
		MH_DisableHook(MH_ALL_HOOKS);
#endif

		::free(g_methodsTable);
		g_methodsTable = NULL;
		g_renderType = RenderType::None;
	}
}

kiero::Status::Enum kiero::bind(uint16_t _index, void** _original, void* _function)
{
	if (g_renderType != RenderType::None)
	{
#ifdef KIERO_USE_MINHOOK
		void* target = (void*)g_methodsTable[_index];
		if (MH_CreateHook(target, _function, _original) != MH_OK || MH_EnableHook(target) != MH_OK)
		{
			return Status::UnknownError;
		}

		return Status::Success;
#endif
	}

	return Status::NotInitializedError;
}

void kiero::unbind(uint16_t _index)
{
	if (g_renderType != RenderType::None)
	{
#ifdef KIERO_USE_MINHOOK
		MH_DisableHook((void*)g_methodsTable[_index]);
#endif
	}
}

kiero::RenderType::Enum kiero::getRenderType()
{
	return g_renderType;
}

uintptr_t* kiero::getMethodsTable()
{
	return g_methodsTable;
} 
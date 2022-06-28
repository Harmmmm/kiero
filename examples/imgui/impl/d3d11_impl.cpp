#include "kiero.h"

#ifdef KIERO_INCLUDE_D3D11

#include "d3d11_impl.h"
#include <d3d11.h>

#include "win32_impl.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"

typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = NULL;

typedef long(__stdcall* ResizeBuffers)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
static ResizeBuffers oResizeBuffers = NULL;

static ID3D11RenderTargetView* pRenderTargetView;
static ID3D11DeviceContext* pDeviceContext;

static void CreateRenderTargetView(IDXGISwapChain* pSwapChain)
{
	ID3D11Device* device = nullptr;
	ID3D11Texture2D* backBuffer = nullptr;

	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device);

	if (device && backBuffer)
	{
		device->CreateRenderTargetView(backBuffer, NULL, &pRenderTargetView);
		backBuffer->Release();
	}
}

long __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	static bool init = false;

	if (!init)
	{
		DXGI_SWAP_CHAIN_DESC desc;
		pSwapChain->GetDesc(&desc);

		ID3D11Device* device;
		pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device);
		
		device->GetImmediateContext(&pDeviceContext);

		CreateRenderTargetView(pSwapChain);

		impl::win32::init(desc.OutputWindow);

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(desc.OutputWindow);
		ImGui_ImplDX11_Init(device, pDeviceContext);

		init = true;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	impl::showExampleWindow("D3D11");

	ImGui::EndFrame();
	ImGui::Render();

	if (pRenderTargetView != nullptr)
		pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(pSwapChain, SyncInterval, Flags);
}

long __stdcall hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	pRenderTargetView->Release();
	long rv = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
	CreateRenderTargetView(pSwapChain);

	return rv;
}

void impl::d3d11::init()
{
	kiero::bind(8, (void**)&oPresent, hkPresent11);
	kiero::bind(13, (void**)&oResizeBuffers, hkResizeBuffers);
}

#endif // KIERO_INCLUDE_D3D11
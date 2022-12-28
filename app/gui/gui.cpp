#include "gui.h"
#include "../variables.h"
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_dx9.h"
#include "../../imgui/imgui_impl_win32.h"
#include "../systeminfo/systeminfo.hpp"
#include "../cpuinfo/getcpuinfo.h"

#include <iostream>
#include <tlhelp32.h>
#include <sysinfoapi.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wideParameter, LPARAM longParameter);

long __stdcall WindowProcess(HWND window, UINT message, WPARAM wideParameter, LPARAM longParameter) {
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message) {
	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED) {
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU)
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter);
	}return 0;

	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON) {
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 && gui::position.x <= gui::WIDTH && gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(gui::window, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
		}

	}return 0;

	}

	return DefWindowProc(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow(const char* windowName) noexcept {
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "class001";
	windowClass.hIconSm = 0;

	RegisterClassEx(&windowClass);

	window = CreateWindowEx(0, "class001", windowName, WS_POPUP, 100, 100, WIDTH, HEIGHT, 0, 0, windowClass.hInstance, 0);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void gui::DestroyHWindow() noexcept {
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept {
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParameters, &device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept {
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept {
	if (device) {
		device->Release();
		device = nullptr;
	}

	if (d3d) {
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.Fonts->AddFontFromFileTTF("assets/fonts/OpenSans-Regular.ttf", 18.2f);

	io.IniFilename = NULL;

	ImGui::StyleColorsDark();

	ImGuiStyle* style = &ImGui::GetStyle();


	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 0.0f;
	style->ChildBorderSize = 2.0f;
	style->ChildRounding = 0.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 0.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 10.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 0.0f;
	style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style->FrameBorderSize = 2.0f;
	style->WindowBorderSize = 3.0f;
	style->PopupRounding = 0.0f;
	style->PopupBorderSize = 2.0f;


	
	//1.0f, 1.0f, 1.00f, 1.0f //white
	//0.963f, 0.440f, 1.00f, 1.0f //blue
	//0.990f, 0.636f, 0.624f, 1.0f //red
	//0.746f, 0.624f, 0.990f, 1.0f //purple

	style->Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.00f, 1.0f);

	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.440f, 0.608f, 1.00f, 1.0f);

	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);

	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

	style->Colors[ImGuiCol_Border] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);

	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.1f, 0.1f, 1.00f);

	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.1f, 0.1f, 0.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);

	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);

	style->Colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);

	style->Colors[ImGuiCol_Header] = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);

	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);

	style->Colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);


	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept {
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept {
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT) {
			isRunning = !isRunning;
			return;
		}
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept {
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

void gui::Render() noexcept {
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::Begin("Aurora", &isRunning, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

	ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

	ImGui::BeginChild("Tabs", ImVec2(150, 440), true, 0);

	//ImGui::Image();

	if (ImGui::Button("Log In", ImVec2(115, 34))) {
		tab = 0;
	}

	if (ImGui::Button("General", ImVec2(115, 34))) {
		tab = 1;
	}

	if (ImGui::Button("Hardware", ImVec2(115, 34))) {
		tab = 2;
	}

	if (ImGui::Button("Console", ImVec2(115, 34))) {
		tab = 3;
	}

	if (ImGui::Button("Apps", ImVec2(115, 34))) {
		tab = 4;
	}

	if (ImGui::Button("Socials", ImVec2(115, 34))) {
		tab = 5;
	}

	for (int i = 0; i < 5; i++) {
		ImGui::NewLine();
	}

	ImGui::Text("     By szlug3ns*");

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("Content", ImVec2(505, 440), true, 0);
	
	if (tab == 0) {
		if (!loggedIn) {
			ImGui::InputText("Username", buffer1, sizeof(buffer1), 0, 0, 0);
			ImGui::InputText("Password", buffer2, sizeof(buffer2), 0, 0, 0);

			if (ImGui::Button("Log In", ImVec2(150, 34)) || GetAsyncKeyState(VK_RETURN)) {
				if (strcmp(buffer1, login) == 0 && strcmp(buffer2, pass) == 0) {
					loggedIn = true;
				}
			}
		}

		if (loggedIn) {
			if (ImGui::Button("Log Out", ImVec2(150, 34))) {
				loggedIn = false;
				memset(buffer1, 0, 255);
				memset(buffer2, 0, 255);
			}
		}
	}

	if (tab == 1) {
		if (ImGui::Button("Close The Application", ImVec2(150, 34))) {
			isRunning = !isRunning;
		}
	}

	if (tab == 2) {
		if (loggedIn) {
			Systeminfo sysinf;

			std::string mb = "Product: " + sysinf.get_productname();
			std::string info = "CPU: " + GetCpuInfo();

			ImGui::Text(mb.c_str());
			ImGui::Text(info.c_str());
		}
		else {
			ImGui::Text("Please Log In!");
		}

	}

	if (tab == 3) {
		if (loggedIn) {
			ImGui::Text("Coming Soon...");
		}
		else {
			ImGui::Text("Please Log In!");
		}
	}

	if (tab == 4) {
		if (loggedIn) {
			HWND steam = FindWindow(NULL, "Steam");

			if (steam) {
				if (ImGui::Button("Terminate Steam", ImVec2(150, 34))) {
					PROCESSENTRY32 entry;
					entry.dwSize = sizeof(PROCESSENTRY32);

					HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

					if (Process32First(snapshot, &entry) == TRUE) {
						while (Process32Next(snapshot, &entry) == TRUE) {
							if (_stricmp(entry.szExeFile, "steam.exe") == 0) {
								HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

								if (TerminateProcess(hProcess, NULL)) {
									MessageBox(window, "Steam Process Terminated!", "Terminate", MB_OK | MB_ICONINFORMATION);
								}
								else {
									MessageBox(window, "Something Went Wrong!", "Terminate", MB_OK | MB_ICONINFORMATION);
								}

								CloseHandle(hProcess);
							}
						}
					}
					CloseHandle(snapshot);
				}
			}
			else {
				if (ImGui::Button("Terminate Steam", ImVec2(150, 34))) {
					MessageBox(window, "Steam Process Not Found!", "Terminate", MB_OK | MB_ICONINFORMATION);
				}
			}
		}
		else {
			ImGui::Text("Please Log In!");
		}

	}

	if (tab == 5) {
		if (ImGui::Button("GitHub", ImVec2(110, 34))) {
			ShellExecute(0, 0, "https://github.com/szlug3ns", 0, 0, SW_SHOW);
		}

		if (ImGui::Button("Twitter", ImVec2(110, 34))) {
			ShellExecute(0, 0, "https://twitter.com/szlug3ns", 0, 0, SW_SHOW);
		}

		if (ImGui::Button("SoundCloud", ImVec2(110, 34))) {
			ShellExecute(0, 0, "https://soundcloud.com/user-92572076", 0, 0, SW_SHOW);
		}
	}

	ImGui::EndChild();

	ImGui::End();
}

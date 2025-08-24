/**
 * @file main.cpp
 * @brief High-performance Windows overlay application with FPS monitoring
 * @author edohb
 * @date 2025
 * @license MIT License
 *
 * This application creates an overlay window using ImGui.
 * If you cant read this code, please check https://learncpp.com/
 */

#include <Windows.h>
#include <d3d11.h>
#include <windowsx.h>
#include <chrono>
#include <dwmapi.h>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "ghoststr.hpp"

#pragma comment(lib, "dwmapi.lib")

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

struct OverlayState {
    bool should_exit = false;
    DWORD last_click_time = 0;
    int click_count = 0;
    HWND window_handle = nullptr;

    float overlay_fps = 0.0f;
    std::chrono::steady_clock::time_point last_frame_time;
    std::chrono::steady_clock::time_point last_fps_update;
    int frame_count = 0;

    char fps_text_cache[64] = "FPS: ...";
    bool fps_initialized = false;
} g_overlay;

HHOOK g_mouse_hook = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_LBUTTONDOWN) {
            DWORD current_time = GetTickCount();

            if (current_time - g_overlay.last_click_time > 500) {
                g_overlay.click_count = 0;
            }

            g_overlay.click_count++;
            g_overlay.last_click_time = current_time;

            if (g_overlay.click_count >= 2) {
                g_overlay.should_exit = true;
                g_overlay.click_count = 0;
                return 1;
            }
        }
    }

    return CallNextHookEx(g_mouse_hook, nCode, wParam, lParam);
}

LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param)) {
        return 0L;
    }

    switch (message) {
    case WM_KEYDOWN: {
        if (w_param == VK_ESCAPE) {
            g_overlay.should_exit = true;
            return 0;
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(window, message, w_param, l_param);
}

void UpdateOverlayFPS() {
    auto current_time = std::chrono::steady_clock::now();

    g_overlay.frame_count++;

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - g_overlay.last_fps_update);
    if (duration.count() >= 1000) {
        float time_elapsed = duration.count() / 1000.0f;
        g_overlay.overlay_fps = g_overlay.frame_count / time_elapsed;

        g_overlay.frame_count = 0;
        g_overlay.last_fps_update = current_time;
        g_overlay.fps_initialized = true;

        auto fps_format = ghostStr("FPS: %.1f");
        auto fps_view = fps_format.scoped();
        sprintf_s(g_overlay.fps_text_cache, fps_view.data(), g_overlay.overlay_fps);
    }

    if (!g_overlay.fps_initialized) {
        auto fps_loading = ghostStr("FPS: ...");
        auto fps_view = fps_loading.scoped();
        strcpy_s(g_overlay.fps_text_cache, fps_view.data());
    }
}

INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show) {
    auto start_time = std::chrono::steady_clock::now();
    g_overlay.last_frame_time = start_time;
    g_overlay.last_fps_update = start_time;

    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

    auto overlay_class = ghostStr_w(L"Overlay");
    auto overlay_title = ghostStr_w(L"Overlay");

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_procedure;
    wc.hInstance = instance;
    {
        auto class_view = overlay_class.scoped();
        wc.lpszClassName = class_view.data();
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

        RegisterClassExW(&wc);

        auto title_view = overlay_title.scoped();
        const HWND overlay = CreateWindowExW(
            WS_EX_TOPMOST,
            class_view.data(),
            title_view.data(),
            WS_POPUP,
            0, 0,
            screenWidth, screenHeight,
            nullptr, nullptr,
            wc.hInstance, nullptr
        );

        if (!overlay) {
            auto error_msg = ghostStr_w(L"Error creating window!");
            auto error_title = ghostStr_w(L"Error");
            auto msg_view = error_msg.scoped();
            auto title_view_err = error_title.scoped();
            MessageBox(nullptr, msg_view.data(), title_view_err.data(), MB_ICONERROR);
            return 1;
        }

        g_overlay.window_handle = overlay;
    }

    g_mouse_hook = SetWindowsHookEx(
        WH_MOUSE_LL,
        LowLevelMouseProc,
        GetModuleHandle(nullptr),
        0
    );

    if (!g_mouse_hook) {
        auto hook_error_msg = ghostStr_w(L"Error installing mouse hook!");
        auto error_title = ghostStr_w(L"Error");
        auto msg_view = hook_error_msg.scoped();
        auto title_view = error_title.scoped();
        MessageBox(nullptr, msg_view.data(), title_view.data(), MB_ICONERROR);
        return 1;
    }

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferDesc.Width = screenWidth;
    sd.BufferDesc.Height = screenHeight;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1U;
    sd.SampleDesc.Quality = 0U;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2U;
    sd.OutputWindow = g_overlay.window_handle;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    constexpr D3D_FEATURE_LEVEL levels[2]{
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };

    ID3D11Device* device{ nullptr };
    ID3D11DeviceContext* device_context{ nullptr };
    IDXGISwapChain* swap_chain{ nullptr };
    ID3D11RenderTargetView* render_target_view{ nullptr };
    D3D_FEATURE_LEVEL level{};

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0U,
        levels, 2U, D3D11_SDK_VERSION, &sd,
        &swap_chain, &device, &level, &device_context
    );

    if (FAILED(hr)) {
        auto dx_error_msg = ghostStr_w(L"Error initializing DirectX!");
        auto error_title = ghostStr_w(L"Error");
        auto msg_view = dx_error_msg.scoped();
        auto title_view = error_title.scoped();
        MessageBox(nullptr, msg_view.data(), title_view.data(), MB_ICONERROR);
        return 1;
    }

    ID3D11Texture2D* back_buffer{ nullptr };
    swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

    if (back_buffer) {
        device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
        back_buffer->Release();
    }
    else {
        auto buffer_error_msg = ghostStr_w(L"Back buffer error!");
        auto error_title = ghostStr_w(L"Error");
        auto msg_view = buffer_error_msg.scoped();
        auto title_view = error_title.scoped();
        MessageBox(nullptr, msg_view.data(), title_view.data(), MB_ICONERROR);
        return 1;
    }

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    ImGui_ImplWin32_Init(g_overlay.window_handle);
    ImGui_ImplDX11_Init(device, device_context);

    ShowWindow(g_overlay.window_handle, SW_SHOW);
    UpdateWindow(g_overlay.window_handle);

    auto overlay_window_id = ghostStr("##BlackOverlay");
    auto discord_text = ghostStr("discord.gg/rankuen");

    bool running = true;
    while (running && !g_overlay.should_exit) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                running = false;
            }
        }

        if (!running || g_overlay.should_exit) break;

        UpdateOverlayFPS();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)screenWidth, (float)screenHeight));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

        {
            auto window_id_view = overlay_window_id.scoped();
            ImGui::Begin(window_id_view.data(), nullptr,
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing);
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImVec2 real_fps_size = ImGui::CalcTextSize(g_overlay.fps_text_cache);
        draw_list->AddText(
            ImVec2((float)screenWidth - real_fps_size.x - 20, 15),
            IM_COL32(150, 150, 150, 255),
            g_overlay.fps_text_cache
        );

        {
            auto discord_view = discord_text.scoped();
            ImVec2 close_text_size = ImGui::CalcTextSize(discord_view.data());
            draw_list->AddText(
                ImVec2(((float)screenWidth - close_text_size.x) / 2, 20),
                IM_COL32(100, 100, 100, 200),
                discord_view.data()
            );
        }

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);

        ImGui::Render();

        float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        device_context->OMSetRenderTargets(1U, &render_target_view, nullptr);
        device_context->ClearRenderTargetView(render_target_view, clear_color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swap_chain->Present(1U, 0U);
    }

    overlay_window_id.clear();
    discord_text.clear();

    if (g_mouse_hook) {
        UnhookWindowsHookEx(g_mouse_hook);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (render_target_view) render_target_view->Release();
    if (swap_chain) swap_chain->Release();
    if (device_context) device_context->Release();
    if (device) device->Release();

    DestroyWindow(g_overlay.window_handle);

    {
        auto final_class_name = ghostStr_w(L"Overlay");
        auto class_view = final_class_name.scoped();
        UnregisterClassW(class_view.data(), wc.hInstance);
    }

    return 0;
}

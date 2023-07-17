#include "window.hpp"

#include <cstdio>
#include <SDL2/SDL.h>
#include "platform/imgui/imgui.h"
#include "platform/imgui/imgui_impl_sdl2.h"

CSDLWindow::CSDLWindow(int width, int height) : running(true) {
    // Create an SDL window that supports Vulkan rendering.
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Failed to initialize SDL!");
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if(window == nullptr) {
        printf("Failed to initialize SDL!");
    }
}

CSDLWindow::~CSDLWindow() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool CSDLWindow::IsRunning() {
    return running;
}

void CSDLWindow::AddEventCallback(void* userdata, SDL_EventFilter filter) {
    SDL_AddEventWatch(filter, userdata);
}

void CSDLWindow::PollEvents() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event); // ImGui event hooking.
        if(event.type == SDL_QUIT) {
            running = false;
            break;
        }
    }
}

SDL_Window* CSDLWindow::GetSDL_Window() {
    return window;
}

module;
#include "platform/sdl.hpp"
module window;

CSDLWindow::CSDLWindow() {
    // Create an SDL window that supports Vulkan rendering.
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Failed to initialize SDL!");
    }
    window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);
    if(window == nullptr) {
        printf("Failed to initialize SDL!");
    }
}

CSDLWindow::~CSDLWindow() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

SDL_Window* CSDLWindow::GetSDLWindow() {
    return window;
}

void CSDLWindow::PollEvents() {
    // Poll for user input.
    bool stillRunning = true;
    while(stillRunning) {

        SDL_Event event;
        while(SDL_PollEvent(&event)) {

            switch(event.type) {

                case SDL_QUIT:
                    stillRunning = false;
                    break;

                default:
                    // Do nothing.
                    break;
            }
        }

        SDL_Delay(10);
    }
}
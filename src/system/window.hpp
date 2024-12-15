#pragma once

#include <SDL2/SDL.h>

class CSDLWindow {
    SDL_Window* window;
    bool running;
public:
    CSDLWindow(int width, int height);
    ~CSDLWindow();
    bool IsRunning();
    void AddEventCallback(void* userdata, SDL_EventFilter filter);
    void PollEvents();
    SDL_Window* GetSDL_Window();
};

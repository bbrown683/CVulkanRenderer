module;
#include "platform/SDL.hpp"
export module window;
import <vector>;

export class CSDLWindow {
    SDL_Window* window;
    bool running;
public:
    CSDLWindow() = default;
    CSDLWindow(int width, int height) : running(true) {
        // Create an SDL window that supports Vulkan rendering.
        if(SDL_Init(SDL_INIT_VIDEO) != 0) {
            printf("Failed to initialize SDL!");
        }
        window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
        if(window == nullptr) {
            printf("Failed to initialize SDL!");
        }
    }

    ~CSDLWindow() {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    
    SDL_Window* GetSDL_Window() {
        return window;
    }

    bool IsRunning() {
        return running;
    }

    void AddEventCallback(void* userdata, SDL_EventFilter filter) {
        SDL_AddEventWatch(filter, userdata);
    }

    void PollEvents() {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                running = false;
                break;
            }
        }
    }
};

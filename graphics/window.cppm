module;
#include "platform/sdl.hpp"
export module window;
import <vector>;
import <map>;
import <functional>;

export class CSDLWindow {
    SDL_Window* window;
public:
    CSDLWindow() {
        // Create an SDL window that supports Vulkan rendering.
        if(SDL_Init(SDL_INIT_VIDEO) != 0) {
            printf("Failed to initialize SDL!");
        }
        window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
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

    void AddEventCallback(void* userdata, SDL_EventFilter filter) {
        SDL_AddEventWatch(filter, userdata);
    }

    void PollEvents() {
        bool stillRunning = true;
        while(stillRunning) {
            SDL_Event event;
            while(SDL_PollEvent(&event)) {
                if(event.type == SDL_QUIT) {
                    stillRunning = false;
                }
                SDL_Delay(10);
            }
        }
    }
};

module;
export module window;
import <vector>;

export struct SDL_Window;

export class CSDLWindow {
    SDL_Window* window;
public:
    CSDLWindow();
    ~CSDLWindow();
    SDL_Window* GetSDLWindow();
    void PollEvents();
};

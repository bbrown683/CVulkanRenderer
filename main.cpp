import window;
import renderer;

auto main() -> int {
    auto window = CSDLWindow(1920, 1080);
    auto renderer = CVulkanRenderer(window.GetSDL_Window());
    window.AddEventCallback(static_cast<void*>(&renderer), CVulkanRenderer::SDL_EventFilterCallback); // Add callback when certain events fire.

    // Main render loop.
    while(window.IsRunning()) {
        window.PollEvents();
        renderer.DrawFrame();
    }

    return 0;
}
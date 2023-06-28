import window;
import renderer;

auto main() -> int {
    auto window = CSDLWindow();
    auto renderer = CVulkanRenderer(window.GetSDL_Window());
    window.AddEventCallback(static_cast<void*>(&renderer), CVulkanRenderer::OnResizeSDL_EventCallback);
    window.PollEvents();
    return 0;
}
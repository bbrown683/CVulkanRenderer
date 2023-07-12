import window;
import renderer;

auto main() -> int {
    CSDLWindow window(1920, 1080);
    CVulkanRenderer renderer(window.GetSDL_Window());
    window.AddEventCallback(static_cast<void*>(&renderer), CVulkanRenderer::SDL_EventFilterCallback); // Add callback when certain events fire.

    while(window.IsRunning()) {
        window.PollEvents();
        renderer.DrawFrame();
    }

    return 0;
}


/*

    double lastTime, currentTime;
    int numFrames;
    float frameTime;


void App::calculateFrameRate() {
    currentTime = glfwGetTime();
    double delta = currentTime - lastTime;

    if (delta >= 1) {
        int framerate{ std::max(1, int(numFrames / delta)) };
        std::stringstream title;
        title << "Running at " << framerate << " fps.";
        glfwSetWindowTitle(window, title.str().c_str());
        lastTime = currentTime;
        numFrames = -1;
        frameTime = float(1000.0 / framerate);
    }

    ++numFrames;
}
*/
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace cvk {
    
class CvkWindow {
public:
    CvkWindow(int w, int h, std::string name);
    ~CvkWindow();

    /*
    Deleting copy constructor and copy operator, because we are using a pointer to the window.
    Resource Acquisition = Initialization. i.e. Resource creation happens when a variable in initialized.
    Cleanup is done by destructors.
    So we never want to copy the window, accidentally destruct it, and then have a dangling pointer.
    */
    CvkWindow(const CvkWindow &) = delete;
    CvkWindow &operator=(const CvkWindow &) = delete;

    bool shouldClose() { return glfwWindowShouldClose(window); }
    VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
    bool wasWindowResized() { return framebufferResized; }
    void resetWindowResizedFlag() { framebufferResized = false; }
    GLFWwindow * getGLFWWindow() const { return window; }

    void createWindowSurface(VkInstance instance,VkSurfaceKHR *surface);
private:
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    void initWindow();

    int width;
    int height;
    bool framebufferResized = false;

    std::string windowName;
    GLFWwindow *window;
};

} // namespace cvk
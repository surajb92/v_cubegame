#include "CvkWindow.hpp"
#include <stdexcept>

namespace cvk {

CvkWindow::CvkWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
    initWindow();
}
CvkWindow::~CvkWindow() {
    glfwDestroyWindow(window);
    glfwTerminate();
}
void CvkWindow::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    // Pairs the window with a pointer to the parent CvkWindow object
    glfwSetWindowUserPointer(window, this);
    // Call function with window,new_width and new_height params when a window resize event occurs.
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void CvkWindow::createWindowSurface(VkInstance instance,VkSurfaceKHR *surface) {
    if(glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Window Surface.");
    }
}

void CvkWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto cvkWindow = reinterpret_cast<CvkWindow*> (glfwGetWindowUserPointer(window));
    cvkWindow->framebufferResized = true;
    cvkWindow->width = width;
    cvkWindow->height = height;
}

} // namespace cvk
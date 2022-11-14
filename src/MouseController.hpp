#pragma once

#include "CvkGameObject.hpp"
#include "CvkWindow.hpp"

namespace cvk {
    class MouseController {
    public:
        void rotateObject(GLFWwindow* window, float dt, CvkGameObject& gameObject);
    private:
        glm::vec2 oldMousePosition{0};
        //glm::vec2 newMp{0};
        glm::vec3 rotation{0};
        float rotateSpeed{1.f};
        float inertia{-0.05f};
        bool mouseHold = false;
    };
} // namespace cvk
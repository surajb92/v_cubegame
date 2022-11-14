#include "MouseController.hpp"
#include <iostream>
#include <cmath>

namespace cvk {
    void MouseController::rotateObject(GLFWwindow* window, float dt, CvkGameObject& gameObject) {
        /*
        Logic
        This function will be called on every frame.
        Inputs -
            1. window (is needed to detect mouse actions)
            2. dt, delta time. Need to know old delta time as well (maybe can save it internally).
            3. object (to move) of course.
        Actions -
            1. If nothing is pressed -
                a. Check if rotation vector > 0, rotate object accordingly.
                b. Apply inertial on rotation vector. If it goes to negative, set it to zero.
            2. If right button pressed -
                a. Move based on previous rotation (for one frame)
                b. Set rotation vector = 0.
                c. Save mouse position as 'mousePos'.
                d. Calculate new rotation based on current mousePos and saved mousePos 
            Check if old Mp > 0, then proceed as if mouse is moving.
            2. If old Mp = 0, check for mouse press action. If mouse pressed, keep calculating old mouse pos and new one. 
        */
        double newMouseX;
        double newMouseY;
        bool rightSide;
        glm::vec2 objPosition(gameObject.transform.translation);
        glfwGetCursorPos(window, &newMouseX, &newMouseY);
        glm::vec2 newMousePosition = {static_cast<float>(newMouseX), static_cast<float>(newMouseY)};

        if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            // Stop rotation if it's happening
            if (glm::dot(rotation, rotation) > std::numeric_limits<float>::epsilon() && !mouseHold) {
                //gameObject.transform.rotation += rotateSpeed * dt * glm::normalize(rotation);
                // rotation = glm::vec3{0};
                // rotation = gameObject.transform.rotation;
                //std::cout<<rotation.x<<" "<<rotation.y<<" "<<rotation.z<<"\n";
            }
            int w,h;
            glfwGetWindowSize(window,&w,&h);
            if (!mouseHold) {
                oldMousePosition = newMousePosition;
                std::cout<<"PRESS ROTATION (INSTANT)\n";
                mouseHold = true;
                rightSide = newMousePosition.x > (w/2)+(w/2)*objPosition.x ? true : false;
                //std::cout<<"OBJ POSITION "<<(w/2)+(w/2)*objPosition.x<<" "<<(h/2)+(h/2)*objPosition.y<<"\n";
                //std::cout<<"MOUSE POSITION "<<newMousePosition.x<<" "<<newMousePosition.y<<"\n";
                //std::cout<<"ROTATION "<<gameObject.transform.rotation.x<<" "<<gameObject.transform.rotation.y<<" "<<gameObject.transform.rotation.z<<"\n";
                std::cout<<"SIDE : "<<rightSide<<"\n";

            }
            glm::vec2 mouseDifference = newMousePosition - oldMousePosition;

            // Set rotation about Y Axis (based on X mouse movement) & normalize the angle
            //rotation.x = mouseDifference.y*1000*dt/h;
            glm::vec3 objRot = gameObject.transform.rotation;
            gameObject.transform.rotation = glm::vec3{0.f};
            rotation.y = -mouseDifference.x*1000*dt/w;
            //rotation.x = -mouseDifference.y*cos(rotation.y)*1000*dt/h;
            //rotation.z = mouseDifference.y*cos(rotation.x)*1000*dt/h;
            //! All rotations can be done with just X and Z values initially.
            //! Y will be the starting point, but after that, ALL OF X, Y and Z will change depending upon the next X
            rotation.x = mouseDifference.y*glm::cos(gameObject.transform.rotation.y)*1000*dt/h;
            rotation.z = mouseDifference.y*glm::sin(gameObject.transform.rotation.y)*1000*dt/h;
            glm::mat3 a = glm::mat3{1.f} * glm::mat3{2.f};
            //std::cout<<;
            if (rightSide) {
            } else {
            }

            //rotation.x = (newMousePosition - oldMousePosition).y*1000*dt/h;
            //gameObject.transform.rotation.x = glm::mod(gameObject.transform.rotation.x, glm::two_pi<float>());
            
            //rotation.z = 1.f;
            if(glm::dot(mouseDifference,mouseDifference) > 0.001f) {
                gameObject.transform.rotation = objRot + rotation;
                //rotation.z += glm::dot(rotation,{0.f,0.f,1.f});
                //std::cout<<"OBJ Z ROT : "<<rotation.z<<"\n";
                std::cout<<"PRESS ROTATION (PERSISTENT)\n";
                std::cout<<rotation.x<<" "<<rotation.y<<" "<<rotation.z<<"\n";
            } else {
                gameObject.transform.rotation = objRot;
            }

            gameObject.transform.rotation.x = glm::mod(gameObject.transform.rotation.x, glm::two_pi<float>());
            gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
            gameObject.transform.rotation.z = glm::mod(gameObject.transform.rotation.z, glm::two_pi<float>());

            // gameObject.transform.rotation += rotateSpeed * dt * glm::normalize(rotation);
        } else if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
            mouseHold = false;
            //gameObject.transform.rotation += rotateSpeed * dt * glm::normalize(rotation);
            //std::cout<<"RELEASE ROTATION\n";
        }
        oldMousePosition = newMousePosition;
        /*


        if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
        if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
        if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
        if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;
        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
        }
        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
        const glm::vec3 rightDir(forwardDir.z, 0.f, -forwardDir.x);
        const glm::vec3 upDir{0.f, -1.f, 0.f};
        
        glm::vec3 moveDir{0.f};
        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;
        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
        */
    }
}
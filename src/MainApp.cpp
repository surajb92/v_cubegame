#include "MainApp.hpp"
#include "CvkCamera.hpp"
#include "SimpleRenderSystem.hpp"
#include "KeyBoardMovementController.hpp"
#include "CvkBuffer.hpp"
#include "MouseController.hpp"

// libraries
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <chrono>

const float MAX_FRAME_TIME = 10.f;

namespace cvk {

struct GlobalUbo {
    alignas(16) glm::mat4 projectionView{1.f};
    alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{-1.f, 3.f, 1.f});
    // now a lot more things can be passed to the shaders
};

MainApp::MainApp() {
    globalPool = CvkDescriptorPool::Builder(cvkDevice)
        .setMaxSets(CvkSwapchain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, CvkSwapchain::MAX_FRAMES_IN_FLIGHT)
        .build();
    // loads the game objects IMMEDIATELY after App is opened.
    loadGameObjects();
}
MainApp::~MainApp() { }

// Main Application commands -
void MainApp::run() {
    
    std::vector<std::unique_ptr<CvkBuffer>> uboBuffers(CvkSwapchain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<CvkBuffer>(
            cvkDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            // We do not use HOST_COHERENT bit here, because we only want to flush the buffer so that we don't
            // interfere with the previous frame that might still be rendering.
            // ! this is no longer the case now, but we are demoing the flush function anyways
        );
        uboBuffers[i]->map();
    }

    // TODO : Might be better to set up a Master Render system that handles the global descriptors.
    auto globalSetLayout = CvkDescriptorSetLayout::Builder(cvkDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .build();

    std::vector<VkDescriptorSet> globalDescriptorSets(CvkSwapchain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        CvkDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

    SimpleRenderSystem simpleRenderSystem(
        cvkDevice,
        cvkRenderer.getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout());
    CvkCamera camera{};
    camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));
    
    // Camera object, never rendered
    auto viewerObject = CvkGameObject::createGameObject();
    KeyBoardMovementController cameraController{};

    // OH BOY HERE WE GO
    MouseController rotationController{};

    auto currentTime = std::chrono::high_resolution_clock::now();
    // TODO : In case you want to implement a custom cursor, this hides the default and locks it to the window.
    // glfwSetInputMode(cvkWindow.getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetInputMode(cvkWindow.getGLFWWindow(),GLFW_STICKY_MOUSE_BUTTONS,GLFW_TRUE);

    while(!cvkWindow.shouldClose()) {
        glfwPollEvents();
        // Calculating time difference so that the game doesn't stutter.
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;
        frameTime = glm::min(frameTime, MAX_FRAME_TIME);

        cameraController.moveInPlaneXZ(cvkWindow.getGLFWWindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
        float aspect = cvkRenderer.getAspectRatio();\
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

        // CONTROLLING ROTATION
        rotationController.rotateObject(cvkWindow.getGLFWWindow(),frameTime,gameObjects[0]);

        if(auto commandBuffer = cvkRenderer.beginFrame()) {
            int frameIndex = cvkRenderer.getFrameIndex();
            FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex]
            };

            // update
            GlobalUbo ubo{};
            ubo.projectionView = camera.getProjection() * camera.getView();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush(); // manually flushing since not HOST_COHERENT

            // render
            cvkRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
            cvkRenderer.endSwapChainRenderPass(commandBuffer);
            cvkRenderer.endFrame();
        }
    }
}

void MainApp::loadGameObjects() {
    // ! Creation of game objects
    // std::shared_ptr<CvkModel> cvkModel = createCubeModel(cvkDevice, {.0f, .0f, .0f});
    std::shared_ptr<CvkModel> cvkModel = CvkModel::createModelFromFile(cvkDevice, "models/smallCube.obj");
    auto testCube = CvkGameObject::createGameObject();
    testCube.model = cvkModel;
    testCube.transform.translation = {-.5f, .5f, 2.5f};
    testCube.transform.scale = glm::vec3(.2f); // uniform scaling
    // testCube.transform.scale = {.3f, .5f, .3f}; // non-uniform scaling
    gameObjects.push_back(std::move(testCube));

    // cvkModel = CvkModel::createModelFromFile(cvkDevice, "models/smooth_vase.obj");
    auto testCube2 = CvkGameObject::createGameObject();
    testCube2.model = cvkModel;
    testCube2.transform.translation = {.5f, .5f, 2.5f};
    testCube2.transform.scale = glm::vec3(.2f); // uniform scaling
    // testCube2.transform.scale = {3.f, 1.5f, 3.f}; // non-uniform scaling
    gameObjects.push_back(std::move(testCube2));
}

} // namespace cvk
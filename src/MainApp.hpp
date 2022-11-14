#pragma once

#include "CvkDevice.hpp"
#include "CvkGameObject.hpp"
#include "CvkWindow.hpp"
#include "CvkRenderer.hpp"
#include "CvkDescriptors.hpp"

// std
#include <memory>

namespace cvk {

// Resource allocation is initialization, so any variable declaration will call the respective constructor.
class MainApp {
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    MainApp();
    ~MainApp();

    // Remove copy constructors if you will only have object in this class (e.g. App class, Window class etc.)
    MainApp(const MainApp &) = delete;
    MainApp &operator=(const MainApp &) = delete;

    void run();
private:
    void loadGameObjects();

    CvkWindow cvkWindow{WIDTH, HEIGHT, "My Puzzle Game"};
    CvkDevice cvkDevice{cvkWindow};
    CvkRenderer cvkRenderer{cvkWindow,cvkDevice};

    // ! Order of declaration matters here
    std::unique_ptr<CvkDescriptorPool> globalPool{}; // has to be created AFTER Device
    std::vector<CvkGameObject> gameObjects;
};

} // namespace cvk
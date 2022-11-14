#pragma once

#include "CvkModel.hpp"

// libraries
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>

namespace cvk {

struct TransformComponent {
    glm::vec3 translation{}; // {position offset}
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation{};
    glm::mat4 mat4();
    glm::mat3 normalMatrix();
};


class CvkGameObject {
public:
    using id_t = unsigned int;
    static CvkGameObject createGameObject() {
        static id_t currentId = 0;
        return CvkGameObject{currentId++};
    }

    CvkGameObject(const CvkGameObject &) = delete;
    CvkGameObject &operator=(const CvkGameObject &) = delete;
    CvkGameObject(CvkGameObject &&) = default;
    CvkGameObject &operator=(CvkGameObject&&) = default;

    const id_t getID() { return id; }

    std::shared_ptr<CvkModel> model;
    glm::vec3 color{};
    TransformComponent transform{};
private:
    id_t id;
    CvkGameObject(id_t obj_id) : id{obj_id} {}
};

} // namespace cvk
module;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
export module mesh;


export class CVulkanMesh {
    // CVulkanBuffer vertexBuffer;
    // CVulkanBuffer indexBuffer;
    // CVulkanTexture texture;
    
    //    std::vertex<CVulkanVertex> vertices;
    glm::mat4 worldTransform;

public:
    CVulkanMesh() {

    }

    void SetLocation(glm::vec3 location) {
        worldTransform = glm::translate(worldTransform, location);
    }

    void SetRotation(glm::vec3 rotation) {
        worldTransform = glm::transpose(glm::rotate(worldTransform, glm::radians(rotation.x), glm::vec3(1, 0, 0)) *
            glm::rotate(worldTransform, glm::radians(rotation.y), glm::vec3(0, 1, 0)) *
            glm::rotate(worldTransform, glm::radians(rotation.z), glm::vec3(0, 0, 1)));
    }

    void SetScale(glm::vec3 scale) {
        worldTransform = glm::scale(worldTransform, scale);
    }
};
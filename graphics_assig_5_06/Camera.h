#include <glm/glm.hpp>
using namespace glm;

class Camera{
public:
    vec3 dir, right, up, pos;
    float theta = -1.f, phi = 1.f;
    float radius = 10.f;

	Camera():dir(glm::vec3(0, 0, -1)), right(glm::vec3(1, 0, 0)), up(glm::vec3(0, 1, 0)), pos(glm::vec3(0)){}
	Camera(glm::vec3 dir, glm::vec3 right, glm::vec3 up, glm::vec3 pos):dir(dir), right(right), up(up), pos(pos){}

	glm::mat4 viewMatrix() const;

	void rotateVertical(float radians);
	void rotateHorizontal(float radians);
	void zoom(glm::vec3 movement);
    void updateCamera();
    vec3 getPosition();
};

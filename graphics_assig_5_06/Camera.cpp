#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>

using namespace std;
using namespace glm;

glm::mat4 Camera::viewMatrix() const
{
    return lookAt(pos, vec3(0,0,0) + dir, up);
}

void Camera::rotateVertical(float radians)
{
    phi += radians;
    updateCamera();
}

void Camera::rotateHorizontal(float radians)
{
    theta += radians;
    updateCamera();
}

void Camera::zoom(vec3 movement)
{
    if (radius <= 2 && movement.z > 0) {
        radius == 2.f;
    } else if (radius >= 10 && movement.z < 0) {
        radius == 10.f;
    } else {
        radius -= movement.z;
    }
    
	pos += movement.z*dir;
    updateCamera();
}

void Camera :: updateCamera()
{
    vec3 front;
    front.x = radius * cos(phi) * sin(theta);
    front.y = radius * cos(theta);
    front.z = radius * sin(phi) * sin(theta);

    pos = front;
    dir = -front;
}

vec3 Camera :: getPosition()
{
    return pos;
}

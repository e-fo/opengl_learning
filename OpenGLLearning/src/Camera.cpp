#include "Camera.hpp"

Camera::Camera() {
	// Assume we are looking out into the world
	// NOTE: This is along '-Z', because otherwise, we'd be looking behind us.
	mEye = glm::vec3(0.0f, 0.0f, 0.0f);
	mViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	//Assume we start on a prefect plane.
	mUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 Camera::GetViewMatrix() const {
	return glm::lookAt(mEye, mViewDirection, mUpVector);
}

void Camera::MoveForward(float speed) {
	// Simple but, not yet correct!
	// need to fix in the next video.
	mEye.z -= speed;
}
void Camera::MoveBackward(float speed) {
	mEye.z += speed;
}
void Camera::MoveLeft(float speed) {

}
void Camera::MoveRight(float speed) {

}
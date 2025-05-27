#include "Camera.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include <iostream>

Camera::Camera() {
	// Assume we are looking out into the world
	// NOTE: This is along '-Z', because otherwise, we'd be looking behind us.
	mEye = glm::vec3(0.0f, 0.0f, 0.0f);
	mViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	//Assume we start on a prefect plane.
	mUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 Camera::GetViewMatrix() const {
	return glm::lookAt(mEye, mEye+mViewDirection, mUpVector);
}

void Camera::MouseLook(int mouseX, int mouseY) {
	std::cout << "mousePos: " << mouseX << "," << mouseY << std::endl;

	glm::vec2 currentMouse = glm::vec2(mouseX, mouseY);

	static bool firstLook = true;
	if (firstLook) {
		mOldMousePosition = currentMouse;
		firstLook = false;
	}

	glm::vec2 mouseDelta = mOldMousePosition - currentMouse;
	mViewDirection = glm::rotate(mViewDirection, glm::radians(mouseDelta.x), mUpVector);

	mOldMousePosition = currentMouse;
}
void Camera::MoveForward(float speed) {
	mEye += (mViewDirection*speed);
}
void Camera::MoveBackward(float speed) {
	mEye -= (mViewDirection * speed);
}
void Camera::MoveLeft(float speed) {
	glm::vec3 rightVector = glm::cross(mViewDirection, mUpVector);
	mEye -= rightVector * speed;
}
void Camera::MoveRight(float speed) {
	glm::vec3 rightVector = glm::cross(mViewDirection, mUpVector);
	mEye += rightVector * speed;
}
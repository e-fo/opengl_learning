#pragma once
#include "glm/glm.hpp"

class Camera {
public:
	Camera();
	/// <summary>
	/// The ultimate view matrix we will produce and return.
	/// </summary>
	glm::mat4 GetViewMatrix() const;

	void SetProjectionMatrix(float fovy, float aspect, float near, float far);
	glm::mat4 GetProjectionMatrix() const;

	void MouseLook(int mouseX, int mouseY);
	void MoveForward(float speed);
	void MoveBackward(float speed);
	void MoveLeft(float speed);
	void MoveRight(float speed);

private:
	glm::mat4 mProjectionMatrix;
	glm::vec3 mEye;
	glm::vec3 mViewDirection;
	glm::vec3 mUpVector;
	glm::vec2 mOldMousePosition;
};
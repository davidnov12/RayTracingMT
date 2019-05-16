/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* FPSCamera.cpp
*/

#include <FPSCamera.h>

#include <iostream>

FPSCamera::FPSCamera() {}


FPSCamera::~FPSCamera() {}


std::vector<glm::vec3> FPSCamera::getScreenCoords() {

	std::vector<glm::vec3> scrCoords;

	// Left down corner
	glm::vec3 tmp = (position - right - up) + (2.0f * front);
	scrCoords.push_back(glm::vec3(tmp.x, tmp.y, tmp.z));

	// Left top corner
	tmp = (position - right + up) + (2.0f * front);
	scrCoords.push_back(glm::vec3(tmp.x, tmp.y, tmp.z));

	// Right down corner
	tmp = (position + right - up) + (2.0f * front);
	scrCoords.push_back(glm::vec3(tmp.x, tmp.y, tmp.z));

	// Right top corner
	tmp = (position + right + up) + (2.0f * front);
	scrCoords.push_back(glm::vec3(tmp.x, tmp.y, tmp.z));

	return scrCoords;
}


void FPSCamera::keyboardEvent(cameraMove dir, float delta) {

	float distance = SPEED * delta;

	// Compute new camera position
	if (dir == FORWARD)
		position += (front * distance);

	else if (dir == BACKWARD)
		position -= (front * distance);

	else if (dir == LEFT)
		position -= (right * distance);

	else if (dir == RIGHT)
		position += (right * distance);

}


void FPSCamera::mouseEvent(float x, float y) {

	yaw += (SENS * x);
	pitch += (SENS * y);

	if (pitch > 89.f)
		pitch = 89.f;

	if (pitch < -89.f)
		pitch = -89.f;

	// Compute new values of camera vector
	updateVectors();
}



void FPSCamera::scrollEvent(float y) {

	zoom -= y;

	if (zoom < 1.0f)
		zoom = 1.0f;

	if (zoom > 45.0f)
		zoom = 45.0f;
}
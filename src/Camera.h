/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* Camera.h
*/

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define C_FORWARD front
#define C_BACKWARD -front
#define C_LEFT -right
#define C_RIGHT right

/**
* @brief General camera class
*/
class Camera {

public:

	/**
	* @brief enumeration of possible camera moves
	*/
	typedef enum {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	} cameraMove;


	/**
	* @brief Comstructor, sets default values of euler angles and position
	*/
	Camera() {
		this->position = glm::vec3(0.0f, 1.0f, 0.5f);
		this->worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		this->yaw = 0.0f;
		this->pitch = 0.0f;
		this->pitchConstrain = 89.0f;

		updateVectors();
	}


	/**
	* @brief Destructor
	*/
	~Camera() {}


	/**
	* @brief Setter for camera attributes
	* @param position New position of camera
	* @param up New up vector of camera
	* @param yaw New yaw angle value
	* @param pitch New pitch angle value
	*/
	void setValues(glm::vec3 position, glm::vec3 up, float yaw, float pitch) {
		this->position = position;
		this->worldUp = up;
		this->yaw = yaw;
		this->pitch = pitch;
	}


	/**
	* @brief Getter for camera position
	* @return Position of camera
	*/
	glm::vec3 getPosition() {
		return position;
	}


	/**
	* @brief Computes view matrix from euler angles
	* @return View matrix of current camera settings
	*/
	glm::mat4 getViewMatrix() {
		return glm::lookAt(position, position + front, up);
	}


	/**
	* @brief Computes projection matrix
	* @param width Width of screen
	* @param height Height of screen
	* @return Computed projection matrix
	*/
	glm::mat4 getProjectionMatrix(int width, int height) {
		return glm::perspective(glm::radians(zoom), width / (float)height, 0.1f, 100.0f);
	}


	/**
	* @brief Sets constrain value for pitch angle
	* @param constrain Maximum value of pitch angle
	*/
	virtual void setPitchConstrain(float constrain) {
		pitchConstrain = constrain;
	}


	/**
	* @brief Implementation of keyboard event (virtual implementation)
	* @param dir Current move of camera
	* @param delta Count of miliseconds between events
	*/
	virtual void keyboardEvent(cameraMove dir, float delta) {}


	/**
	* @brief Implementation of mouse cursor event (virtual implementation)
	* @param x x-coordination of mouse cursor
	* @param y y-coordination of mouse cursor
	*/
	virtual void mouseEvent(float x, float y) {}


	/**
	* @brief Implementation of mouse scroll event (virtual implementation)
	* @param y y-mouse coordination
	*/
	virtual void scrollEvent(float y) {}


//protected:

	const float ZOOM = 45.0f;
	const float SPEED = 0.5f;//2.0f;
	const float SENS = 0.2f;//0.4f;

	glm::vec3 position, worldUp, right, front, up;
	float pitch, yaw, pitchConstrain;
	float zoom = ZOOM;


	/**
	* @brief Computes new values of euler angles
	*/
	void updateVectors() {
		glm::vec3 tmp;
		tmp.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		tmp.y = sin(glm::radians(pitch));
		tmp.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		front = glm::normalize(tmp);
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
	}

};
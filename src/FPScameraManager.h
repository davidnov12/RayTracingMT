/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* FPSCameraManager.h
*/


#pragma once

#include <Camera.h>
#include <FPSCamera.h>

#include <GLFW/glfw3.h>

/**
* @brief Wrapper of first person camera
*/
class FPSCameraManager {

public:

	/**
	* @brief Constructor - empty
	*/
	FPSCameraManager(){
	}

	// Attributes - actual camera + helper variables
	FPSCamera c;
	
	bool first = true;
	double prevX, prevY;
	bool pressed = false;

	float delta = 0.0f, last = 0.0f;

	/**
	* @brief Upadtes values of timer variables
	* @param currentTime Current time (timestamp)
	*/
	void values_update(float currentTime);

	/**
	* @brief Implementation of mouse cursor action
	* @param window Pointer to window of current context
	*/
	void mouse_callback(GLFWwindow* window);

	/**
	* @brief Implementation of keyboard action
	* @param window Pointer to window of current context
	*/
	void key_callback(GLFWwindow* window);

	/**
	* @brief Implementation of whole camera move
	* @param window Pointer to window of current context
	* @param currentWindow Current time (timestamp)
	*/
	void camera_move(GLFWwindow* window, float currentTime);
};
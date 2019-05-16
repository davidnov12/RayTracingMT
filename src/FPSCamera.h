/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* FPSCamera.h
*/

#pragma once

#include <Camera.h>

#include <vector>

/**
* @brief Camera implementation, First person view camera
*/
class FPSCamera : public Camera {

public:

	/**
	* @brief Constructor - empty
	*/
	FPSCamera();


	/**
	* @brief Destructor
	*/
	~FPSCamera();


	/**
	* @brief Computes screen plane coordinations
	* @return Vector of screen planes coordinations
	*/
	std::vector<glm::vec3> getScreenCoords();


	/**
	* @brief Implementation of keyboard event
	* @param dir Current move of camera
	* @param delta Count of miliseconds between events
	*/
	virtual void keyboardEvent(cameraMove dir, float delta) override;


	/**
	* @brief Implementation of mouse cursor event
	* @param x x-coordination of mouse cursor
	* @param y y-coordination of mouse cursor
	*/
	virtual void mouseEvent(float x, float y) override;



	/**
	* @brief Implementation of mouse scroll event (virtual implementation)
	* @param y y-mouse coordination
	*/
	virtual void scrollEvent(float y) override;

};
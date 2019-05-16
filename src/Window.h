/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* Window.h
*/

#pragma once

#include <iostream>
#include <string>
#include <memory>

#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>

#include <GLFW/glfw3.h>

/**
* @brief Window class (based on GLFW3 library)
*/
class Window {

public:

	/**
	* @brief constructor of the class
	* @param width width of window (in pixels)
	* @param height height of window (in pixels)
	* @param title title of window
	*/
	Window(int width, int height, std::string title);


	/**
	* @brief destructor of the class
	*/
	~Window();


	/**
	* @brief gets current window close state
	* @return true if window gets close, else false
	*/
	bool isClosed();


	/**
	* @brief swaps framebuffers
	*/
	void swapBuffers();


	/**
	* @brief function to get window object
	* @return reference to window object
	*/
	GLFWwindow* getWindow();


	/**
	* @brief function to get window width
	* @return window width in pixels
	*/
	int getWidth();


	/**
	* @brief function to get window height
	* @return window height in pixels
	*/
	int getHeight();


	/**
	* @brief make this context current
	*/
	void makeContextCurrent();


	/**
	* @brief sets window properties to default values
	*/
	void setDefaults();


	/**
	* @brief creates graphics context and window
	* @return true if success, else false
	*/
	bool createWindow();

	/**
	* @brief Checks if window was resized
	* @return true, if window was resized
	*/
	bool isResized();

	/**
	* @brief Shows window with given parameters
	*/
	void showWindow();

	/**
	* @brief Checks, if viewport was resized and in case, if yes, it marks this situation
	*/
	void checkViewportResize();


private:

	/**
	* @brief Destructor structure for window
	*/
	struct DestroyWin {

		void operator()(GLFWwindow* ptr) {
			glfwDestroyWindow(ptr);
		}

	};


	// Attributes of window
	int width, height;
	bool viewportResize = false;
	std::string title;
	std::unique_ptr<GLFWwindow, DestroyWin> window;
	
};
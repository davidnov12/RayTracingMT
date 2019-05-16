/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* Window.cpp
*/

#include <Window.h>

Window::Window(int width, int height, std::string title) {
	this->width = width;
	this->height = height;
	this->title = title;
}


Window::~Window() {
	//destroyGUI();
	window.reset();
	glfwDestroyWindow(window.get());
	glfwTerminate();
	//window.reset();
}


bool Window::isClosed() {

	// Is window closed?
	if (glfwWindowShouldClose(window.get()))
		return true;

	glfwPollEvents();
	checkViewportResize();
	return false;
}


void Window::swapBuffers() {
	glfwSwapBuffers(window.get());
}


GLFWwindow* Window::getWindow() {
	return window.get();
}


int Window::getWidth() {
	return width;
}


int Window::getHeight() {
	return height;
}


void Window::makeContextCurrent() {
	glfwMakeContextCurrent(window.get());
}


void Window::setDefaults() {

	// Setup default values for window
	glfwWindowHint(GLFW_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 16);
}


bool Window::createWindow() {

	// Initialization
	if (!glfwInit())
		return false;

	window.reset(glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr));

	if (window.get() == nullptr)
		return false;

	glfwMakeContextCurrent(window.get());

	ge::gl::init();
	// Initialization

	// Set viewport
	int w, h;
	glfwGetFramebufferSize(window.get(), &w, &h);
	ge::gl::glViewport(0, 0, w, h);

	return true;
}

bool Window::isResized(){

	if (!viewportResize)
		return false;

	viewportResize = false;
	return true;
}


void Window::showWindow() {
	setDefaults();
	if (!createWindow()) throw std::exception("Cannot create window\n");
}

void Window::checkViewportResize(){

	int w, h;

	glfwGetFramebufferSize(window.get(), &w, &h);

	// Window was resized -> mark resize flag and change viewport
	if(w != width || h != height){
		viewportResize = true;
		width = w;
		height = h;
		ge::gl::glViewport(0, 0, width, height);
	}
}

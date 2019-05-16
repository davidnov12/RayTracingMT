/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* RayTracing.h
*/

#pragma once

#include <Scene.h>
#include <Window.h>
#include <Renderer.h>
#include <FPScameraManager.h>
#include <bvhPreprocessor.h>

#include <iostream>
#include <fstream>
#include <thread>

#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>

#include <geCore/Text.h>

#ifndef COMPUTE_SHADER_PATH
#define COMPUTE_SHADER_PATH "../shaders/trace.cs"
#endif

#ifndef VERTEX_SHADER_PATH
#define VERTEX_SHADER_PATH "../shaders/display.vs"
#endif

#ifndef FRAGMENT_SHADER_PATH
#define FRAGMENT_SHADER_PATH "../shaders/display.fs"
#endif

/**
* @brief Ray Tracing renderer
*/
class RayTracing : public Renderer {

public:

	/**
	* @brief Constructor
	*/
	RayTracing(){}

	/**
	* @brief Destructor (deletes used resources)
	*/
	~RayTracing(){
		ge::gl::glDeleteTextures(1, &renderTexture);
	}

	/**
	* @brief Initialization of class attributes
	*/
	void init() override;
	
	/**
	* @brief Set window object for renderer (window of current context)
	* @param w Pointer to window
	*/
	void setWindowObject(std::shared_ptr<Window> w) override;
	
	/**
	* @brief Render one screen (by use of this technique)
	*/
	void render() override;
	
	/**
	* @brief Setup data object with user interface variables
	* @param data User interface data object
	*/
	void setupUIData(std::shared_ptr<UserInterface::uiData> data) override;
	
	/**
	* @brief Update new scene into renderer
	* @param s Refernce to new (loaded) scene
	*/
	void updateScene(Scene& s) override;
	
	/**
	* @brief Setup CPU BVH acceleration structure to renderer
	* @param rootNode Pointer to root node of BVH
	*/
	void setupCPUBVH(std::shared_ptr<ge::sg::BVH<ge::sg::AABB_SAH_BVH>> rootNode) override;
	
	/**
	* @brief Setup GPU BVH acceleration structure to renderer
	* @param bvh Pointer to GPU BVH structure
	*/
	void setupGPUBVH(std::shared_ptr<ge::sg::BVH<ge::sg::RadixTree_BVH>> bvh) override;
	
private:

	/**
	* @brief Setup new position of light
	*/
	void lightPosEvent();

	bool drawFrame = true;
	
	// Renderer's attributes (window, camera, ui, programs)
	std::shared_ptr<Window> win;
	std::shared_ptr<ge::gl::Program> display;
	std::shared_ptr<ge::gl::Program> tracer;
	std::shared_ptr<FPSCameraManager> camera;
	std::shared_ptr<UserInterface::uiData> guiData;

	// Ray tracer buffers - geometry, materials, BVH nodes
	std::shared_ptr<ge::gl::Buffer> geomBuff;
	std::shared_ptr<ge::gl::Buffer> matBuff;
	std::shared_ptr<ge::gl::Buffer> renderBuff;
	std::shared_ptr<ge::gl::Buffer> nodeBuff;
	std::shared_ptr<ge::gl::Buffer> indBuff;

	// Image object for screen rendering
	GLuint renderTexture;
	
	// GL query for render duration + helper variables
	GLuint query;
	GLuint64 elapsed_time;
	int done = 0;
	glm::vec3 lightPos = glm::vec3(4.0f, 7.0f, 1.0f);

};
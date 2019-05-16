/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* Renderer.h
*/

#pragma once

#include <memory>

#include <Window.h>
#include <Scene.h>
#include <UserInterface.h>

#include <geSG/AABB.h>
#include <BVH.h>
#include <AABB_SAH_BVH.h>
#include <RadixTree_BVH.h>
#include <BVH_Node.h>

/**
* @brief General interface for renderer
*/
class Renderer {

public:

	/**
	* @brief Initialization of class attributes
	*/
	virtual void init(){}

	/**
	* @brief Render one screen (by use of this technique)
	*/
	virtual void render(){}

	/**
	* @brief Setup data object with user interface variables
	* @param data User interface data object
	*/
	virtual void setupUIData(std::shared_ptr<UserInterface::uiData> data){}
	
	/**
	* @brief Update new scene into renderer
	* @param s Refernce to new (loaded) scene
	*/
	virtual void updateScene(Scene& s){}
	
	/**
	* @brief Set window object for renderer (window of current context)
	* @param w Pointer to window
	*/
	virtual void setWindowObject(std::shared_ptr<Window> win){}
	
	/**
	* @brief Setup CPU BVH acceleration structure to renderer
	* @param rootNode Pointer to root node of BVH
	*/
	virtual void setupCPUBVH(std::shared_ptr<ge::sg::BVH<ge::sg::AABB_SAH_BVH>> rootNode){}
	
	/**
	* @brief Setup GPU BVH acceleration structure to renderer
	* @param bvh Pointer to GPU BVH structure
	*/
	virtual void setupGPUBVH(std::shared_ptr<ge::sg::BVH<ge::sg::RadixTree_BVH>> bvh){}

};
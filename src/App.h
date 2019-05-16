/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* App.h
*/

#pragma once

#include <memory>
#include <string>
#include <iostream>

#include <Window.h>
#include <UserInterface.h>
#include <Renderer.h>

#include <BVH.h>
#include <geSG/AABB.h>
#include <AABB_SAH_BVH.h>
#include <RadixTree_BVH.h>

#define APP_DEFAULT_WIDTH 1200
#define APP_DEFAULT_HEIGHT 800
#define APP_DEFAULT_TITLE "windowTitle"

/**
* @brief Base app class, maintains application attributes
*/
template <typename RenderTech> class App {

public:

	/**
	* @brief Constructor of class, sets all necessary attributes
	* @param width Width of app window (in pixels)
	* @param height Height of app window (in pixels)
	* @param title Title string of window
	*/
	App(unsigned width = APP_DEFAULT_WIDTH, unsigned height = APP_DEFAULT_HEIGHT, std::string title = APP_DEFAULT_TITLE);

	/**
	* @brief initialization of application objects and attributes
	* @param width Width of app window (in pixels)
	* @param height Height of app window (in pixels)
	* @param title Title string of window
	*/
	void init(unsigned width, unsigned height, std::string title);
	
	/**
	* @brief runs actual application
	*/
	void run();

	// Application attributes
	std::shared_ptr<Window> win;
	std::shared_ptr<UserInterface> ui;
	std::shared_ptr<Renderer> ren;
	std::shared_ptr<UserInterface::uiData> ui_data;
	std::shared_ptr<Scene> scene;
	
	// Acceleration structures
	std::shared_ptr<ge::sg::BVH<ge::sg::AABB_SAH_BVH>> sah_bvh;
	std::shared_ptr<ge::sg::BVH<ge::sg::RadixTree_BVH>> gpu_bvh;
	std::shared_ptr<float> tmp;

	bool initScene = false;
	
};

template<typename RenderTech>
inline App<RenderTech>::App(unsigned width, unsigned height, std::string title){

	init(width, height, title);

}

template<typename RenderTech>
inline void App<RenderTech>::init(unsigned width, unsigned height, std::string title){

	win = std::make_shared<Window>(width, height, title);
	win->showWindow();

	ui_data = std::make_shared<UserInterface::uiData>();
	ui = std::make_shared<UserInterface>(win, ui_data);
	ui->initGUI();

	ren = std::make_shared<RenderTech>();
	ren->setWindowObject(win);
	ren->setupUIData(ui_data);
	ren->init();

	scene = std::make_shared<Scene>();

	sah_bvh = std::make_shared<ge::sg::BVH<ge::sg::AABB_SAH_BVH>>();
	gpu_bvh = std::make_shared<ge::sg::BVH<ge::sg::RadixTree_BVH>>();

	
	tmp = std::make_shared<float>();

}

template<typename RenderTech>
inline void App<RenderTech>::run(){

	while (!win->isClosed()) {
		
		// Load new scene
		if (ui_data->changeNotify) {
			initScene = true;
			ui_data->changeNotify = false;

			scene->loadScene(ui_data->sceneFile, ui_data->bvhType);
			
			// CPU BVH usage
			if (!ui_data->bvhType) {

				sah_bvh->setGeometryData(*((scene->getSceneMesh()).get()));
				sah_bvh->setDepth(35);
				//sah_bvh->setMinimumPrimitivesInNode(25);
				sah_bvh->setMinimumPrimitivesInNode(25);
				sah_bvh->buildBVH();
				
				ren->setupCPUBVH(sah_bvh);
					
				scene->prepareScene();
			}
			
			// GPU BVH usage
			else {
				
				tmp.reset(scene->getPositionsVector().data());

				gpu_bvh->setGeometryData(tmp, scene->getPositionsVector().size());
				gpu_bvh->buildBVH();
				
				ren->setupGPUBVH(gpu_bvh);

			}

			ren->updateScene(*scene.get());
				
		}

		// Render new screen
		if(initScene) ren->render();
		ui->renderGUI();
		win->swapBuffers();
	}

}

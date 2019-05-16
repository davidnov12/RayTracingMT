/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* UserInterface.h
*/

#pragma once


#include <Window.h>
#include <numeric>

/**
* @brief Class for User Interface control
*/
class UserInterface {

public:

	/**
	* @brief Structure for UI data
	*/
	typedef struct {
		int shadowSamples = 1;
		int indirectSamples = 0;
		int dofSamples = 1;
		int aoSamples = 0;
		std::string sceneFile = "";
		int bvhType;
		bool renderMode = true;
		bool changeNotify = false;
		std::vector<float> renderTimes;
	} uiData;

	/**
	* @brief Constructor of the class
	* @param w Attached window object
	* @param d UI data structure
	*/
	UserInterface(std::shared_ptr<Window> w, std::shared_ptr<uiData> d) {
		window = w;
		data = d;
	}

	/**
	* @brief Class destructor
	*/
	~UserInterface();

	/**
	* @brief Initialization of GUI (context, important variables)
	*/
	void initGUI();

	/**
	* @brief Render actual GUI
	*/
	void renderGUI();

	/**
	* @brief take screenshot of the application
	*/
	void takeScreen();

	// Informations about context
	std::shared_ptr<Window> window;
	std::shared_ptr<uiData> data;

	// Helper control variables
	bool showGUI = true;
	bool showFileBrowser = false;
	bool showProfiler = false;
	bool showbvhChoose = false;
	bool showHelp = false;

};
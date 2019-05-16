/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* Main.cpp
*/

#pragma once

#include <iostream>

#include <App.h>
#include <RayTracing.h>

#define APP_SUCCESS 0
#define APP_FAIL 1

int main(int, char**) {

	try{
		App<RayTracing> a(1200, 800, "RayTracing");
		a.run();
	}
	catch (std::exception e) {
		return APP_FAIL;
	}

	return APP_SUCCESS;
}
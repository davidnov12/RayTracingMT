/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* UserInterface.cpp
*/

#include <UserInterface.h>

#include <3rd_party/imgui/imgui.h>
#include <3rd_party/imgui/imgui_impl_glfw.h>
#include <3rd_party/imgui/imgui_impl_opengl3.h>
#include <3rd_party/imgui/ImGuiFileDialog.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <3rd_party/image/stb_image_write.h>

#include <iostream>
#include <string>

#ifndef FONT_FILE_DEST
#define FONT_FILE_DEST "DroidSans.ttf"
#endif



UserInterface::~UserInterface() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void UserInterface::initGUI() {

	// ImGui initialization
	IMGUI_CHECKVERSION();
	
	if(ImGui::CreateContext() == nullptr)
		throw std::exception("Cannot create GUI context\n");
	
	
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	if(&io == nullptr)
		throw std::exception("Cannot create GUI\n");

	io.Fonts->AddFontFromFileTTF(/*"DroidSans.ttf"*/FONT_FILE_DEST, 12.0f);

	if(!ImGui_ImplGlfw_InitForOpenGL(window->getWindow(), true))
		throw std::exception("Cannot create GUI\n");

	if(!ImGui_ImplOpenGL3_Init())
		throw std::exception("Cannot create GUI\n");

	// ImGui initialization

	//ImGui::StyleColorsLight();
	ImGui::StyleColorsDark();

	data->renderTimes = std::vector<float>(100);

}

void UserInterface::renderGUI() {

	if (!showGUI)
		return;

	std::string filePathName;

	// GUI elements draw
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Stochastic Ray Tracing");
	ImGui::NewLine();

	// Sliders -------
	ImGui::Text("Soft shadow samples");
	ImGui::SliderInt("S", &(data->shadowSamples), 0, 100);
	ImGui::NewLine();
	
	ImGui::Text("Indirect illumination samples");
	ImGui::SliderInt("I", &(data->indirectSamples), 0, 100);
	ImGui::NewLine();

	ImGui::Text("Ambient occlusion samples");
	ImGui::SliderInt("A", &(data->aoSamples), 0, 100);
	ImGui::NewLine();
	// Sliders -------

	// Render mode -------
	ImGui::Text("Rendering mode");
	
	if (ImGui::RadioButton("Standard", data->renderMode))
		data->renderMode = true;

	if (ImGui::RadioButton("Heatmap", !data->renderMode))
		data->renderMode = false;
	// Render mode -------

	// Control buttons -------
	ImGui::NewLine();

	if (ImGui::Button("Load scene..."))
		showFileBrowser = true;
	
	ImGui::SameLine();
	if (ImGui::Button("Show profile"))
		showProfiler = true;
	
	if (ImGui::Button("Take picture"))
		takeScreen();
	
	ImGui::SameLine();
	if (ImGui::Button("Show help"))
		showHelp = true;
	// Control buttons -------


	// FileBrowser Dialog -------
	if (showFileBrowser && ImGuiFileDialog::Instance()->FileDialog("Choose Scene File", ".obj\0.stl\0.ply\0.dae\0\0", ".", "")) {
		
		if (ImGuiFileDialog::Instance()->IsOk == true){
			
			data->sceneFile = ImGuiFileDialog::Instance()->GetFilepathName();
			filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
			showbvhChoose = true;
		}
		showFileBrowser = false;
	}
	// FileBrowser Dialog -------


	// BVH choose Dialog -------
	if (showbvhChoose) {
		ImGui::Begin("Choose BVH type");

		ImGui::Text("Type of BVH");

		if (ImGui::RadioButton("CPU implementation", !data->bvhType)) {
			data->bvhType = 0;
		}
		if (ImGui::RadioButton("GPU implementation", data->bvhType)) {
			data->bvhType = 1;
		}
		ImGui::NewLine();
		if (ImGui::Button("Confirm")) {
			data->changeNotify = true;
			showbvhChoose = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			showbvhChoose = false;
		}

		ImGui::End();
	}
	// BVH choose Dialog -------

	ImGui::NewLine();
	ImGui::End();
	// GUI elements draw
	
	

	// Profiler -------
	if (showProfiler) {
		
		ImGui::Begin("Profiler");
		ImGui::PlotLines("Rendering times [ms]", data->renderTimes.data(), data->renderTimes.size(), 0, "", 0.0f, 1500.0f, ImVec2(250, 90));
		ImGui::Text("Average %.2f ms", std::accumulate(data->renderTimes.begin(), data->renderTimes.end(), 0.0) / data->renderTimes.size());
		ImGui::Text("Current %.2f ms", data->renderTimes[99]);
		ImGui::NewLine();
		
		if (ImGui::Button("Close"))
			showProfiler = false;

		ImGui::End();
	}
	// Profiler -------

	// Help menu -------
	if (showHelp) {

		ImGui::Begin("Help");

		ImGui::Text("W, A. S, D - move");
		ImGui::Text("I, J, K, L, N, M - light move (x, y, z axis)");
		ImGui::Text("Mouse (Drag & Drop) - Camera looking, app controls");

		if (ImGui::Button("Close"))
			showHelp = false;

		ImGui::End();

	}
	// Help menu -------

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void UserInterface::takeScreen(){

	static int order = 0;

	// Read pixels
	std::vector<char> pixels(window->getWidth() * window->getHeight() * 4);
	ge::gl::glReadPixels(0, 0, window->getWidth(), window->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	// Write into the file
	stbi_flip_vertically_on_write(true);
	std::string file = "screen" + std::to_string(order) + ".png";
	stbi_write_png(file.c_str(), window->getWidth(), window->getHeight(), 4, pixels.data(), 0);

	std::cout << "Picture saved" << std::endl;

	order++;
}

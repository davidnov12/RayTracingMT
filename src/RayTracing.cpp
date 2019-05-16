/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* RayTracing.cpp
*/

#include <RayTracing.h>

void RayTracing::init(){

	camera = std::make_shared<FPSCameraManager>();

	// Compute shader - Ray Tracing
	auto cs = std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, ge::core::loadTextFile(COMPUTE_SHADER_PATH));
	tracer = std::make_shared<ge::gl::Program>(cs);
	// Compute shader - Ray Tracing


	// Vertex shader - display
	auto vs = std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER, ge::core::loadTextFile(VERTEX_SHADER_PATH));
	// Vertex shader - display


	// Fragment shader - display
	auto fs = std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER, ge::core::loadTextFile(FRAGMENT_SHADER_PATH));
	display = std::make_shared<ge::gl::Program>(vs, fs);
	// Fragment shader - display


	// SSBOs
	geomBuff = std::make_shared<ge::gl::Buffer>(sizeof(Scene::gpu_triangle));
	matBuff = std::make_shared<ge::gl::Buffer>(sizeof(Scene::gpu_material));
	nodeBuff = std::make_shared<ge::gl::Buffer>(2 * sizeof(bvhPreprocessor::gpuNode));
	indBuff = std::make_shared<ge::gl::Buffer>(sizeof(unsigned));
	// SSBOs

	// Rendering texture
	ge::gl::glGenTextures(1, &renderTexture);
	ge::gl::glActiveTexture(GL_TEXTURE0);
	ge::gl::glBindTexture(GL_TEXTURE_2D, renderTexture);
	ge::gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	ge::gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	ge::gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	ge::gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	ge::gl::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, win->getWidth(), win->getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
	ge::gl::glBindImageTexture(0, renderTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	// Rendering texture

	ge::gl::glGenQueries(1, &query);

}

void RayTracing::setWindowObject(std::shared_ptr<Window> w){
	win = w;
}

void RayTracing::render(){
#define DT_DGB float
	std::vector<DT_DGB> dbg_data(400);
	std::shared_ptr<ge::gl::Buffer> dbg = std::make_shared<ge::gl::Buffer>(sizeof(DT_DGB) * 400);

	if (drawFrame) {

		// Move of camera (recompute vectors values)
		camera->camera_move(win->getWindow(), static_cast<float>(glfwGetTime()));

		lightPosEvent();

		// Window resize
		if (win->isResized()) {
			ge::gl::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, win->getWidth(), win->getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
			printf("resize window to %d %d\n", win->getWidth(), win->getHeight());
		}
		
		ge::gl::glClearColor(0.1f, 0.9f, 0.4f, 1.0f);
		ge::gl::glClear(GL_COLOR_BUFFER_BIT);

		// ----- Ray trace -----
		tracer->use();

		// Compute screen plane vectors
		std::vector<glm::vec3> sp = camera->c.getScreenCoords();
		glm::vec3 spx = sp[2] - sp[0];
		glm::vec3 spy = sp[1] - sp[0];
		
		tracer->set3f("screen_plane[0]", spx.x, spx.y, spx.z);
		tracer->set3f("screen_plane[1]", spy.x, spy.y, spy.z);
		tracer->set3f("screen_plane[2]", sp[0].x, sp[0].y, sp[0].z);
		
		tracer->set3f("view_pos", camera->c.getPosition().x, camera->c.getPosition().y, camera->c.getPosition().z);
		tracer->set1i("width", win->getWidth());
		tracer->set1i("height", win->getHeight());
		tracer->set1i("renderMode", !guiData->renderMode);
		tracer->set1i("bvhType", guiData->bvhType);
		tracer->set3f("light_pos", lightPos.x, lightPos.y, lightPos.z);
	
		tracer->set1i("shadowSamples", guiData->shadowSamples);
		tracer->set1i("indirectSamples", guiData->indirectSamples);
		tracer->set1i("aoSamples", guiData->aoSamples);

		// Bind all buffers
		geomBuff->bindBase(GL_SHADER_STORAGE_BUFFER, 1);
		matBuff->bindBase(GL_SHADER_STORAGE_BUFFER, 2);
		nodeBuff->bindBase(GL_SHADER_STORAGE_BUFFER, 3);
		indBuff->bindBase(GL_SHADER_STORAGE_BUFFER, 4);

		dbg->bindBase(GL_SHADER_STORAGE_BUFFER, 7);

		GLint wgs[3];
		tracer->getComputeWorkGroupSize(wgs);
		
		// Computation of Ray Tracing in Compute shaders
		ge::gl::glBeginQuery(GL_TIME_ELAPSED, query);
		
		ge::gl::glDispatchCompute(ceil(win->getWidth() / static_cast<float>(wgs[0])), ceil(win->getHeight() / static_cast<float>(wgs[1])), 1);
		ge::gl::glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		
		ge::gl::glEndQuery(GL_TIME_ELAPSED);
		// Computation of Ray Tracing in Compute shaders

		while (!done)
			ge::gl::glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done);

		// Get rendering time
		done = 0;
		ge::gl::glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
		guiData->renderTimes.push_back(elapsed_time / 1000000.0);
		guiData->renderTimes.erase(guiData->renderTimes.begin());
		
		dbg->getData(dbg_data);
		//for (int i = 0; i < dbg_data.size(); i++)
		//	printf("node %f\n", dbg_data[i]);

		//system("pause");
		
		geomBuff->unbindBase(GL_SHADER_STORAGE_BUFFER, 0);
		matBuff->unbindBase(GL_SHADER_STORAGE_BUFFER, 0);
		// ----- Ray trace -----

		// Draw frame
		display->use();
		
		ge::gl::glActiveTexture(GL_TEXTURE0);
		ge::gl::glBindTexture(GL_TEXTURE_2D, renderTexture);
		ge::gl::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		// Draw frame

	}

}

void RayTracing::setupUIData(std::shared_ptr<UserInterface::uiData> data){
	guiData = data;
}

void RayTracing::updateScene(Scene & s){

	drawFrame = false;

	// Reload scene

	// Geometry
	//printf("geo %d\n", s.getGeometry().size());
	geomBuff->realloc(sizeof(Scene::gpu_triangle) * s.getGeometry().size());
	geomBuff->setData(s.getGeometry());
	
	// Materials
	matBuff->realloc(sizeof(Scene::gpu_material) * s.getMaterials().size());
	matBuff->setData(s.getMaterials());

	drawFrame = true;

}

void RayTracing::setupCPUBVH(std::shared_ptr<ge::sg::BVH<ge::sg::AABB_SAH_BVH>> rootNode){

	// Preprocess BVH into linear structure
	bvhPreprocessor bp;
	bp.transformBVH(rootNode->getRoot().get(), rootNode->getRoot()->first);
	
	// Insert converted structure on the GPU
	nodeBuff->realloc(bp.getTree()->size() * sizeof(bvhPreprocessor::gpuNode));
	nodeBuff->setData(bp.getTree()->data());

}

void RayTracing::setupGPUBVH(std::shared_ptr<ge::sg::BVH<ge::sg::RadixTree_BVH>> bvh){

	// Setup nodes and indices buffers
	nodeBuff = bvh->getNodes();
	indBuff = bvh->getIndices();

}

void RayTracing::lightPosEvent(){

	if (glfwGetKey(win->getWindow(), GLFW_KEY_I) == GLFW_PRESS)
		lightPos.y += 0.1f;

	if (glfwGetKey(win->getWindow(), GLFW_KEY_J) == GLFW_PRESS)
		lightPos.x += 0.1f;

	if (glfwGetKey(win->getWindow(), GLFW_KEY_K) == GLFW_PRESS)
		lightPos.y -= 0.1f;

	if (glfwGetKey(win->getWindow(), GLFW_KEY_L) == GLFW_PRESS)
		lightPos.x -= 0.1f;

	if (glfwGetKey(win->getWindow(), GLFW_KEY_N) == GLFW_PRESS)
		lightPos.z += 0.1f;

	if (glfwGetKey(win->getWindow(), GLFW_KEY_M) == GLFW_PRESS)
		lightPos.z -= 0.1f;

}

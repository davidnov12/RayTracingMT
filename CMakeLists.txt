cmake_minimum_required(VERSION 3.12)

project(RayTracing)

add_library(ste INTERFACE)
target_include_directories(ste INTERFACE src/3rd_party/ste/concepts.h src/3rd_party/ste/concepts_undef.h src/3rd_party/ste/DAG.h src/3rd_party/ste/stl_extension.h)

find_package(OpenGL)
#find_package(assimp)
find_package(glfw3)
find_package(glm)
find_package(GPUEngine COMPONENTS geGL geCore geSG)
find_package(AssimpModelLoader HINTS ${GPUEngine_SOURCE_DIR}/geAd/AssimpModelLoader/cmake)

#if(NOT OpenGL_FOUND OR NOT glfw3_FOUND OR NOT glm_FOUND OR NOT TARGET geGL OR NOT TARGET geCore OR NOT TARGET geSG)
   # return()
#endif(NOT OpenGL_FOUND OR NOT glfw3_FOUND OR NOT glm_FOUND OR NOT TARGET geGL OR NOT TARGET geCore OR NOT TARGET geSG)


set(src_rt src/App.h
		   src/bvhPreprocessor.cpp
		   src/bvhPreprocessor.h
		   src/Camera.h
		   src/FPSCamera.h
		   src/FPSCamera.cpp
           src/FPSCameraManager.h
		   src/FPSCameraManager.cpp
		   src/Main.cpp
		   src/RayTracing.h
		   src/RayTracing.cpp
           src/Renderer.h
		   src/Scene.h
		   src/Scene.cpp
           src/UserInterface.cpp
           src/UserInterface.h
		   src/Window.h
		   src/Window.cpp)
		
set(src_bvh src/BVH/AABB_SAH_BVH.cpp
			src/BVH/AABB_SAH_BVH.h
			src/BVH/BVH.h
			src/BVH/GeneralCPUBVH.h
			src/BVH/GeneralCPUBVH.cpp
			src/BVH/GeneralGPUBVH.h
			src/BVH/GeneralGPUBVH.cpp
			src/BVH/BVH_Node.h
			src/BVH/RadixTree_BVH.cpp
			src/BVH/RadixTree_BVH.h)

set(src_3rd src/3rd_party/imgui/imgui.cpp
			src/3rd_party/imgui/imgui_draw.cpp
			src/3rd_party/imgui/imgui_impl_glfw.cpp
			src/3rd_party/imgui/imgui_impl_opengl3.cpp
			src/3rd_party/imgui/imgui_widgets.cpp
			src/3rd_party/imgui/ImGuiFileDialog.cpp
			src/3rd_party/imgui/imconfig.h
			src/3rd_party/imgui/imgui.h
			src/3rd_party/imgui/imgui_impl_glfw.h
			src/3rd_party/imgui/imgui_impl_opengl3.h
			src/3rd_party/imgui/imgui_internal.h
			src/3rd_party/imgui/ImGuiFileDialog.h
			src/3rd_party/imgui/imstb_rectpack.h
			src/3rd_party/imgui/imstb_textedit.h
			src/3rd_party/imgui/imstb_truetype.h
			src/3rd_party/imgui/stb_rect_pack.h
			src/3rd_party/imgui/stb_textedit.h
			src/3rd_party/imgui/stb_truetype.h
			src/3rd_party/imgui/dirent/dirent.h)			

set(src ${src_rt} ${src_bvh} ${src_3rd})

message(source ${src})

#source_group(AssimpModelLoader FILES ${AssimpModelLoader_SOURCE_FILES} ${AssimpModelLoader_HEADER_FILES})
source_group(BVH FILES ${src_bvh})
source_group(RayTracing FILES ${src_rt})
source_group(3rd_party FILES ${src_3rd} ${AssimpModelLoader_SOURCE_FILES} ${AssimpModelLoader_HEADER_FILES})

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE src/3rd_party/image/stb_image.h src/3rd_party/image/stb_image_write.h)

set(vertexShader ${CMAKE_CURRENT_LIST_DIR}/shaders/display.vs)
set(fragmentShader ${CMAKE_CURRENT_LIST_DIR}/shaders/display.fs)
set(computeShader ${CMAKE_CURRENT_LIST_DIR}/shaders/trace.cs)

set(mortonKernel ${CMAKE_CURRENT_LIST_DIR}/src/BVH/kernels/mortonCodeComputation.cs)
set(radixSort ${CMAKE_CURRENT_LIST_DIR}/src/BVH/kernels/parallelRadixSort.cs)
set(treeKernel ${CMAKE_CURRENT_LIST_DIR}/src/BVH/kernels/radixTreeBuilder.cs)

set(fontPath ${CMAKE_CURRENT_LIST_DIR}/font/DroidSans.ttf)

link_directories(${assimp_DIR}/../../../lib ${glfw3_DIR}/../../../lib ${geGL_DIR}/../../../lib ${geCore_DIR}/../../../lib ${geSG_DIR}/../../../lib)
add_executable(${PROJECT_NAME} ${src})
target_link_libraries(${PROJECT_NAME} stb_image glfw3 geGL geCore AssimpModelLoader glm opengl32)
include_directories(${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_CURRENT_LIST_DIR}/src/BVH ${assimp_DIR}/../../../include ${glfw3_DIR}/../../../include ${geGL_DIR}/../../../include ${geCore_DIR}/../../../include ${geSG_DIR}/../../../include)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
target_include_directories(${PROJECT_NAME} PUBLIC "src/" "src/3rd_party")
target_compile_definitions(${PROJECT_NAME} PUBLIC "VERTEX_SHADER_PATH=\"${vertexShader}\"" "FRAGMENT_SHADER_PATH=\"${fragmentShader}\"" "COMPUTE_SHADER_PATH=\"${computeShader}\"" "FONT_FILE_DEST=\"${fontPath}\"" "MORTON_KERNEL=\"${mortonKernel}\"" "RADIX_SORT_KERNEL=\"${radixSort}\"" "TREE_KERNEL=\"${treeKernel}\"")

if(WIN32)
	configure_file(${assimp_DIR}/../../../bin/assimp.dll ${CMAKE_CURRENT_BINARY_DIR}/assimp.dll COPYONLY)
	configure_file(${GPUEngine_DIR}/../../../../bin/geSG.dll ${CMAKE_CURRENT_BINARY_DIR}/geSG.dll COPYONLY)
	#configure_file(${geGL_DIR}/../../../../bin/geGL.dll ${CMAKE_CURRENT_BINARY_DIR}/geSG.dll COPYONLY)
	configure_file(${GPUEngine_DIR}/../../../../bin/geCore.dll ${CMAKE_CURRENT_BINARY_DIR}/geCore.dll COPYONLY)
endif()

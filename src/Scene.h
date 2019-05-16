/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* Scene.h
*/

#pragma once

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <AssimpModelLoader.h>
#include <geSG/MeshTriangleIterators.h>
#include <geSG/MeshPrimitiveIterator.h>
#include <geSG/Model.h>
#include <geSG/Mesh.h>
#include <geSG/AttributeDescriptor.h>

#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>

#include <iostream>
#include <vector>

/**
* @brief Scene manager class
*/
class Scene {

public:

	/**
	* @brief Structure of GPU triangle
	*/
	typedef struct {
		glm::vec4 coord_a, coord_b, coord_c;
		glm::vec4 normal_a, normal_b, normal_c;
		glm::vec2 uv_a, uv_b, uv_c;
		int material_id, align;
	} gpu_triangle;

	/**
	* @brief Structure on material on GPU
	*/
	typedef struct {
		glm::vec3 diffuseColor;
		float refIndex;
		//glm::vec2 gap;
		GLuint64 diffuseTexture;
		float roughness;
		float metalness;
	} gpu_material;

	/**
	* @brief Constructor, initializes class members
	*/
	Scene() { init(); }

	/**
	* @brief Destructor
	*/
	~Scene();

	/**
	* @brief loads scene from given file
	* @param file path to file with scene data
	* @param mode mode of loading
	* @return true if success
	*/
	bool loadScene(std::string file, int mode);
	
	/**
	* @brief Converts scene into vector of triangles, prepares geometry for transfer on GPU
	*/
	void prepareScene();
	//bool prepareGeometry(ge::sg::MeshIndexedTriangleIterator start, ge::sg::MeshIndexedTriangleIterator end);

	/**
	* @brief Getter for geometry data (triangles with all attributes)
	* @return Vector of triangles in scene
	*/
	std::vector<gpu_triangle>& getGeometry();
	
	/**
	* @brief Getter for material data
	* @return Vector of materials in scene
	*/
	std::vector<gpu_material>& getMaterials();

	/**
	* @brief Getter for position vectors of triangles
	* @return Vector of position vectors
	*/
	std::vector<float>& getPositionsVector();

	/**
	* @brief Gets Mesh object, which includes all triangles in scene (may be useful for BVH build)
	* @return Mesh object with all scene triangles
	*/
	std::shared_ptr<ge::sg::Mesh>& getSceneMesh();

private:

	/**
	* @brief Initialization procedure
	*/
	void init();

	/**
	* @brief Loads texture from file
	* @param path Path to file with texture
	* @return true if success
	*/
	bool loadTexture(std::string path);

	// Loader objects
	AssimpModelLoader ml;
	std::shared_ptr<ge::sg::Mesh> whole_scene = std::make_shared<ge::sg::Mesh>();
	std::shared_ptr<ge::sg::AttributeDescriptor> positions = std::make_shared<ge::sg::AttributeDescriptor>();
	std::shared_ptr<ge::sg::AttributeDescriptor> indices = std::make_shared<ge::sg::AttributeDescriptor>();

	// Triangle attributes
	std::vector<float> coords;
	std::vector<float> normals;
	std::vector<float> texcoords;

	// Material attributes
	std::vector<unsigned> mats;
	std::vector<GLuint> textureIDs;
	std::vector<GLuint64> texHandles;

	// Data for usage on GPU
	std::vector<gpu_triangle> triangles;
	std::vector<gpu_material> materials;
	std::vector<float> posVector;

};
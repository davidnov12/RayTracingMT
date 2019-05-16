/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* bvhPreprocessor.h
*/

#pragma once

#include <memory>
#include <vector>
#include <map>

#include <glm/glm.hpp>

#include <BVH.h>
#include <AABB_SAH_BVH.h>

/**
* @brief Transformation BVH structure into GPU SSBO buffer
*/
class bvhPreprocessor {

public:

	/**
	* @brief structure of BVH node on GPU
	*/
	typedef struct {
		glm::vec4 _min;
		glm::vec4 _max;
		int left;
		int right;
		int first;
		int last;
		int parent;
		int sibling;
		int gapA, gapB;
	} gpuNode;

	/**
	* @brief Constructor - empty
	*/
	bvhPreprocessor(){}

	/**
	* @brief Transformation of BVH structure into vector of GPU nodes
	* @param root Root node of BVH structure to be transformed
	* @param first Iterator of first primitive in BVH
	*/
	void transformBVH(ge::sg::BVH_Node<ge::sg::AABB>* root, ge::sg::IndexedTriangleIterator first);
	
	/**
	* @brief Getter for vector of transformed BVH
	* @return Vector of GPU nodes based on given BVH
	*/
	std::vector<gpuNode>* getTree();

private:

	/**
	* @brief Traversal of BVH to gain informations in nodes
	* @param node current node
	* @param first first primitive in BVH structure
	* @param parent id number of parent node
	*/
	void traverseTree(ge::sg::BVH_Node<ge::sg::AABB>* node, ge::sg::IndexedTriangleIterator first, int parent);

	/**
	* @brief Second traversal pass to get connections between nodes
	* @param node current node
	* @param sibling id number of current node's sibling
	*/
	void connectTree(ge::sg::BVH_Node<ge::sg::AABB>* node, int sibling);

	// Result vector
	std::vector<gpuNode> tree;
	
	// Neccesary attributes for transformation
	std::map<ge::sg::BVH_Node<ge::sg::AABB>*, int> associatedIndices;
	int id = 0, tmp_cnt = 0;

};
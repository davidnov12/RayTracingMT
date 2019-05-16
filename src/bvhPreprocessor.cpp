/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* bvhPreprocessor.cpp
*/

#include <bvhPreprocessor.h>

//#define PRINT_NODES

void bvhPreprocessor::transformBVH(ge::sg::BVH_Node<ge::sg::AABB>* root, ge::sg::IndexedTriangleIterator first){

	tree.shrink_to_fit();
	associatedIndices.clear();
	tmp_cnt = id = 0;

	associatedIndices.insert(std::pair<ge::sg::BVH_Node<ge::sg::AABB>*, int>(nullptr, -1));
	traverseTree(root, first, -1);
	connectTree(root, -1);

}

std::vector<bvhPreprocessor::gpuNode>* bvhPreprocessor::getTree(){
	return &tree;
}

void bvhPreprocessor::traverseTree(ge::sg::BVH_Node<ge::sg::AABB>* node, ge::sg::IndexedTriangleIterator first, int parent){

	int cnt = tmp_cnt;
	
	gpuNode n;
	n._min = glm::vec4(node->volume.min, 0.0f);
	n._max = glm::vec4(node->volume.max, 0.0f);
	n.first = (node->first - first);
	n.last = (node->last - first);
	n.parent = parent;
	
	associatedIndices.insert(std::pair<ge::sg::BVH_Node<ge::sg::AABB>*, int>(node, tmp_cnt));
	
	tmp_cnt++;
	tree.push_back(n);

	if (node->left != nullptr)
		traverseTree(node->left.get(), first, cnt);

	if (node->right != nullptr)
		traverseTree(node->right.get(), first, cnt);

}

void bvhPreprocessor::connectTree(ge::sg::BVH_Node<ge::sg::AABB>* node, int sibling){

	tree[id].left = associatedIndices.at(node->left.get());
	tree[id].right = associatedIndices.at(node->right.get());
	
	if (tree[id].left != -1 || tree[id].right != -1) {
			tree[id].first = -1;
			tree[id].last = -1;
	}
	
	tree[id].sibling = sibling;
	
#ifdef PRINT_NODES
	printf("tree %d: extent: %d %d childs: %d %d\n", id, tree[id].first, tree[id].last, tree[id].left, tree[id].right);
	printf("tree %d: min: %f %f %f\n", id, tree[id]._min.x, tree[id]._min.y, tree[id]._min.z);
	printf("tree %d: max: %f %f %f\n\n", id, tree[id]._max.x, tree[id]._max.y, tree[id]._max.z);
#endif

	id++;

	if (node->left != nullptr)
		connectTree(node->left.get(), associatedIndices.at(node->right.get()));

	if (node->right != nullptr)
		connectTree(node->right.get(), associatedIndices.at(node->left.get()));

}

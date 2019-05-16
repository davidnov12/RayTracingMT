/*
* RayTracing pro GPUEngine
* Diplomova prace - master's thesis
* Bc. David Novák
* FIT VUT Brno
* 2018/2019
*
* Scene.cpp
*/


#include <Scene.h>

#define STB_IMAGE_IMPLEMENTATION
#include <3rd_party/image/stb_image.h>

Scene::~Scene(){
	triangles.shrink_to_fit();
	materials.shrink_to_fit();
}

bool Scene::loadScene(std::string file, int mode) {

	triangles.shrink_to_fit();
	materials.shrink_to_fit();

	std::string directory;
	std::vector<float> tmp_pos, tmp_nor, tmp_uv;
	std::vector<unsigned> tmp_mat, tmp_ind;
	std::map<std::shared_ptr<ge::sg::Material>, int> asoc_mat;
	float* tmp;
	int mat_id = 0;
	int ind_offset = 0;

	std::replace(file.begin(), file.end(), '\\', '/');
	std::cout << file << std::endl;
	
	auto scene = ml.loadScene(file.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

	if (scene == nullptr)
		return false;

	const size_t last_slash_idx = file.rfind('/');
	if (std::string::npos != last_slash_idx)
		directory = file.substr(0, last_slash_idx);
	
	std::cout << "Scene " << file << " loaded" << std::endl;

	for (auto model : scene->models) {

		// Materials
		for (auto material : model->materials) {

			asoc_mat.insert(std::pair<std::shared_ptr<ge::sg::Material>, int>(material, mat_id));
			mat_id++;

			gpu_material m;

			for (auto comp : material->materialComponents) {

				if (comp->getType() == ge::sg::MaterialComponent::ComponentType::SIMPLE) {
					ge::sg::MaterialSimpleComponent* ms = (ge::sg::MaterialSimpleComponent*)comp.get();
					
					if (ms->semantic == ge::sg::MaterialSimpleComponent::Semantic::diffuseColor) {

						float* col = (float*)ms->data.get();
						m.diffuseColor = glm::vec3(col[0], col[1], col[2]);
					}

					else if (ms->semantic == ge::sg::MaterialSimpleComponent::Semantic::specularColor) {

						float* col = (float*)ms->data.get();
						m.metalness = col[0];
					}

					else if (ms->semantic == ge::sg::MaterialSimpleComponent::Semantic::ambientColor) {

						float* col = (float*)ms->data.get();
						m.roughness = col[0];
					}
				}

				if (comp->getType() == ge::sg::MaterialComponent::ComponentType::IMAGE) {
					
					ge::sg::MaterialImageComponent* mi = (ge::sg::MaterialImageComponent*)comp.get();
					
					if (mi->semantic == ge::sg::MaterialImageComponent::Semantic::diffuseTexture) {

						std::replace(mi->filePath.begin(), mi->filePath.end(), '\\', '/');

						std::string fullPath;

						if (std::count(mi->filePath.begin(), mi->filePath.end(), '/') < 3)
							fullPath = directory + '/' + mi->filePath;
						else
							fullPath = mi->filePath;

						if(loadTexture(fullPath))
							m.diffuseTexture = texHandles[texHandles.size() - 1];
					}
				}

			}
			materials.push_back(m);
		}

		// Meshes
		for (auto mesh : model->meshes) {

			tmp_mat.insert(tmp_mat.end(), mesh->count, asoc_mat.at(mesh->material));

			for (auto attr : mesh->attributes) {

				if (attr->semantic == ge::sg::AttributeDescriptor::Semantic::position) {
					tmp = static_cast<float*>(attr->data.get());
					std::copy(tmp, tmp + (attr->size / sizeof(float)), std::back_inserter(tmp_pos));
					
				}
				else if (attr->semantic == ge::sg::AttributeDescriptor::Semantic::normal) {
					tmp = static_cast<float*>(attr->data.get());
					std::copy(tmp, tmp + (attr->size / sizeof(float)), std::back_inserter(tmp_nor));
				}
				else if (attr->semantic == ge::sg::AttributeDescriptor::Semantic::texcoord) {
					tmp = static_cast<float*>(attr->data.get());
					std::copy(tmp, tmp + (attr->size / sizeof(float)), std::back_inserter(tmp_uv));
				}
				else if (attr->semantic == ge::sg::AttributeDescriptor::Semantic::indices) {
					unsigned* ind = static_cast<unsigned*>(attr->data.get());
					std::copy(ind, ind + (attr->size / sizeof(unsigned)), std::back_inserter(tmp_ind));

					for (int i = ind_offset; i < tmp_ind.size(); i++){
						tmp_ind[i] += ind_offset;
					}

					ind_offset = tmp_ind.size();
				}

			}
		}
	}

	
	whole_scene->count = tmp_pos.size() / 3;

	positions->size = tmp_pos.size() * sizeof(float);
	positions->numComponents = 3;
	positions->semantic = ge::sg::AttributeDescriptor::Semantic::position;
	positions->data.reset();

	indices->size = tmp_ind.size() * sizeof(unsigned);
	indices->numComponents = 1;
	indices->semantic = ge::sg::AttributeDescriptor::Semantic::indices;
	indices->data.reset();

	float* a = new float[tmp_pos.size()];
	std::memcpy(a, tmp_pos.data(), sizeof(float) * tmp_pos.size());
	positions->data = std::shared_ptr<void>((void*)a);

	unsigned* b = new unsigned[tmp_ind.size()];
	std::memcpy(b, tmp_ind.data(), sizeof(unsigned) * tmp_ind.size());
	indices->data = std::shared_ptr<void>((void*)b);

	whole_scene->attributes.push_back(positions);
	whole_scene->attributes.push_back(indices);
	whole_scene->primitive = ge::sg::Mesh::PrimitiveType::TRIANGLES;

	void* atr = whole_scene->attributes[0]->data.get();
	float* att = (float*)atr;

	coords = tmp_pos;
	normals = tmp_nor;
	texcoords = tmp_uv;
	mats = tmp_mat;

	if(!mode)
		return true;

	for (int i = 0; i < tmp_ind.size(); i += 3) {
		gpu_triangle t;

		t.coord_a = glm::vec4(tmp_pos[3 * i], tmp_pos[(3 * i) + 1], tmp_pos[(3 * i) + 2], 1.0f);
		t.coord_b = glm::vec4(tmp_pos[3 * (i + 1)], tmp_pos[(3 * (i + 1)) + 1], tmp_pos[(3 * (i + 1)) + 2], 1.0f);
		t.coord_c = glm::vec4(tmp_pos[3 * (i + 2)], tmp_pos[(3 * (i + 2)) + 1], tmp_pos[(3 * (i + 2)) + 2], 1.0f);

		posVector.push_back(t.coord_a.x);
		posVector.push_back(t.coord_a.y);
		posVector.push_back(t.coord_a.z);

		posVector.push_back(t.coord_b.x);
		posVector.push_back(t.coord_b.y);
		posVector.push_back(t.coord_b.z);

		posVector.push_back(t.coord_c.x);
		posVector.push_back(t.coord_c.y);
		posVector.push_back(t.coord_c.z);

		t.normal_a = glm::vec4(tmp_nor[3 * (i)], tmp_nor[(3 * i) + 1], tmp_nor[(3 * i) + 2], 1.0f);
		t.normal_b = glm::vec4(tmp_nor[3 * (i + 1)], tmp_nor[(3 * (i + 1)) + 1], tmp_nor[(3 * (i + 1)) + 2], 1.0f);
		t.normal_c = glm::vec4(tmp_nor[3 * (i + 2)], tmp_nor[(3 * (i + 2)) + 1], tmp_nor[(3 * (i + 2)) + 2], 1.0f);

		if (!tmp_uv.empty()) {
			t.uv_a = glm::vec2(tmp_uv[2 * i], tmp_uv[(2 * i) + 1]);
			t.uv_b = glm::vec2(tmp_uv[2 * (i + 1)], tmp_uv[(2 * (i + 1)) + 1]);
			t.uv_c = glm::vec2(tmp_uv[2 * (i + 2)], tmp_uv[(2 * (i + 2)) + 1]);
		}

		t.material_id = tmp_mat[i];

		triangles.push_back(t);
	}

	return true;
}

void Scene::prepareScene(){

	std::vector<unsigned> indices = std::vector<unsigned>(static_cast<unsigned*> (this->indices->data.get()), static_cast<unsigned*> (this->indices->data.get()) + (this->indices->size / sizeof(unsigned)));
	triangles.shrink_to_fit();
	
	for (int i = 0; i < indices.size(); i += 3) {
		gpu_triangle t;
	
		t.coord_a = glm::vec4(coords[3 * indices[i]], coords[(3 * indices[i]) + 1], coords[(3 * indices[i]) + 2], 1.0f);
		t.coord_b = glm::vec4(coords[3 * indices[i + 1]], coords[(3 * indices[i + 1]) + 1], coords[(3 * indices[i + 1]) + 2], 1.0f);
		t.coord_c = glm::vec4(coords[3 * indices[i + 2]], coords[(3 * indices[i + 2]) + 1], coords[(3 * indices[i + 2]) + 2], 1.0f);

		t.normal_a = glm::vec4(normals[3 * indices[i]], normals[(3 * indices[i]) + 1], normals[(3 * indices[i]) + 2], 1.0f);
		t.normal_b = glm::vec4(normals[3 * indices[i + 1]], normals[(3 * indices[i + 1]) + 1], normals[(3 * indices[i + 1]) + 2], 1.0f);
		t.normal_c = glm::vec4(normals[3 * indices[i + 2]], normals[(3 * indices[i + 2]) + 1], normals[(3 * indices[i + 2]) + 2], 1.0f);

		if (!texcoords.empty()) {
			t.uv_a = glm::vec2(texcoords[2 * indices[i]], texcoords[(2 * indices[i]) + 1]);
			t.uv_b = glm::vec2(texcoords[2 * indices[i + 1]], texcoords[(2 * indices[i + 1]) + 1]);
			t.uv_c = glm::vec2(texcoords[2 * indices[i + 2]], texcoords[(2 * indices[i + 2]) + 1]);
		}

		t.material_id = mats[indices[i]];
		
		triangles.push_back(t);
	}

}

std::vector<Scene::gpu_triangle>& Scene::getGeometry(){
	return triangles;
}

std::vector<Scene::gpu_material>& Scene::getMaterials(){
	return materials;
}

std::vector<float>& Scene::getPositionsVector(){
	return posVector;
}

std::shared_ptr<ge::sg::Mesh>& Scene::getSceneMesh(){
	return whole_scene;
}

void Scene::init(){

	whole_scene->attributes.push_back(positions);
	whole_scene->attributes.push_back(indices);

	positions->semantic = ge::sg::AttributeDescriptor::Semantic::position;
	positions->numComponents = 3;
	positions->type = ge::sg::AttributeDescriptor::DataType::FLOAT;

	indices->semantic = ge::sg::AttributeDescriptor::Semantic::indices;
	indices->numComponents = 1;
	indices->type = ge::sg::AttributeDescriptor::DataType::UNSIGNED_INT;
	
}

bool Scene::loadTexture(std::string path){
	
	GLuint textureID;
	int width, height, comp;

	ge::gl::glGenTextures(1, &textureID);

	std::unique_ptr<unsigned char> imageData;
	
	// Load texture
	imageData.reset(stbi_load(path.c_str(), &width, &height, &comp, STBI_default));

	if (imageData.get() == nullptr){
		std::cout << "Texture " << path << " loading failed" << std::endl;
		return false;
	}

	GLenum components = comp == 3 ? GL_RGB : comp == 1 ? GL_RED : GL_RGBA;

	// Setup texture
	ge::gl::glBindTexture(GL_TEXTURE_2D, textureID);

	ge::gl::glTexImage2D(GL_TEXTURE_2D, 0, components, width, height, 0, components, GL_UNSIGNED_BYTE, imageData.get());
	ge::gl::glGenerateMipmap(GL_TEXTURE_2D);

	ge::gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	ge::gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	ge::gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	ge::gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	ge::gl::glBindTexture(GL_TEXTURE_2D, 0);

	// Bindless texture
	textureIDs.push_back(textureID);
	texHandles.push_back(ge::gl::glGetTextureHandleARB(textureIDs.at(textureIDs.size() - 1)));
	ge::gl::glMakeTextureHandleResidentARB(texHandles.at(texHandles.size() - 1));
	
	stbi_image_free(imageData.get());
	imageData.release();

#ifdef TEXTURES_PRINT
	std::cout << "Loaded " << path.c_str() << std::endl;
#endif

	return true;

}

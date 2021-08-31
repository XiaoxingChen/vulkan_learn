#if !defined(_GLTF_UTILS_H_)
#define _GLTF_UTILS_H_

#include <vector>
#include "tiny_gltf.h"

bool loadModel(tinygltf::Model& model, const std::string& filename);
std::vector<float> loadMeshAttributes(tinygltf::Model& model, size_t mesh_idx, const std::string& type);
std::vector<uint16_t> loadMeshIndices(tinygltf::Model& model,size_t mesh_idx);
const tinygltf::Image& loadMeshTexture(tinygltf::Model& model, size_t tex_idx);
float adaptiveModelScale(tinygltf::Model& model, size_t mesh_idx, float target_magnitide=1.f);

#endif // _GLTF_UTILS_H_

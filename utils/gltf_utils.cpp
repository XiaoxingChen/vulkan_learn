#include <iostream>
#include <map>
#include <vector>
#include "tiny_gltf.h"
#include <stb_image.h>

bool loadModel(tinygltf::Model& model, const std::string& filename)
{
    tinygltf::TinyGLTF loader;
    std::string        err;
    std::string        warn;

    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    // bool res(true);
    if (!warn.empty())
    {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty())
    {
        std::cout << "ERR: " << err << std::endl;
    }

    if (!res)
        std::cout << "Failed to load glTF: " << filename << std::endl;
    else
        std::cout << "Loaded glTF: " << filename << std::endl;

    return res;
}

float adaptiveModelScale(
    tinygltf::Model& model,
    size_t mesh_idx,
    float target_magnitide)
{
    const std::string type("POSITION");

    auto & mesh(model.meshes.at(mesh_idx));
    auto& primitive = mesh.primitives.at(0);
    assert(primitive.attributes.count(type) > 0);

    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes[type]];
    float max_length = 0;
    for(size_t i = 0; i < accessor.minValues.size(); i++)
    {
        float length = accessor.maxValues.at(i) - accessor.minValues.at(i);
        max_length = std::max(max_length, length);
    }
    return target_magnitide / max_length;
}

std::vector<float> loadMeshAttributes(
    tinygltf::Model& model,
    size_t mesh_idx,
    const std::string& type)
{
    std::vector<float> ret;
    auto & mesh(model.meshes.at(mesh_idx));
    std::cout << "load " + type << std::endl;
    auto& primitive = mesh.primitives.at(0);

    if(primitive.attributes.count(type) == 0)
        throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__));

    if(primitive.mode != TINYGLTF_MODE_TRIANGLES)
        throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__));


    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes[type]];

    if(accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
        throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__));

    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    if(bufferView.target != TINYGLTF_TARGET_ARRAY_BUFFER)
        throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__));

    const tinygltf::Buffer&     buffer     = model.buffers[bufferView.buffer];

    // const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
    // std::cout << "bufferView.byteStride: " << bufferView.byteStride << std::endl;
    // std::cout << "accessor.byteOffset: " << accessor.byteOffset << std::endl;

    size_t data_width = accessor.type == TINYGLTF_TYPE_VEC3 ? 3
        : accessor.type == TINYGLTF_TYPE_VEC2 ? 2 : -1;

    // ret = std::make_shared<Mat>(Shape({data_width, accessor.count}));
    ret.resize(data_width * accessor.count);
    size_t byteStride = bufferView.byteStride > 0 ? bufferView.byteStride : data_width * sizeof(float);

    for (size_t i = 0; i < accessor.count; ++i)
    {
        const float* local_addr = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + i * byteStride + accessor.byteOffset]);
        for(size_t j = 0; j < data_width; j++)
        {
            // (*ret)(j,i) = local_addr[j];
            ret.at(i * data_width + j) = local_addr[j];
        }

    }
    return ret;
}

std::vector<uint16_t> loadMeshIndices(
    tinygltf::Model& model,
    size_t mesh_idx)
{

    auto & mesh(model.meshes.at(mesh_idx));
    std::cout << "mesh : " << mesh.name << std::endl;
    auto& primitive = mesh.primitives.at(0);

    if(primitive.mode != TINYGLTF_MODE_TRIANGLES)
        throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__));

    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];;

    // if(accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
    // {
    //     std::cout << "accessor.componentType: " << accessor.componentType << std::endl;
    //     throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__));
    // }

    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer&     buffer     = model.buffers[bufferView.buffer];

    std::vector<uint16_t> ret(accessor.count);
    if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
    {
        const uint16_t* indices = reinterpret_cast<const uint16_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
        for(size_t i = 0; i < ret.size(); i++) ret.at(i) = indices[i];
    }else if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
    {
        const uint8_t* indices = reinterpret_cast<const uint8_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
        for(size_t i = 0; i < ret.size(); i++) ret.at(i) = indices[i];
    }else
    {
        std::cout << "accessor.componentType: " << accessor.componentType << std::endl;
        throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__));
    }

    return ret;
}

const tinygltf::Image& loadMeshTexture(tinygltf::Model& model, size_t tex_idx)
{
    if (model.textures.size() == 0)
        throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__));

    tinygltf::Texture &tex = model.textures[tex_idx];

    if (tex.source < 0)
        throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__));

    tinygltf::Image &image = model.images[tex.source];

    if(image.component != TINYGLTF_TYPE_VEC3 && image.component != TINYGLTF_TYPE_VEC4)
    {
        std::cout << "image.component: " << image.component << std::endl;
        throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__));
    }

    size_t pixel_stride =
        image.component == TINYGLTF_TYPE_VEC3 ? 3 :
        image.component == TINYGLTF_TYPE_VEC4 ? 4 : 0;

    std::cout << "pixel channel: " << pixel_stride
    << ", image w: " << image.width << ", h: " << image.height
    << std::endl;

    if (image.bits != 8)
        throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__));

    return image;
}

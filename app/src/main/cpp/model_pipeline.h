#pragma once
#include <android/asset_manager.h>
#include <cstdint>
#include <string>
#include <vector>
namespace apocalypse::assets {
struct ModelVertex { float position[3]; float normal[3]; float uv[2]; };
struct PbrMaterial {
    std::string name, albedoPath, normalPath, metallicRoughnessPath;
    float baseColor[4]={1,1,1,1};
    float metallic=0.0f, roughness=1.0f;
};
struct ModelMesh {
    std::vector<ModelVertex> vertices;
    std::vector<uint32_t> indices;
    PbrMaterial material;
};
struct ModelAsset { std::vector<ModelMesh> meshes; };
bool loadObj(AAssetManager* assets,const char* path,ModelAsset& out,std::string& error);
bool loadGltf(AAssetManager* assets,const char* path,ModelAsset& out,std::string& error);
}

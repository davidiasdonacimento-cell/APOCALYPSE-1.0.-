#pragma once
#include <android/asset_manager.h>
#include <GLES3/gl3.h>
#include "model_pipeline.h"
namespace apocalypse::assets {
class ModelRenderer {
public:
 bool initialize(AAssetManager* assets);
 void render(const float* viewProjection);
 void shutdown();
private:
 struct GpuMesh { GLuint vertex=0,index=0,albedo=0,normal=0,metalRough=0; GLsizei indexCount=0; float base[4]={1,1,1,1}; float metallic=0,roughness=1; };
 GLuint program_=0;
 GpuMesh meshes_[16]{};
 int meshCount_=0;
};
}

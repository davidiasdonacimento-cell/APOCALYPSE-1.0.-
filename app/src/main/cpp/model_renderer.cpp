#include "model_renderer.h"
#include <android/asset_manager.h>
#include <android/log.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"
#define MR_TAG "APOCALYPSE_PBR"
#define MR_LOG(...) __android_log_print(ANDROID_LOG_ERROR,MR_TAG,__VA_ARGS__)
namespace apocalypse::assets {
namespace {
GLuint shader(GLenum type,const char* src){GLuint s=glCreateShader(type);glShaderSource(s,1,&src,nullptr);glCompileShader(s);GLint ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);if(!ok){char log[1024]={};glGetShaderInfoLog(s,sizeof(log),nullptr,log);MR_LOG("shader: %s",log);glDeleteShader(s);return 0;}return s;}
GLuint makeTexture(AAssetManager* assets,const std::string& path,int kind,const char* material){
 int w=0,h=0,n=0;unsigned char* pixels=nullptr;
 if(assets&&!path.empty()){
  AAsset* a=AAssetManager_open(assets,path.c_str(),AASSET_MODE_BUFFER);
  if(a){off_t len=AAsset_getLength(a);if(len>0&&len<32*1024*1024){std::vector<unsigned char> bytes((size_t)len);int got=AAsset_read(a,bytes.data(),bytes.size());if(got==len)pixels=stbi_load_from_memory(bytes.data(),got,&w,&h,&n,kind==0?STBI_rgb:STBI_rgb);}AAsset_close(a);}
 }
 if(!pixels){
  w=h=128;n=3;pixels=(unsigned char*)STBI_MALLOC(w*h*3);if(!pixels)return 0;
  for(int y=0;y<h;y++)for(int x=0;x<w;x++){
   unsigned int q=(unsigned int)(x*73856093u)^(unsigned int)(y*19349663u);q^=q>>13;q*=1274126177u;q^=q>>16;
   int grain=(int)(q%19u)-9,at=(y*w+x)*3;
   if(kind==0){int brick=((x/32+y/18)%2)*-12;bool window=(std::strstr(material,"glass")!=nullptr);pixels[at]=(unsigned char)std::clamp((window?42:142)+grain+brick,0,255);pixels[at+1]=(unsigned char)std::clamp((window?83:132)+grain+brick,0,255);pixels[at+2]=(unsigned char)std::clamp((window?105:118)+grain+brick,0,255);}
   else if(kind==1){pixels[at]=128;pixels[at+1]=(unsigned char)(126+(q%5));pixels[at+2]=255;}
   else if(kind==2){pixels[at]=205;pixels[at+1]=205;pixels[at+2]=205;}
   else if(kind==3){unsigned char metal=(unsigned char)(std::strstr(material,"metal")||std::strstr(material,"glass")?210:12);pixels[at]=metal;pixels[at+1]=metal;pixels[at+2]=metal;}
   else {pixels[at]=255;pixels[at+1]=205;pixels[at+2]=(unsigned char)(std::strstr(material,"metal")?210:12);}
  }
 }
 GLenum format=GL_RGB;GLuint tex=0;glGenTextures(1,&tex);glBindTexture(GL_TEXTURE_2D,tex);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w,h,0,format,GL_UNSIGNED_BYTE,pixels);glGenerateMipmap(GL_TEXTURE_2D);
 stbi_image_free(pixels);return tex;
}
}
bool ModelRenderer::initialize(AAssetManager* assets){
 const char* vs="#version 300 es\nprecision highp float;layout(location=0) in vec3 aPos;layout(location=1) in vec3 aNormal;layout(location=2) in vec2 aUV;uniform mat4 uVP;out vec3 worldPos;out vec3 normal;out vec2 uv;void main(){worldPos=aPos;normal=aNormal;uv=aUV;gl_Position=uVP*vec4(aPos,1.0);}";
 const char* fs="#version 300 es\nprecision highp float;in vec3 worldPos;in vec3 normal;in vec2 uv;uniform sampler2D uAlbedo;uniform sampler2D uNormal;uniform sampler2D uRoughnessMap;uniform sampler2D uMetallicMap;uniform int uPacked;uniform vec4 uBaseColor;uniform float uMetallic;uniform float uRoughness;out vec4 fragColor;const float PI=3.14159265359;vec3 fresnel(float c,vec3 f0){return f0+(1.0-f0)*pow(1.0-c,5.0);}void main(){vec3 N=normalize(normal);vec3 dp1=dFdx(worldPos),dp2=dFdy(worldPos);vec2 duv1=dFdx(uv),duv2=dFdy(uv);vec3 T=normalize(dp1*duv2.y-dp2*duv1.y);vec3 B=normalize(-dp1*duv2.x+dp2*duv1.x);vec3 mapN=texture(uNormal,uv).xyz*2.0-1.0;N=normalize(mat3(T,B,N)*mapN);vec4 albedo=texture(uAlbedo,uv)*uBaseColor;float roughSample=uPacked==1?texture(uRoughnessMap,uv).g:texture(uRoughnessMap,uv).r;float metalSample=uPacked==1?texture(uMetallicMap,uv).b:texture(uMetallicMap,uv).r;float metallic=clamp(uMetallic*metalSample,0.0,1.0);float rough=clamp(uRoughness*roughSample,0.045,1.0);vec3 V=normalize(vec3(0.0,4.0,8.0)-worldPos);vec3 L=normalize(vec3(-0.45,0.85,0.35));vec3 H=normalize(V+L);float NoV=max(dot(N,V),0.001),NoL=max(dot(N,L),0.0),NoH=max(dot(N,H),0.0),VoH=max(dot(V,H),0.0);float a=rough*rough,a2=a*a;float d=a2/(PI*pow(NoH*NoH*(a2-1.0)+1.0,2.0));float k=pow(rough+1.0,2.0)/8.0;float gv=NoV/(NoV*(1.0-k)+k),gl=NoL/(NoL*(1.0-k)+k);vec3 f0=mix(vec3(0.04),albedo.rgb,metallic);vec3 F=fresnel(VoH,f0);vec3 spec=(d*gv*gl*F)/(4.0*NoV*max(NoL,0.001));vec3 kd=(1.0-F)*(1.0-metallic);vec3 diffuse=kd*albedo.rgb/PI;vec3 ambient=albedo.rgb*0.20;vec3 color=ambient+(diffuse+spec)*NoL*3.0;color=color/(color+vec3(1.0));color=pow(color,vec3(1.0/2.2));fragColor=vec4(color,albedo.a);}";
 GLuint v=shader(GL_VERTEX_SHADER,vs),f=shader(GL_FRAGMENT_SHADER,fs);if(!v||!f)return false;
 program_=glCreateProgram();glAttachShader(program_,v);glAttachShader(program_,f);glLinkProgram(program_);glDeleteShader(v);glDeleteShader(f);GLint ok=0;glGetProgramiv(program_,GL_LINK_STATUS,&ok);if(!ok){char log[1024]={};glGetProgramInfoLog(program_,sizeof(log),nullptr,log);MR_LOG("link: %s",log);glDeleteProgram(program_);program_=0;return false;}
 ModelAsset asset;std::string error;
 if(!loadObj(assets,"models/apocalypse_hospital.obj",asset,error)){MR_LOG("OBJ: %s",error.c_str());return false;}
 meshCount_=std::min((int)asset.meshes.size(),16);
 for(int i=0;i<meshCount_;i++){auto& src=asset.meshes[i];auto& dst=meshes_[i];dst.indexCount=(GLsizei)src.indices.size();std::copy(src.material.baseColor,src.material.baseColor+4,dst.base);dst.metallic=src.material.metallic;dst.roughness=src.material.roughness;
  glGenBuffers(1,&dst.vertex);glBindBuffer(GL_ARRAY_BUFFER,dst.vertex);glBufferData(GL_ARRAY_BUFFER,src.vertices.size()*sizeof(ModelVertex),src.vertices.data(),GL_STATIC_DRAW);
  glGenBuffers(1,&dst.index);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,dst.index);glBufferData(GL_ELEMENT_ARRAY_BUFFER,src.indices.size()*sizeof(uint32_t),src.indices.data(),GL_STATIC_DRAW);
  dst.albedo=makeTexture(assets,src.material.albedoPath,0,src.material.name.c_str());dst.normal=makeTexture(assets,src.material.normalPath,1,src.material.name.c_str());dst.packedMetalRough=!src.material.metallicRoughnessPath.empty();
  if(dst.packedMetalRough){dst.roughnessMap=makeTexture(assets,src.material.metallicRoughnessPath,4,src.material.name.c_str());dst.metallicMap=dst.roughnessMap;}
  else{dst.roughnessMap=makeTexture(assets,src.material.roughnessPath,2,src.material.name.c_str());dst.metallicMap=makeTexture(assets,src.material.metallicPath,3,src.material.name.c_str());}
 }
 MR_LOG("OBJ importado: %d submalhas",meshCount_);return meshCount_>0;
}
void ModelRenderer::render(const float* vp){
 if(!program_||meshCount_==0)return;glUseProgram(program_);glUniformMatrix4fv(glGetUniformLocation(program_,"uVP"),1,GL_FALSE,vp);
 const char* names[4]={"uAlbedo","uNormal","uRoughnessMap","uMetallicMap"};
 for(int i=0;i<meshCount_;i++){auto& m=meshes_[i];GLuint tex[4]={m.albedo,m.normal,m.roughnessMap,m.metallicMap};for(int k=0;k<4;k++){glActiveTexture(GL_TEXTURE0+k);glBindTexture(GL_TEXTURE_2D,tex[k]);glUniform1i(glGetUniformLocation(program_,names[k]),k);}
  glUniform1i(glGetUniformLocation(program_,"uPacked"),m.packedMetalRough?1:0);glUniform4fv(glGetUniformLocation(program_,"uBaseColor"),1,m.base);glUniform1f(glGetUniformLocation(program_,"uMetallic"),m.metallic);glUniform1f(glGetUniformLocation(program_,"uRoughness"),m.roughness);
  glBindBuffer(GL_ARRAY_BUFFER,m.vertex);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m.index);
  glEnableVertexAttribArray(0);glEnableVertexAttribArray(1);glEnableVertexAttribArray(2);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(ModelVertex),(void*)offsetof(ModelVertex,position));
  glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(ModelVertex),(void*)offsetof(ModelVertex,normal));
  glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(ModelVertex),(void*)offsetof(ModelVertex,uv));
  glDrawElements(GL_TRIANGLES,m.indexCount,GL_UNSIGNED_INT,0);
 }
 glDisableVertexAttribArray(0);glDisableVertexAttribArray(1);glDisableVertexAttribArray(2);
}
void ModelRenderer::shutdown(){for(int i=0;i<meshCount_;i++){auto& m=meshes_[i];glDeleteBuffers(1,&m.vertex);glDeleteBuffers(1,&m.index);glDeleteTextures(1,&m.albedo);glDeleteTextures(1,&m.normal);glDeleteTextures(1,&m.roughnessMap);if(!m.packedMetalRough)glDeleteTextures(1,&m.metallicMap);}if(program_)glDeleteProgram(program_);program_=0;meshCount_=0;}
}

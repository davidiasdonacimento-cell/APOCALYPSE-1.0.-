#include "model_pipeline.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <unordered_map>
#include <array>
#include <tuple>
#include <memory>
#define CGLTF_IMPLEMENTATION
#include "vendor/cgltf.h"

namespace apocalypse::assets {
namespace {
bool readAsset(AAssetManager* manager,const char* path,std::vector<unsigned char>& out){
    if(!manager||!path)return false;
    AAsset* a=AAssetManager_open(manager,path,AASSET_MODE_BUFFER);if(!a)return false;
    off_t n=AAsset_getLength(a);if(n<=0||n>64*1024*1024){AAsset_close(a);return false;}
    out.resize((size_t)n);int got=AAsset_read(a,out.data(),out.size());AAsset_close(a);
    return got==n;
}
struct Key { int p,t,n; bool operator==(const Key& o)const{return p==o.p&&t==o.t&&n==o.n;} };
struct KeyHash { size_t operator()(const Key& k)const{return (size_t)(k.p*73856093u)^(size_t)(k.t*19349663u)^(size_t)(k.n*83492791u);} };
int resolveIndex(int i,size_t count){return i>0?i-1:(i<0?(int)count+i:-1);}
bool parseFaceIndex(const std::string& token,Key& k){
    k={-1,-1,-1};int a=0,b=0,c=0;
    size_t p1=token.find('/');if(p1==std::string::npos)return std::sscanf(token.c_str(),"%d",&a)==1&&(k.p=a,true);
    std::string x=token.substr(0,p1);if(std::sscanf(x.c_str(),"%d",&a)!=1)return false;
    size_t p2=token.find('/',p1+1);
    if(p2==std::string::npos){std::string y=token.substr(p1+1);if(!y.empty())std::sscanf(y.c_str(),"%d",&b);}
    else {std::string y=token.substr(p1+1,p2-p1-1),z=token.substr(p2+1);if(!y.empty())std::sscanf(y.c_str(),"%d",&b);if(!z.empty())std::sscanf(z.c_str(),"%d",&c);}
    k={a,b,c};return true;
}
void loadMtl(AAssetManager* assets,const std::string& path,std::unordered_map<std::string,PbrMaterial>& mats){
    std::vector<unsigned char> raw;if(!readAsset(assets,path.c_str(),raw))return;
    std::istringstream in(std::string(raw.begin(),raw.end()));std::string line;PbrMaterial* m=nullptr;
    while(std::getline(in,line)){std::istringstream row(line);std::string tag;row>>tag;
        if(tag=="newmtl"){std::string name;row>>name;m=&mats[name];m->name=name;}
        else if(!m)continue;
        else if(tag=="Kd")row>>m->baseColor[0]>>m->baseColor[1]>>m->baseColor[2];
        else if(tag=="d")row>>m->baseColor[3];
        else if(tag=="Pr")row>>m->roughness;
        else if(tag=="Pm")row>>m->metallic;
        else if(tag=="map_Kd"){std::string f;row>>f;m->albedoPath=f;}
        else if(tag=="map_Bump"||tag=="bump"||tag=="norm"){std::string f;row>>f;m->normalPath=f;}
        else if(tag=="map_Pr"){std::string f;row>>f;m->roughnessPath=f;}
        else if(tag=="map_Pm"){std::string f;row>>f;m->metallicPath=f;}
        else if(tag=="map_RMA"){std::string f;row>>f;m->metallicRoughnessPath=f;}
    }
}
bool unpackAttribute(cgltf_accessor* accessor,size_t components,std::vector<float>& out){
    if(!accessor||cgltf_num_components(accessor->type)!=components)return false;
    out.resize(accessor->count*components);
    return cgltf_accessor_unpack_floats(accessor,out.data(),out.size())==out.size();
}
struct GltfFileContext { AAssetManager* assets; };
cgltf_result gltfRead(const cgltf_memory_options*,const cgltf_file_options* fo,const char* path,cgltf_size* size,void** data){
    auto* ctx=(GltfFileContext*)fo->user_data;std::vector<unsigned char> bytes;
    if(!readAsset(ctx->assets,path,bytes))return cgltf_result_file_not_found;
    void* mem=std::malloc(bytes.size());if(!mem)return cgltf_result_out_of_memory;
    std::memcpy(mem,bytes.data(),bytes.size());*size=bytes.size();*data=mem;return cgltf_result_success;
}
void gltfRelease(const cgltf_memory_options*,const cgltf_file_options*,void* data,cgltf_size){std::free(data);}
std::string imagePath(cgltf_texture_view view){
    if(!view.texture||!view.texture->image||!view.texture->image->uri)return {};
    return view.texture->image->uri;
}
}
bool loadObj(AAssetManager* assets,const char* path,ModelAsset& out,std::string& error){
    out.meshes.clear();std::vector<unsigned char> raw;
    if(!readAsset(assets,path,raw)){error="OBJ não encontrado no pacote de assets";return false;}
    std::istringstream in(std::string(raw.begin(),raw.end()));std::string line,base(path);
    auto slash=base.find_last_of('/');std::string folder=slash==std::string::npos?"":base.substr(0,slash+1);
    std::vector<std::array<float,3>> positions,normals;std::vector<std::array<float,2>> uvs;
    std::unordered_map<std::string,PbrMaterial> materials;std::unordered_map<Key,uint32_t,KeyHash> cache;
    ModelMesh current;current.material.name="default";bool hasFaces=false;
    auto flush=[&](){if(!current.indices.empty()){out.meshes.push_back(std::move(current));current=ModelMesh{};cache.clear();hasFaces=false;}};
    while(std::getline(in,line)){
        std::istringstream row(line);std::string tag;row>>tag;if(tag.empty()||tag[0]=='#')continue;
        if(tag=="v"){std::array<float,3> v{};if(row>>v[0]>>v[1]>>v[2])positions.push_back(v);}
        else if(tag=="vn"){std::array<float,3> n{};if(row>>n[0]>>n[1]>>n[2])normals.push_back(n);}
        else if(tag=="vt"){std::array<float,2> t{};if(row>>t[0]>>t[1])uvs.push_back(t);}
        else if(tag=="mtllib"){std::string f;row>>f;loadMtl(assets,folder+f,materials);}
        else if(tag=="usemtl"){flush();std::string name;row>>name;auto it=materials.find(name);if(it!=materials.end())current.material=it->second;else current.material.name=name;}
        else if(tag=="f"){
            std::vector<uint32_t> face;std::string token;
            while(row>>token){Key k;if(!parseFaceIndex(token,k))continue;k.p=resolveIndex(k.p,positions.size());if(k.t>=0)k.t=resolveIndex(k.t,uvs.size());if(k.n>=0)k.n=resolveIndex(k.n,normals.size());
                auto found=cache.find(k);uint32_t idx;
                if(found!=cache.end())idx=found->second;else{
                    if(k.p<0||k.p>=(int)positions.size())continue;
                    ModelVertex v{};for(int q=0;q<3;q++)v.position[q]=positions[k.p][q];
                    if(k.n>=0&&k.n<(int)normals.size())for(int q=0;q<3;q++)v.normal[q]=normals[k.n][q];
                    if(k.t>=0&&k.t<(int)uvs.size()){v.uv[0]=uvs[k.t][0];v.uv[1]=1.0f-uvs[k.t][1];}
                    idx=(uint32_t)current.vertices.size();current.vertices.push_back(v);cache.emplace(k,idx);
                }face.push_back(idx);
            }
            for(size_t i=1;i+1<face.size();i++){current.indices.push_back(face[0]);current.indices.push_back(face[i]);current.indices.push_back(face[i+1]);}
            hasFaces=true;
        }
    }
    flush();
    if(out.meshes.empty()){error="OBJ sem faces válidas";return false;}
    for(auto& mesh:out.meshes){
        bool missing=false;for(auto& v:mesh.vertices)if(v.normal[0]==0&&v.normal[1]==0&&v.normal[2]==0){missing=true;break;}
        if(missing){for(size_t i=0;i+2<mesh.indices.size();i+=3){auto& a=mesh.vertices[mesh.indices[i]];auto& b=mesh.vertices[mesh.indices[i+1]];auto& c=mesh.vertices[mesh.indices[i+2]];
            float ux=b.position[0]-a.position[0],uy=b.position[1]-a.position[1],uz=b.position[2]-a.position[2];
            float vx=c.position[0]-a.position[0],vy=c.position[1]-a.position[1],vz=c.position[2]-a.position[2];
            float nx=uy*vz-uz*vy,ny=uz*vx-ux*vz,nz=ux*vy-uy*vx;float len=std::sqrt(nx*nx+ny*ny+nz*nz);if(len>1e-6f){nx/=len;ny/=len;nz/=len;}
            for(int q=0;q<3;q++){auto& v=mesh.vertices[mesh.indices[i+q]];v.normal[0]+=nx;v.normal[1]+=ny;v.normal[2]+=nz;}
        }}
        for(auto& v:mesh.vertices){float l=std::sqrt(v.normal[0]*v.normal[0]+v.normal[1]*v.normal[1]+v.normal[2]*v.normal[2]);if(l>1e-6f){v.normal[0]/=l;v.normal[1]/=l;v.normal[2]/=l;}else{v.normal[1]=1.0f;}}
    }
    return true;
}
bool loadGltf(AAssetManager* assets,const char* path,ModelAsset& out,std::string& error){
    out.meshes.clear();std::vector<unsigned char> json;if(!readAsset(assets,path,json)){error="glTF/GLB não encontrado";return false;}
    cgltf_options options{};GltfFileContext context{assets};options.file.read=gltfRead;options.file.release=gltfRelease;options.file.user_data=&context;
    cgltf_data* data=nullptr;cgltf_result result=cgltf_parse(&options,json.data(),json.size(),&data);
    if(result!=cgltf_result_success||!data){error="Falha ao interpretar glTF 2.0";return false;}
    result=cgltf_load_buffers(&options,data,path);
    if(result!=cgltf_result_success){cgltf_free(data);error="Falha ao carregar buffers externos do glTF";return false;}
    for(cgltf_size mi=0;mi<data->meshes_count;mi++)for(cgltf_size pi=0;pi<data->meshes[mi].primitives_count;pi++){
        cgltf_primitive& primitive=data->meshes[mi].primitives[pi];if(primitive.type!=cgltf_primitive_type_triangles)continue;
        cgltf_accessor *pa=nullptr,*na=nullptr,*ta=nullptr;
        for(cgltf_size ai=0;ai<primitive.attributes_count;ai++){auto& a=primitive.attributes[ai];if(a.type==cgltf_attribute_type_position&&a.index==0)pa=a.data;else if(a.type==cgltf_attribute_type_normal&&a.index==0)na=a.data;else if(a.type==cgltf_attribute_type_texcoord&&a.index==0)ta=a.data;}
        std::vector<float> p,n,t;if(!unpackAttribute(pa,3,p))continue;
        unpackAttribute(na,3,n);unpackAttribute(ta,2,t);
        ModelMesh mesh;mesh.vertices.resize(pa->count);
        for(size_t i=0;i<mesh.vertices.size();i++){auto& v=mesh.vertices[i];for(int k=0;k<3;k++){v.position[k]=p[i*3+k];v.normal[k]=n.size()>i*3+k?n[i*3+k]:(k==1?1.0f:0.0f);}v.uv[0]=t.size()>i*2?t[i*2]:0;v.uv[1]=t.size()>i*2+1?t[i*2+1]:0;}
        if(primitive.indices){mesh.indices.resize(primitive.indices->count);cgltf_accessor_unpack_indices(primitive.indices,mesh.indices.data(),sizeof(uint32_t),mesh.indices.size());}
        else{mesh.indices.resize(mesh.vertices.size());for(uint32_t i=0;i<mesh.indices.size();i++)mesh.indices[i]=i;}
        if(primitive.material){auto& src=*primitive.material;auto& dst=mesh.material;
            if(src.has_pbr_metallic_roughness){auto& pbr=src.pbr_metallic_roughness;for(int k=0;k<4;k++)dst.baseColor[k]=pbr.base_color_factor[k];dst.metallic=pbr.metallic_factor;dst.roughness=pbr.roughness_factor;dst.albedoPath=imagePath(pbr.base_color_texture);dst.metallicRoughnessPath=imagePath(pbr.metallic_roughness_texture);}
            dst.normalPath=imagePath(src.normal_texture);dst.name=src.name?src.name:"gltf-material";
        }
        if(!mesh.indices.empty())out.meshes.push_back(std::move(mesh));
    }
    cgltf_free(data);
    if(out.meshes.empty()){error="glTF sem primitivas triangulares com POSITION";return false;}
    return true;
}
}

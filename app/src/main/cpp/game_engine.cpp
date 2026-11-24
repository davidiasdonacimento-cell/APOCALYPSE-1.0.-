#include <jni.h>
#include <android/asset_manager_jni.h>
#include "model_renderer.h"
#include <GLES3/gl3.h>
#include <android/log.h>
#include <vector>
#include <cmath>
#include <time.h>
#include <cstdio>
#include "survival_system.h"
#include "math3d.h"
#include "verity_companion.h"
static double androidGetCurrentTimeNanos(){struct timespec t{};clock_gettime(CLOCK_MONOTONIC,&t);return (double)t.tv_sec*1000000000.0+(double)t.tv_nsec;}

#define LOG_TAG "APOCALYPSE_ENGINE"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace {
struct Vertex { float x,y,z,r,g,b,u,v,material; };
GLuint program=0, vertexBuffer=0, playerBuffer=0, survivalBuffer=0;
GLuint surfaceTextures[3]={0,0,0};
GLint vertexCount=0, playerVertexCount=0;
int surfaceWidth=1, surfaceHeight=1;
float playerX=0.0f, playerZ=0.0f, moveX=0.0f, moveZ=0.0f;
std::vector<Vertex> world, playerMesh;
struct WorldCollider { float minX, maxX, minZ, maxZ; };
std::vector<WorldCollider> worldColliders;
apocalypse::SurvivalSystem survival;
apocalypse::assets::ModelRenderer importedModels;

Vertex v(float x,float y,float z,float r,float g,float b){return {x,y,z,r,g,b,0,0,3};}
Vertex vt(float x,float y,float z,float r,float g,float b,float u,float vv,float m){return {x,y,z,r,g,b,u,vv,m};}
void triangle(Vertex a,Vertex b,Vertex c){world.push_back(a);world.push_back(b);world.push_back(c);}
void quad(Vertex a,Vertex b,Vertex c,Vertex d){triangle(a,b,c);triangle(a,c,d);}

// Malhas orgânicas trianguladas: folhagem, troncos, pneus e anatomia.
template<class Sink>
void ellipsoid(Sink& out,float cx,float cy,float cz,float rx,float ry,float rz,
               float r,float g,float b,int rings=10,int segments=14){
    const float pi=3.14159265358979323846f;
    for(int iy=0;iy<rings;iy++){
        float p0=-pi*0.5f+pi*(float)iy/(float)rings;
        float p1=-pi*0.5f+pi*(float)(iy+1)/(float)rings;
        for(int ix=0;ix<segments;ix++){
            float t0=2.0f*pi*(float)ix/(float)segments;
            float t1=2.0f*pi*(float)(ix+1)/(float)segments;
            auto point=[&](float p,float t){
                float cp=cosf(p);
                return v(cx+rx*cp*cosf(t),cy+ry*sinf(p),cz+rz*cp*sinf(t),r,g,b);
            };
            Vertex a=point(p0,t0),bb=point(p0,t1),c=point(p1,t1),d=point(p1,t0);
            out.push_back(a);out.push_back(bb);out.push_back(c);
            out.push_back(a);out.push_back(c);out.push_back(d);
        }
    }
}
template<class Sink>
void taperedCylinder(Sink& out,float x,float y,float z,float radiusBottom,
                     float radiusTop,float height,float r,float g,float b,int segments=12){
    const float pi=3.14159265358979323846f;
    for(int i=0;i<segments;i++){
        float a0=2.0f*pi*(float)i/(float)segments;
        float a1=2.0f*pi*(float)(i+1)/(float)segments;
        Vertex p0=v(x+cosf(a0)*radiusBottom,y,z+sinf(a0)*radiusBottom,r*.72f,g*.72f,b*.72f);
        Vertex p1=v(x+cosf(a1)*radiusBottom,y,z+sinf(a1)*radiusBottom,r*.72f,g*.72f,b*.72f);
        Vertex q0=v(x+cosf(a0)*radiusTop,y+height,z+sinf(a0)*radiusTop,r,g,b);
        Vertex q1=v(x+cosf(a1)*radiusTop,y+height,z+sinf(a1)*radiusTop,r,g,b);
        out.push_back(p0);out.push_back(p1);out.push_back(q1);
        out.push_back(p0);out.push_back(q1);out.push_back(q0);
    }
}
void organicTree(float x,float z,float scale=1.0f){
    taperedCylinder(world,x,0.12f,z,0.13f*scale,0.085f*scale,1.45f*scale,0.28f,0.17f,0.10f);
    ellipsoid(world,x,1.72f*scale+0.12f,z,0.72f*scale,0.78f*scale,0.68f*scale,0.12f,0.27f,0.13f,12,16);
    ellipsoid(world,x-0.38f*scale,1.48f*scale+0.12f,z+0.10f*scale,0.48f*scale,0.52f*scale,0.46f*scale,0.15f,0.31f,0.15f,10,14);
    ellipsoid(world,x+0.36f*scale,1.55f*scale+0.12f,z-0.08f*scale,0.46f*scale,0.54f*scale,0.45f*scale,0.10f,0.23f,0.12f,10,14);
}
float terrainHeight(float x,float z){
    // Subtle, deterministic undulation; roads and building pads remain level.
    return 0.045f*std::sin(x*0.31f)*std::cos(z*0.27f)
         + 0.025f*std::sin((x+z)*0.53f);
}
void addTerrain(){
    const int cells=144; const float start=-112.0f, step=224.0f/(float)cells;
    for(int ix=0;ix<cells;ix++) for(int iz=0;iz<cells;iz++){
        float x=start+ix*step,z=start+iz*step;
        float h1=terrainHeight(x,z),h2=terrainHeight(x+step,z);
        float h3=terrainHeight(x+step,z+step),h4=terrainHeight(x,z+step);
        float tint=((ix*7+iz*11)%9)*0.003f;
        float r=0.15f+tint,g=0.205f+tint,b=0.145f+tint;
        quad(vt(x,h1,z,r,g,b,0,0,0),vt(x+step,h2,z,r,g,b,step,0,0),
             vt(x+step,h3,z+step,r*0.98f,g*0.98f,b,step,step,0),
             vt(x,h4,z+step,r*0.96f,g*0.96f,b,0,step,0));
    }
}
void ground(float x,float z,float w,float d,float r,float g,float b,float y=0.105f){
    float m=(g>r*1.18f)?0.0f:((r<0.20f&&g<0.23f)?2.0f:1.0f); float t=0.65f;
    quad(vt(x,y,z,r,g,b,0,0,m),vt(x+w,y,z,r,g,b,w*t,0,m),vt(x+w,y,z+d,r,g,b,w*t,d*t,m),vt(x,y,z+d,r,g,b,0,d*t,m));
}
void sidewalkH(float x,float z,float w,float d){
    ground(x,z,w,d,0.39f,0.39f,0.35f,0.125f);
}
void sidewalkV(float x,float z,float w,float d){
    ground(x,z,w,d,0.39f,0.39f,0.35f,0.125f);
}
void building(float x,float z,float w,float d,float h,float r,float g,float b){
    float x2=x+w,z2=z+d,y=0.12f;
    // A colisão usa o volume físico da construção, independente da malha visual.
    worldColliders.push_back({x-0.12f,x+w+0.12f,z-0.12f,z+d+0.12f});
    Vertex a=v(x,y,z,r*.52f,g*.52f,b*.52f), b0=v(x2,y,z,r*.58f,g*.58f,b*.58f);
    Vertex c=v(x2,y,z2,r*.48f,g*.48f,b*.48f), d0=v(x,y,z2,r*.50f,g*.50f,b*.50f);
    Vertex A=v(x,y+h,z,r,g,b), B=v(x2,y+h,z,r*.92f,g*.92f,b*.92f);
    Vertex C=v(x2,y+h,z2,r*.82f,g*.82f,b*.82f), D=v(x,y+h,z2,r*.88f,g*.88f,b*.88f);
    quad(A,B,C,D);quad(a,b0,B,A);quad(b0,c,C,B);quad(c,d0,D,C);quad(d0,a,A,D);
}
void facadeWindow(float x,float y,float z,float w,float h,float r=0.10f,float g=0.19f,float b=0.23f){
    quad(v(x,y,z,r,g,b),v(x+w,y,z,r,g,b),v(x+w,y+h,z,r*0.82f,g*0.82f,b*0.82f),v(x,y+h,z,r*0.82f,g*0.82f,b*0.82f));
}
void detailedBuilding(float x,float z,float w,float d,float h,float r,float g,float b,int floors){
    building(x,z,w,d,h,r,g,b);
    const float floorHeight=h/(float)floors;
    // Recessed-looking front windows and a visible entrance.
    for(int floor=0;floor<floors;floor++){
        float wy=0.34f+floor*floorHeight;
        for(float wx=x+0.30f;wx<x+w-0.35f;wx+=0.78f){
            if(wx+0.42f>x+w-0.12f) break;
            facadeWindow(wx,wy,z-0.012f,0.42f,0.34f);
            // Narrow sill directly below each front-facing window.
            facadeWindow(wx,wy-0.055f,z-0.014f,0.42f,0.045f,0.48f,0.49f,0.46f);
        }
    }
    // Entrance door on the street-facing side.
    facadeWindow(x+w*0.42f,0.13f,z-0.018f,w*0.20f,0.78f,0.12f,0.10f,0.08f);
    // Roof parapet and ventilation equipment.
    building(x-0.04f,z-0.04f,w+0.08f,d+0.08f,0.16f,0.34f,0.35f,0.34f);
    building(x+w*0.62f,z+d*0.55f,0.55f,0.48f,0.38f,0.28f,0.29f,0.28f);
}
void streetLight(float x,float z){
    taperedCylinder(world,x,0.12f,z,0.075f,0.045f,3.05f,0.20f,0.22f,0.22f,10);
    taperedCylinder(world,x,3.12f,z-0.38f,0.045f,0.035f,0.08f,0.20f,0.22f,0.22f,10);
    ellipsoid(world,x,3.12f,z-0.70f,0.20f,0.10f,0.15f,0.92f,0.78f,0.48f,8,12);
}
void abandonedCar(float x,float z,float bodyR,float bodyG,float bodyB){
    building(x,z,1.55f,2.65f,0.62f,bodyR,bodyG,bodyB);
    building(x+0.20f,z+0.38f,1.15f,1.45f,0.48f,0.16f,0.22f,0.25f);
    // Pneus arredondados, sem rodas cúbicas.
    ellipsoid(world,x+0.02f,0.39f,z+0.52f,0.16f,0.19f,0.30f,0.045f,0.05f,0.055f,10,14);
    ellipsoid(world,x+1.43f,0.39f,z+0.52f,0.16f,0.19f,0.30f,0.045f,0.05f,0.055f,10,14);
    ellipsoid(world,x+0.02f,0.39f,z+2.10f,0.16f,0.19f,0.30f,0.045f,0.05f,0.055f,10,14);
    ellipsoid(world,x+1.43f,0.39f,z+2.10f,0.16f,0.19f,0.30f,0.045f,0.05f,0.055f,10,14);
}
void addStreetDetails(){
    // Broken lane markings, crossings and drainage strips.
    for(int p=-21;p<=21;p+=6){
        ground((float)p,-0.055f,2.0f,0.11f,0.72f,0.69f,0.53f,0.137f);
        ground(-0.055f,(float)p,0.11f,2.0f,0.72f,0.69f,0.53f,0.138f);
    }
    for(int side=-1;side<=1;side+=2){
        for(int p=-18;p<=18;p+=9){
            streetLight((float)p+side*3.1f,-3.0f);
            streetLight((float)p+side*3.1f,3.0f);
        }
    }
    abandonedCar(-7.0f,-1.0f,0.34f,0.20f,0.14f);
    abandonedCar(4.0f,1.0f,0.20f,0.25f,0.30f);
    abandonedCar(-16.0f,8.0f,0.30f,0.29f,0.22f);
    // Rubble piles and broken concrete barriers.
    for(int i=0;i<18;i++){
        float x=-21.0f+(float)((i*7)%43);
        float z=-20.0f+(float)((i*13)%41);
        building(x,z,0.34f+(i%3)*0.12f,0.30f+(i%2)*0.16f,0.18f+(i%4)*0.08f,0.28f,0.27f,0.24f);
    }
}
void addResidentialDetails(){
    // Small trees, fences and utility structures in selected lots.
    for(int i=0;i<22;i++){
        float x=-22.0f+(float)((i*11)%43);
        float z=-20.0f+(float)((i*17)%39);
        organicTree(x,z,0.78f+0.08f*(i%4));
    }
    for(int i=0;i<10;i++){
        float x=-20.0f+(float)(i*4.0f),z=13.5f;
        building(x,z,0.12f,0.12f,1.35f,0.28f,0.29f,0.27f);
        building(x+0.12f,z,0.45f,0.10f,0.85f,0.34f,0.32f,0.27f);
    }
}


void addExplorableClinic(){
    // Small ground-floor clinic with a real doorway and interior floor.
    const float x=13.0f,z=10.5f,w=6.2f,d=6.0f,h=2.75f,t=0.22f,door=1.15f;
    ground(x,z,w,d,0.34f,0.35f,0.33f,0.145f);
    // Rear and side walls; front wall is split to leave a walkable entrance.
    building(x,z+d-t,w,t,h,0.67f,0.68f,0.64f);
    building(x,z,t,d,h,0.62f,0.64f,0.61f);
    building(x+w-t,z,t,d,h,0.62f,0.64f,0.61f);
    building(x,z,(w-door)*0.5f,t,h,0.67f,0.68f,0.64f);
    building(x+(w+door)*0.5f,z,(w-door)*0.5f,t,h,0.67f,0.68f,0.64f);
    // Interior partitions, reception counter, beds and storage.
    building(x+2.75f,z+2.1f,0.14f,2.0f,2.25f,0.58f,0.59f,0.56f);
    building(x+0.75f,z+1.1f,1.65f,0.62f,0.78f,0.42f,0.45f,0.43f);
    building(x+3.65f,z+3.35f,1.35f,0.62f,0.42f,0.72f,0.73f,0.69f);
    building(x+0.75f,z+4.35f,1.2f,0.72f,0.58f,0.43f,0.46f,0.44f);
    // Exterior sign and awning.
    ground(x+1.8f,z-0.38f,2.6f,0.22f,0.72f,0.76f,0.70f,2.25f);
}
void addForestRegion(){
    // Contaminated woodland east of the metro; layered canopy and clearings.
    ground(38.0f,-22.0f,42.0f,44.0f,0.12f,0.20f,0.12f,0.115f);
    for(int i=0;i<88;i++){
        float x=40.0f+(float)((i*17)%39);
        float z=-20.0f+(float)((i*29)%40);
        if((x>49.0f&&x<64.0f&&z>-7.0f&&z<8.0f)) continue;
        organicTree(x,z,0.85f+0.12f*(i%5));
        if(i%4==0) ellipsoid(world,x+0.4f,0.24f,z+0.25f,0.34f,0.18f,0.28f,0.22f,0.25f,0.18f,7,10);
    }
    // Forest service road and a derelict ranger outpost.
    ground(28.0f,-2.1f,57.0f,4.2f,0.16f,0.17f,0.15f,0.145f);
    detailedBuilding(57.0f,-3.8f,5.2f,4.4f,2.8f,0.31f,0.29f,0.22f,1);
    building(55.9f,-4.4f,1.0f,0.7f,0.35f,0.23f,0.25f,0.19f);
}
void addMilitaryRegion(){
    // Fenced military perimeter with hangar, barracks and watch posts.
    ground(42.0f,-91.0f,39.0f,34.0f,0.23f,0.24f,0.22f,0.12f);
    ground(28.0f,-65.0f,58.0f,4.0f,0.17f,0.18f,0.17f,0.15f);
    detailedBuilding(51.0f,-88.0f,15.0f,9.0f,5.2f,0.31f,0.34f,0.35f,1);
    detailedBuilding(44.0f,-77.0f,7.0f,4.2f,3.0f,0.38f,0.36f,0.29f,2);
    detailedBuilding(65.0f,-77.0f,7.0f,4.2f,3.0f,0.38f,0.36f,0.29f,2);
    for(int i=0;i<15;i++){
        float x=43.0f+i*2.35f;
        building(x,-92.0f,0.12f,0.12f,2.2f,0.32f,0.34f,0.31f);
        building(x,-58.0f,0.12f,0.12f,2.2f,0.32f,0.34f,0.31f);
    }
    for(int i=0;i<7;i++){
        float z=-90.0f+i*5.0f;
        building(42.0f,z,0.12f,0.12f,2.2f,0.32f,0.34f,0.31f);
        building(78.0f,z,0.12f,0.12f,2.2f,0.32f,0.34f,0.31f);
    }
    taperedCylinder(world,46.0f,0.12f,-86.0f,0.18f,0.12f,7.0f,0.35f,0.37f,0.35f,12);
    ellipsoid(world,46.0f,7.2f,-86.0f,0.55f,0.32f,0.55f,0.38f,0.42f,0.38f,10,14);
}
void addRuralRegion(){
    // Rural settlement southwest, connected by a secondary paved road.
    ground(-93.0f,39.0f,42.0f,42.0f,0.27f,0.24f,0.16f,0.12f);
    ground(-112.0f,29.0f,72.0f,3.8f,0.22f,0.20f,0.15f,0.15f);
    for(int i=0;i<9;i++){
        float x=-88.0f+(float)(i%3)*12.0f;
        float z=45.0f+(float)(i/3)*11.0f;
        detailedBuilding(x,z,5.2f,4.8f,2.8f+(i%2)*0.35f,0.49f,0.39f,0.27f,1);
        building(x+5.5f,z+1.0f,2.8f,2.6f,1.8f,0.39f,0.30f,0.20f);
        organicTree(x-2.0f,z+2.0f,0.9f);
        organicTree(x+7.0f,z+5.0f,1.05f);
    }
    // Fields, crop rows and a small water channel.
    for(int row=0;row<8;row++){
        ground(-106.0f+row*4.2f,84.0f,0.22f,14.0f,0.31f,0.34f,0.16f,0.132f);
    }
    ground(-103.0f,98.0f,28.0f,2.0f,0.12f,0.25f,0.30f,0.13f);
}
void addConnectedRegions(){
    addExplorableClinic();
    // Continuous road corridors link the four authored districts.
    ground(24.0f,-2.0f,22.0f,4.0f,0.15f,0.16f,0.15f,0.145f);
    ground(44.0f,-66.0f,4.0f,66.0f,0.15f,0.16f,0.15f,0.145f);
    ground(-88.0f,24.0f,112.0f,4.0f,0.20f,0.19f,0.15f,0.145f);
    addForestRegion();
    addMilitaryRegion();
    addRuralRegion();
}

void addSidewalkNetwork(){
    // Main avenue sidewalks on both sides.
    sidewalkH(-24,-2.85f,48,0.65f); sidewalkH(-24,2.20f,48,0.65f);
    sidewalkV(-2.85f,-24,0.65f,48); sidewalkV(2.20f,-24,0.65f,48);
    // Side-street sidewalks.
    for(int p=-18;p<=18;p+=9){
        sidewalkH(-24,(float)p-1.15f,48,0.35f);
        sidewalkH(-24,(float)p+0.80f,48,0.35f);
        sidewalkV((float)p-1.15f,-24,0.35f,48);
        sidewalkV((float)p+0.80f,-24,0.35f,48);
    }
}
void addPlots(){
    // Raised, level building pads and vacant lots between streets.
    for(int x=-20;x<=16;x+=9) for(int z=-20;z<=16;z+=9){
        if((x==-2&&z==-2)||(x==7&&z==-2)) continue;
        ground((float)x,(float)z,7.0f,7.0f,0.24f,0.245f,0.22f,0.115f);
        // Dirt / rubble patches on some vacant corners.
        if(((x+z)/9)%2==0) ground(x+0.25f,z+0.25f,1.8f,1.4f,0.30f,0.24f,0.16f,0.122f);
    }
}
void buildCity(){
    world.clear();
    worldColliders.clear();
    addTerrain();
    // Asphalt roads above terrain to avoid z-fighting.
    ground(-24,-2.2f,48,4.4f,0.105f,0.115f,0.13f,0.13f);
    ground(-2.2f,-24,4.4f,48,0.105f,0.115f,0.13f,0.13f);
    for(int p=-18;p<=18;p+=9){
        ground(-24,(float)p-0.75f,48,1.5f,0.17f,0.18f,0.18f,0.132f);
        ground((float)p-0.75f,-24,1.5f,48,0.17f,0.18f,0.18f,0.132f);
    }
    addSidewalkNetwork();
    addPlots();
    // Distinct residential, commercial and industrial building silhouettes.
    for(int x=-20;x<=16;x+=9) for(int z=-20;z<=16;z+=9){
        if((x==-2&&z==-2)||(x==7&&z==-2)) continue;
        const int pattern=((x+20)/9+(z+20)/9)%4;
        if(pattern==0){
            detailedBuilding(x+0.5f,z+0.6f,2.4f,2.5f,2.0f,0.30f,0.34f,0.38f,2);
            detailedBuilding(x+3.4f,z+0.8f,2.7f,2.0f,3.8f,0.25f,0.31f,0.37f,3);
        }else if(pattern==1){
            detailedBuilding(x+0.6f,z+0.7f,5.4f,2.5f,2.7f,0.38f,0.31f,0.24f,2);
            building(x+4.4f,z+3.8f,1.5f,1.6f,1.2f,0.28f,0.30f,0.32f);
        }else if(pattern==2){
            detailedBuilding(x+0.5f,z+0.6f,2.2f,2.4f,5.0f,0.29f,0.33f,0.39f,4);
            detailedBuilding(x+3.1f,z+0.8f,2.8f,2.1f,2.1f,0.42f,0.36f,0.27f,2);
        }else{
            detailedBuilding(x+0.7f,z+0.7f,4.8f,4.7f,1.65f,0.34f,0.30f,0.26f,1);
        }
    }
    // Hospital pad, emergency wing and service annex.
    ground(5.0f,5.0f,7.0f,7.0f,0.29f,0.30f,0.28f,0.14f);
    detailedBuilding(6.0f,6.0f,4.8f,4.5f,5.2f,0.58f,0.61f,0.62f,4);
    building(5.4f,9.0f,1.0f,1.1f,1.8f,0.72f,0.72f,0.68f);
    // Contaminated park with trees, paths and benches.
    ground(-20.0f,5.0f,7.0f,7.0f,0.10f,0.22f,0.12f,0.14f);
    ground(-19.4f,8.0f,5.8f,0.55f,0.42f,0.39f,0.29f,0.15f);
    for(int i=0;i<8;i++){
        float tx=-19.0f+(i%4)*1.6f,tz=6.0f+(i/4)*3.0f;
        organicTree(tx,tz,0.82f+0.06f*(i%3));
    }
    addStreetDetails();
    addResidentialDetails();
    addConnectedRegions();
}

bool collidesWithWorld(float x,float z){
    constexpr float radius=0.34f;
    for(const auto& box:worldColliders){
        const float nearestX=fmaxf(box.minX,fminf(x,box.maxX));
        const float nearestZ=fmaxf(box.minZ,fminf(z,box.maxZ));
        const float dx=x-nearestX,dz=z-nearestZ;
        if(dx*dx+dz*dz<radius*radius) return true;
    }
    return false;
}
void movePlayerWithCollision(float dx,float dz){
    // Resolve os eixos separadamente para permitir deslizar pelas paredes.
    const float nextX=fmaxf(-108.0f,fminf(108.0f,playerX+dx));
    if(!collidesWithWorld(nextX,playerZ)) playerX=nextX;
    const float nextZ=fmaxf(-108.0f,fminf(108.0f,playerZ+dz));
    if(!collidesWithWorld(playerX,nextZ)) playerZ=nextZ;
}
void addPlayerModel(){
    playerMesh.clear();
    // Sobrevivente: botas reforçadas, calça, joelheiras e jaqueta em camadas.
    ellipsoid(playerMesh,-0.13f,0.13f,-0.035f,0.16f,0.12f,0.24f,0.075f,0.085f,0.09f);
    ellipsoid(playerMesh, 0.13f,0.13f,-0.035f,0.16f,0.12f,0.24f,0.075f,0.085f,0.09f);
    taperedCylinder(playerMesh,-0.13f,0.24f,0.0f,0.105f,0.12f,0.31f,0.20f,0.24f,0.19f);
    taperedCylinder(playerMesh, 0.13f,0.24f,0.0f,0.105f,0.12f,0.31f,0.20f,0.24f,0.19f);
    ellipsoid(playerMesh,-0.13f,0.48f,0.0f,0.13f,0.09f,0.13f,0.12f,0.16f,0.13f);
    ellipsoid(playerMesh, 0.13f,0.48f,0.0f,0.13f,0.09f,0.13f,0.12f,0.16f,0.13f);
    ellipsoid(playerMesh,0.0f,0.76f,0.0f,0.29f,0.20f,0.19f,0.15f,0.19f,0.15f,12,16);
    ellipsoid(playerMesh,0.0f,0.91f,0.0f,0.32f,0.28f,0.21f,0.22f,0.29f,0.22f,14,18);
    // Colete, bolsos, zíper e alças frontais.
    ellipsoid(playerMesh,0.0f,0.91f,-0.155f,0.235f,0.23f,0.075f,0.12f,0.16f,0.14f,10,14);
    ellipsoid(playerMesh,-0.13f,0.79f,-0.222f,0.075f,0.075f,0.035f,0.35f,0.31f,0.20f);
    ellipsoid(playerMesh, 0.13f,0.79f,-0.222f,0.075f,0.075f,0.035f,0.35f,0.31f,0.20f);
    ellipsoid(playerMesh,0.0f,0.91f,-0.232f,0.012f,0.19f,0.012f,0.58f,0.53f,0.36f,6,8);
    // Mochila compacta visível por trás do ombro.
    ellipsoid(playerMesh,0.0f,0.92f,0.205f,0.22f,0.29f,0.13f,0.11f,0.15f,0.13f,10,12);
    ellipsoid(playerMesh,-0.37f,0.88f,0.0f,0.12f,0.20f,0.13f,0.19f,0.25f,0.19f);
    ellipsoid(playerMesh, 0.37f,0.88f,0.0f,0.12f,0.20f,0.13f,0.19f,0.25f,0.19f);
    taperedCylinder(playerMesh,-0.38f,0.52f,0.0f,0.085f,0.075f,0.29f,0.20f,0.25f,0.19f);
    taperedCylinder(playerMesh, 0.38f,0.52f,0.0f,0.085f,0.075f,0.29f,0.20f,0.25f,0.19f);
    ellipsoid(playerMesh,-0.38f,0.48f,-0.01f,0.09f,0.08f,0.09f,0.12f,0.13f,0.11f);
    ellipsoid(playerMesh, 0.38f,0.48f,-0.01f,0.09f,0.08f,0.09f,0.12f,0.13f,0.11f);
    ellipsoid(playerMesh,0.0f,1.22f,0.0f,0.17f,0.20f,0.16f,0.70f,0.53f,0.39f,12,16);
    // Cabelo, orelhas e traços do rosto voltados para a direção de avanço.
    ellipsoid(playerMesh,0.0f,1.36f,0.015f,0.18f,0.10f,0.17f,0.13f,0.10f,0.075f,10,14);
    ellipsoid(playerMesh,-0.17f,1.24f,0.015f,0.045f,0.12f,0.12f,0.13f,0.10f,0.075f);
    ellipsoid(playerMesh, 0.17f,1.24f,0.015f,0.045f,0.12f,0.12f,0.13f,0.10f,0.075f);
    ellipsoid(playerMesh,-0.065f,1.25f,-0.145f,0.018f,0.014f,0.012f,0.07f,0.08f,0.07f,6,8);
    ellipsoid(playerMesh, 0.065f,1.25f,-0.145f,0.018f,0.014f,0.012f,0.07f,0.08f,0.07f,6,8);
    ellipsoid(playerMesh,0.0f,1.20f,-0.16f,0.025f,0.035f,0.025f,0.58f,0.40f,0.31f,6,8);
}


void addBox(std::vector<Vertex>& out,float x,float y,float z,float w,float h,float d,float r,float g,float b){
    float x2=x+w,y2=y+h,z2=z+d;
    Vertex a=v(x,y,z,r*.62f,g*.62f,b*.62f),b0=v(x2,y,z,r*.62f,g*.62f,b*.62f);
    Vertex c=v(x2,y,z2,r*.62f,g*.62f,b*.62f),d0=v(x,y,z2,r*.62f,g*.62f,b*.62f);
    Vertex A=v(x,y2,z,r,g,b),B=v(x2,y2,z,r,g,b),C=v(x2,y2,z2,r*.9f,g*.9f,b*.9f),D=v(x,y2,z2,r*.9f,g*.9f,b*.9f);
    auto tri=[&](Vertex p,Vertex q,Vertex t){out.push_back(p);out.push_back(q);out.push_back(t);};
    auto face=[&](Vertex p,Vertex q,Vertex t,Vertex u){tri(p,q,t);tri(p,t,u);};
    face(A,B,C,D);face(a,b0,B,A);face(b0,c,C,B);face(c,d0,D,C);face(d0,a,A,D);
}
void buildSurvivalMesh(std::vector<Vertex>& mesh){
    mesh.clear();
    for(const auto& z:survival.zombies()){
        if(z.state==2) continue;
        // Zumbi humanoide 3D: pelvis, tronco, pescoço, cabeça, braços, pernas e calçados.
        // Todas as peças são malhas volumétricas trianguladas; nenhum billboard/PNG é usado.
        const float skinR=0.39f,skinG=0.46f,skinB=0.30f;
        const float gait=sinf(z.patrolPhase*7.0f);
        ellipsoid(mesh,z.x,0.67f,z.z,0.22f,0.16f,0.17f,0.20f,0.25f,0.18f,10,14);
        ellipsoid(mesh,z.x,0.98f,z.z,0.29f,0.31f,0.19f,0.23f,0.31f,0.22f,12,16);
        ellipsoid(mesh,z.x,1.27f,z.z-0.015f,0.105f,0.12f,0.11f,skinR*0.82f,skinG*0.82f,skinB*0.82f,8,12);
        ellipsoid(mesh,z.x,1.47f,z.z-0.025f,0.19f,0.215f,0.17f,skinR,skinG,skinB,12,16);
        // Ombros caídos, braços em duas secções e mãos.
        ellipsoid(mesh,z.x-0.30f,1.10f,z.z,0.145f,0.15f,0.15f,0.25f,0.33f,0.22f,9,12);
        ellipsoid(mesh,z.x+0.30f,1.10f,z.z,0.145f,0.15f,0.15f,0.25f,0.33f,0.22f,9,12);
        ellipsoid(mesh,z.x-0.39f,0.88f+gait*0.065f,z.z-0.015f,0.105f,0.24f,0.105f,skinR,skinG,skinB,9,12);
        ellipsoid(mesh,z.x+0.39f,0.88f-gait*0.065f,z.z+0.025f,0.105f,0.24f,0.105f,skinR,skinG,skinB,9,12);
        ellipsoid(mesh,z.x-0.42f,0.64f,z.z-0.035f,0.09f,0.105f,0.09f,skinR*0.85f,skinG*0.85f,skinB*0.85f,8,10);
        ellipsoid(mesh,z.x+0.42f,0.64f,z.z+0.005f,0.09f,0.105f,0.09f,skinR*0.85f,skinG*0.85f,skinB*0.85f,8,10);
        // Calças rasgadas, joelhos, canelas e botas separados.
        ellipsoid(mesh,z.x-0.12f,0.39f+gait*0.085f,z.z,0.115f,0.28f,0.12f,0.12f,0.16f,0.14f,10,12);
        ellipsoid(mesh,z.x+0.12f,0.39f-gait*0.085f,z.z,0.115f,0.28f,0.12f,0.12f,0.16f,0.14f,10,12);
        ellipsoid(mesh,z.x-0.12f,0.12f+gait*0.085f,z.z-0.055f,0.14f,0.085f,0.20f,0.075f,0.085f,0.075f,8,12);
        ellipsoid(mesh,z.x+0.12f,0.12f-gait*0.085f,z.z-0.055f,0.14f,0.085f,0.20f,0.075f,0.085f,0.075f,8,12);
        // Costuras/rasgos do casaco, olhos e boca dão leitura facial à distância.
        ellipsoid(mesh,z.x,1.00f,z.z-0.178f,0.045f,0.23f,0.018f,0.12f,0.18f,0.13f,8,10);
        ellipsoid(mesh,z.x-0.072f,1.49f,z.z-0.176f,0.026f,0.018f,0.012f,0.12f,0.035f,0.025f,6,8);
        ellipsoid(mesh,z.x+0.072f,1.49f,z.z-0.176f,0.026f,0.018f,0.012f,0.12f,0.035f,0.025f,6,8);
        ellipsoid(mesh,z.x,1.385f,z.z-0.179f,0.035f,0.012f,0.012f,0.18f,0.055f,0.045f,6,8);
    }
    for(const auto& p:survival.pickups()){
        if(!p.active) continue;
        float r=0.25f,g=0.65f,b=0.85f;
        if(p.type==1){r=0.85f;g=0.62f;b=0.20f;}
        if(p.type==2){r=0.65f;g=0.65f;b=0.68f;}
        if(p.type==3){r=0.25f;g=0.85f;b=0.40f;}
        addBox(mesh,p.x-0.18f,0.16f,p.z-0.18f,0.36f,0.36f,0.36f,r,g,b);
    }
}

GLuint compileShader(GLenum type,const char* src){
    GLuint s=glCreateShader(type);glShaderSource(s,1,&src,nullptr);glCompileShader(s);
    GLint ok=GL_FALSE;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(ok!=GL_TRUE){char log[1024]={};glGetShaderInfoLog(s,sizeof(log),nullptr,log);LOGE("Shader compile: %s",log);glDeleteShader(s);return 0;}
    return s;
}
}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeInit(JNIEnv* env,jclass,jobject javaAssets){
    const char* vs="#version 300 es\nlayout(location=0) in vec3 aPos; layout(location=1) in vec3 aColor; layout(location=2) in vec2 aUV; layout(location=3) in float aMaterial; uniform mat4 uMVP; out vec3 color; out vec3 worldPosition; out vec2 uv; flat out int materialId; void main(){worldPosition=aPos;gl_Position=uMVP*vec4(aPos,1.0);color=aColor;uv=aUV;materialId=int(aMaterial+0.5);}";
    const char* fs="#version 300 es\nprecision mediump float;in vec3 color;in vec3 worldPosition;in vec2 uv;flat in int materialId;uniform sampler2D uGrass;uniform sampler2D uConcrete;uniform sampler2D uAsphalt;out vec4 fragColor;void main(){vec3 texel=vec3(1.0);if(materialId==0)texel=texture(uGrass,uv).rgb;else if(materialId==1)texel=texture(uConcrete,uv).rgb;else if(materialId==2)texel=texture(uAsphalt,uv).rgb;vec3 normal=normalize(cross(dFdx(worldPosition),dFdy(worldPosition)));vec3 lightDir=normalize(vec3(-0.35,0.82,0.44));float diffuse=0.25+0.48*abs(dot(normal,lightDir));float distanceFog=clamp((gl_FragCoord.z-0.90)*2.8,0.0,0.20);vec3 lit=color*texel*diffuse*0.88;vec3 fog=vec3(0.10,0.14,0.16);fragColor=vec4(mix(lit,fog,distanceFog),1.0);}";
    GLuint a=compileShader(GL_VERTEX_SHADER,vs),b=compileShader(GL_FRAGMENT_SHADER,fs);
    if(!a||!b)return;
    program=glCreateProgram();glAttachShader(program,a);glAttachShader(program,b);glLinkProgram(program);
    glDeleteShader(a);glDeleteShader(b);
    GLint linked=GL_FALSE;glGetProgramiv(program,GL_LINK_STATUS,&linked);
    if(linked!=GL_TRUE){char log[1024]={};glGetProgramInfoLog(program,sizeof(log),nullptr,log);LOGE("Program link: %s",log);glDeleteProgram(program);program=0;return;}
    buildCity();
    survival.reset();
    if(!importedModels.initialize(AAssetManager_fromJava(env,javaAssets))) LOGE("Falha ao inicializar modelos OBJ/PBR");
    apocalypse::initializeVerityCompanion();
    // Three small tileable starter material maps generated in memory: no unused APK files.
    const unsigned char base[3][3]={{48,72,43},{112,108,96},{48,52,56}};
    for(int k=0;k<3;k++){
        unsigned char pixels[64*64*3];
        for(int y=0;y<64;y++)for(int x=0;x<64;x++){
            unsigned int n=(unsigned int)((x*73856093u)^(y*19349663u)^(k*83492791u));
            n^=n>>13;n*=1274126177u;n^=n>>16;
            int grain=(int)(n%25u)-12;int at=(y*64+x)*3;
            for(int c=0;c<3;c++)pixels[at+c]=(unsigned char)fmaxf(0.0f,fminf(255.0f,(float)base[k][c]+grain));
        }
        glGenTextures(1,&surfaceTextures[k]);glBindTexture(GL_TEXTURE_2D,surfaceTextures[k]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,64,64,0,GL_RGB,GL_UNSIGNED_BYTE,pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    glGenBuffers(1,&vertexBuffer);glBindBuffer(GL_ARRAY_BUFFER,vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(world.size()*sizeof(Vertex)),world.data(),GL_STATIC_DRAW);
    vertexCount=(GLint)world.size();
    addPlayerModel();
    glGenBuffers(1,&playerBuffer);glBindBuffer(GL_ARRAY_BUFFER,playerBuffer);
    glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(playerMesh.size()*sizeof(Vertex)),playerMesh.data(),GL_STATIC_DRAW);
    playerVertexCount=(GLint)playerMesh.size();
    glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);
}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeSetMove(JNIEnv*,jclass,jfloat x,jfloat z){moveX=x;moveZ=z;}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeResize(JNIEnv*,jclass,jint w,jint h){
    surfaceWidth=w>0?w:1;surfaceHeight=h>0?h:1;glViewport(0,0,surfaceWidth,surfaceHeight);
}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeRender(JNIEnv*,jclass){
    glViewport(0,0,surfaceWidth,surfaceHeight);
    glClearColor(0.035f,0.055f,0.075f,1.0f);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    if(!program||!vertexBuffer||vertexCount<=0)return;
    static double lastTime=0.0;
    double now=(double)androidGetCurrentTimeNanos()/1000000000.0;
    float dt=lastTime>0.0?(float)(now-lastTime):0.016f;lastTime=now;
    if(dt>0.05f)dt=0.05f;
    float length=sqrtf(moveX*moveX+moveZ*moveZ);
    if(length>0.01f){
        const float step=(3.2f*dt)/length;
        movePlayerWithCollision(moveX*step,moveZ*step);
    }
    survival.update(dt,playerX,playerZ);
    glUseProgram(program);
    const char* texNames[3]={"uGrass","uConcrete","uAsphalt"};
    for(int i=0;i<3;i++){glActiveTexture(GL_TEXTURE0+i);glBindTexture(GL_TEXTURE_2D,surfaceTextures[i]);glUniform1i(glGetUniformLocation(program,texNames[i]),i);}
    const float aspect=(float)surfaceWidth/(float)surfaceHeight;
    const apocalypse::Mat4 projection=apocalypse::perspective(58.0f*3.14159265f/180.0f,aspect,0.10f,240.0f);
    const apocalypse::Vec3 eye{playerX+0.72f,2.85f,playerZ+4.65f};
    const apocalypse::Vec3 target{playerX,1.02f,playerZ-0.72f};
    const apocalypse::Mat4 view=apocalypse::lookAt(eye,target,{0.0f,1.0f,0.0f});
    const apocalypse::Mat4 viewProjection=apocalypse::multiply(projection,view);
    const GLint mvpLocation=glGetUniformLocation(program,"uMVP");
    const apocalypse::Mat4 identity=apocalypse::identity();
    glUniformMatrix4fv(mvpLocation,1,GL_FALSE,viewProjection.m);
    glBindBuffer(GL_ARRAY_BUFFER,vertexBuffer);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(6*sizeof(float)));
    glEnableVertexAttribArray(3);glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(8*sizeof(float)));
    glDrawArrays(GL_TRIANGLES,0,vertexCount);
    importedModels.render(viewProjection.m);
    glUseProgram(program);
    for(int i=0;i<3;i++){glActiveTexture(GL_TEXTURE0+i);glBindTexture(GL_TEXTURE_2D,surfaceTextures[i]);glUniform1i(glGetUniformLocation(program,texNames[i]),i);}
    glEnableVertexAttribArray(0);glEnableVertexAttribArray(1);glEnableVertexAttribArray(2);glEnableVertexAttribArray(3);
    std::vector<Vertex> dynamicMesh;
    buildSurvivalMesh(dynamicMesh);
    if(!dynamicMesh.empty()){
        if(!survivalBuffer) glGenBuffers(1,&survivalBuffer);
        glUniformMatrix4fv(mvpLocation,1,GL_FALSE,viewProjection.m);
        glBindBuffer(GL_ARRAY_BUFFER,survivalBuffer);
        glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(dynamicMesh.size()*sizeof(Vertex)),dynamicMesh.data(),GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(6*sizeof(float)));
        glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(8*sizeof(float)));
        glDrawArrays(GL_TRIANGLES,0,(GLsizei)dynamicMesh.size());
    }
    const apocalypse::Mat4 playerModel=apocalypse::translation(playerX,0.0f,playerZ);
    const apocalypse::Mat4 playerMvp=apocalypse::multiply(viewProjection,playerModel);
    glUniformMatrix4fv(mvpLocation,1,GL_FALSE,playerMvp.m);
    glBindBuffer(GL_ARRAY_BUFFER,playerBuffer);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(6*sizeof(float)));
    glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(8*sizeof(float)));
    glDrawArrays(GL_TRIANGLES,0,playerVertexCount);
    apocalypse::renderVerityCompanion(viewProjection.m,playerX+1.72f,playerZ+0.18f,length>0.01f);
    glDisableVertexAttribArray(0);glDisableVertexAttribArray(1);glDisableVertexAttribArray(2);glDisableVertexAttribArray(3);
}
extern "C" JNIEXPORT jboolean JNICALL
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeAttack(JNIEnv*,jclass){
    return survival.usePulse(playerX,playerZ)?JNI_TRUE:JNI_FALSE;
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeGetHud(JNIEnv* env,jclass){
    char text[180];
    std::snprintf(text,sizeof(text),"VIDA %d/100   ZUMBIS %d\nAGUA %d  COMIDA %d  MATERIAIS %d  KITS %d",
        survival.health(),survival.aliveZombies(),survival.water(),survival.food(),survival.materials(),survival.medkits());
    return env->NewStringUTF(text);
}

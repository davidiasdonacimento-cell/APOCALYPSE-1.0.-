#include <jni.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <vector>
#include <cmath>
#include <time.h>
#include <cstdio>
#include "survival_system.h"
static double androidGetCurrentTimeNanos(){struct timespec t{};clock_gettime(CLOCK_MONOTONIC,&t);return (double)t.tv_sec*1000000000.0+(double)t.tv_nsec;}

#define LOG_TAG "APOCALYPSE_ENGINE"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace {
struct Vertex { float x,y,z,r,g,b; };
GLuint program=0, vertexBuffer=0, playerBuffer=0, survivalBuffer=0;
GLint vertexCount=0, playerVertexCount=0;
int surfaceWidth=1, surfaceHeight=1;
float playerX=0.0f, playerZ=0.0f, moveX=0.0f, moveZ=0.0f;
std::vector<Vertex> world, playerMesh;
apocalypse::SurvivalSystem survival;

Vertex v(float x,float y,float z,float r,float g,float b){return {x,y,z,r,g,b};}
void triangle(Vertex a,Vertex b,Vertex c){world.push_back(a);world.push_back(b);world.push_back(c);}
void quad(Vertex a,Vertex b,Vertex c,Vertex d){triangle(a,b,c);triangle(a,c,d);}
float terrainHeight(float x,float z){
    // Subtle, deterministic undulation; roads and building pads remain level.
    return 0.045f*std::sin(x*0.31f)*std::cos(z*0.27f)
         + 0.025f*std::sin((x+z)*0.53f);
}
void addTerrain(){
    const int cells=48; const float start=-30.0f, step=60.0f/(float)cells;
    for(int ix=0;ix<cells;ix++) for(int iz=0;iz<cells;iz++){
        float x=start+ix*step,z=start+iz*step;
        float h1=terrainHeight(x,z),h2=terrainHeight(x+step,z);
        float h3=terrainHeight(x+step,z+step),h4=terrainHeight(x,z+step);
        float tint=((ix*7+iz*11)%9)*0.003f;
        float r=0.15f+tint,g=0.205f+tint,b=0.145f+tint;
        quad(v(x,h1,z,r,g,b),v(x+step,h2,z,r,g,b),
             v(x+step,h3,z+step,r*0.98f,g*0.98f,b),
             v(x,h4,z+step,r*0.96f,g*0.96f,b));
    }
}
void ground(float x,float z,float w,float d,float r,float g,float b,float y=0.105f){
    quad(v(x,y,z,r,g,b),v(x+w,y,z,r,g,b),v(x+w,y,z+d,r,g,b),v(x,y,z+d,r,g,b));
}
void sidewalkH(float x,float z,float w,float d){
    ground(x,z,w,d,0.39f,0.39f,0.35f,0.125f);
}
void sidewalkV(float x,float z,float w,float d){
    ground(x,z,w,d,0.39f,0.39f,0.35f,0.125f);
}
void building(float x,float z,float w,float d,float h,float r,float g,float b){
    float x2=x+w,z2=z+d,y=0.12f;
    Vertex a=v(x,y,z,r*.52f,g*.52f,b*.52f), b0=v(x2,y,z,r*.58f,g*.58f,b*.58f);
    Vertex c=v(x2,y,z2,r*.48f,g*.48f,b*.48f), d0=v(x,y,z2,r*.50f,g*.50f,b*.50f);
    Vertex A=v(x,y+h,z,r,g,b), B=v(x2,y+h,z,r*.92f,g*.92f,b*.92f);
    Vertex C=v(x2,y+h,z2,r*.82f,g*.82f,b*.82f), D=v(x,y+h,z2,r*.88f,g*.88f,b*.88f);
    quad(A,B,C,D);quad(a,b0,B,A);quad(b0,c,C,B);quad(c,d0,D,C);quad(d0,a,A,D);
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
    // Buildings distributed over the prepared urban plots.
    for(int x=-20;x<=16;x+=9) for(int z=-20;z<=16;z+=9){
        if((x==-2&&z==-2)||(x==7&&z==-2)) continue;
        building(x+0.5f,z+0.6f,2.4f,2.5f,2.0f,0.30f,0.34f,0.38f);
        building(x+3.4f,z+0.8f,2.7f,2.0f,3.8f,0.25f,0.31f,0.37f);
        building(x+1.2f,z+3.8f,2.2f,2.4f,1.5f,0.34f,0.29f,0.25f);
    }
    // Hospital pad and building landmark.
    ground(5.0f,5.0f,7.0f,7.0f,0.29f,0.30f,0.28f,0.14f);
    building(6.0f,6.0f,4.8f,4.5f,5.2f,0.58f,0.61f,0.62f);
    // Contaminated park with a low-poly tree grove.
    ground(-20.0f,5.0f,7.0f,7.0f,0.10f,0.22f,0.12f,0.14f);
    for(int i=0;i<8;i++){
        float tx=-19.0f+(i%4)*1.6f,tz=6.0f+(i/4)*3.0f;
        building(tx,tz,0.65f,0.65f,1.6f,0.12f,0.30f,0.14f);
    }
}

void addPlayerModel(){
    playerMesh.clear();
    // Stylized survivor: boots/legs, torso, arms and head.
    auto cube=[&](float x,float y,float z,float w,float h,float d,float r,float g,float b){
        float x2=x+w,y2=y+h,z2=z+d;
        Vertex a=v(x,y,z,r*.65f,g*.65f,b*.65f),b0=v(x2,y,z,r*.65f,g*.65f,b*.65f);
        Vertex c=v(x2,y,z2,r*.65f,g*.65f,b*.65f),d0=v(x,y,z2,r*.65f,g*.65f,b*.65f);
        Vertex A=v(x,y2,z,r,g,b),B=v(x2,y2,z,r,g,b),C=v(x2,y2,z2,r*.9f,g*.9f,b*.9f),D=v(x,y2,z2,r*.9f,g*.9f,b*.9f);
        auto tri=[&](Vertex p,Vertex q,Vertex t){playerMesh.push_back(p);playerMesh.push_back(q);playerMesh.push_back(t);};
        auto quad=[&](Vertex p,Vertex q,Vertex t,Vertex u){tri(p,q,t);tri(p,t,u);};
        quad(A,B,C,D);quad(a,b0,B,A);quad(b0,c,C,B);quad(c,d0,D,C);quad(d0,a,A,D);
    };
    cube(-0.22f,0.05f,-0.16f,0.17f,0.42f,0.30f,0.18f,0.20f,0.24f);
    cube(0.05f,0.05f,-0.16f,0.17f,0.42f,0.30f,0.18f,0.20f,0.24f);
    cube(-0.25f,0.47f,-0.19f,0.50f,0.55f,0.38f,0.20f,0.34f,0.29f);
    cube(-0.34f,0.48f,-0.15f,0.12f,0.48f,0.27f,0.72f,0.56f,0.39f);
    cube(0.22f,0.48f,-0.15f,0.12f,0.48f,0.27f,0.72f,0.56f,0.39f);
    cube(-0.16f,1.05f,-0.13f,0.32f,0.32f,0.30f,0.72f,0.56f,0.39f);
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
        addBox(mesh,z.x-0.28f,0.12f,z.z-0.22f,0.56f,0.78f,0.44f,0.28f,0.43f,0.25f);
        addBox(mesh,z.x-0.19f,0.90f,z.z-0.16f,0.38f,0.36f,0.34f,0.42f,0.48f,0.32f);
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
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeInit(JNIEnv*,jclass){
    const char* vs="#version 300 es\nlayout(location=0) in vec3 aPos; layout(location=1) in vec3 aColor; uniform vec2 uCamera; uniform vec2 uOffset; uniform float uAspect; out vec3 color; void main(){vec3 p=aPos;p.x+=uOffset.x-uCamera.x;p.z+=uOffset.y-uCamera.y;float sx=(p.x-p.z)*0.032;float sy=(p.x+p.z)*0.018-p.y*0.075;sx*=min(uAspect,1.0);sy*=min(1.0,1.0/uAspect);float depth=clamp((p.x+p.z)*0.0008-p.y*0.001,-0.08,0.08);gl_Position=vec4(sx,sy,depth,1.0);color=aColor;}";
    const char* fs="#version 300 es\nprecision mediump float;in vec3 color;out vec4 fragColor;void main(){fragColor=vec4(color,1.0);}";
    GLuint a=compileShader(GL_VERTEX_SHADER,vs),b=compileShader(GL_FRAGMENT_SHADER,fs);
    if(!a||!b)return;
    program=glCreateProgram();glAttachShader(program,a);glAttachShader(program,b);glLinkProgram(program);
    glDeleteShader(a);glDeleteShader(b);
    GLint linked=GL_FALSE;glGetProgramiv(program,GL_LINK_STATUS,&linked);
    if(linked!=GL_TRUE){char log[1024]={};glGetProgramInfoLog(program,sizeof(log),nullptr,log);LOGE("Program link: %s",log);glDeleteProgram(program);program=0;return;}
    buildCity();
    survival.reset();
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
    if(length>0.01f){playerX+=(moveX/length)*3.2f*dt;playerZ+=(moveZ/length)*3.2f*dt;}
    if(playerX < -25.0f)playerX=-25.0f;if(playerX>25.0f)playerX=25.0f;
    if(playerZ < -25.0f)playerZ=-25.0f;if(playerZ>25.0f)playerZ=25.0f;
    survival.update(dt,playerX,playerZ);
    glUseProgram(program);
    GLint cam=glGetUniformLocation(program,"uCamera");
    GLint off=glGetUniformLocation(program,"uOffset");
    GLint asp=glGetUniformLocation(program,"uAspect");
    float aspect=(float)surfaceWidth/(float)surfaceHeight;
    glUniform2f(cam,playerX,playerZ);glUniform2f(off,0.0f,0.0f);glUniform1f(asp,aspect);
    glBindBuffer(GL_ARRAY_BUFFER,vertexBuffer);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
    glDrawArrays(GL_TRIANGLES,0,vertexCount);
    std::vector<Vertex> dynamicMesh;
    buildSurvivalMesh(dynamicMesh);
    if(!dynamicMesh.empty()){
        if(!survivalBuffer) glGenBuffers(1,&survivalBuffer);
        glUniform2f(off,0.0f,0.0f);
        glBindBuffer(GL_ARRAY_BUFFER,survivalBuffer);
        glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(dynamicMesh.size()*sizeof(Vertex)),dynamicMesh.data(),GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
        glDrawArrays(GL_TRIANGLES,0,(GLsizei)dynamicMesh.size());
    }
    glUniform2f(off,playerX,playerZ);
    glBindBuffer(GL_ARRAY_BUFFER,playerBuffer);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
    glDrawArrays(GL_TRIANGLES,0,playerVertexCount);
    glDisableVertexAttribArray(0);glDisableVertexAttribArray(1);
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

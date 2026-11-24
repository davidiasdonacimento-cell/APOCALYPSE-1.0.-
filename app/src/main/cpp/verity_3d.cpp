#include <jni.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <vector>
#include <cmath>
#include <algorithm>

#define TAG "APOCALYPSE_VERITY_3D"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

namespace {
struct V { float x,y,z,r,g,b; };
struct Vec { float x,y,z; };
std::vector<V> mesh;
GLuint program=0, buffer=0;
GLint count=0, mvpLoc=-1, timeLoc=-1;
int width=1,height=1;
float elapsed=0.0f;

Vec add(Vec a,Vec b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
Vec sub(Vec a,Vec b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Vec mul(Vec a,float s){return {a.x*s,a.y*s,a.z*s};}
float dot(Vec a,Vec b){return a.x*b.x+a.y*b.y+a.z*b.z;}
Vec cross(Vec a,Vec b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
Vec norm(Vec a){float l=sqrtf(dot(a,a));return l>0.0001f?mul(a,1.0f/l):Vec{0,1,0};}
void tri(V a,V b,V c){mesh.push_back(a);mesh.push_back(b);mesh.push_back(c);}
V vertex(Vec p,float r,float g,float b){return {p.x,p.y,p.z,r,g,b};}

void sphere(Vec c,Vec radii,float r,float g,float b,int rings=12,int sectors=18){
    for(int y=0;y<rings;y++){
        float a0=-1.5707963f+3.1415926f*(float)y/rings;
        float a1=-1.5707963f+3.1415926f*(float)(y+1)/rings;
        for(int x=0;x<sectors;x++){
            float t0=6.2831853f*(float)x/sectors,t1=6.2831853f*(float)(x+1)/sectors;
            auto point=[&](float a,float t){return add(c,{radii.x*cosf(a)*cosf(t),radii.y*sinf(a),radii.z*cosf(a)*sinf(t)});};
            Vec p0=point(a0,t0),p1=point(a0,t1),p2=point(a1,t1),p3=point(a1,t0);
            tri(vertex(p0,r,g,b),vertex(p1,r,g,b),vertex(p2,r,g,b));
            tri(vertex(p0,r,g,b),vertex(p2,r,g,b),vertex(p3,r,g,b));
        }
    }
}
void cylinder(Vec a,Vec b,float radius,float r,float g,float bl,int sides=12){
    Vec axis=norm(sub(b,a));
    Vec ref=fabsf(axis.y)<0.9f?Vec{0,1,0}:Vec{1,0,0};
    Vec u=norm(cross(axis,ref)),v=norm(cross(axis,u));
    for(int i=0;i<sides;i++){
        float t0=6.2831853f*i/sides,t1=6.2831853f*(i+1)/sides;
        Vec o0=add(mul(u,cosf(t0)*radius),mul(v,sinf(t0)*radius));
        Vec o1=add(mul(u,cosf(t1)*radius),mul(v,sinf(t1)*radius));
        V p0=vertex(add(a,o0),r,g,bl),p1=vertex(add(a,o1),r,g,bl);
        V p2=vertex(add(b,o1),r,g,bl),p3=vertex(add(b,o0),r,g,bl);
        tri(p0,p1,p2);tri(p0,p2,p3);
    }
}
void buildCharacter(){
    mesh.clear();
    // Round display base.
    sphere({0,0.06f,0},{0.82f,0.08f,0.62f},0.08f,0.18f,0.27f,8,24);
    // Boots and legs.
    sphere({-0.17f,0.20f,0.02f},{0.14f,0.19f,0.16f},0.10f,0.13f,0.20f);
    sphere({ 0.17f,0.20f,0.02f},{0.14f,0.19f,0.16f},0.10f,0.13f,0.20f);
    cylinder({-0.17f,0.27f,0},{-0.16f,0.78f,0},0.105f,0.18f,0.23f,0.34f);
    cylinder({ 0.17f,0.27f,0},{ 0.16f,0.78f,0},0.105f,0.18f,0.23f,0.34f);
    // Jacket torso and waist.
    sphere({0,1.02f,0},{0.34f,0.39f,0.22f},0.07f,0.13f,0.25f);
    sphere({0,0.79f,0},{0.29f,0.18f,0.20f},0.10f,0.17f,0.29f);
    // Bright central jacket seam.
    cylinder({0,0.83f,-0.205f},{0,1.30f,-0.205f},0.018f,0.25f,0.72f,0.98f,8);
    // Neck and face.
    cylinder({0,1.32f,0},{0,1.47f,0},0.095f,0.77f,0.62f,0.52f);
    sphere({0,1.66f,-0.005f},{0.235f,0.285f,0.205f},0.82f,0.70f,0.62f,16,24);
    // Long pale hair: cap, side locks and back fall.
    sphere({0,1.82f,0.015f},{0.25f,0.17f,0.22f},0.82f,0.88f,0.98f,12,24);
    sphere({-0.205f,1.57f,0.025f},{0.075f,0.34f,0.09f},0.78f,0.85f,0.96f);
    sphere({ 0.205f,1.57f,0.025f},{0.075f,0.34f,0.09f},0.78f,0.85f,0.96f);
    sphere({0,1.42f,0.12f},{0.20f,0.30f,0.11f},0.74f,0.82f,0.94f);
    // Eyes and subtle face details on the front (-Z).
    sphere({-0.082f,1.68f,-0.196f},{0.025f,0.018f,0.012f},0.08f,0.13f,0.22f,8,12);
    sphere({ 0.082f,1.68f,-0.196f},{0.025f,0.018f,0.012f},0.08f,0.13f,0.22f,8,12);
    // Arms, gloves and hands.
    cylinder({-0.31f,1.25f,0},{-0.48f,0.91f,-0.015f},0.095f,0.07f,0.13f,0.24f);
    cylinder({-0.48f,0.91f,-0.015f},{-0.43f,0.72f,-0.07f},0.07f,0.72f,0.58f,0.50f);
    sphere({-0.43f,0.70f,-0.07f},{0.075f,0.085f,0.07f},0.82f,0.70f,0.62f);
    cylinder({0.31f,1.25f,0},{0.48f,0.91f,-0.015f},0.095f,0.07f,0.13f,0.24f);
    cylinder({0.48f,0.91f,-0.015f},{0.43f,0.72f,-0.07f},0.07f,0.72f,0.58f,0.50f);
    sphere({0.43f,0.70f,-0.07f},{0.075f,0.085f,0.07f},0.82f,0.70f,0.62f);
}

// Column-major 4x4 matrix helpers.
void identity(float* m){for(int i=0;i<16;i++)m[i]=0; m[0]=m[5]=m[10]=m[15]=1;}
void multiply(float* out,const float* a,const float* b){
    float t[16];
    for(int c=0;c<4;c++)for(int r=0;r<4;r++){t[c*4+r]=0;for(int k=0;k<4;k++)t[c*4+r]+=a[k*4+r]*b[c*4+k];}
    for(int i=0;i<16;i++)out[i]=t[i];
}
void perspective(float* m,float fovy,float aspect,float n,float f){
    for(int i=0;i<16;i++)m[i]=0;
    float q=1.0f/tanf(fovy*0.5f);
    m[0]=q/aspect;m[5]=q;m[10]=(f+n)/(n-f);m[11]=-1;m[14]=(2*f*n)/(n-f);
}
void lookAt(float* m,Vec eye,Vec center,Vec up){
    Vec f=norm(sub(center,eye)),s=norm(cross(f,up)),u=cross(s,f);
    identity(m);
    m[0]=s.x;m[4]=s.y;m[8]=s.z;
    m[1]=u.x;m[5]=u.y;m[9]=u.z;
    m[2]=-f.x;m[6]=-f.y;m[10]=-f.z;
    m[12]=-dot(s,eye);m[13]=-dot(u,eye);m[14]=dot(f,eye);
}
GLuint shader(GLenum type,const char* src){
    GLuint s=glCreateShader(type);glShaderSource(s,1,&src,nullptr);glCompileShader(s);
    GLint ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){char log[1024]={};glGetShaderInfoLog(s,sizeof(log),nullptr,log);LOGE("shader: %s",log);glDeleteShader(s);return 0;}
    return s;
}
void init(){
    const char* vs="#version 300 es\nlayout(location=0) in vec3 aPos;layout(location=1) in vec3 aColor;uniform mat4 uMVP;uniform float uTime;out vec3 color;void main(){vec3 p=aPos;p.y+=sin(uTime*2.0)*0.012;gl_Position=uMVP*vec4(p,1.0);color=aColor;}";
    const char* fs="#version 300 es\nprecision mediump float;in vec3 color;out vec4 fragColor;void main(){float light=0.72+0.28*max(dot(normalize(vec3(0.3,0.8,0.5)),vec3(0.0,0.0,1.0)),0.0);fragColor=vec4(color*light,1.0);}";
    GLuint a=shader(GL_VERTEX_SHADER,vs),b=shader(GL_FRAGMENT_SHADER,fs);if(!a||!b)return;
    program=glCreateProgram();glAttachShader(program,a);glAttachShader(program,b);glLinkProgram(program);glDeleteShader(a);glDeleteShader(b);
    GLint ok=0;glGetProgramiv(program,GL_LINK_STATUS,&ok);if(!ok){char log[1024]={};glGetProgramInfoLog(program,sizeof(log),nullptr,log);LOGE("link: %s",log);glDeleteProgram(program);program=0;return;}
    mvpLoc=glGetUniformLocation(program,"uMVP");timeLoc=glGetUniformLocation(program,"uTime");
    buildCharacter();glGenBuffers(1,&buffer);glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glBufferData(GL_ARRAY_BUFFER,mesh.size()*sizeof(V),mesh.data(),GL_STATIC_DRAW);count=(GLint)mesh.size();
    glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);
}
void draw(float dt){
    if(!program||!buffer||count<=0)return;
    elapsed+=dt;
    glViewport(0,0,width,height);glClearColor(0.025f,0.045f,0.085f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    float p[16],v[16],pv[16];perspective(p,0.78f,(float)width/(float)height,0.1f,50.0f);
    lookAt(v,{0,2.05f,-5.3f},{0,1.0f,0},{0,1,0});multiply(pv,p,v);
    glUseProgram(program);glUniformMatrix4fv(mvpLoc,1,GL_FALSE,pv);glUniform1f(timeLoc,elapsed);
    glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glEnableVertexAttribArray(0);glEnableVertexAttribArray(1);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(V),(void*)0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(V),(void*)(3*sizeof(float)));
    glDrawArrays(GL_TRIANGLES,0,count);
    glDisableVertexAttribArray(0);glDisableVertexAttribArray(1);
}
}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_Verity3DActivity_00024Renderer_nativeInit(JNIEnv*,jclass){init();}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_Verity3DActivity_00024Renderer_nativeResize(JNIEnv*,jclass,jint w,jint h){width=std::max(1,(int)w);height=std::max(1,(int)h);glViewport(0,0,width,height);}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_Verity3DActivity_00024Renderer_nativeRender(JNIEnv*,jclass){draw(1.0f/60.0f);}

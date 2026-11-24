#include <jni.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <vector>

#define LOG_TAG "APOCALYPSE_ENGINE"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace {
struct Vertex { float x,y,z,r,g,b; };
GLuint program=0, vertexBuffer=0;
GLint vertexCount=0;
GLint colorLocation=-1;
int surfaceWidth=1, surfaceHeight=1;
std::vector<Vertex> world;

void triangle(Vertex a, Vertex b, Vertex c) {
    world.push_back(a); world.push_back(b); world.push_back(c);
}
Vertex v(float x,float y,float z,float r,float g,float b){ return {x,y,z,r,g,b}; }

void quad(Vertex a,Vertex b,Vertex c,Vertex d) {
    triangle(a,b,c); triangle(a,c,d);
}
void boxBuilding(float x,float z,float w,float d,float h,float r,float g,float b) {
    const float x2=x+w, z2=z+d;
    Vertex a=v(x,0,z,r*.55f,g*.55f,b*.55f), b0=v(x2,0,z,r*.62f,g*.62f,b*.62f);
    Vertex c=v(x2,0,z2,r*.48f,g*.48f,b*.48f), d0=v(x,0,z2,r*.52f,g*.52f,b*.52f);
    Vertex A=v(x,h,z,r,g,b), B=v(x2,h,z,r*.92f,g*.92f,b*.92f);
    Vertex C=v(x2,h,z2,r*.82f,g*.82f,b*.82f), D=v(x,h,z2,r*.88f,g*.88f,b*.88f);
    quad(A,B,C,D); quad(a,b0,B,A); quad(b0,c,C,B); quad(c,d0,D,C); quad(d0,a,A,D);
}
void ground(float x,float z,float w,float d,float r,float g,float b) {
    quad(v(x,0,z,r,g,b),v(x+w,0,z,r,g,b),v(x+w,0,z+d,r,g,b),v(x,0,z+d,r,g,b));
}
void buildCity() {
    world.clear();
    ground(-24,-24,48,48,0.16f,0.20f,0.17f);
    // Main asphalt avenues crossing the city.
    ground(-24,-2.2f,48,4.4f,0.12f,0.13f,0.15f);
    ground(-2.2f,-24,4.4f,48,0.12f,0.13f,0.15f);
    // Side streets.
    for(int p=-18;p<=18;p+=9) {
        ground(-24,(float)p-0.75f,48,1.5f,0.19f,0.20f,0.21f);
        ground((float)p-0.75f,-24,1.5f,48,0.19f,0.20f,0.21f);
    }
    // Urban blocks with alleys and varied building heights.
    for(int x=-20;x<=16;x+=9) for(int z=-20;z<=16;z+=9) {
        if ((x== -2 && z== -2) || (x==7 && z==-2)) continue;
        ground((float)x,(float)z,7.0f,7.0f,0.22f,0.23f,0.21f);
        boxBuilding(x+0.5f,z+0.6f,2.4f,2.5f,2.0f,0.30f,0.34f,0.38f);
        boxBuilding(x+3.4f,z+0.8f,2.7f,2.0f,3.8f,0.25f,0.31f,0.37f);
        boxBuilding(x+1.2f,z+3.8f,2.2f,2.4f,1.5f,0.34f,0.29f,0.25f);
    }
    // Distinct landmark: hospital compound.
    ground(5.0f,5.0f,7.0f,7.0f,0.27f,0.28f,0.25f);
    boxBuilding(6.0f,6.0f,4.8f,4.5f,5.2f,0.58f,0.61f,0.62f);
    // Park / contaminated green space.
    ground(-20.0f,5.0f,7.0f,7.0f,0.12f,0.25f,0.15f);
    for(int i=0;i<8;i++) {
        float tx=-19.0f+(i%4)*1.6f, tz=6.0f+(i/4)*3.0f;
        // Low polygonal tree canopies represented as compact green blocks.
        boxBuilding(tx,tz,0.65f,0.65f,1.6f,0.12f,0.30f,0.14f);
    }
}
GLuint shader(GLenum type,const char* src) {
    GLuint s=glCreateShader(type); glShaderSource(s,1,&src,nullptr); glCompileShader(s);
    GLint ok=GL_FALSE; glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(ok!=GL_TRUE){char log[1024]={};glGetShaderInfoLog(s,sizeof(log),nullptr,log);LOGE("Shader: %s",log);glDeleteShader(s);return 0;}
    return s;
}
}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeInit(JNIEnv*,jclass) {
    const char* vs="#version 300 es\nlayout(location=0) in vec3 aPos; layout(location=1) in vec3 aColor; out vec3 color; void main(){ float sx=(aPos.x-aPos.z)*0.040; float sy=(aPos.x+aPos.z)*0.022-aPos.y*0.085; gl_Position=vec4(sx,sy,0.0,1.0); color=aColor; }";
    const char* fs="#version 300 es\nprecision mediump float; in vec3 color; out vec4 fragColor; void main(){fragColor=vec4(color,1.0);}";
    GLuint a=shader(GL_VERTEX_SHADER,vs), b=shader(GL_FRAGMENT_SHADER,fs);
    if(!a||!b)return;
    program=glCreateProgram(); glAttachShader(program,a); glAttachShader(program,b); glLinkProgram(program);
    glDeleteShader(a); glDeleteShader(b);
    GLint linked=GL_FALSE; glGetProgramiv(program,GL_LINK_STATUS,&linked);
    if(linked!=GL_TRUE){char log[1024]={};glGetProgramInfoLog(program,sizeof(log),nullptr,log);LOGE("Link: %s",log);glDeleteProgram(program);program=0;return;}
    buildCity();
    glGenBuffers(1,&vertexBuffer); glBindBuffer(GL_ARRAY_BUFFER,vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,world.size()*sizeof(Vertex),world.data(),GL_STATIC_DRAW);
    vertexCount=(GLint)world.size();
}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeResize(JNIEnv*,jclass,jint w,jint h){
    surfaceWidth=w>0?w:1; surfaceHeight=h>0?h:1; glViewport(0,0,surfaceWidth,surfaceHeight);
}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeRender(JNIEnv*,jclass){
    glViewport(0,0,surfaceWidth,surfaceHeight);
    glClearColor(0.035f,0.055f,0.075f,1.0f); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    if(!program||!vertexBuffer||vertexCount<=0)return;
    glUseProgram(program); glBindBuffer(GL_ARRAY_BUFFER,vertexBuffer);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
    glDrawArrays(GL_TRIANGLES,0,vertexCount);
    glDisableVertexAttribArray(0); glDisableVertexAttribArray(1);
}
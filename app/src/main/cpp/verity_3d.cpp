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
int width=1,height=1, animationMode=0; // 0 idle, 1 wave, 2 talking, 3 walk
float elapsed=0.0f;
const float PI=3.14159265358979323846f;

Vec add(Vec a,Vec b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
Vec sub(Vec a,Vec b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Vec mul(Vec a,float s){return {a.x*s,a.y*s,a.z*s};}
float dot(Vec a,Vec b){return a.x*b.x+a.y*b.y+a.z*b.z;}
Vec cross(Vec a,Vec b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
Vec norm(Vec a){float l=sqrtf(dot(a,a));return l>0.0001f?mul(a,1.0f/l):Vec{0,1,0};}
V vertex(Vec p,float r,float g,float b){return {p.x,p.y,p.z,r,g,b};}
void tri(V a,V b,V c){mesh.push_back(a);mesh.push_back(b);mesh.push_back(c);}

void sphere(Vec c,Vec radii,float r,float g,float b,int rings=9,int sectors=14){
    for(int y=0;y<rings;y++){
        float a0=-PI*.5f+PI*(float)y/rings,a1=-PI*.5f+PI*(float)(y+1)/rings;
        for(int x=0;x<sectors;x++){
            float t0=2*PI*(float)x/sectors,t1=2*PI*(float)(x+1)/sectors;
            auto point=[&](float a,float t){return add(c,{radii.x*cosf(a)*cosf(t),radii.y*sinf(a),radii.z*cosf(a)*sinf(t)});};
            Vec p0=point(a0,t0),p1=point(a0,t1),p2=point(a1,t1),p3=point(a1,t0);
            tri(vertex(p0,r,g,b),vertex(p1,r,g,b),vertex(p2,r,g,b));
            tri(vertex(p0,r,g,b),vertex(p2,r,g,b),vertex(p3,r,g,b));
        }
    }
}
void capsule(Vec a,Vec b,float radius,float r,float g,float bl){
    Vec axis=norm(sub(b,a)),ref=fabsf(axis.y)<.9f?Vec{0,1,0}:Vec{1,0,0};
    Vec u=norm(cross(axis,ref)),v=norm(cross(axis,u));
    for(int i=0;i<10;i++){
        float t0=2*PI*i/10,t1=2*PI*(i+1)/10;
        Vec o0=add(mul(u,cosf(t0)*radius),mul(v,sinf(t0)*radius));
        Vec o1=add(mul(u,cosf(t1)*radius),mul(v,sinf(t1)*radius));
        V p0=vertex(add(a,o0),r,g,bl),p1=vertex(add(a,o1),r,g,bl);
        V p2=vertex(add(b,o1),r,g,bl),p3=vertex(add(b,o0),r,g,bl);
        tri(p0,p1,p2);tri(p0,p2,p3);
    }
    sphere(a,{radius,radius,radius},r,g,bl,6,10);
    sphere(b,{radius,radius,radius},r,g,bl,6,10);
}
Vec rotateX(Vec p,Vec pivot,float angle){
    float c=cosf(angle),s=sinf(angle);p=sub(p,pivot);
    return add(pivot,{p.x,p.y*c-p.z*s,p.y*s+p.z*c});
}
void bone(Vec a,Vec b,float radius,float r,float g,float bl){capsule(a,b,radius,r,g,bl);}

// Lightweight procedural skeleton: root -> spine -> neck/head; shoulders -> elbows -> hands;
// hips -> knees -> feet. The pose is evaluated each frame and drives the skinned joint positions.
void buildCharacter(float t){
    mesh.clear();
    float idle=sinf(t*2.0f);
    float walk=animationMode==3? sinf(t*7.0f):0.0f;
    float wave=animationMode==1? (0.5f+0.5f*sinf(t*9.0f)):0.0f;
    float talk=animationMode==2? (0.5f+0.5f*sinf(t*8.5f)):0.0f;
    float blink=(fmodf(t,3.7f)<0.12f)?0.12f:1.0f;
    float bodyY=0.015f*idle;
    Vec root{0,0.10f+bodyY,0};

    // Display plinth.
    sphere({0,0.06f,0},{0.82f,0.08f,0.62f},0.08f,0.18f,0.27f,7,20);

    // Leg chains.
    Vec hipL=add(root,{-0.16f,0.78f,0}),hipR=add(root,{0.16f,0.78f,0});
    Vec kneeL=add(hipL,{-0.015f,-0.29f,0.07f*walk});
    Vec kneeR=add(hipR,{0.015f,-0.29f,-0.07f*walk});
    Vec ankleL=add(kneeL,{-0.01f,-0.31f,-0.08f*walk});
    Vec ankleR=add(kneeR,{0.01f,-0.31f,0.08f*walk});
    bone(hipL,kneeL,0.105f,0.18f,0.23f,0.34f);
    bone(hipR,kneeR,0.105f,0.18f,0.23f,0.34f);
    bone(kneeL,ankleL,0.085f,0.13f,0.18f,0.29f);
    bone(kneeR,ankleR,0.085f,0.13f,0.18f,0.29f);
    sphere(add(ankleL,{0,-0.055f,-0.075f}),{0.14f,0.08f,0.21f},0.08f,0.12f,0.19f);
    sphere(add(ankleR,{0,-0.055f,-0.075f}),{0.14f,0.08f,0.21f},0.08f,0.12f,0.19f);

    // Torso, waist, collar and jacket detail.
    Vec pelvis=add(root,{0,0.76f,0});
    Vec chest=add(root,{0,1.17f+bodyY,0});
    sphere(pelvis,{0.29f,0.18f,0.20f},0.10f,0.17f,0.29f);
    sphere(chest,{0.34f,0.38f,0.22f},0.07f,0.13f,0.25f);
    capsule({0,0.86f,-0.205f},{0,1.31f+bodyY,-0.205f},0.016f,0.25f,0.72f,0.98f);

    // Shoulder and arm chains, right arm raises and waves in wave mode.
    Vec shoulderL=add(chest,{-0.30f,0.10f,0}),shoulderR=add(chest,{0.30f,0.10f,0});
    Vec elbowL=add(shoulderL,{-0.15f,-0.29f,0});
    Vec handL=add(elbowL,{-0.02f,-0.20f,-0.04f});
    Vec elbowR=add(shoulderR,{0.17f,-0.28f-wave*0.16f,0});
    Vec handR=add(elbowR,{0.02f,-0.20f-wave*0.20f,-0.03f});
    if(animationMode==1){
        elbowR=add(shoulderR,{0.20f,0.03f-wave*0.04f,0});
        handR=add(elbowR,{0.08f,0.25f+wave*0.07f,0});
    }
    bone(shoulderL,elbowL,0.09f,0.07f,0.13f,0.24f);
    bone(elbowL,handL,0.065f,0.72f,0.58f,0.50f);
    bone(shoulderR,elbowR,0.09f,0.07f,0.13f,0.24f);
    bone(elbowR,handR,0.065f,0.72f,0.58f,0.50f);

    // Neck, head and pale hair.
    Vec neck=add(chest,{0,0.36f,0});
    Vec head=add(neck,{0,0.25f+0.012f*idle,0});
    capsule(neck,add(neck,{0,0.16f,0}),0.085f,0.77f,0.62f,0.52f);
    sphere(head,{0.235f,0.285f,0.205f},0.82f,0.70f,0.62f,13,18);
    sphere(add(head,{0,0.16f,0.015f}),{0.25f,0.17f,0.22f},0.82f,0.88f,0.98f,10,18);
    sphere(add(head,{-0.205f,-0.09f,0.03f}),{0.075f,0.34f,0.09f},0.78f,0.85f,0.96f,9,12);
    sphere(add(head,{0.205f,-0.09f,0.03f}),{0.075f,0.34f,0.09f},0.78f,0.85f,0.96f,9,12);
    sphere(add(head,{0,-0.24f,0.12f}),{0.20f,0.30f,0.11f},0.74f,0.82f,0.94f,9,12);

    // Eyes blink by scaling their vertical radius; mouth opens while speaking.
    sphere(add(head,{-0.082f,0.02f,-0.196f}),{0.025f,0.018f*blink,0.012f},0.08f,0.13f,0.22f,6,8);
    sphere(add(head,{0.082f,0.02f,-0.196f}),{0.025f,0.018f*blink,0.012f},0.08f,0.13f,0.22f,6,8);
    float mouthOpen=0.012f+0.035f*talk;
    sphere(add(head,{0,-0.075f,-0.199f}),{0.035f,mouthOpen,0.012f},0.35f,0.10f,0.16f,6,8);
    // Cheek highlights.
    sphere(add(head,{-0.13f,-0.055f,-0.17f}),{0.025f,0.014f,0.008f},0.95f,0.48f,0.52f,5,7);
    sphere(add(head,{0.13f,-0.055f,-0.17f}),{0.025f,0.014f,0.008f},0.95f,0.48f,0.52f,5,7);
}

void identity(float* m){for(int i=0;i<16;i++)m[i]=0;m[0]=m[5]=m[10]=m[15]=1;}
void multiply(float* out,const float* a,const float* b){
    float q[16];for(int c=0;c<4;c++)for(int r=0;r<4;r++){q[c*4+r]=0;for(int k=0;k<4;k++)q[c*4+r]+=a[k*4+r]*b[c*4+k];}
    for(int i=0;i<16;i++)out[i]=q[i];
}
void perspective(float* m,float fovy,float aspect,float n,float f){
    for(int i=0;i<16;i++)m[i]=0;float q=1.0f/tanf(fovy*.5f);
    m[0]=q/aspect;m[5]=q;m[10]=(f+n)/(n-f);m[11]=-1;m[14]=(2*f*n)/(n-f);
}
void lookAt(float* m,Vec eye,Vec center,Vec up){
    Vec f=norm(sub(center,eye)),s=norm(cross(f,up)),u=cross(s,f);identity(m);
    m[0]=s.x;m[4]=s.y;m[8]=s.z;m[1]=u.x;m[5]=u.y;m[9]=u.z;
    m[2]=-f.x;m[6]=-f.y;m[10]=-f.z;m[12]=-dot(s,eye);m[13]=-dot(u,eye);m[14]=dot(f,eye);
}
GLuint makeShader(GLenum type,const char* src){
    GLuint s=glCreateShader(type);glShaderSource(s,1,&src,nullptr);glCompileShader(s);
    GLint ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){char log[1024]={};glGetShaderInfoLog(s,sizeof(log),nullptr,log);LOGE("shader: %s",log);glDeleteShader(s);return 0;}return s;
}
void init(){
    program=0;buffer=0;count=0;
    const char* vs="#version 300 es\nlayout(location=0) in vec3 aPos;layout(location=1) in vec3 aColor;uniform mat4 uMVP;out vec3 color;void main(){gl_Position=uMVP*vec4(aPos,1.0);color=aColor;}";
    const char* fs="#version 300 es\nprecision mediump float;in vec3 color;out vec4 fragColor;void main(){fragColor=vec4(color*0.92,1.0);}";
    GLuint a=makeShader(GL_VERTEX_SHADER,vs),b=makeShader(GL_FRAGMENT_SHADER,fs);if(!a||!b)return;
    program=glCreateProgram();glAttachShader(program,a);glAttachShader(program,b);glLinkProgram(program);glDeleteShader(a);glDeleteShader(b);
    GLint ok=0;glGetProgramiv(program,GL_LINK_STATUS,&ok);
    if(!ok){char log[1024]={};glGetProgramInfoLog(program,sizeof(log),nullptr,log);LOGE("link: %s",log);glDeleteProgram(program);program=0;return;}
    mvpLoc=glGetUniformLocation(program,"uMVP");glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);
}
void draw(float dt){
    elapsed+=dt;glViewport(0,0,width,height);
    glClearColor(0.025f,0.045f,0.085f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    if(!program)return;
    buildCharacter(elapsed);
    if(!buffer)glGenBuffers(1,&buffer);
    glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glBufferData(GL_ARRAY_BUFFER,mesh.size()*sizeof(V),mesh.data(),GL_DYNAMIC_DRAW);
    float p[16],v[16],pv[16];perspective(p,0.78f,(float)width/(float)height,0.1f,50.0f);
    lookAt(v,{0,2.05f,-5.3f},{0,1.0f,0},{0,1,0});multiply(pv,p,v);
    glUseProgram(program);glUniformMatrix4fv(mvpLoc,1,GL_FALSE,pv);
    glEnableVertexAttribArray(0);glEnableVertexAttribArray(1);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(V),(void*)0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(V),(void*)(3*sizeof(float)));
    glDrawArrays(GL_TRIANGLES,0,(GLsizei)mesh.size());
    glDisableVertexAttribArray(0);glDisableVertexAttribArray(1);
}
}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_Verity3DActivity_00024Renderer_nativeInit(JNIEnv*,jclass){init();}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_Verity3DActivity_00024Renderer_nativeResize(JNIEnv*,jclass,jint w,jint h){width=std::max(1,(int)w);height=std::max(1,(int)h);glViewport(0,0,width,height);}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_Verity3DActivity_00024Renderer_nativeRender(JNIEnv*,jclass){draw(1.0f/60.0f);}
extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_Verity3DActivity_00024Renderer_nativeSetAnimation(JNIEnv*,jclass,jint mode){animationMode=std::max(0,std::min(3,(int)mode));}

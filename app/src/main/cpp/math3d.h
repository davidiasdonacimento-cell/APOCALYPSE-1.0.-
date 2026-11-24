#pragma once
#include <cmath>

namespace apocalypse {
struct Vec3 { float x, y, z; };
struct Mat4 { float m[16]; };

inline Vec3 subtract(Vec3 a, Vec3 b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
inline Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}
inline float dot(Vec3 a, Vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
inline Vec3 normalize(Vec3 v) {
    const float length=std::sqrt(std::max(dot(v,v),0.000001f));
    return {v.x/length,v.y/length,v.z/length};
}
inline Mat4 identity() {
    Mat4 out{};
    out.m[0]=out.m[5]=out.m[10]=out.m[15]=1.0f;
    return out;
}
inline Mat4 multiply(const Mat4& a,const Mat4& b) {
    Mat4 out{};
    for(int column=0;column<4;++column)
        for(int row=0;row<4;++row)
            for(int k=0;k<4;++k)
                out.m[column*4+row]+=a.m[k*4+row]*b.m[column*4+k];
    return out;
}
inline Mat4 perspective(float fovRadians,float aspect,float nearPlane,float farPlane) {
    Mat4 out{};
    const float safeAspect=aspect>0.001f?aspect:1.0f;
    const float f=1.0f/std::tan(fovRadians*0.5f);
    out.m[0]=f/safeAspect;
    out.m[5]=f;
    out.m[10]=(farPlane+nearPlane)/(nearPlane-farPlane);
    out.m[11]=-1.0f;
    out.m[14]=(2.0f*farPlane*nearPlane)/(nearPlane-farPlane);
    return out;
}
inline Mat4 lookAt(Vec3 eye,Vec3 center,Vec3 up) {
    const Vec3 forward=normalize(subtract(center,eye));
    const Vec3 side=normalize(cross(forward,up));
    const Vec3 correctedUp=cross(side,forward);
    Mat4 out=identity();
    out.m[0]=side.x; out.m[4]=side.y; out.m[8]=side.z;
    out.m[1]=correctedUp.x; out.m[5]=correctedUp.y; out.m[9]=correctedUp.z;
    out.m[2]=-forward.x; out.m[6]=-forward.y; out.m[10]=-forward.z;
    out.m[12]=-dot(side,eye);
    out.m[13]=-dot(correctedUp,eye);
    out.m[14]=dot(forward,eye);
    return out;
}
inline Mat4 translation(float x,float y,float z) {
    Mat4 out=identity();
    out.m[12]=x; out.m[13]=y; out.m[14]=z;
    return out;
}
}

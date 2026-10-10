#pragma once

// Thin layer over HandmadeMath (external/HandmadeMath.h, v2.0.0, public domain).
// The library stays untouched; this file just picks names and conventions once.
//
// Conventions:
//   - matrices are column-major (same as glm, so shaders don't change)
//   - right-handed (same as glm's default)
//   - perspective uses depth -1..1 (HMM's "NO"), which is what glm was doing.
//     Vulkan's native clip depth is 0..1 ("ZO"); swap to HMM_Perspective_RH_ZO
//     below when you want that on purpose.
//   - angles are radians

#include <HandmadeMath.h>

typedef HMM_Vec2 Vec2;
typedef HMM_Vec3 Vec3;
typedef HMM_Vec4 Vec4;
typedef HMM_Mat2 Mat2;
typedef HMM_Mat3 Mat3;
typedef HMM_Mat4 Mat4;
typedef HMM_Quat Quat;

// constructors
inline Vec2 v2(float x, float y)                   { return HMM_V2(x, y); }
inline Vec3 v3(float x, float y, float z)          { return HMM_V3(x, y, z); }
inline Vec4 v4(float x, float y, float z, float w) { return HMM_V4(x, y, z, w); }
inline Mat4 identity()                             { return HMM_M4D(1.0f); }

// vectors
inline float dotV2(Vec2 a, Vec2 b)    { return HMM_DotV2(a, b); }
inline float dotV3(Vec3 a, Vec3 b)    { return HMM_DotV3(a, b); }
inline float lengthV2(Vec2 a)         { return HMM_LenV2(a); }
inline float lengthV3(Vec3 a)         { return HMM_LenV3(a); }
inline Vec2 normalizeV2(Vec2 a)       { return HMM_NormV2(a); }
inline Vec3 normalizeV3(Vec3 a)       { return HMM_NormV3(a); }
inline Vec3 cross(Vec3 a, Vec3 b)     { return HMM_Cross(a, b); }

// matrices
inline Mat2 inverseM2(Mat2 m)         { return HMM_InvGeneralM2(m); }
inline Mat4 inverseM4(Mat4 m)         { return HMM_InvGeneralM4(m); }

// angles
inline float radians(float degrees)   { return HMM_AngleDeg(degrees); }

// transforms / camera
inline Mat4 translate(Vec3 t)         { return HMM_Translate(t); }

inline Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up) {
    return HMM_LookAt_RH(eye, center, up);
}

inline Mat4 perspective(float fovRadians, float aspect, float zNear, float zFar) {
    return HMM_Perspective_RH_NO(fovRadians, aspect, zNear, zFar);
}

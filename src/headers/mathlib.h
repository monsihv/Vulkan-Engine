#pragma once

#include <math.h>

// Vectors: clang ext vectors. Infix + - * / (elementwise), scalar broadcast, swizzles.
// Everything 3D lives in Vec4. Convention: points w = 1, directions w = 0.
typedef float Vec2 __attribute__((ext_vector_type(2)));
typedef float Vec4 __attribute__((ext_vector_type(4)));

// Matrices: clang matrix extension (-fenable-matrix). Column-major in memory,
// indexed m[row][col]. Vectors in matrix-land are N x 1 matrices.
typedef float Mat2 __attribute__((matrix_type(2, 2)));
typedef float Mat4 __attribute__((matrix_type(4, 4)));
typedef float Col2 __attribute__((matrix_type(2, 1)));
typedef float Col4 __attribute__((matrix_type(4, 1)));

// Never brace-init a matrix (it silently does the wrong thing). Build them through these.

// ---- constructors -------------------------------------------------------------------
static inline Vec4 point3(float x, float y, float z) { return (Vec4){x, y, z, 1.0f}; }
static inline Vec4 dir3(float x, float y, float z)   { return (Vec4){x, y, z, 0.0f}; }

// arguments are column by column
static inline Mat2 mat2(float c0x, float c0y, float c1x, float c1y) {
    float m[4] = {c0x, c0y, c1x, c1y};
    return __builtin_matrix_column_major_load(m, 2, 2, 2);
}

static inline Col2 col2(float x, float y) {
    float v[2] = {x, y};
    return __builtin_matrix_column_major_load(v, 2, 1, 2);
}

// m is 16 floats, column-major
static inline Mat4 mat4FromColumnMajor(const float m[16]) {
    return __builtin_matrix_column_major_load(m, 4, 4, 4);
}

static inline Mat4 identity4(void) {
    float m[16] = {1, 0, 0, 0,
                   0, 1, 0, 0,
                   0, 0, 1, 0,
                   0, 0, 0, 1};
    return mat4FromColumnMajor(m);
}

// ---- ext vector <-> matrix column -----------------------------------------------------
static inline Col4 toCol4(Vec4 v) {
    float t[4] = {v.x, v.y, v.z, v.w};
    return __builtin_matrix_column_major_load(t, 4, 1, 4);
}

static inline Vec4 toVec4(Col4 c) {
    float t[4];
    __builtin_matrix_column_major_store(c, t, 4);
    return (Vec4){t[0], t[1], t[2], t[3]};
}

// ---- 3D ops: these ignore w -----------------------------------------------------------
static inline float dot3(Vec4 a, Vec4 b) {
    Vec4 m = a * b;
    return m.x + m.y + m.z;
}

static inline float length3(Vec4 v) { return sqrtf(dot3(v, v)); }

static inline Vec4 normalize3(Vec4 v) {
    Vec4 r = v / length3(v);
    r.w = v.w;
    return r;
}

// w comes out 0 automatically: a.w*b.w - a.w*b.w
static inline Vec4 cross3(Vec4 a, Vec4 b) {
    return a.yzxw * b.zxyw - a.zxyw * b.yzxw;
}

// ---- 2D -----------------------------------------------------------------------------
static inline float dot2(Vec2 a, Vec2 b) {
    Vec2 m = a * b;
    return m.x + m.y;
}

static inline float length2(Vec2 v)   { return sqrtf(dot2(v, v)); }
static inline Vec2 normalize2(Vec2 v) { return v / length2(v); }

// ---- matrices -----------------------------------------------------------------------
static inline float radians(float degrees) { return degrees * (3.14159265358979323846f / 180.0f); }

// [a c]        1    [ d -c]
// [b d]^-1 = ----- [-b  a]
//            ad-bc
static inline Mat2 inverse2(Mat2 m) {
    float a = m[0][0], c = m[0][1];
    float b = m[1][0], d = m[1][1];
    float invDet = 1.0f / (a * d - c * b);
    return mat2(d, -b, -c, a) * invDet;
}

static inline Mat4 translate(Vec4 t) {
    float m[16] = {1,   0,   0,   0,
                   0,   1,   0,   0,
                   0,   0,   1,   0,
                   t.x, t.y, t.z, 1};
    return mat4FromColumnMajor(m);
}

// Right-handed view matrix (same result as glm::lookAt / HMM_LookAt_RH).
static inline Mat4 lookAt(Vec4 eye, Vec4 center, Vec4 up) {
    Vec4 f = normalize3(center - eye);
    Vec4 s = normalize3(cross3(f, up));
    Vec4 u = cross3(s, f);

    float m[16] = {s.x,             u.x,             -f.x,           0,
                   s.y,             u.y,             -f.y,           0,
                   s.z,             u.z,             -f.z,           0,
                   -dot3(s, eye),   -dot3(u, eye),   dot3(f, eye),   1};
    return mat4FromColumnMajor(m);
}

// Right-handed perspective, depth -1..1 (same result as glm::perspective / HMM_Perspective_RH_NO).
// Vulkan's native clip depth is 0..1: for that, c[10] = zFar / (zNear - zFar)
// and c[14] = (zNear * zFar) / (zNear - zFar).
static inline Mat4 perspective(float fovRadians, float aspect, float zNear, float zFar) {
    float tanHalf = tanf(fovRadians * 0.5f);
    float m[16] = {0};
    m[0]  = 1.0f / (aspect * tanHalf);
    m[5]  = 1.0f / tanHalf;
    m[10] = -(zFar + zNear) / (zFar - zNear);
    m[11] = -1.0f;
    m[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
    return mat4FromColumnMajor(m);
}

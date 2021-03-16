#if !defined(MATH_H)
#define MATH_H

#define internal static 
#define global_variable static 
#define local_persist static

#define PI 3.14159265359

#include <stdint.h>
#include <stddef.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef i8 b8;
typedef size_t sizet;


struct vec2
{
    union
    {
        struct
        {
            f32 x, y;
        };
        f32 data[2];
    };

    f32 &operator[](sizet i);
};

struct vec3 
{
    union
    {
        struct
        {
            f32 x, y, z;
        };
        f32 data[3];
    };

    f32 &operator[](sizet i);
    vec3 operator-(vec3& other);
};

struct vec3i 
{
    union
    {
        struct
        {
            i32 x, y, z;
        };
        i32 data[3];
    };

    i32 &operator[](sizet i);
};

struct vec4
{
    union
    {
        struct
        {
            f32 x, y, z, w;
        };
        f32 data[4];
    };

    f32 &operator[](sizet i);
    vec4 operator+(vec4& other);
    vec4 operator-(vec4& other);
};

struct mat4
{
    union
    {
        vec4 rows[4];
        f32 data[16];
    };

    vec4 &operator[](sizet i);
    mat4 operator*(mat4& other);
    vec4 operator*(vec4& other);
};

struct Camera
{
    vec3 position;
    vec3 direction;
    vec3 up;
};

inline f32 
&vec2::operator[](sizet i) 
{
    return data[i];
}

inline i32 
&vec3i::operator[](sizet i) 
{
    return data[i];
}

inline vec3
vec3::operator-(vec3& other)
{
    return {x - other.x, y - other.y, z - other.z};
}

inline f32 
&vec3::operator[](sizet i)
{
    return data[i];
}

inline f32
vec3_length(vec3 v)
{
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline vec3
vec3_normalized(vec3 v)
{
    f32 len = vec3_length(v);

    v.x = v.x / len;
    v.y = v.y / len;
    v.z = v.z / len;

    return v;
}

inline vec3
vec3_cross(vec3 a, vec3 b)
{
    return {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

inline f32
vec3_dot(vec3 a, vec3 b)
{
    return (a.x*b.x + a.y*b.y + a.z*b.z);
}


inline f32 
&vec4::operator[](sizet i)
{
    return data[i];
}

inline vec4 
vec4::operator+(vec4& other)
{
    vec4 out;
    for (sizet i = 0; i < 4; ++i)
    {
        out[i] = data[i] + other[i];
    }
    return out;
}

inline vec4 
vec4::operator-(vec4& other)
{
    vec4 out;
    for (sizet i = 0; i < 4; ++i)
    {
        out[i] = data[i] - other[i];
    }
    return out;
}

inline vec4 
&mat4::operator[](sizet i)
{
    return rows[i];
}

inline mat4 
mat4::operator*(mat4& other)
{
    mat4 out;
    for (sizet x = 0; x < 4; ++x)
    {
        for (sizet y = 0; y < 4; ++y)
        {
            f32 sum = 0.0f;
            for (sizet m = 0; m < 4; ++m)
            {
                sum += rows[x][m] * other[m][y];
            }
            out[x][y] = sum;
        }
    }
    return out;
}


inline vec4 
mat4::operator*(vec4& other)
{
    vec4 out = {};
    for (sizet x = 0; x < 4; ++x)
    {
        for (sizet y = 0; y < 1; ++y)
        {
            f32 sum = 0.0f;
            for (sizet m = 0; m < 4; ++m)
            {
                sum += rows[x][m] * other[m];
            }
            out[x] = sum;
        }
    }
    return out;
}

inline mat4
mat4_identity()
{
    mat4 out = {};
    out[0][0] = 1.0f;
    out[1][1] = 1.0f;
    out[2][2] = 1.0f;
    out[3][3] = 1.0f;

    return out;
}

inline mat4
mat4_scale(vec3 v)
{
    mat4 out = mat4_identity();
    out[0][0] = v.x;
    out[1][1] = v.y;
    out[2][2] = v.z;

    return out;
}

inline mat4
mat4_translate(vec3 v)
{
    mat4 out = mat4_identity();

    out[0][3] = v.x;
    out[1][3] = v.y;
    out[2][3] = v.z;

    return out;
}

inline mat4
mat4_rotate(f32 angle, vec3 v)
{
    f32 radians = angle * PI / 180;
    f32 cos = cosf(radians);
    f32 sin = sinf(radians);

    mat4 out = 
    {
        cos + v.x*v.x * (1-cos),       v.x*v.y * (1-cos) - v.z*sin,  v.x*v.z * (1-cos) + v.y*sin, 0.0f,
        v.y*v.x * (1-cos) + v.z*sin,  cos + v.y*v.y * (1-cos),       v.y*v.z * (1-cos) - v.x*sin, 0.0f,
        v.z*v.x * (1-cos) - v.y*sin,  v.z*v.y * (1-cos) + v.x*sin,  cos + v.z*v.z * (1-cos),      0.0f,
        0.0f,                            0.0f,                            0.0f,                           1.0f
    };

    return out;
}


inline mat4
mat4_transpose(mat4 matrix)
{
    mat4 out;
    for (sizet i = 0; i < 4; ++i)
    {
        for (sizet k = 0; k < 4; ++k)
        {
            out[i][k] = matrix[k][i];
        }
    }

    return out;
}

inline mat4
mat4_orthographic(f32 w, f32 h)
{
    mat4 out = {
        2/w,  0.0f, 0.0f, -1.0f, 
        0.0f, 2/h,  0.0f, -1.0f,
        0.0f, 0.0f, 0.1f,  0.0f,
        0.0f, 0.0f, 0.0f,  1.0f
    };

    return out;
}

inline f32
degrees_to_radians(f32 d) 
{
    return (d*((f32)PI/180));
}

inline mat4
mat4_perspective(f32 w, f32 h, f32 fov, f32 n, f32 f)
{
    mat4 out =
    {
        2/w,    0.0f, 0.0f, -1.0f,
        0.0f,   2/h,  0.0f, -1.0f,
        0.0f,   0.0f, 1.0f,  0.0f,
        0.0f,   0.0f, 0.5f,  1.0f
    };
    return out;
}

inline mat4
camera_transform(Camera* cam)
{
    vec3 f = vec3_normalized(cam->direction);
    vec3 u = vec3_normalized(cam->up);
    vec3 r = vec3_normalized(vec3_cross(cam->up, cam->direction));
    vec3 t = cam->position;

    mat4 out =
    {
        r.x,  r.y,  r.z,  -t.x,
        u.x,  u.y,  u.z,  -t.y,
        f.x,  f.y,  f.z,  -t.z,
        0.0f, 0.0f, 0.0f,  1.0f
    };

    return out;
}
#endif

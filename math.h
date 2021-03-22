#if !defined(MATH_H)
#define MATH_H


#define PI 3.1415926535f

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

struct Box
{
    f32 x1, y1, x2, y2;

    f32& operator[](i32 index) 
    {
        return *(&x1 + index);
    }
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

internal inline mat4
mat4_identity()
{
    mat4 out = {};
    out[0][0] = 1.0f;
    out[1][1] = 1.0f;
    out[2][2] = 1.0f;
    out[3][3] = 1.0f;

    return out;
}

internal inline mat4
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

internal inline mat4
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


internal inline mat4
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
internal inline f32
degrees_to_radians(f32 d) 
{
    return (d*((f32)PI/180));
}


internal inline mat4
camera_transform(Camera* cam)
{
    vec3 z = vec3_normalized(cam->direction);
    vec3 y = vec3_normalized(cam->up);
    vec3 x = vec3_normalized(vec3_cross(cam->up, cam->direction));
    vec3 t = cam->position;

    mat4 out =
    {
        x.x,  x.y,  x.z,  -t.x,
        y.x,  y.y,  y.z,  -t.y,
        z.x,  z.y,  z.z,  -t.z,
        0.0f, 0.0f, 0.0f,  1.0f
    };

    return out;
}

internal inline mat4
mat4_look_at(vec3 eye, vec3 target, vec3 up)
{
    vec3 z = vec3_normalized(eye-target);
    vec3 x = vec3_normalized(vec3_cross(vec3_normalized(up), z));
    vec3 y = vec3_normalized(vec3_cross(z, x));
    mat4 out = mat4_identity();

    for (sizet i = 0; i < 3; ++i)
    {
        out[0][i] = x[i];
        out[1][i] = y[i];
        out[2][i] = z[i];
        out[i][3] = -target[i];
    }

    return out;
}


internal inline Box
triangle_bounding_box(vec3 p[3])
{
    Box out = {};

    out.x1 = min(p[0].x, min(p[1].x, p[2].x));
    out.y1 = min(p[0].y, min(p[1].y, p[2].y));

    out.x2 = max(p[0].x, max(p[1].x, p[2].x));
    out.y2 = max(p[0].y, max(p[1].y, p[2].y));

    return out;
}

internal inline vec3 
barycentric_coord(vec3 pts[3], vec2 p)
{
    vec3 s[2];
    for (sizet i = 2; i--;)
    {
        s[i][0] = pts[2][i]-pts[0][i]; 
        s[i][1] = pts[1][i]-pts[0][i]; 
        s[i][2] = pts[0][i]-p[i];
    }
    vec3 u = vec3_cross(s[0], s[1]);

    if (std::abs(u.z)>1e-2)
        return {1.0f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z};

    return {-1.0f,1.0f,1.0f};

}

internal inline mat4
mat4_view_port(f32 x, f32 y, f32 w, f32 h, f32 d)
{
    mat4 out = {
        w/2.0f, 0.0f,   0.0f,   x+w/2.0f,
        0.0f,   h/2.0f, 0.0f,   y+h/2.0f,
        0.0f,   0.0f,   d,      d, 
        0.0f,   0.0f,   0.0f,   1.0f,
    };

    return out;
}
#endif

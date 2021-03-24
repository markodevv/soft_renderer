#include "math.h"

struct Face
{
    vec3i vertex_index;
    vec3i text_coord;
    vec3i normal;
};

struct ModelInfo
{
    i32 vertex_count;
    i32 face_count;
    i32 uv_count;
    i32 vertex_normal_count;
};

struct Model
{
    vec3* vertices;
    Face* faces;
    vec3* texture_uvs;
    vec3* vertex_normals;

    vec3 position;
    vec3 rotation;
    vec3 scale;

    ModelInfo model_info;
};

internal b8
str_equal(char* s1, char* s2, i32 count)
{
    for (sizet i = 0; i < count; ++i)
    {
        if (!(s1[i] == s2[i]))
            return false;
    }
    return true;
}
 
internal ModelInfo
get_model_info(char* file)
{
    ModelInfo out = {};
    char line[512];
    FILE* fp = fopen(file, "r");

    if (fp)
    {
        char keyword[] = "  ";
        while(fgets(line, 512, fp)) 
        {
            keyword[0] = line[0];
            keyword[1] = line[1];

            if (str_equal(keyword, "v ", 2))
            {
                out.vertex_count++;
            }
            else if (str_equal(keyword, "f ", 2))
            {
                out.face_count++;
            }
            else if (str_equal(keyword, "vt ", 2))
            {
                out.uv_count++;
            }
            else if (str_equal(keyword, "vn ", 2))
            {
                out.vertex_normal_count++;
            }
        }
    }
    fclose(fp);

    return out;
}

internal vec3
get_vec3_from_line(char line[512])
{
    vec3 out = {};
    f32 data[3];
    char number_str[50];
    i32 vertex_axis = 0;

    for (i32 i = 0; line[i] != '\0'; ++i)
    {
        i32 sign = 1;

        if (line[i] == '-')
        {
            sign = -1;
            i++;
        }
        if (isdigit(line[i]))
        {
            i32 number_str_index = 0;
            while(isdigit(line[i]) || line[i] == '.')
            {
                number_str[number_str_index] = line[i];
                number_str_index++;
                i++;
            }
            number_str[number_str_index] = '\0';
            data[vertex_axis] = (f32)(atof(number_str) * sign);
            vertex_axis++;
            number_str_index = 0;
        }
    }

    out.x = data[0];
    out.y = data[1];
    out.z = data[2];


    return out;
}

internal Face
get_face_from_line(char line[512])
{
    Face out = {};
    i32 member_index = 0;

    for (i32 i = 0; line[i] != '\0'; ++i)
    {
        if (isdigit(line[i]))
        {
            char number_str[64];
            i32 data[3];

            for (i32 j = 0; j < 3; ++j)
            {
                i32 number_str_index = 0;
                while(isdigit(line[i]))
                {
                    number_str[number_str_index] = line[i];
                    number_str_index++;
                    i++;
                }
                number_str[number_str_index] = '\0';
                data[j] = (atoi(number_str));
                if (j != 2)
                {
                    i++;
                }
            }
            out.vertex_index[member_index] = data[0];
            out.text_coord[member_index] = data[1];
            out.normal[member_index]= data[2];

            member_index++;
        }
    }

    return out;
}


internal Model
load_model(char* path)
{
    Model out = {};
    out.model_info = get_model_info(path);

    char line[512];
    FILE* file = fopen(path, "r");

    out.vertices = (vec3*)malloc(sizeof(vec3) * out.model_info.vertex_count);
    out.faces = (Face*)malloc(sizeof(Face) * out.model_info.face_count);
    out.texture_uvs = (vec3*)malloc(sizeof(vec3) * out.model_info.uv_count);
    out.vertex_normals = (vec3*)malloc(sizeof(vec3) * out.model_info.vertex_normal_count);

    vec3* vertices = out.vertices;
    Face* faces = out.faces;
    vec3* texture_uvs = out.texture_uvs;
    vec3* vertex_normals = out.vertex_normals;

    if (file)
    {
        char keyword[] = "  ";
        while(fgets(line, 512, file)) 
        {
            keyword[0] = line[0];
            keyword[1] = line[1];

            if (str_equal(keyword, "v ", 2))
            {
                *vertices = get_vec3_from_line(line);
                vertices++;
            }
            else if (str_equal(keyword, "f ", 2))
            {
                *faces = get_face_from_line(line);
                faces++;
            }
            else if (str_equal(keyword, "vt ", 2))
            {
                *texture_uvs = get_vec3_from_line(line);
                texture_uvs++;
            }
            else if (str_equal(keyword, "vn ", 2))
            {
                *vertex_normals = get_vec3_from_line(line);
                vertex_normals++;
            }
        }
    }
    else
    {
        DEBUG_PRINT("Failed to load model: %s.\n", path);
    }

    return out;
}

inline internal vec3
vertex_from_face(Face face, vec3* vertices, int index)
{
    return vertices[face.vertex_index[index]-1];
}

inline internal vec3
texture_uv_from_face(Face face, vec3* tex_vertices, int index)
{
    return tex_vertices[face.text_coord[index]-1];
}

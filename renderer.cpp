#include "tga_image.cpp"
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <algorithm>

#include "math.h"

const int WIDTH = 1024;
const int HEIGHT = 1024;
const int DEPTH = 600;

struct Box
{
    float x1, y1, x2, y2;

    float& operator[](int index) 
    {
        return *(&x1 + index);
    }
};


struct Face
{
    vec3i vertex_index;
    vec3i text_coord;
    vec3i normal;
};

static void 
swap(int* x, int* y)
{
    int temp = *x;
    *x = *y;
    *y = temp;
}

static void 
swap(float* x, float* y)
{
    float temp = *x;
    *x = *y;
    *y = temp;
}

static void 
swap(vec3* x, vec3* y)
{
    vec3 temp = *x;
    *x = *y;
    *y = temp;
}



static void 
draw_line(int x0, int y0, int x1, int y1, TGAImage* image, TGAColor color)
{
    bool steep = false;
    if (abs(x0-x1) < abs(y0-y1))
    {
        swap(&x0, &y0);
        swap(&x1, &y1);
        steep = true;
    }

    if (x0 > x1)
    {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    for (int x = x0; x <= x1; ++x)
    {
        float t = (x-x0)/(float)(x1-x0);
        int y = (int)(y0*(1.0f-t) + y1*t);
        if (steep)
        {
            image->set(y, x, color);
        }
        else
        {
            image->set(x, y, color);
        }
    }
}



struct ModelInfo
{
    int vertex_count;
    int face_count;
    int texture_vertex_count;
};

static ModelInfo
get_model_info(char* file)
{
    ModelInfo out = {};
    char line[512];
    FILE* fp = fopen(file, "r");

    if (file)
    {
        while(fgets(line, 512, fp)) 
        {
            if (line[0] == 'f' && line[1] == ' ')
            {
                out.face_count++;
            }
            else if (line[0] == 'v' && line[1] == 't')
            {
                out.texture_vertex_count++;
            }
            else if (line[0] == 'v' && line[1] == ' ')
            {
                out.vertex_count++;
            }
        }
    }
    fclose(fp);

    return out;
}



static vec3
get_vertex_from_line(char line[512])
{
    vec3 out = {};
    float data[3];
    char number_str[50];
    int vertex_axis = 0;

    for (int i = 0; line[i] != '\0'; ++i)
    {
        int sign = 1;

        if (line[i] == '-')
        {
            sign = -1;
            i++;
        }
        if (isdigit(line[i]))
        {
            int number_str_index = 0;
            while(isdigit(line[i]) || line[i] == '.')
            {
                number_str[number_str_index] = line[i];
                number_str_index++;
                i++;
            }
            number_str[number_str_index] = '\0';
            data[vertex_axis] = (float)(atof(number_str) * sign);
            vertex_axis++;
            number_str_index = 0;
        }
    }

    out.x = data[0];
    out.y = data[1];
    out.z = data[2];

    return out;
}

static Face
get_face_from_line(char line[512])
{
    Face out = {};
    int member_index = 0;

    for (int i = 0; line[i] != '\0'; ++i)
    {
        if (isdigit(line[i]))
        {
            char number_str[64];
            int data[3];

            for (int j = 0; j < 3; ++j)
            {
                int number_str_index = 0;
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
static vec3
get_texture_vert_from_line(char line[512])
{
    vec3 out = {};
    float data[3];
    char number_str[50];
    int vertex_axis = 0;

    for (int i = 0; line[i] != '\0'; ++i)
    {
        int sign = 1;

        if (line[i] == '-')
        {
            sign = -1;
            i++;
        }
        if (isdigit(line[i]))
        {
            int number_str_index = 0;
            while(isdigit(line[i]) || line[i] == '.')
            {
                number_str[number_str_index] = line[i];
                number_str_index++;
                i++;
            }
            number_str[number_str_index] = '\0';
            data[vertex_axis] = (float)(atof(number_str) * sign);
            vertex_axis++;
            number_str_index = 0;
        }
    }

    out.x = data[0];
    out.y = data[1];
    out.z = data[2];

    return out;
}

inline static vec3
vertex_from_face(Face face, vec3* vertices, int index)
{
    return vertices[face.vertex_index[index]-1];
}

inline static vec3
texture_uv_coords(Face face, vec3* tex_vertices, int index)
{
    return tex_vertices[face.text_coord[index]-1];
}

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);



static Box
triangle_bounding_box(vec3 p[3])
{
    Box out = {};

    out.x1 = std::min(p[0].x, std::min(p[1].x, p[2].x));
    out.y1 = std::min(p[0].y, std::min(p[1].y, p[2].y));

    out.x2 = std::max(p[0].x, std::max(p[1].x, p[2].x));
    out.y2 = std::max(p[0].y, std::max(p[1].y, p[2].y));

    return out;
}


vec3 
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

static void
draw_triangle(vec3 p[3], vec3 tc[3], float light_intensity, TGAImage* texture, TGAImage* image, float **zbuffer)
{
    if (p[0].y > p[1].y) { swap(&p[0], &p[1]); }
    if (p[0].y > p[2].y) { swap(&p[0], &p[2]); }
    if (p[1].y > p[2].y) { swap(&p[1], &p[2]); }
    if (tc[0].y > tc[1].y) { swap(&tc[0], &tc[1]); }
    if (tc[0].y > tc[2].y) { swap(&tc[0], &tc[2]); }
    if (tc[1].y > tc[2].y) { swap(&tc[1], &tc[2]); }

    Box box = triangle_bounding_box(p);

    for (i32 y = (i32)box.y1; y <= (i32)box.y2; ++y)
    {
        for (i32 x = (i32)box.x1; x <= (i32)box.x2; ++x)
        {
            vec3 bc_coord = barycentric_coord(p, {(f32)x, (f32)y});
            if (bc_coord.x<0.0f || bc_coord.y<0.0f || bc_coord.z<0.0f) 
                continue; 

            f32 z = 0;
            int texture_x = 0;
            int texture_y = 0;
            for (int i = 0; i < 3; ++i)
            {
                z += p[i].z * bc_coord[i];
                texture_x += (int)(tc[i].x * bc_coord[i] * texture->get_width());
                texture_y += (int)(tc[i].y * bc_coord[i] * texture->get_height());
            }

            if (z > zbuffer[x][y])
            {
                zbuffer[x][y] = z;
                TGAColor color = texture->get(texture_x, texture_y); 
                color.r *= light_intensity;
                color.g *= light_intensity;
                color.b *= light_intensity;
                image->set(x, y, color);
            }
        }
    }
}

int
main(int argc, char** argv)
{
    char* head_model = "../head.obj";

    ModelInfo model_info = get_model_info(head_model);

    vec3* vertices = (vec3*)malloc(sizeof(vec3) * model_info.vertex_count);
    Face* faces = (Face*)malloc(sizeof(Face) * model_info.face_count);
    vec3* texture_vertices = (vec3*)malloc(sizeof(vec3) * model_info.texture_vertex_count);

    char line[512];
    FILE* file = fopen(head_model, "r");

    if (file)
    {
        int vertex_index = 0;
        int face_index = 0;
        int texture_vertex_index = 0;
        while(fgets(line, 512, file)) 
        {
            if (line[0] == 'v' && line[1] == ' ')
            {
                vertices[vertex_index] = get_vertex_from_line(line);
                vertex_index++;
            }
            else if (line[0] == 'f' && line[1] == ' ')
            {
                faces[face_index] = get_face_from_line(line);
                face_index++;
            }
            else if (line[0] == 'v' && line[1] == 't')
            {
                texture_vertices[texture_vertex_index] = get_texture_vert_from_line(line);
                texture_vertex_index++;
            }
        }
    }
	TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

    const char* texture_name = "../african_head_diffuse.tga";
    TGAImage head_texture;
    if (!head_texture.read_tga_file(texture_name))
    {
        printf("Failed to load %s .\n", texture_name);
    }
    head_texture.flip_vertically();

    f32 **zbuffer = (f32**)malloc(WIDTH * sizeof(f32*));
    for (sizet i = 0; i < WIDTH; ++i)
    {
        zbuffer[i] = (f32*)malloc(HEIGHT * sizeof(f32));
    }

    for (int x = 0; x < WIDTH; ++x)
    {
        for (int y = 0; y < HEIGHT; ++y)
        {
            zbuffer[x][y] = -214748364.0f;
        }
    }

    vec3 light_direction = {0.0f, 0.0f, 1.0f};
    light_direction = vec3_normalized(light_direction);
    float c = -3.0f;

    mat4 projection;
    projection = {
        1.0f, 0.0f, 0.0f,    0.0f,
        0.0f, 1.0f, 0.0f,    0.0f,
        0.0f, 0.0f, 1.0f,    0.0f,
        0.0f, 0.0f, 1.0f/c, 1.0f,
    };

    f32 x = WIDTH/8.0f;
    f32 y = HEIGHT/8.0f;
    f32 w = WIDTH*3/4;
    f32 h = HEIGHT*3/4;
    f32 d = DEPTH/2.0f;

    mat4 model = mat4_translate({x+w/2.0f, y+h/2.0f, d}) * mat4_scale({w/2.0f, h/2.0f, d});

    for (int i = 0; i < model_info.face_count; ++i)
    {
        Face face = faces[i];
        vec3 world_coord[3];
        vec3 local_coord[3];
        vec3 uv_coord[3];
        for (int j = 0; j < 3; ++j)
        {
            vec3 vertex = vertex_from_face(face, vertices, j);
            uv_coord[j] = texture_uv_coords(face, texture_vertices, j);


            vec4 vec4d = {vertex.x, vertex.y, vertex.z, 1.0f};
            vec4 temp = model * projection * vec4d;
            world_coord[j] = {temp.x/temp.w, temp.y/temp.w, temp.z/temp.w};
            local_coord[j] = vertex;
        }
        vec3 normal = vec3_cross(local_coord[0]-local_coord[1], local_coord[1]-local_coord[2]);
        float light_intensity = vec3_dot(vec3_normalized(normal), light_direction);
        if (light_intensity > 0)
        {
            draw_triangle(world_coord,
                          uv_coord,
                          light_intensity,
                          &head_texture,
                          &image, 
                          zbuffer);
        }
    }

	image.flip_vertically(); 
	image.write_tga_file("output.tga");
	return 0;
}

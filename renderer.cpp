#include "tga_image.cpp"
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <algorithm>

struct v2i
{
    int x, y;

    int& operator[](int index) 
    {
        return *(&x + index);
    }

    v2i operator-(v2i other)
    {
        return {x - other.x, y - other.y};
    }

    v2i operator+(v2i other)
    {
        return {x + other.x, y + other.y};
    }

    v2i operator*(float s)
    {
        return {(int)(x * s), (int)(y * s)};
    }
};

struct v3i
{
    int x, y, z;

    int& operator[](int index) 
    {
        return *(&x + index);
    }
};

struct v3
{
    float x, y, z;

    float& operator[](int index) 
    {
        return *(&x + index);
    }

    v3 operator-(v3 other)
    {
        return {x - other.x, y - other.y, z - other.z};
    }
};

struct Box
{
    int x1, y1, x2, y2;

    int& operator[](int index) 
    {
        return *(&x1 + index);
    }
};


struct Face
{
    v3i vertex_index;
    v3i text_coord;
    v3i normal;
};

static void 
swap(int* x, int* y)
{
    int temp = *x;
    *x = *y;
    *y = temp;
}

static void 
swap(v2i* x, v2i* y)
{
    v2i temp = *x;
    *x = *y;
    *y = temp;
}

static v3
cross_product(v3 a, v3 b)
{
    v3 out = {
        a.y*b.z-a.z*b.y,
        a.z*b.x-a.x*b.z,
        a.x*b.y-a.y*b.x
        };

    return out;
}

static float
dot_product(v3 a, v3 b)
{
    return a.x*b.x+a.y*b.y+a.z*b.z;
}

static float
v3_length(v3 a)
{
    return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

static v3
v3_normalize(v3 a)
{
    float len = v3_length(a);
    return {a.x/len, a.y/len, a.z/len};
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
        int y = y0*(1.0f-t) + y1*t;
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



static int 
get_vertex_count_from_file(char* file)
{
    char line[512];
    FILE* fp = fopen(file, "r");
    int vertex_count = 0;

    if (file)
    {
        while(fgets(line, 512, fp)) 
        {
            if (line[0] == 'v' && line[1] == ' ')
            {
                vertex_count++;
            }
        }
    }
    fclose(fp);

    return vertex_count;
}

static int 
get_face_count_from_file(char* file)
{
    char line[512];
    FILE* fp = fopen(file, "r");
    int face_count = 0;

    if (file)
    {
        while(fgets(line, 512, fp)) 
        {
            if (line[0] == 'f' && line[1] == ' ')
            {
                face_count++;
            }
        }
    }
    fclose(fp);

    return face_count;
}


static v3
get_vertex_from_line(char line[512])
{
    v3 out = {};
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

static v3
vertex_from_face(Face face, v3* vertices, int index)
{
    v3 out = vertices[face.vertex_index[index]-1];
    return out;
}

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

static void
draw_triangle(v2i p1, v2i p2, v2i p3, TGAImage* image, TGAColor color)
{

    if (p1.y > p2.y) { swap(&p1, &p2); }
    if (p1.y > p3.y) { swap(&p1, &p3); }
    if (p2.y > p3.y) { swap(&p2, &p3); }

    int total_height = p3.y - p1.y;
    for (int y = p1.y; y <= p2.y; ++y)
    {
        int segment_height = p2.y - p1.y + 1;

        float alpha = (float)(y-p1.y)/total_height;
        float beta = (float)(y-p1.y)/segment_height;

        v2i A = p1 + (p3 - p1) * alpha;
        v2i B = p1 + (p2 - p1) * beta;

        if (A.x > B.x) 
            swap(&A, &B);

        for (int j = A.x; j < B.x; ++j)
        {
            image->set(j, y, color);
        }
    }

    for (int y = p2.y; y <= p3.y; ++y)
    {
        int segment_height = p3.y - p2.y + 1;

        float alpha = (float)(y-p1.y)/total_height;
        float beta = (float)(y-p2.y)/segment_height;

        v2i A = p1 + (p3 - p1) * alpha;
        v2i B = p2 + (p3 - p2) * beta;

        if (A.x > B.x) 
            swap(&A, &B);

        for (int j = A.x; j < B.x; ++j)
        {
            image->set(j, y, color);
        }
    }
}

static Box
get_bounding_box(v2i p[3])
{
    Box out = {};

    out.x1 = std::min(p[0].x, std::min(p[1].x, p[2].x));
    out.y1 = std::min(p[0].y, std::min(p[1].y, p[2].y));

    out.x2 = std::max(p[0].x, std::max(p[1].x, p[2].x));
    out.y2 = std::max(p[0].y, std::max(p[1].y, p[2].y));

    return out;
}


v3 
barycentric_coord(v2i pts[3], v2i p)
{
      v3 u = cross_product({(float)pts[2].x-pts[0].x, (float)pts[1].x-pts[0].x, (float)pts[0].x-p.x},
                           {(float)pts[2].y-pts[0].y, (float)pts[1].y-pts[0].y, (float)pts[0].y-p.y});

    if (std::abs(u.z)<1.0f) 
        return {-1.0f,1.0f,1.0f};

    return {1.0f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z};
}

static void
draw_triangle(v2i p[3], TGAImage* image, TGAColor color)
{
    if (p[0].y > p[1].y) { swap(&p[0], &p[1]); }
    if (p[0].y > p[2].y) { swap(&p[0], &p[2]); }
    if (p[1].y > p[2].y) { swap(&p[1], &p[2]); }

    Box box = get_bounding_box(p);

    for (int y = box.y1; y <= box.y2; ++y)
    {
        for (int x = box.x1; x <= box.x2; ++x)
        {
            v3 bc_coord = barycentric_coord(p, {x, y});
            if (bc_coord.x<0.0f || bc_coord.y<0.0f || bc_coord.z<0.0f) 
                continue; 
            image->set(x, y, color); 
        }
    }
}

int
main(int argc, char** argv)
{
    char* head_model = "../head.obj";
    int vertex_index = 0;
    int face_index = 0;

    int vertex_count = 0;
    vertex_count = get_vertex_count_from_file(head_model);
    v3* vertices = (v3*)malloc(sizeof(v3) * vertex_count);

    int face_count = 0;
    face_count = get_face_count_from_file(head_model);
    Face* faces = (Face*)malloc(sizeof(Face) * face_count);

    char line[512];
    FILE* file = fopen(head_model, "r");

    if (file)
    {
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
        }
    }
    int w = 1280;
    int h = 1280;
	TGAImage image(w, h, TGAImage::RGB);

    v3 light_direction = {0.0f, 0.0f, 1.0f};

    for (int i = 0; i < face_count; ++i)
    {
        Face face = faces[i];
        v2i t[3];
        v3 vert[3];
        for (int j = 0; j < 3; ++j)
        {
            v3 v = vertex_from_face(face, vertices, j);

            int x = (v.x+1.0f)*w/2.0f; 
            int y = (v.y+1.0f)*h/2.0f; 
            t[j] = {x, y};
            vert[j] = v;
        }
        v3 normal = cross_product(vert[0]-vert[1], vert[1]-vert[2]);
        float light = dot_product(v3_normalize(normal), light_direction);
        if (light > 0)
        {
            draw_triangle(t, &image, TGAColor(light*255, light*255, light*255, 255));
        }
    }


	image.flip_vertically(); 
	image.write_tga_file("output.tga");
	return 0;
}

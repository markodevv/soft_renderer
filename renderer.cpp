#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <algorithm>
#include <windows.h>
#include <wingdi.h>
#include <stdio.h>


#define internal static 
#define global_variable static 
#define local_persist static

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

#ifdef DEBUG

inline internal void
DEBUG_print(char* text)
{
    OutputDebugString(text);
}

global_variable char DEBUG_string_buffer[512];

#define DEBUG_PRINT(msg, ...) \
    sprintf_s(DEBUG_string_buffer, msg, __VA_ARGS__); \
    DEBUG_print(DEBUG_string_buffer);

#else

#define DEBUG_PRINT(msg, ...) 

#endif 


#include "math.h"
#include "model.cpp"

global_variable i32 Width = 1280;
global_variable i32 Height = 768;
global_variable i32 Depth = 600;
global_variable BITMAPINFO bitmap_info;
global_variable void* bitmap_memory;
global_variable f32** zbuffer;
global_variable vec3 light_direction = {0.0f, 0.0f, 1.0f};

struct Color
{
    u8 r, g, b;
};

struct Box
{
    f32 x1, y1, x2, y2;

    f32& operator[](int index) 
    {
        return *(&x1 + index);
    }
};


static void 
swap(int* x, int* y)
{
    int temp = *x;
    *x = *y;
    *y = temp;
}

static void 
swap(f32* x, f32* y)
{
    f32 temp = *x;
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



//TODO: make it work with window
/*
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
        f32 t = (x-x0)/(f32)(x1-x0);
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
*/






static Box
triangle_bounding_box(vec3 p[3])
{
    Box out = {};

    out.x1 = min(p[0].x, min(p[1].x, p[2].x));
    out.y1 = min(p[0].y, min(p[1].y, p[2].y));

    out.x2 = max(p[0].x, max(p[1].x, p[2].x));
    out.y2 = max(p[0].y, max(p[1].y, p[2].y));

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

internal void
bitmap_set_pixel(i32 x, i32 y, Color color)
{
    u32* pixel = (u32*)bitmap_memory + (y * Width + x);
    u8* channel = (u8*)pixel;
    *channel = color.b;
    channel++;
    *channel = color.g;
    channel++;
    *channel = color.r;
    channel++;
}

internal void
bitmap_draw_rect(i32 px, i32 py, i32 w, i32 h)
{
    for (sizet y = py; y < h; ++y)
    {
        for (sizet x = px; x < w; ++x)
        {
            u32* pixel = (u32*)bitmap_memory + (y * Width + x);
        }
    }
}


internal void
draw_triangle(vec3 p[3], vec3 tc[3], f32 light_intensity, u8* texture, f32 **zbuffer)
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
                /*
                texture_x += (int)(tc[i].x * bc_coord[i] * texture->get_width());
                texture_y += (int)(tc[i].y * bc_coord[i] * texture->get_height());
                */
            }

            if (z > zbuffer[x][y])
            {
                zbuffer[x][y] = z;
                Color color = {255, 255, 255};
                color.r *= light_intensity;
                color.g *= light_intensity;
                color.b *= light_intensity;
                bitmap_set_pixel(x, y, color);
            }
        }
    }
}

internal inline mat4
mat4_view_port(i32 x, i32 y, i32 w, i32 h)
{
    i32 d = Depth/2;

    mat4 out = {
        w/2.0f, 0.0f,   0.0f,   x+w/2.0f,
        0.0f,   h/2.0f, 0.0f,   y+h/2.0f,
        0.0f,   0.0f,   d/2.0f, d/2.0f, 
        0.0f,   0.0f,   0.0f,   1.0f,
    };

    return out;
}

internal void
draw_model(Model* model)
{

    mat4 projection;
    projection = {
        1.0f, 0.0f,  0.0f,   0.0f,
        0.0f, 1.0f,  0.0f,   0.0f,
        0.0f, 0.0f,  1.0f,   0.0f,
        0.0f, 0.0f, -1.0f/3, 1.0f,
    };

    mat4 view_port = mat4_view_port(100, 100, Width*3/4, Height*3/4);

    Camera cam;
    cam.position = {0.0f, 0.0f, 3.0f};
    cam.direction = {0.0f, 0.0f, 1.0f};
    cam.up = {0.0f, 1.0f, 0.0f};

    mat4 view = camera_transform(&cam);

    view = mat4_look_at(cam.position, {}, cam.up);
    for (int i = 0; i < model->model_info.face_count; ++i)
    {
        Face face = model->faces[i];
        vec3 world_coord[3];
        vec3 local_coord[3];
        vec3 uv_coord[3];
        for (int j = 0; j < 3; ++j)
        {
            vec3 vertex = vertex_from_face(face, model->vertices, j);
            uv_coord[j] = texture_uv_from_face(face, model->texture_uvs, j);

            vec4 vec4d = {vertex.x, vertex.y, vertex.z, 1.0f};
            vec4 temp = view_port * projection * view * vec4d;
            world_coord[j] = {temp.x/temp.w, temp.y/temp.w, temp.z/temp.w};
            local_coord[j] = vertex;
        }
        vec3 normal = vec3_cross(local_coord[0]-local_coord[1], local_coord[1]-local_coord[2]);
        f32 light_intensity = vec3_dot(vec3_normalized(normal), light_direction);
        if (light_intensity > 0)
        {
            draw_triangle(world_coord,
                          uv_coord,
                          light_intensity,
                          NULL,
                          zbuffer);
        }
    }
}



internal void
win32_resize_dib_section(i32 w, i32 h)
{
    if (bitmap_memory)
    {
        free(bitmap_memory);
        bitmap_memory = NULL;
    }

    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = w;
    bitmap_info.bmiHeader.biHeight = h;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    i32 bytes_per_pixel = 4;
    bitmap_memory = calloc(w * h * bytes_per_pixel, sizeof(u8));
}

internal void
win32_update_window(HDC device_context, i32 x, i32 y, i32 w, i32 h)
{
    StretchDIBits(device_context,
                 x, y, w, h,
                 x, y, w, h,
                 bitmap_memory,
                 &bitmap_info,
                 DIB_RGB_COLORS,
                 SRCCOPY);
}

LRESULT CALLBACK 
win32_window_callback(
  _In_ HWND   window_handle,
  _In_ UINT   message,
  _In_ WPARAM w_param,
  _In_ LPARAM l_param
) 
{
    LRESULT result = 0;

    switch(message)
    {

        case WM_SIZE:
        {
            RECT screen_rect;
            GetClientRect(window_handle, &screen_rect);
            i32 w = screen_rect.right - screen_rect.left;
            i32 h = screen_rect.bottom - screen_rect.top;
            Width = w;
            Height = h;
            win32_resize_dib_section(w, h);
        } break;
        default:
        {
            result = DefWindowProc(window_handle, message, w_param, l_param);
        } break;
    }
    return result;
}


i32
WinMain(HINSTANCE hinstance,
            HINSTANCE prev_hinstance,
            LPSTR cmd_line,
            i32 show_code)
{
    DEBUG_PRINT("Testing \n");
    WNDCLASS window_class = {};
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = win32_window_callback;
    window_class.hInstance = hinstance;
    window_class.lpszClassName = "Win32WindowClass";

    RegisterClass(&window_class);
    HWND window_handle = CreateWindowEx(
            0,
            window_class.lpszClassName,
            "Game",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            hinstance, 
            0); 

    Model head_model = load_model("../head.obj");

    zbuffer = (f32**)malloc(Width * sizeof(f32*));
    for (sizet i = 0; i < Width; ++i)
    {
        zbuffer[i] = (f32*)malloc(Height * sizeof(f32));
    }

    for (int x = 0; x < Width; ++x)
    {
        for (int y = 0; y < Height; ++y)
        {
            zbuffer[x][y] = -214748364.0f;
        }
    }


    b8 running = true;
    while(running)
    {
        MSG message;
        while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) 
        {
            switch(message.message)
            {
                case WM_PAINT:
                {
                    PAINTSTRUCT paint;
                    HDC device_context = BeginPaint(window_handle, &paint);
                    i32 x = paint.rcPaint.left;
                    i32 y = paint.rcPaint.top;
                    i32 w = paint.rcPaint.right - paint.rcPaint.left;
                    i32 h = paint.rcPaint.bottom - paint.rcPaint.top;
                    EndPaint(window_handle, &paint);
                } break;
                case WM_CLOSE:
                case WM_DESTROY:
                {
                    running = false;
                } break;
                case WM_KEYUP:
                case WM_KEYDOWN:
                {
                    draw_model(&head_model);
                    win32_update_window(GetDC(window_handle), 0, 0, Width, Height);

                    b8 is_down = ((message.lParam & (1 << 31)) == 0);
                    b8 was_down = ((message.lParam & (1 << 30)) != 0);
                    u32 vk_code = (u32)message.wParam;

                    if (is_down != was_down)
                    {
                        switch(vk_code)
                        {

                        }
                    }
                } break;
                default:
                {
                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                } break;
            }
        }
    }
	return 0;
}

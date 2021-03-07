#include "tga_image.cpp"
#include "math.h"

static void 
swap(int* x, int* y)
{
    int temp = *x;
    *x = *y;
    *y = temp;
}

static void 
line(int x0, int y0, int x1, int y1, TGAImage* image, TGAColor color)
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

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor pink   = TGAColor(255, 0,   125,   255);

int main(int argc, char** argv) {
	TGAImage image(100, 100, TGAImage::RGB);

    line(13, 20, 80, 40, &image, white); 
    line(20, 13, 40, 80, &image, pink); 
	image.flip_vertically(); 
	image.write_tga_file("output.tga");
	return 0;
}


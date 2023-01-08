#include <iostream>
#include <cmath>
#include "tgaimage.h"
using namespace std;

// dimenzije slike
const int width  = 512;
const int height = 512;

// definirajmo boje
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0, 0, 255);
const TGAColor blue  = TGAColor(0, 0, 255, 255);
const TGAColor green = TGAColor(0,255,0,255);

void set_color(int x, int y, TGAImage &image, TGAColor color, bool invert = false)
{
    image.set(y, x, color);
}

float line(float x0, float y0, float x1, float y1, float x, float y)
{
    return (y0 - y1) * x + (x1 - x0) * y + x0 * y1 - x1 * y0;
}

void draw_triangle_2d(TGAImage &image, float x0, float y0, float x1, float y1, float x2, float y2, TGAColor color){
    float x_min = floor(min(min(x0,x1),x2));
    float y_min = floor(min(min(y0,y1),y2));
    float x_max = ceil(max(max(x0,x1),x2));
    float y_max=ceil(max(max(y0,y1),y2));

    for(float y=y_min ;y <= y_max;y++)
        for(float x=x_min; x <= x_max;x++){
            float a = line(x1,y1,x2,y2,x,y) / line(x1,y1,x2,y2,x0,y0);
            float b=line(x2,y2,x0,y0,x,y)/line(x2,y2,x0,y0,x1,y1);
            float c=line(x0,y0,x1,y1,x,y)/line(x0,y0,x1,y1,x2,y2);

            if(a > 0 && a < 1 && b>0 && b<1 && c>0 && c<1)
                set_color(x,y,image, color);
        }

}


int main()
{
    // definiraj sliku
    TGAImage image(width, height, TGAImage::RGB);

    /*   // nacrtaj nekoliko duzina
       draw_line(10, 20, 180, 60, image, white);
       draw_line(180, 80, 10, 40, image, white);

       draw_line(20, 180, 140, 170, image, red);

       draw_line(80, 40, 110, 120, image, blue);
       */
    // spremi sliku 

    draw_triangle_2d(image, 150,180,330,230,250,350,red);
    draw_triangle_2d(image,400,300,300,150,250,300,blue);

    image.flip_vertically();
    image.write_tga_file("zad1.tga");
}
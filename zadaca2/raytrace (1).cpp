#include <cmath>
#include <vector>
#include <fstream>
#include <algorithm>
#include "geometry.h"
#include "ray.h"
#include "objects.h"
#include "light.h"
#include "material.h"
#include <limits>
using namespace std;

typedef vector<Vec3f> Image;
typedef vector<Object*> Objects;
typedef vector<Light*> Lights;

// funkcija koja ispisuje sliku u .ppm file
void save_image(const Image &image, const int width, const int height, const string path)
{
    ofstream ofs;
    ofs.open(path, ofstream::binary);

    // format ppm
    ofs << "P6\n" << width << " " << height << "\n255\n";

    // ispis pixela
    for (int i = 0; i < width * height; ++i)
    {
        ofs << (char)(255 * min(max(image[i][0], 0.f), 1.f));
        ofs << (char)(255 * min(max(image[i][1], 0.f), 1.f));
        ofs << (char)(255 * min(max(image[i][2], 0.f), 1.f));
    }

    // zatvori file
    ofs.close();
}

// funkcija koja provjerava sijece li zraka jedan od objekata
bool scene_intersect(const Ray &ray, const Objects &objs, Material &hit_material, Vec3f &hit_point, Vec3f &hit_normal)
{
    float best_dist = numeric_limits<float>::max();
    float dist = numeric_limits<float>::max();

    Vec3f normal;

    for (auto obj : objs)
    {
        if (obj->ray_intersect(ray, dist, normal) && dist < best_dist)
        {
            best_dist = dist;             // udaljenost do sfere
            hit_material = obj->material; // materijal pogodjene sfere
            hit_normal = normal;          // normala na pogodjeni objekt
            hit_point = ray.origin + ray.direction * dist; // pogodjena tocka
        }
    }

    return best_dist < 1000;
}

// funkcija koja vraca boju
Vec3f cast_ray(const Ray &ray, const Objects &objs, const Lights &lights)
{
    Vec3f hit_normal;
    Vec3f hit_point;
    Material hit_material;

    if (!scene_intersect(ray, objs, hit_material, hit_point, hit_normal))
    {
        return Vec3f(0.8, 0.8, 1); // vrati boju pozadine
    }
    else
    {
        float diffuse_light_intensity = 0;

        for (auto light : lights)
        {
            Vec3f light_dir = (light->position - hit_point).normalize();
            float light_dist = (light->position - hit_point).norm();

            // SJENE
            // ideja: - rekurzivno pozovi scene_intersect od objekta do svijetla
            //        - ako se nesto nalazi izmedju svjetla i objekta,
            //          tada to svijetlo ignoriramo
            Material shadow_hit_material;
            Vec3f shadow_hit_normal;
            Vec3f shadow_hit_point;

            // zbog gresaka u zaokrizivanju moze se dogoditi da zraka zapocne
            // unutar samog objekta. Da to izbjegnemu, origin zrake  za malo
            // pomicemo u smjeru zrake
            Vec3f shadow_origin;
            if (light_dir * hit_normal < 0) // skalarni produkt je manji od 0 ako su suprotne orijentacije
            {
                shadow_origin = hit_point - hit_normal * 0.001;
            }
            else
            {
                shadow_origin = hit_point + hit_normal * 0.001;
            }
            Ray shadow_ray(shadow_origin, light_dir);

            // provjeri hoce li zraka shadow_ray presijecatiobjekt
            if (scene_intersect(shadow_ray, objs, shadow_hit_material, shadow_hit_point, shadow_hit_normal))
            {
                // zraka sijece neki objekt
                // trebamo jos provjeriti zaklanja li taj objekt svjetlo
                // tj. nalazi li se izmedju hit_point i light->position
                float dist = (shadow_hit_point - hit_point).norm();
                if (dist < light_dist)
                {
                    // objekt zaklanja svijetlo, preskacemo ovu iteraciju
                    continue;
                }
            }

            // I / r^2
            float dist_factor = light->intensity / (light_dist * light_dist);

            // difuzno sjenacanje (Lambertov model)
            diffuse_light_intensity +=  hit_material.diffuse_coef * dist_factor * max(0.f, hit_normal * light_dir);
        }

        Vec3f diffuse_color = hit_material.diffuse_color * diffuse_light_intensity;

        return diffuse_color;
    }
}

// funkcija koja napravi zraku iz točke origin
// koja prolazi kroz pixel (i, j) na slici
// (formula s predavanja 3)
Ray ray_to_pixel(Vec3f origin, int i, int j, int width, int height)
{
    Ray ray = Ray();
    ray.origin = origin;

    float fov = 1.855; // 106.26° u radijanima
    float tg = tan(fov / 2.);

    float x =  (-1 + 2 * (i + 0.5) / (float)width) * tg;
    float y = -(-1 + 2 * (j + 0.5) / (float)height);
    float z = -1;

    ray.direction = Vec3f(x, y, z).normalize();
    return ray;
}

void draw_image(Objects objs, Lights lights)
{
    // dimenzije slike
    const int width = 1024;
    const int height = 768;

    Image img(width * height);

    // ishodište zrake
    Vec3f origin = Vec3f(0, 0, 0);

    // crtanje slike, pixel po pixel
    for (int j = 0; j < height; ++j)
    {
        for (int i = 0; i < width; ++i)
        {
            Ray ray = ray_to_pixel(origin, i, j, width, height);
            Vec3f color = cast_ray(ray, objs, lights);
            img[i + j * width] = color;
        }
    }

    // snimi sliku na disk
    save_image(img, width, height, "./render.ppm");
}

struct Cuboid :Object{
    Vec3f v1, v2, n1,n2,n3;
    Cuboid(const Vec3f &v1, const Vec3f &v2,const Material &m) :v1(v1), v2(v2){
        Object::material = m;
    }

    bool ray_intersect(const Vec3f &pocetak, const Vec3f &direction,float &distance){
        float tn = numeric_limits<float> ::min();
        float tf = numeric_limits<float>::max();
        float minx= min(v1.x,v2.x);
        float miny= min(v1.y,v2.y);
        float minz=min(v1.z, v2.z);
        float maxx=max(v1.x, v2.x);
        float maxy=max(v1.y,v2.y);
        float maxz=max(v1.z, v2.z);

        if(direction.x==0){
            if(pocetak.x < minx || pocetak.x > maxx)
                return false;
        }
        else{
            float t1=(minx-pocetak.x) / direction.x;
            float t2=(maxx-pocetak.x) /direction.x;

            if(t1 > t2)
                swap(t1,t2);
            tn=max(tn, t1);
            tf=min(tf, t2);
            if(tn > tf || tf <0)
                return false;
        }
        distance=tn;

    if(direction.y==0){
        if(pocetak.y < miny || pocetak.y > maxy)
            return false;
        else{
            float t1=(miny-pocetak.y)/direction.y;
            float t2=(maxy-pocetak.y)/direction.y;
            if(t1>t2)
                swap (t1,t2);
            tn=max(tn, t1);
            tf=min(tf, t2);
            if(tn > tf || tf <0)
                return false;
        }
        distance=tn;
    }
    else{
        float t1=(minz-pocetak.z)/direction.z;
        float t2=(maxz-pocetak.z)/direction.z;
        if(t1>t2)
            swap(t1,t2);
        tn=max(tn,t1);
        tf=min(tf, t2);
        if(tn>tf || tf<0)
            return false;
    }
    distance=tn;
    return true;
    }

};

int main()
{
    // definiraj materijale
    Material red(Vec3f(1, 0, 0),1);
    red.specular_coef = 1;
    red.phong_exp = 50;

    Material green(Vec3f(0, 0.5, 0),1);
    green.specular_coef = 1;
    green.phong_exp = 1000;

    Material blue(Vec3f(0, 0, 1),1);

    Material grey(Vec3f(0.5, 0.5, 0.5),1);


    // definiraj objekte u sceni
    Sphere s1(Vec3f(-3,    0,   -16), 2, red);
    Sphere s2(Vec3f(-1.0, -1.5, -12), 2, green);
    Sphere s3(Vec3f( 1.5, -0.5, -18), 3, blue);
    Sphere s4(Vec3f( 7,    5,   -18), 4, grey);
    Objects objs = { &s1, &s2, &s3, &s4 };

    // definiraj svjetla
    Light l1 = Light(Vec3f(-20, 20, 20), 1500);
    Light l2 = Light(Vec3f(20, 30, 20), 1800);
    Lights lights = { &l1, &l2 };

    draw_image(objs, lights);
    return 0;
}
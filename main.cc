#include "utility.h"

#include "bvh.h"
#include "camera.h"
#include "constant_medium.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "quad.h"
#include "texture.h"

void bouncing_spheres() {
    HittableList world;

    auto checker_ground = make_shared<CheckerTexture>(0.32, Color(.2, .2, .2), Color(.9, .9, .9));
    world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<Lambertian>(checker_ground)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            Point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<Material> Sphere_Material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = Color::random() * Color::random();
                    Sphere_Material = make_shared<Lambertian>(albedo);
                    auto center2 = center + Vec3(0, random_double(0,.5), 0);
                    world.add(make_shared<Sphere>(center, center2, 0.2, Sphere_Material));
                } else if (choose_mat < 0.95) {
                    // Metal
                    auto albedo = Color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    Sphere_Material = make_shared<Metal>(albedo, fuzz);
                    world.add(make_shared<Sphere>(center, 0.2, Sphere_Material));
                } else {
                    // glass
                    Sphere_Material = make_shared<Dielectric>(1.5);
                    world.add(make_shared<Sphere>(center, 0.2, Sphere_Material));
                }
            }
        }
    }

    auto Material1 = make_shared<Dielectric>(1.5);
    world.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, Material1));

    auto Material2 = make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
    world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, Material2));

    auto Material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, Material3));

    world = HittableList(make_shared<BVHNode>(world));

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 720;
    cam.rndm_smpls_pxl = 50;
    cam.max_depth = 50;

    cam.vfov = 20;
    cam.lookfrom = Point3(13,2,3);
    cam.lookat = Point3(0,0,0);
    cam.vup = Vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist = 10.0;
    cam.background = Color(0.70, 0.80, 1.00);

    cam.render(world);
}

void checkered_spheres() {
    HittableList world;

    auto checker = make_shared<CheckerTexture>(0.5, Color(.2, .2, .2), Color(.9, .9, .9));

    world.add(make_shared<Sphere>(Point3(0, -10, 0), 10, make_shared<Lambertian>(checker)));
    world.add(make_shared<Sphere>(Point3(0, 10, 0), 10, make_shared<Lambertian>(checker)));

    world = HittableList(make_shared<BVHNode>(world));

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 400;
    cam.rndm_smpls_pxl = 50;
    cam.max_depth = 50;

    cam.vfov = 20;
    cam.lookfrom = Point3(13,2,3);
    cam.lookat = Point3(0,0,0);
    cam.vup = Vec3(0,1,0);

    cam.defocus_angle = 0;
    cam.background = Color(0.70, 0.80, 1.00);

    cam.render(world);
}

void image() {
    auto earth_texture = make_shared<ImageTexture>("link_zelda.jpg");
    auto earth_surface = make_shared<Lambertian>(earth_texture);
    auto globe = make_shared<Sphere>(Point3(0,0,0), 2, earth_surface);

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.rndm_smpls_pxl = 100;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = Point3(0,0,12);
    cam.lookat   = Point3(0,0,0);
    cam.vup      = Vec3(0,1,0);

    cam.defocus_angle = 0;
    cam.background = Color(0.70, 0.80, 1.00);

    cam.render(HittableList(globe));
}

void perlin_spheres() {
    HittableList world;

    auto per_tex = make_shared<NoiseTexture>(4);
    world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<Lambertian>(per_tex)));
    world.add(make_shared<Sphere>(Point3(0, 2, 0), 2, make_shared<Lambertian>(per_tex)));

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 720;
    cam.rndm_smpls_pxl = 100;
    cam.max_depth = 50;

    cam.vfov = 20;
    cam.lookfrom = Point3(13, 2, 3);
    cam.lookat = Point3(0, 0, 0);
    cam.vup = Vec3(0, 1, 0);

    cam.defocus_angle = 0;
    cam.background = Color(0.70, 0.80, 1.00);

    cam.render(world);
}

void quads() {
    HittableList world;

    // Materials
    auto left_red     = make_shared<Lambertian>(Color(1.0, 0.2, 0.2));
    auto back_green   = make_shared<Lambertian>(Color(0.2, 1.0, 0.2));
    auto right_blue   = make_shared<Lambertian>(Color(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<Lambertian>(Color(1.0, 0.5, 0.0));
    auto lower_teal   = make_shared<Lambertian>(Color(0.2, 0.8, 0.8));

    // Quads
    world.add(make_shared<Quad>(Point3(-3,-2, 5), Vec3(0, 0,-4), Vec3(0, 4, 0), left_red));
    world.add(make_shared<Quad>(Point3(-2,-2, 0), Vec3(4, 0, 0), Vec3(0, 4, 0), back_green));
    world.add(make_shared<Quad>(Point3( 3,-2, 1), Vec3(0, 0, 4), Vec3(0, 4, 0), right_blue));
    world.add(make_shared<Quad>(Point3(-2, 3, 1), Vec3(4, 0, 0), Vec3(0, 0, 4), upper_orange));
    world.add(make_shared<Quad>(Point3(-2,-3, 5), Vec3(4, 0, 0), Vec3(0, 0,-4), lower_teal));

    camera cam;

    cam.aspect_ratio = 1.0;
    cam.image_width = 400;
    cam.rndm_smpls_pxl = 100;
    cam.max_depth = 50;

    cam.vfov = 80;
    cam.lookfrom = Point3(0,0,9);
    cam.lookat = Point3(0,0,0);
    cam.vup = Vec3(0,1,0);

    cam.defocus_angle = 0;
    cam.background = Color(0.70, 0.80, 1.00);

    cam.render(world);
}

void simple_light() {
    HittableList world;

    auto pertext = make_shared<NoiseTexture>(6);
    world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<Lambertian>(pertext)));
    world.add(make_shared<Sphere>(Point3(0, 2, 0), 2, make_shared<Lambertian>(pertext)));

    auto difflight = make_shared<DiffuseLight>(Color(4));
    world.add(make_shared<Quad>(Point3(3, 1, -2), Vec3(2, 0, 0), Vec3(0, 2, 0), difflight));

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 600;
    cam.rndm_smpls_pxl = 100;
    cam.max_depth = 50;

    cam.vfov = 20;
    cam.lookfrom = Point3(26, 3, 6);
    cam.lookat = Point3(0,2,0);
    cam.vup = Vec3(0,1,0);

    cam.defocus_angle = 0;
    cam.background = Color(0);

    cam.render(world);
}

void cornell_box() {
    HittableList world;

    auto red = make_shared<Lambertian>(Color(.65, .05, .05));
    auto white = make_shared<Lambertian>(Color(.73, .73, .73));
    auto green = make_shared<Lambertian>(Color(.12, .45, .15));
    auto light = make_shared<DiffuseLight>(Color(15, 15, 15));

    world.add(make_shared<Quad>(Point3(555,0,0), Vec3(0,555,0), Vec3(0,0,555), green));
    world.add(make_shared<Quad>(Point3(0,0,0), Vec3(0,555,0), Vec3(0,0,555), red));
    world.add(make_shared<Quad>(Point3(343, 554, 332), Vec3(-130,0,0), Vec3(0,0,-105), light));
    world.add(make_shared<Quad>(Point3(0,0,0), Vec3(555,0,0), Vec3(0,0,555), white));
    world.add(make_shared<Quad>(Point3(555, 555, 555), Vec3(-555,0,0), Vec3(0,0,-555), white));
    world.add(make_shared<Quad>(Point3(0,0,555), Vec3(555,0,0), Vec3(0,555,0), white));

    shared_ptr<Hittable> box1 = box(Point3(0,0,0), Point3(165,330,165), white);
    box1 = make_shared<RotateY>(box1, 15);
    box1 = make_shared<Translate>(box1, Vec3(265,0,295));
    world.add(box1);

    shared_ptr<Hittable> box2 = box(Point3(0,0,0), Point3(165,165,165), white);
    box2 = make_shared<RotateY>(box2, -18);
    box2 = make_shared<Translate>(box2, Vec3(130,0,65));
    world.add(box2);

    camera cam;

    cam.aspect_ratio = 1.0;
    cam.image_width = 600;
    cam.rndm_smpls_pxl = 100;
    cam.max_depth = 50;
    cam.background = Color(0,0,0);

    cam.vfov = 40;
    cam.lookfrom = Point3(278, 278, -800);
    cam.lookat = Point3(278, 278, 0);
    cam.vup = Vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void cornell_smoke() {
    HittableList world;

    auto red   = make_shared<Lambertian>(Color(.65, .05, .05));
    auto white = make_shared<Lambertian>(Color(.73, .73, .73));
    auto green = make_shared<Lambertian>(Color(.12, .45, .15));
    auto light = make_shared<DiffuseLight>(Color(7, 7, 7));

    world.add(make_shared<Quad>(Point3(555,0,0), Vec3(0,555,0), Vec3(0,0,555), green));
    world.add(make_shared<Quad>(Point3(0,0,0), Vec3(0,555,0), Vec3(0,0,555), red));
    world.add(make_shared<Quad>(Point3(113,554,127), Vec3(330,0,0), Vec3(0,0,305), light));
    world.add(make_shared<Quad>(Point3(0,555,0), Vec3(555,0,0), Vec3(0,0,555), white));
    world.add(make_shared<Quad>(Point3(0,0,0), Vec3(555,0,0), Vec3(0,0,555), white));
    world.add(make_shared<Quad>(Point3(0,0,555), Vec3(555,0,0), Vec3(0,555,0), white));

    shared_ptr<Hittable> box1 = box(Point3(0,0,0), Point3(165,330,165), white);
    box1 = make_shared<RotateY>(box1, 15);
    box1 = make_shared<Translate>(box1, Vec3(265,0,295));

    shared_ptr<Hittable> box2 = box(Point3(0,0,0), Point3(165,165,165), white);
    box2 = make_shared<RotateY>(box2, -18);
    box2 = make_shared<Translate>(box2, Vec3(130,0,65));

    world.add(make_shared<ConstantMedium>(box1, 0.01, Color(0,0,0)));
    world.add(make_shared<ConstantMedium>(box2, 0.01, Color(1,1,1)));

    camera cam;

    cam.aspect_ratio = 1.0;
    cam.image_width = 600;
    cam.rndm_smpls_pxl = 200;
    cam.max_depth = 50;
    cam.background = Color(0,0,0);

    cam.vfov = 40;
    cam.lookfrom = Point3(278, 278, -800);
    cam.lookat = Point3(278, 278, 0);
    cam.vup = Vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void final_scene(int image_width, int samples_per_pixel, int max_depth) {
    HittableList boxes1;
    auto ground = make_shared<Lambertian>(Color(0.48, 0.83, 0.53));

    int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i*w;
            auto z0 = -1000.0 + j*w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1,101);
            auto z1 = z0 + w;

            boxes1.add(box(Point3(x0,y0,z0), Point3(x1,y1,z1), ground));
        }
    }

    HittableList world;

    world.add(make_shared<BVHNode>(boxes1));

    auto light = make_shared<DiffuseLight>(Color(7, 7, 7));
    world.add(make_shared<Quad>(Point3(123,554,147), Vec3(300,0,0), Vec3(0,0,265), light));

    auto center1 = Point3(400, 400, 200);
    auto center2 = center1 + Vec3(30,0,0);
    auto sphere_material = make_shared<Lambertian>(Color(0.7, 0.3, 0.1));
    world.add(make_shared<Sphere>(center1, center2, 50, sphere_material));

    world.add(make_shared<Sphere>(Point3(260, 150, 45), 50, make_shared<Dielectric>(1.5)));
    world.add(make_shared<Sphere>(
        Point3(0, 150, 145), 50, make_shared<Metal>(Color(0.8, 0.8, 0.9), 1.0)
    ));

    auto boundary = make_shared<Sphere>(Point3(360,150,145), 70, make_shared<Dielectric>(1.5));
    world.add(boundary);
    world.add(make_shared<ConstantMedium>(boundary, 0.2, Color(0.2, 0.4, 0.9)));
    boundary = make_shared<Sphere>(Point3(0,0,0), 5000, make_shared<Dielectric>(1.5));
    world.add(make_shared<ConstantMedium>(boundary, .0001, Color(1,1,1)));

    auto emat = make_shared<Lambertian>(make_shared<ImageTexture>("link_zelda.jpg"));
    world.add(make_shared<Sphere>(Point3(400,200,400), 100, emat));
    auto pertext = make_shared<NoiseTexture>(0.2);
    world.add(make_shared<Sphere>(Point3(220,280,300), 80, make_shared<Lambertian>(pertext)));

    HittableList boxes2;
    auto white = make_shared<Lambertian>(Color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<Sphere>(Point3::random(0,165), 10, white));
    }

    world.add(make_shared<Translate>(
        make_shared<RotateY>(
            make_shared<BVHNode>(boxes2), 15),
            Vec3(-100,270,395)
        )
    );

    camera cam;

    cam.aspect_ratio = 1.0;
    cam.image_width = image_width;
    cam.rndm_smpls_pxl = samples_per_pixel;
    cam.max_depth = max_depth;
    cam.background = Color(0,0,0);

    cam.vfov = 40;
    cam.lookfrom = Point3(478, 278, -600);
    cam.lookat = Point3(278, 278, 0);
    cam.vup = Vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

int main() {
    switch(9) {
        case 1: bouncing_spheres(); break;
        case 2: checkered_spheres(); break;
        case 3: image(); break;
        case 4: perlin_spheres(); break;
        case 5: quads(); break;
        case 6: simple_light(); break;
        case 7: cornell_box(); break;
        case 8: cornell_smoke(); break;
        case 9:  final_scene(800, 1000, 40); break;
        default: final_scene(400, 250, 4); break;
    }
}

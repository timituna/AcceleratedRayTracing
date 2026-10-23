#include "utility.h"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

#include <curand_kernel.h>

#define checkCudaErrors(val) check_cuda( (val), #val, __FILE__, __LINE__ )
#define A 22
#define B 22
constexpr int NUM_RND_OBJ = A * B;
constexpr int NUM_OBJ = NUM_RND_OBJ + 4;

void check_cuda(cudaError_t result, char const *const func, const char *const file, int const line) {
	if (result) {
		std::cerr << "CUDA error = " << static_cast<unsigned int>(result) << " at " <<
		file << ":" << line << " '" << func << "' \n";
		// Make sure we call CUDA Device Reset before exiting
		cudaDeviceReset();
		exit(99);
	}
}

__device__ 
float hit_sphere(const Point3& center, float radius, const Ray& r) {
    Vec3 oc = center - r.get_origin();
    float a = r.get_direction().length_squared();
    float h = dot(r.get_direction(), oc);
    float c = oc.length_squared() - radius * radius;
    float discriminant = h * h - a * c;
    
    if (discriminant < 0) {
        return -1.0f;
    }
    else {
        return (h - std::sqrt(discriminant)) / a;
    }
}

__global__
void render_init(int max_x, int max_y, curandState* rand_state) {
   int i = threadIdx.x + blockIdx.x * blockDim.x;
   int j = threadIdx.y + blockIdx.y * blockDim.y;
   if((i >= max_x) || (j >= max_y)) return;
   int pixel_index = j * max_x + i;
   //Each thread gets same seed, a different sequence number, no offset
   curand_init(2004, pixel_index, 0, &rand_state[pixel_index]);
}

__global__
void render_kernel(Vec3* fb, Camera* cam, Hittable** world, curandState* rand_state) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;
    int image_height = cam->get_image_height();
    int image_width = cam->image_width;
    if(i >= image_width || j >= image_height) return;

    int pixel_idx = j * image_width + i;
    curandState local_rand_state = rand_state[pixel_idx];

    Color pixel_color = cam->render(i, j, fb, world, &local_rand_state);

    fb[pixel_idx] = pixel_color;
}

__global__
void create_world(Hittable** d_list, Hittable** d_world, curandState* rand_state) {
    if(threadIdx.x == 0 && blockIdx.x == 0) {
        *(d_list) = new Sphere(Point3(0, -1000, 0), 1000.0f, new Lambertian(Color(0.5)));
        int counter = 1;

        for(int a = -11; a < 11; a++) {
            for(int b = -11; b < 11; b++) {
                auto choose_mat = d_random_float(rand_state);
                Point3 center(a + 0.9f * d_random_float(rand_state), 0.2f, 
                    b + 0.9f * d_random_float(rand_state));
                
                if((center - Point3(4, 0.2, 0)).length() > 0.9f) {

                    if(choose_mat < 0.7f) {
                        auto albedo = Color::random(rand_state) * Color::random(rand_state);
                        d_list[counter++] = new Sphere(center, 0.2f, new Lambertian(albedo));  
                    }
                    else if(choose_mat < 0.9f) {
                        auto albedo = Color::random(rand_state, 0.5f, 1.0f);
                        auto fuzz = d_random_float(rand_state, 0.0f, 0.5f);
                        d_list[counter++] = new Sphere(center, 0.2f, new Metal(albedo, fuzz));
                    }
                    else {
                        d_list[counter++] = new Sphere(center, 0.2f, new Dielectric(1.5f));
                    }
                }
            }
        }

        d_list[counter++] = new Sphere(Point3(0, 1, 0), 1.0f, new Dielectric(1.5f));
        d_list[counter++] = new Sphere(Point3(-4, 1, 0), 1.0f, new Lambertian(Color(0.4, 0.2, 0.1)));
        d_list[counter++] = new Sphere(Point3(4, 1, 0), 1.0f, new Metal(Color(0.7, 0.6, 0.5), 0.1f));
        *(d_world) = new HittableList(d_list, counter);
    }
}

__global__ 
void free_world(Hittable** d_list, Hittable** d_world) {
    auto* wl = static_cast<HittableList*>(*d_world);

    for(int i = 0; i < wl->size; i++) {
        delete *(d_list + i);
    }
    delete *d_world;
}

void output_image(Vec3* fb, int image_width, int image_height) {
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height - 1; j >= 0; j--) { 
        //PPM row 0 is the top, framebuffer row 0 is the bottom
		//Write pixels from top to bottom of the buffer 
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            size_t pixel_idx = j * image_width + i;
            
            auto rgb = fb[pixel_idx];

            write_color(std::cout, rgb);
        }
    }

    std::clog << "\rDone.                 \n";
}

int main() {

    size_t sz = 8192;  // 32 KB per thread — try 16 KB first, go to 128 KB if needed
    cudaError_t err = cudaDeviceSetLimit(cudaLimitStackSize, sz);
    if (err != cudaSuccess) {
        std::cerr << "cudaDeviceSetLimit failed: " << cudaGetErrorString(err) << "\n";
        std::exit(1);
    }

    size_t actual = 0;
    cudaDeviceGetLimit(&actual, cudaLimitStackSize);
    std::clog << "actual stack size: " << actual << " bytes\n";

    //World
    Hittable** d_list;
    checkCudaErrors(cudaMalloc((void**)&d_list, NUM_OBJ * sizeof(Hittable*)));
    Hittable** d_world;
    checkCudaErrors(cudaMalloc((void**)&d_world, sizeof(Hittable*)));

    Camera* cam;
    checkCudaErrors(cudaMallocManaged((void**)&cam, sizeof(Camera)));

    cam->image_width = 1920;
    cam->aspect_ratio = 16.0 / 9.0;
    cam->samples_per_pixel = 500;
    cam->max_depth = 50;
    cam->vfov = 20;
    cam->look_from = Point3(13, 2, 3);
    cam->look_at = Point3(0, 0, 0);
    cam->vup = Vec3(0, 1, 0);
    cam->defocus_angle = 0.6f;
    cam->focus_dist = 10.0f;
    cam->init();

    int num_pixels = cam->image_size();
    size_t fb_size = num_pixels * sizeof(Vec3);
    int tx = 8;
    int ty = 8;

    //Ceiling division
    dim3 blocks((cam->image_width + tx - 1) / tx, (cam->get_image_height() + ty - 1) / ty);
    dim3 threads(tx, ty);
    
    Vec3* fb;
    checkCudaErrors(cudaMallocManaged((void**)&fb, fb_size));

    curandState* d_rand_state;
    checkCudaErrors(cudaMalloc((void **)&d_rand_state, num_pixels * sizeof(curandState)));

    render_init<<<blocks, threads>>>(cam->image_width, cam->get_image_height(), d_rand_state);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    create_world<<<1, 1>>>(d_list, d_world, d_rand_state);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    render_kernel<<<blocks, threads>>>(fb, cam, d_world, d_rand_state);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    output_image(fb, cam->image_width, cam->get_image_height());
    checkCudaErrors(cudaDeviceSynchronize());

    checkCudaErrors(cudaFree(d_rand_state));
    checkCudaErrors(cudaFree(cam));

    free_world<<<1,1>>>(d_list, d_world);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    checkCudaErrors(cudaFree(d_list));
    checkCudaErrors(cudaFree(d_world));
    checkCudaErrors(cudaFree(fb));

    cudaDeviceReset();
}

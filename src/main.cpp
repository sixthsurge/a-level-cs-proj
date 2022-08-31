#include <array>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include <thread>

#include <glm/glm.hpp>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#define SFML_STATIC
#include <SFML/Graphics.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <stb_image.h>
#include <stb_image_write.h>

#include "app.hpp"
#include "bsdf.hpp"
#include "camera.hpp"
#include "image.hpp"
#include "material.hpp"
#include "render.hpp"
#include "scene.hpp"
#include "shape.hpp"
#include "utility.hpp"

void pcg(std::uint32_t& seed) {
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    seed = (word >> 22u) ^ word;
}

float random01(std::uint32_t& rngState) {
	pcg(rngState);
	return glm::fract((float) rngState / (float) 0xffffffffu);
}

glm::vec3 uniformSphereSample(glm::vec2 hash) {
	hash.x *= tau; hash.y = 2.0f * hash.y - 1.0f;
	return glm::vec3(glm::vec2(glm::sin(hash.x), glm::cos(hash.x)) * glm::sqrt(1.0f - hash.y * hash.y), hash.y);
}

glm::vec3 cosineWeightedHemisphereSample(glm::vec3 vector, glm::vec2 hash) {
	auto dir = glm::normalize(uniformSphereSample(hash) + vector);
	return glm::dot(dir, vector) < 0.0 ? -dir : dir;
}

glm::vec3 tracePath(Ray ray, const Scene& scene, int depth, std::uint32_t rngState) {
	if (depth > 5) return glm::vec3(0.0f);

	Hit hit;
	if (scene.intersects(ray, hit)) {
		glm::vec2 hash;
		hash.x = random01(rngState);
		hash.y = random01(rngState);

		Ray newRay;
		newRay.o = hit.pos + hit.normal * 0.0001f;
		newRay.d = cosineWeightedHemisphereSample(hit.normal, hash);

		glm::vec3 irradiance = tracePath(newRay, scene, depth + 1, rngState);

		return irradiance * hit.material.albedo + hit.material.emission;
	} else {
		return glm::vec3(0.0f);
	}
}

int main() {
	std::string warning, error;
	Scene scene;

	if (scene.loadFromFile("cornell_box.obj", warning, error)) {
		std::cout << "yay" << std::endl;
		std::cout << "warning" << warning << std::endl;
	} else {
		std::cout << "sadge" << std::endl;
		std::cout << "warning" << warning << std::endl;
		std::cout << "error" << error << std::endl;
	}

	PerspectiveCamera camera;
	camera.aspectRatio = 1280.0f / 720.0f;
	camera.fov = 70.0f;
	camera.position = glm::vec3(250.0f, 300.0f, -500.0f);
	camera.rotation = glm::vec2(0.0f);

	Image<u8vec3> image(1280, 720);

	image.process([&] (glm::vec2 coord) {
		glm::vec3 color(0.0f);
		coord.y = 1.0f - coord.y;

		glm::ivec2 pixel = glm::ivec2(coord * glm::vec2(1280.0f, 720.0f));

		Ray ray = camera.getPrimaryRay(coord);

		std::uint32_t rngState = 200u * std::uint32_t(pixel.y * 1280 + pixel.x);

		const int spp = 300;

		for (int i = 0; i < spp; ++i) color += tracePath(ray, scene, 0, ++rngState);

		color *= 20.0f;
		color /= (float) spp;
		color /= color + 1.0f;
		color  = glm::pow(color, glm::vec3(2.2f));

		return u8vec3(255.0f * color);
	}, 8);

	image.writeToFile("test.png");

	return 0;
}

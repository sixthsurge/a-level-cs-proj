// Includes

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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
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

int main() {
	PerspectiveCamera camera;
	camera.position = glm::vec3(0.0f);
	camera.rotation = glm::vec2(0.0f, 90.0f);
	camera.fov      = 70.0f;
	camera.aspectRatio = 1280.0f / 720.0f;

	Image<u8vec3> image(1280, 720);

	Material material;
	material.albedo = glm::vec3(1.0f, 0.0f, 0.0f);

	SphereShape sphere(material, glm::vec3(0.0, -5.0, 0.0), 0.25f);

	glm::vec3 lightDir = glm::normalize(-glm::vec3(0.3, 1.0, 0.6));

	image.process([&] (glm::vec2 coord) {
		glm::vec3 color = glm::vec3(0.0f);
		Ray ray = camera.getPrimaryRay(coord);

		float t;
		if (sphere.intersects(ray, t)) {
			color = sphere.getMaterial().albedo * (glm::max(0.0f, glm::dot(lightDir, sphere.getNormal())) * 0.5f + 0.5f);
		}

		return u8vec3(color * 255.0f);
	}, 8);

	image.writeToFile("test.png");
}

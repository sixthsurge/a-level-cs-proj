#define SFML_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include <array>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include <thread>

#include <fmt/format.h>
#include <glm/glm.hpp>
#include <SFML/Graphics.hpp>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <tinyobjloader/tiny_obj_loader.h>

#include "bsdf.hh"
#include "camera.hh"
#include "image.hh"
#include "material.hh"
#include "renderer.hh"
#include "scene.hh"
#include "shape.hh"
#include "utility.hh"

constexpr auto windowSize = glm::ivec2(960, 540); // Initial window size in pixels

enum class AppState
{
	Preview,  // Before rendering, state where the user can select a model to render and configure the camera
	Rendering // Rendering in progress
};

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
	if (depth > 3) return glm::vec3(0.0f);

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

int main()
{
	sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "Lumos");
	sf::Texture displayTexture;
	sf::Sprite sprite;

	displayTexture.create(windowSize.x, windowSize.y);

	Image<glm::vec3> radianceImage(windowSize.x, windowSize.y);
	Image<glm::u8vec4> displayImage(windowSize.x, windowSize.y);

	Scene scene;
	Renderer renderer;
	PerspectiveCamera camera;

	AppState appState = AppState::Rendering;

	camera.aspectRatio = 960.0f / 720.0f;
	camera.fov = 60.0f;
	camera.position = glm::vec3(250.0f, 300.0f, -500.0f);
	camera.rotation = glm::vec2(0.0f);

	std::string warning, error;
	if (!scene.loadFromFile("cornell_box.obj", warning, error)) {
		std::cout << "failed to load scene: " << error;
	} else {
		std::cout << warning << std::endl;
	}

	uint frameIndex = 0;

	while (window.isOpen())
	{
		++frameIndex;

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) window.close();
		}

		if (appState == AppState::Preview)
		{
			
		}		
		else
		{
			radianceImage.process(
				[&] (glm::ivec2 pos)
				{
					glm::vec2 coord = glm::vec2(pos) / glm::vec2(windowSize);

					coord.y = 1.0f - coord.y;

					Ray ray = camera.getPrimaryRay(coord);

					std::uint32_t rngState = 323920u * std::uint32_t(pos.y * windowSize.x + pos.x) + frameIndex;

					//glm::vec3 color = tracePath(ray, scene, 0, rngState);
					glm::vec3 color = glm::vec3(0.0f);

					if (frameIndex == 1)
					{
						return glm::vec3(coord.x, coord.y, 1.0f);
					}
					else
					{
						glm::vec3 previousColor = radianceImage.load(pos);
						float weight = 1.0f / (float) frameIndex;

						return mix(previousColor, color, 0.01f);
					}
				},
				8
			);

			displayImage.process(
				[&] (glm::ivec2 pos)
				{
					glm::vec3 color = radianceImage.load(pos);

					color *= 20.0f;
					color /= color + 1.0f;
					color  = glm::pow(color, glm::vec3(2.2f));

					return u8vec4(255.0f * glm::vec4(color, 1.0f));
				},
				8
			);

			displayTexture.update(displayImage.data());
			sprite.setTexture(displayTexture);
		}


		window.clear();
		window.draw(sprite);
		window.display();
	}

	displayImage.writeToFile("test.png");

	return 0;
}

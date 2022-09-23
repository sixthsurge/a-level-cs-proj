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

glm::vec3 tracePath(Ray ray, const Scene& scene, int depth, glm::vec2 random, bool insideTransparentMaterial) {
	if (depth > 5) return glm::vec3(0.0f);

	Hit hit;
	if (scene.intersects(ray, hit)) {
		glm::vec3 tint;

		Ray newRay;
		newRay.d = importanceSampleBsdf(hit.material, ray.d, hit.normal, random, insideTransparentMaterial, tint);
		newRay.o = hit.pos + 0.0001f * newRay.d;

		glm::vec3 irradiance = tracePath(newRay, scene, depth + 1, hash(random), insideTransparentMaterial);

		return irradiance * tint + hit.material.emission;
	} else {
		return glm::vec3(0.0f);
	}
}

// Filmic tonemapping operator by Jim Hejl and Richard Burgess
// Source: http://filmicworlds.com/blog/filmic-tonemapping-operators/
glm::vec3 tonemapHejlBurgess(glm::vec3 rgb)
{
	return (rgb * (6.2f * rgb + 0.5f)) / (rgb * (6.2f * rgb + 1.7f) + 0.06f);
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "Lumos");
	sf::Texture displayTexture;
	sf::Sprite sprite;

	displayTexture.create(windowSize.x, windowSize.y);

	Image<glm::vec3> radianceImage(windowSize.x, windowSize.y);
	Image<u8vec4> displayImage(windowSize.x, windowSize.y);

	Image<u8vec4> blueNoiseImage(512, 512);
	blueNoiseImage.loadFromFile("assets/bluenoise.png");

	Scene scene;
	Renderer renderer;
	PerspectiveCamera camera;

	AppState appState = AppState::Rendering;

	camera.aspectRatio = 960.0f / 720.0f;
	camera.fov = 60.0f;
	camera.position = glm::vec3(274.0f, 250.0f, -800.0f);
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

					auto blueNoise = blueNoiseImage.load(pos % 512);
					glm::vec2 random = R2(frameIndex, glm::clamp(glm::vec2(blueNoise.x, blueNoise.y) / 255.0f, 0.0f, 1.0f));

					glm::vec3 color = tracePath(ray, scene, 0, random, false);

					if (frameIndex == 1)
					{
						return color;
					}
					else
					{
						glm::vec3 previousColor = radianceImage.load(pos);
						float historyWeight = (float) frameIndex / ((float) frameIndex + 1.0f);
						return glm::mix(color, previousColor, historyWeight);
					}
				},
				8
			);

			displayImage.process(
				[&] (glm::ivec2 pos)
				{
					glm::vec3 color = radianceImage.load(pos);

					color = tonemapHejlBurgess(color);

					return u8vec4(255.0f * glm::vec4(color, 1.0f));
				},
				8
			);

			displayTexture.update(displayImage.data());
			sprite.setTexture(displayTexture);

			++frameIndex;
		}


		window.clear();
		window.draw(sprite);
		window.display();
	}

	displayImage.writeToFile("test.png");

	return 0;
}

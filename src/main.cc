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

int main()
{
	sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "Lumos");

	Renderer renderer(windowSize);
	Scene scene;
	PerspectiveCamera camera;

	renderer.setScene(&scene);
	renderer.setCamera(&camera);

	// Set up camera
	camera.aspectRatio = 960.0f / 720.0f;
	camera.fov = 60.0f;
	camera.position = glm::vec3(274.0f, 250.0f, -800.0f);
	camera.rotation = glm::vec2(0.0f);

	// Set up scene
	std::string warning, error;
	if (!scene.loadFromFile("cornell_box.obj", warning, error)) {
		std::cout << "failed to load scene: " << error;
	} else {
		std::cout << warning << std::endl;
	}

	AppState appState = AppState::Rendering;

	while (window.isOpen())
	{
		// Handle system events
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
			renderer.render();
			renderer.display(window);
		}

		window.display();
	}

	return 0;
}

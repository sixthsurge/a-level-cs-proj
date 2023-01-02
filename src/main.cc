#define SFML_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include <array>
#include <exception>
#include <functional>
#include <iostream>
#include <map>
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
#include "config.hh"
#include "image.hh"
#include "material.hh"
#include "renderer.hh"
#include "scene.hh"
#include "shape.hh"
#include "utility.hh"

int help(std::vector<std::string> args)
{
	
}

// Resets all config variables
int reset(std::vector<std::string> args)
{
	Config config(".lumos");
	config.reset();
	config.save();
	return 0;
}

// Prints out a single config variable
//
// eg: lumos get camera_fov_angle
// Prints the current camera fov angle
int get(std::vector<std::string> args)
{
	Config config(".lumos");
	fmt::print("{}: {}", args[2], config.get(args[2]));
	return 0;
}

// Set a single config variable
//
// eg: lumos set camera_fov_angle 70
// Sets the camera field-of-view angle to 70 degrees
int set(std::vector<std::string> args)
{
	Config config(".lumos");
	config.set(args[2], args[3]);
	config.save();
	fmt::print("Updated configuration variable \"{}\" to \"{}\".\n", args[2], args[3]);
	return 0;
}

int render(std::vector<std::string> args)
{
	Config config(".lumos");

	// Get window size from config file
	glm::ivec2 windowSize;
	windowSize.x = config.getInt("image_width", 1280);
	windowSize.y = config.getInt("image_height", 720);

	// Setup window to display the image as it is rendered
	sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "Lumos");

	Scene scene;
	PerspectiveCamera camera;

	Renderer renderer(windowSize);
	renderer.setScene(&scene);
	renderer.setCamera(&camera);

	// Set up camera
	camera.aspectRatio = (float) windowSize.x / (float) windowSize.y;
	camera.fov         = config.getFloat("camera_fov_angle", 60.0f);
	camera.position    = glm::vec3(config.getFloat("camera_position_x", 0.0f), config.getFloat("camera_position_y", 0.0f), config.getFloat("camera_position_z", 0.0f));
	camera.rotation    = glm::vec2(config.getFloat("camera_rotation_x", 0.0f), config.getFloat("camera_rotation_y", 0.0f));

	// Load scene from model
	std::string modelPath = config.get("model");

	std::string warning, error;
	if (!scene.loadFromFile(modelPath.c_str(), warning, error))
	{
		std::cout << "failed to load model: " << modelPath << "\n" << error;
		return 1;
	} 
	else
	{
		std::cout << warning << std::endl;
	}

	while (window.isOpen())
	{
		// Handle system events
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) window.close();
		}

		renderer.render();
		renderer.display(window);
		window.display();
	}

	return 0;
}

int main(int argc, char** argv)
{
	// Transfer command line arguments into std::vector
	std::vector<std::string> args;
	for (int i = 0; i < argc; ++i) args.emplace_back(argv[i]);

	// Map of command word (eg "render") to command handler functions
	std::map<std::string, std::function<int(std::vector<std::string>)>> handlers;

	// Link command words to handler functions 
	handlers["reset"]  = reset;
	handlers["get"]    = get;
	handlers["set"]    = set;
	handlers["render"] = render;
	
	if (handlers.count(args[1])) {
		// Handler exists for command
		return handlers[args[1]](args);
	} else {
		// No handler for command
		fmt::print("Unkown command: {}\n", args[1]);
		fmt::print("See `$ lumos help` for more information\n");
	}
}

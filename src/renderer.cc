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

#include "renderer.hh"

#include "bsdf.hh"
#include "camera.hh"
#include "config.hh"
#include "image.hh"
#include "material.hh"
#include "renderer.hh"
#include "scene.hh"
#include "shape.hh"
#include "utility.hh"

// Resolution of the precomputed blue noise texture
#define BLUE_NOISE_RES 512

// Tone mapping operator by Jim Hejl and Richard Burgess
// Maps radiance values on [0, inf] to colors on [0, 1]
// Source: http://filmicworlds.com/blog/filmic-tonemapping-operators/
inline glm::vec3 tonemapHejlBurgess(glm::vec3 rgb)
{
	return (rgb * (6.2f * rgb + 0.5f)) / (rgb * (6.2f * rgb + 1.7f) + 0.06f);
}

Renderer::Renderer(glm::ivec2 windowSize) :
    m_groupCount(2),
    m_windowSize(windowSize),
    m_radianceImage(windowSize),
    m_displayImage(windowSize),
    m_blueNoiseImage(glm::ivec2(BLUE_NOISE_RES)),
    m_scene(nullptr),
    m_camera(nullptr)
{
    m_blueNoiseImage.loadFromFile("assets/blueNoise.png");
    m_displayTexture.create((unsigned) windowSize.x, (unsigned) windowSize.y);

    Config config(".lumos");
    m_ambient.r = config.getFloat("ambient_r", 0.0f);
    m_ambient.g = config.getFloat("ambient_g", 0.0f);
    m_ambient.b = config.getFloat("ambient_b", 0.0f);

    reset();
}

glm::vec3 Renderer::tracePathSegment(const Ray& ray, const glm::vec2& random, int depth, int maxDepth, bool insideTransparentMaterial)
{
    // Return zero if the path depth exceeds the maximum path depth - preventing infinite recursion
    if (depth > maxDepth) return glm::vec3(0.0f);

    // Invoke the ray-scene intersection algorithm to determine if the ray hit anything or not
    Hit hit; // will store data about the hit surface - its material properties and normal vector
    if (m_scene->intersects(ray, hit))
    {
        glm::vec3 fr(1.0f); // Multiplicative component of the BSDF

        // Construct the new ray using BSDF importance sampling
        Ray outgoingRay;
        outgoingRay.o = hit.pos;
        outgoingRay.d = importanceSampleBsdf(hit.material, hit.normal, ray.d, random, insideTransparentMaterial, fr);

        // Add a tiny bias in the direction of the new ray to its origin to prevent self-intersections
        outgoingRay.o += outgoingRay.d * 0.0001f;

        // Sample the radiance along the new ray
        auto incidentRadiance = tracePathSegment(outgoingRay, hash(random), depth + 1, maxDepth, insideTransparentMaterial);

        // Evaluate the rendering equation integrand
        return hit.material.emission + incidentRadiance * fr;
    }
    else
    {
        return m_ambient;
    }
}

void Renderer::reset()
{
    m_frameIndex = 0;
}

void Renderer::render()
{
    if (m_scene == nullptr) return; // No scene to render
    if (m_camera == nullptr) return; // No camera to render for

    Config config(".lumos");
    int maxPathDepth = config.getInt("max_path_depth", 3);

    // Trace one path for every pixel in the image
    m_radianceImage.process([&] (glm::ivec2 pos)
    {
        // Calculate the position of this pixel on the image on [0, 1]
        auto coord = glm::vec2(pos) / glm::vec2(m_windowSize);

        // Load blue noise pattern from the precomputed texture
        auto blueNoiseInt = m_blueNoiseImage.load(pos % BLUE_NOISE_RES);
        auto blueNoise = glm::clamp(glm::vec2(blueNoiseInt) / 255.0f, 0.0f, 1.0f);

        // Calculate quasi-random numbers as input for the path tracer for this sample
        auto random = R2(m_frameIndex, blueNoise);

        // Apply a random offset to the pixel position (anti-aliasing)
        auto aaOffset = R2(m_frameIndex + 43, blueNoise);
        coord += 2.0f * (aaOffset - 0.5f) / glm::vec2(m_windowSize);

        // Get the primary ray from the camera for this pixel
        auto ray = m_camera->getPrimaryRay(coord);

        // Invoke the path tracer
        auto color = tracePathSegment(ray, random, 0, maxPathDepth, false);

        // Accumulate the path traced result in the radiance image
        if (m_frameIndex == 0)
        {
            return color;
        }
        else
        {
            auto historyColor = m_radianceImage.load(pos);
            auto historyWeight = (float) m_frameIndex / (float) (m_frameIndex + 1);

            return glm::mix(color, historyColor, historyWeight);
        }
    }, m_groupCount);

    // Increment frame counter for the next frame
    ++m_frameIndex;
}

void Renderer::display(sf::RenderWindow& window) {
    if (m_scene == nullptr) return; // No scene to render
    if (m_camera == nullptr) return; // No camera to render for

    // Update display image with the latest path-traced result
    m_displayImage.process([&] (glm::ivec2 pos)
        {
            // Load the radiance value from the radiance image
            auto radiance = m_radianceImage.load(pos);

            // Tone map the radiance to obtain the final color
            auto color = tonemapHejlBurgess(radiance);

            // Store the color in the image
			return u8vec4(255.0f * glm::vec4(color, 1.0f));
        }, m_groupCount
    );

    // Upload the display image to the GPU as a sf::Texture
    m_displayTexture.update(m_displayImage.data());

    // Display the display texture to the screen using a sf::Sprite
    window.draw(sf::Sprite(m_displayTexture));
}

void Renderer::setScene(const Scene* scene)
{
    m_scene = scene;
}

void Renderer::setCamera(const Camera* camera)
{
    m_camera = camera;
}

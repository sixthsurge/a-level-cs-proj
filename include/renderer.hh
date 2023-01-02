#pragma once

#include "camera.hh"
#include "image.hh"
#include "scene.hh"

class Renderer
{
public:
    Renderer(glm::ivec2 windowSize);

    void reset();                           // Resets the renderer, ready to render a new image
    void render();                          // Traces one path for every pixel in the image
    void display(sf::RenderWindow& window); // Displays the current image to the screen
    void preview(sf::RenderWindow& window); // Displays a preview of the scene to the screen (diffuse color only)
    void saveImage(const char* path);       // Saves the current image to a file
    void setScene(const Scene* scene);      // Sets the scene to be rendered
    void setCamera(const Camera* camera);   // Sets the camera used to render the scene

private:
    // Recursive path-tracing algorithm
    glm::vec3 tracePathSegment(const Ray& ray, const glm::vec2& random, int depth, int maxDepth, bool insideTransparentMaterial);

    int              m_frameIndex;     // Incremented each frame
    int              m_groupCount;     // Number of pixel groups (threads) used by the renderer
    glm::vec3        m_ambient;        // Color of ambient light source
    glm::ivec2       m_windowSize;     // Size of the window in pixels
	Image<glm::vec3> m_radianceImage;  // Image used to store the result of the path tracer as a floating point colour
	Image<u8vec4>    m_displayImage;   // The result of the path tracer as an 8-bit image, tone mapped and converted to sRGB
	Image<u8vec4>    m_blueNoiseImage; // Image containing 2 channels of blue noise, used for the monte-carlo sampling
	sf::Texture      m_displayTexture; // Texture used to display the image to the screen
    const Scene*     m_scene;          // The scene to render
    const Camera*    m_camera;         // The camera used to render the scene
};
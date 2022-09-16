#pragma once

#include <glm/glm.hpp>

using uint = unsigned int;

using u8vec2 = glm::vec<2, glm::u8, glm::highp>;
using u8vec3 = glm::vec<3, glm::u8, glm::highp>;
using u8vec4 = glm::vec<4, glm::u8, glm::highp>;

// Constants

const float inf = 1.0f / 0.0f; // evaluates to positive infinity (cannot be a constant expression)

constexpr float eps = 1e-6f;
constexpr float tau = 6.283185307f;
constexpr float pi  = 0.5f * tau;

constexpr float degrees = tau / 360.0f; // size of one degree in radians

// Useful classes

/*
 * A ray - stores its origin and direction
 */
struct Ray
{
    glm::vec3 o; // The ray origin in world-space
    glm::vec3 d; // The ray direction in world-space

    // Returns the point t units along the ray
    glm::vec3 operator() (float t) const { return o + d * t; }
};

/*
 * An axis-aligned bounding box
 */
struct Box
{
    glm::vec3 min; // The box's lower bound
    glm::vec3 max; // The box's upper bound
};

// Useful functions

// Returns a 3D vector of the first 3 components of a given 4D vector
// Replacement for GLSL swizzle vec4.xyz
inline glm::vec3 xyz(const glm::vec4& xyzw)
{
    return glm::vec3(xyzw.x, xyzw.y, xyzw.z);
}

// Returns a 3D vector of the first 3 components of an array
// Used to convert from vectors stored as an array to glm::vec3
inline glm::vec3 toVec3(float* array) {
    return glm::vec3(array[0], array[1], array[2]);
}

// True if x is between min and max
inline bool between(float x, float min, float max)
{
    return x >= min && x <= max;
}

inline bool between(glm::vec3 x, float min, float max)
{
    return glm::all(glm::greaterThan(x, glm::vec3(min))) && glm::all(glm::lessThan(x, glm::vec3(max)));
}

// Returns the nth value in the R2 sequence with seed s (low discrepancy sequence, results in faster convergence compared to random sampling)
inline glm::vec2 R2(int n, glm::vec2 s) {
	constexpr float phi2 = 1.3247179572f; // Plastic constant, solution to x^3 = x + 1

    constexpr glm::vec2 alpha = 1.0f / glm::vec2(phi2, phi2 * phi2);

    return glm::fract(s + (float) n * alpha);
}

// vec2->vec2 hash function by David Hoskins
// https://www.shadertoy.com/view/4djSRW
inline glm::vec2 hash(glm::vec2 p) {
	glm::vec3 p3 = glm::fract(glm::vec3(p.x, p.y, p.x) * glm::vec3(0.1031f, 0.1030f, 0.0973f));
    p3 += glm::dot(p3, glm::vec3(p3.y, p3.z, p3.x) + 33.33f);
    return glm::fract((glm::vec2(p3.x, p3.x) + glm::vec2(p3.y, p3.z)) * glm::vec2(p3.z, p3.y));
}

// Returns a 2D matrix encoding a counter clockwise rotation by theta radians
// [ cos(theta),  sin(theta)]
// [-sin(theta),  cos(theta)]
inline glm::mat2 getRotationMatrix(float theta)
{
    return glm::mat2(cos(theta), sin(theta), -sin(theta), cos(theta));
}

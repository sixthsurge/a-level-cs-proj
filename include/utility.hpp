#ifndef UTILITY
#define UTILITY

#include <glm/glm.hpp>

using uint = unsigned int;

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
struct Ray {
    glm::vec3 o; // The ray origin in world-space
    glm::vec3 d; // The ray direction in world-space

    // Returns the point t units along the ray
    glm::vec3 operator() (float t) const { return o + d * t; }
};

/*
 * An axis-aligned bounding box
 */
struct Box {
    glm::vec3 min; // The box's lower bound
    glm::vec3 max; // The box's upper bound
};

// Useful functions

// True if x is between min and max
inline bool between(float x, float min, float max) {
    return x >= min && x <= max;
}
inline bool between(glm::vec3 x, float min, float max) {
    return glm::all(glm::greaterThan(x, glm::vec3(min))) && glm::all(glm::lessThan(x, glm::vec3(max)));
}

// Returns a 2D matrix encoding a counter clockwise rotation by theta radians
// [ cos(theta),  sin(theta)]
// [-sin(theta),  cos(theta)]
inline glm::mat2 getRotationMatrix(float theta) {
    return glm::mat2(cos(theta), sin(theta), -sin(theta), cos(theta));
}

#endif // UTILITY

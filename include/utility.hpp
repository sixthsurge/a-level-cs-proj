#ifndef INCLUDE_CORE_UTILITY
#define INCLUDE_CORE_UTILITY

#include <glm/glm.hpp>

using uint = unsigned int;

//--// Constants

const float inf = 1.0f / 0.0f; // not a constant expression

constexpr float eps = 1e-6f;

//--// Useful classes

/*
 * A ray - its origin and direction
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

//--// Useful functions

// True if x is between min and max
inline bool between(float x, float min, float max) {
    return x >= min && x <= max;
}
inline bool between(glm::vec3 x, float min, float max) {
    return glm::all(glm::greaterThan(x, glm::vec3(min))) && glm::all(glm::lessThan(x, glm::vec3(max)));
}

#endif /* INCLUDE_CORE_UTILITY */

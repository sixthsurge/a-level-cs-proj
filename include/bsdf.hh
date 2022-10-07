#pragma once

#include "material.hh"
#include "utility.hh"

// Computes x^5 using 3 multiplications
inline float pow5(float x)
{
    float x2 = x * x;
    return x2 * x2 * x;
}

// Returns the fresnel coefficient according to Schlick's approximation
inline glm::vec3 fresnelSchlick(glm::vec3 f0, float cosTheta)
{
    return f0 + (1.0f - f0) * pow5(1.0f - cosTheta);
}

// Returns a uniformly sampled random direction
// I did not design this function; I found it online but can't remember the original source
inline glm::vec3 uniformSphereSample(glm::vec2 hash)
{
	hash.x *= 2.0f * pi; hash.y = 2.0f * hash.y - 1.0f;
	return glm::vec3(glm::vec2(glm::sin(hash.x), glm::cos(hash.x)) * glm::sqrt(1.0f - hash.y * hash.y), hash.y);
}

// Returns a randomly selected direction oriented to a hemisphere using uniformSphereSample
inline glm::vec3 uniformHemisphereSample(glm::vec2 hash, glm::vec3 axis)
{
    glm::vec3 dir = uniformSphereSample(hash);
    return glm::dot(dir, axis) < 0.0f ? -dir : dir;
}

// Returns a pseudo-randomly selected direction where the probability density of a direction being chosen
// is proportional to the BSDF
inline glm::vec3 importanceSampleBsdf(
    const Material& material,           // The hit material
    const glm::vec3& normal,            // The hit normal
    const glm::vec3& incidentDirection, // The incident ray direction
    const glm::vec2& random,            // Two quasi-random numbers on [0,1]
    bool& insideTransparentMaterial,    // Whether the path-tracer currently believes it is inside a transparent material like glass
    glm::vec3& tint                     // Set to the remaining, non-importance-sampled part of the BSDF. The radiance should be multiplied by this
) {
    // Calculate fresnel coefficient
    auto fresnel = fresnelSchlick(material.specular, -glm::dot(incidentDirection, normal));
    if (fresnel.x != 0.0f) tint = fresnel / fresnel.x;

    // Decide whether the scattering event represents a reflection or a refraction
    auto reflect = hash(random + 0.1f).x < fresnel.x;

    // Randomly offset normal using surface roughness
    // (fake roughness)
    auto newNormal = glm::normalize(normal + material.roughness * uniformHemisphereSample(random, normal));

    // Calculate refracted direction and check if a total internal reflection occurred
    auto eta = insideTransparentMaterial ? material.refractiveIndex : 1.0f / material.refractiveIndex;
    glm::vec3 refractedDir = glm::refract(incidentDirection, newNormal, eta);
    if (glm::length(refractedDir) < 0.5f) reflect = true; // Total internal reflection

    if (reflect) // Specular reflection
    {
        return glm::reflect(incidentDirection, newNormal);
    }
    else if (material.isOpaque) // Diffuse reflection
    {
        tint = material.diffuse;
        auto direction = glm::normalize(normal + uniformSphereSample(random));
        return glm::dot(direction, incidentDirection) < 0.0f ? direction : -direction;
    }
    else // Specular refraction
    {
        insideTransparentMaterial = !insideTransparentMaterial;
        return refractedDir;
    }
}

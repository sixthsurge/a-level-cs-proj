#pragma once

/*
 * Stores information about the material of an object, which is used by the BSDF
 */
struct Material
{
    glm::vec3 diffuse       = glm::vec3(1.0f); // Diffuse albedo. This is effectively the object's color. For metals, this property is zero because metals do not exhibit diffuse reflection
    glm::vec3 specular      = glm::vec3(0.0f); // f0 - fresnel reflectance at normal incident. This controls the reflectivity of the material. For dielectric materials like wood or grass this is monochrome, but for metals it may be colored
    glm::vec3 emission      = glm::vec3(0.0f); // Light emitted at the surface
    glm::vec3 transmittance = glm::vec3(1.0f); // For transparent materials; tints light that passes through the material
    float refractiveIndex   = 1.5f;            // For transparent materials; index of refraction of the surface
    float roughness         = 0.1f;            // The material's roughness, with 0.0 representing an ideally smooth surface and 1.0 representing a very rough surface
    bool isOpaque           = true;            // Whether the material is opaque
};

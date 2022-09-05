#pragma once

/*
 * Stores information about the material of an object, which is used by the BSDF
 */
struct Material
{
    glm::vec3 albedo = glm::vec3(1.0f); // Diffuse albedo. This is effectively the object's color. For metals, this property is not used because metals do not exhibit diffuse reflection
    glm::vec3 f0     = glm::vec3(0.0f); // f0 - fresnel reflectance at normal incident. This controls the reflectivity of the material. For dielectric materials like wood or grass this is monochrome, but for metals it may be colored
    glm::vec3 emission = glm::vec3(0.0f);
    float roughness  = 0.5f;            // The material's roughness, with 0.0 representing an ideally smooth surface and 1.0 representing a very rough surface
    bool opaque      = true;
};
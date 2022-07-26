#ifndef INCLUDE_CORE_SHAPE
#define INCLUDE_CORE_SHAPE

#include <array>
#include <initializer_list>

#include <glm/glm.hpp>

#include "core/material.h"
#include "core/utility.h"

/*
 * Base class for primitive objects from which the scene is composed - triangles, spheres, etc
 */
class Shape {
public:
    virtual ~Shape() {};

    /*
     * Returns true if the ray intersects the shape. In this case, t is set to the distance along
     * the ray to the point of intersection. This function may also store information about the
     * intersection for the Shape implementation to use later
     */
    virtual bool intersects(const Ray& ray, float& t) = 0;

    /*
     * Returns the material of the shape at the intersection position
     */
    virtual Material getMaterial() const = 0;

    /*
     * Returns a unit vector perpendicular to the shape at the last intersection position
     */
    virtual glm::vec3 getNormal() const = 0;

    /*
     * Returns the smallest possible axis-aligned box that encloses the entire shape
     */
    virtual Box getBoundingBox() const = 0;
};

class TriangleShape : public Shape {
public:
    struct Vertex {
        glm::vec3 pos; // The position of the vertex
    };

    explicit TriangleShape(std::array<Vertex, 3>&& vertices) :
        m_vertices(vertices) {}
    
    /*
     * Ray-triangle intersection calculation using the Möller-Trumbore algorithm
     *
     * I learnt the algorithm from this tutorial:
     * https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
     */
    bool intersects(const Ray& ray, float& t) override {
        // Get the position of each vertex
        glm::vec3 a = m_vertices[0].pos;
        glm::vec3 b = m_vertices[1].pos;
        glm::vec3 c = m_vertices[2].pos;

        // Compute the vectors from vertex A to the other two vertices
        glm::vec3 ab = b - a;
        glm::vec3 ac = c - a;

        // Compute the determinant of the matrix taking [t, u, v] to [x, y, z] as the scalar triple
        // product of the ray direction, ac and ab
        glm::vec3 p = glm::cross(ray.d, ac);
        float det = glm::dot(p, ab);

        if (abs(det) < eps) return false; // The ray misses the triangle

        // Precompute reciprocal of determinant
        det = 1.0f / det;

        // Compute and validate u
        float u = glm::dot(ray.o - a, p) * det;
        if (!between(u, 0.0f, 1.0f)) return false;

        // Compute and validate v
        glm::vec3 q = glm::cross(ray.o - a, ab);
        float v = glm::dot(ray.d, q) * det;
        if (v < 0.0f || u + v > 1.0f) return false;

        // Finally, compute t
        t = dot(q, ac) * det;

        // Compute normal for later
        m_normal  = normalize(cross(ac, ab));
        m_normal *= glm::sign(dot(m_normal, ray.d));

        // Store barycentric coordinates for later
        m_uv.x = u;
        m_uv.y = v;

        return true;
    }

    Material getMaterial() const override {
        Material material;
        return material;
    }

    glm::vec3 getNormal() const override {
        return m_normal;
    }

    Box getBoundingBox() const override {
        // Get the position of each vertex
        glm::vec3 a = m_vertices[0].pos;
        glm::vec3 b = m_vertices[1].pos;
        glm::vec3 c = m_vertices[2].pos;

        return {
            glm::min(a, glm::min(b, c)),
            glm::max(a, glm::max(b, c))
        };
    }

private:
    const std::array<Vertex, 3> m_vertices;

    glm::vec2 m_uv; // barycentric coordinates from last intersection
    glm::vec3 m_normal; // normal vector from last intersection
};

class SphereShape : public Shape {
public:
    SphereShape(Material material, glm::vec3 origin, float radius) :
        m_material(material),
        m_origin(origin),
        m_radius(radius) {}

    /*
     * This ray-sphere intersection algorithm is also based on the one described on scratchapixel.com
     * It works by solving the quadratic equation for the intersections between the ray and the sphere
     * If the discriminant is zero then there are no solutions and thus no intersections. Otherwise,
     * the result from solving the equation is the distance to enter and exit the sphere
     */
    bool intersects(const Ray& ray, float& t) override {
        // Translate the working space such that the sphere's origin is the origin
        glm::vec3 translatedRayOrigin = ray.o - m_origin;

        // Compute the discrimimant of the quadratic equation describing the intersections between
        // the ray and the sphere
        float b = glm::dot(translatedRayOrigin, ray.d);
        float discriminant = b * b - dot(translatedRayOrigin, translatedRayOrigin) + m_radius * m_radius;

        if (discriminant < 0.0) return false; // solutions => no intersections

        // Compute the distances to enter and to exit the sphere
        discriminant = sqrt(discriminant);
        glm::vec2 distances = -b + glm::vec2(-discriminant, discriminant);

        if (distances.x < 0.0 && distances.y < 0.0) {
            return false; // the ray is outside of the sphere and pointing away from it; no intersection
        } else if (distances.x < 0.0) {
            t = distances.y; // the ray is inside of the sphere; take the distance to exit the sphere
        } else {
            t = distances.x; // the ray is outside of the sphere and pointing towards it; take the distacne to enter the sphere
        }

        // Compute and store the normal vector to the sphere at the intersection point
        m_normal = (ray(t) - m_origin) / m_radius;

        return true;
    }

    Material getMaterial() const override {
        return m_material;
    }

    glm::vec3 getNormal() const override {
        return m_normal;
    }

    Box getBoundingBox() const override {
        return {
            m_origin - m_radius,
            m_origin + m_radius
        };
    }

private:
    const Material m_material;
    const glm::vec3 m_origin;
    const float m_radius;

    glm::vec3 m_normal; // normal vector from last intersection
};

#endif /* INCLUDE_CORE_SHAPE */

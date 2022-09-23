#pragma once

#include "material.hh"
#include "shape.hh"

// Stores information about an intersection
struct Hit
{
	glm::vec3 pos;     // Position of the point of intersection
	glm::vec3 normal;  // Normal vector at the point of intersection
	Material material; // Material at the point of intersection
};

// A scene composed of many shapes. This class is responsible for performing the ray-scene
// intersection calculation.
class Scene
{
public:
	// Adds a shape instance to the scene, taking ownership of said instance
	template <typename ShapeType>
	void add(const ShapeType* shape)
	{
		m_shapes.emplace_back(dynamic_cast<const Shape*>(shape));
	}

	// Clears the scene, deleting all existing shapes
	void clear()
	{
		m_shapes.clear();
	}

	bool loadFromFile(const char* path, std::string& warning, std::string& error)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		if (tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, path))
		{
			for (std::size_t shapeIndex = 0; shapeIndex < shapes.size(); ++shapeIndex) // For each tinyobj shape
			{ 
				std::size_t indexOffset = 0;
				std::size_t triCount = shapes[shapeIndex].mesh.num_face_vertices.size();

				for (std::size_t triIndex = 0; triIndex < triCount; ++triIndex) // For each triangle
				{ 
					// Vertices of the new triangle to be added
					std::array<TriangleShape::Vertex, 3> vertices;

					bool hasNormalData = false;
					const auto& tinyobjMaterial = materials[shapes[shapeIndex].mesh.material_ids[triIndex]];
					
					// Convert from tinyobjloader material format to Lumos material format
					Material material;
					material.diffuse         = toVec3((float*) tinyobjMaterial.diffuse);
					material.specular        = toVec3((float*) tinyobjMaterial.specular);
					material.emission        = toVec3((float*) tinyobjMaterial.ambient);
					material.transmittance   = toVec3((float*) tinyobjMaterial.transmittance);
					material.refractiveIndex = tinyobjMaterial.ior;
					material.roughness       = tinyobjMaterial.roughness == 0.0f ? 1.0f : tinyobjMaterial.roughness;
					material.isOpaque        = tinyobjMaterial.dissolve > 0.5f;

					// Set each vertex from the 3D model
					int i = 0;
					for (auto& vertex : vertices)
					{
						tinyobj::index_t indices = shapes[shapeIndex].mesh.indices[indexOffset + i];

						// Get vertex position
						vertex.pos.x = attrib.vertices[indices.vertex_index * 3 + 0];
						vertex.pos.y = attrib.vertices[indices.vertex_index * 3 + 1];
						vertex.pos.z = attrib.vertices[indices.vertex_index * 3 + 2];

						// If normal_index is negative, there is no normal data for this vertex so
						// we compute the normal later
						if (indices.normal_index >= 0)
						{
							hasNormalData = true;
							vertex.normal.x = attrib.normals[indices.normal_index * 3 + 0];
							vertex.normal.y = attrib.normals[indices.normal_index * 3 + 1];
							vertex.normal.z = attrib.normals[indices.normal_index * 3 + 2];
						}

						// If texcoord_index is negative, there is no texCoord data for this vertex
						if (indices.texcoord_index >= 0)
						{
							vertex.texCoord.x = attrib.texcoords[indices.texcoord_index * 2 + 0];
							vertex.texCoord.y = attrib.texcoords[indices.texcoord_index * 2 + 1];
						}
						else
						{
							vertex.texCoord = glm::vec2(-1.0f); // Don't use any texture
						}

						++i;
					}

					if (!hasNormalData)
					{
						// Compute normal vector from vertex positions
						// If a triangle has vertices P1, P2 and P3 then its normal vector may be
						// given by the cross product (P2 - P1) x (P3 - P1)
						glm::vec3 normal = glm::normalize(glm::cross(vertices[1].pos - vertices[0].pos, vertices[2].pos - vertices[0].pos));
						vertices[0].normal = normal;
						vertices[1].normal = normal;
						vertices[2].normal = normal;
					}

					// Add the new triangle to the scene
					add(new TriangleShape(material, vertices));

					// Update index offset for next triangle
					indexOffset += 3;
				}
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	// Returns true if the ray intersects with the scene, and stores information about the intersection in `hit`
	bool intersects(const Ray& ray, Hit& hit) const
	{
		float minT = inf;                               // distance to the point of intersection
		const Shape* closestIntersectedShape = nullptr; // pointer to the closest intersected shape
		glm::vec4 closestIntersectionInfo;              // information about the closest intersection used to compute the material and normal vectors

		// Test all shapes in order, tracking the closest one to intersect the ray. Very unoptimised.
		for (const auto& shape : m_shapes)
		{
			glm::vec4 intersectionInfo; float t;
			if (shape->intersects(ray, t, intersectionInfo) && t < minT)
			{
				closestIntersectedShape = shape.get();
				closestIntersectionInfo = intersectionInfo;
				minT = t;
			}
		}

		// If an intersection was found, get the material and normal vector at the point of intersection
		// and store it for later in `hit`
		if (closestIntersectedShape != nullptr)
		{
			hit.pos      = ray(minT);
			hit.normal   = closestIntersectedShape->getNormal(closestIntersectionInfo);
			hit.material = closestIntersectedShape->getMaterial(closestIntersectionInfo);

			// Make sure that the normal points away from the surface and is a unit vector
			hit.normal *= (glm::dot(hit.normal, ray.d) < 0.0f) ? 1.0f : -1.0f;
			hit.normal  = normalize(hit.normal);

			return true;
		}
		else
		{
			return false;
		}
	}

private:
	std::vector<std::unique_ptr<const Shape>> m_shapes;
};

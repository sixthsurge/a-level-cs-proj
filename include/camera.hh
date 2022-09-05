#pragma once

#include "utility.hh"

class Camera
{
public:
	virtual ~Camera() {};
	virtual Ray getPrimaryRay(const glm::vec2& coord) const = 0;

	glm::vec3 position; // position of the camera in the world
	glm::vec2 rotation; // azimuthal angle (yaw), altitude (pitch), in degrees
};

class PerspectiveCamera : public Camera
{
public:
	Ray getPrimaryRay(const glm::vec2& coord) const;

	float fov; // horizontal field-of-view angle, in degrees
	float aspectRatio; // image width / image height
};

#include "camera.hh"


Ray PerspectiveCamera::getPrimaryRay(const glm::vec2& coord) const {
    // Imagine the camera as a single point which represents the human viewing the image. The
    // image represents the screen in front of them, both in the real world and in the scene.
    // The angle made by the vertical edges of the "screen" and the origin point is the FoV
    // angle, this determines how far away from the viewer the screen seems to be.
    //
    // To compute the ray direction, first we must find the position of the pixel on the imaginary "screen"
    // For simplicity, we let the screen's width be 1.0 and its height be 1/aspectRatio, so that
    // the image appears neither squashed nor stretched
    // The position of the pixel with respect to the screen is `coord`, so, assuming for now that
    // the camera points along the z axis, the position of the pixel is vec3(coord.x, coord.y / aspectRatio, x)
    // where x is the perpendicular distance from the origin to the screen. Looking at the view frustum
    // from the top down, we see two right angled triangles where one angle is fov/2, the opposite side
    // is 1/2 and the adjacent side is x. Using trigonometry, x can be calculated as 0.5 / tan(fov / 2.0).
    // Finally, I normalize the position to calculate the ray direction and apply rotation matrices
    // to orient the ray to the camera's rotation

    // Calculate perpendicular distance
    float perpendicularDistance = 0.5f / glm::tan(0.5f * fov * degrees);

    // Calculate position of pixel on the screen
    glm::vec3 pixelPos = { (0.5f - coord.x), (0.5f - coord.y) / aspectRatio, perpendicularDistance };

    // Calculate ray direction, assuming that the camera is facing along the z axis
    glm::vec3 rayDir = normalize(pixelPos);

    // Compute sine and cosine of yaw and pitch angles
    float cosYaw   = cos(rotation.x * degrees), sinYaw   = sin(rotation.x * degrees);
    float cosPitch = cos(rotation.y * degrees), sinPitch = sin(rotation.y * degrees);

    // Matrices to orient the ray direction to the camera's rotation
    glm::mat3 rotateYaw   = glm::mat3(cosYaw, 0.0f, -sinYaw, 0.0f, 1.0f, 0.0f, sinYaw, 0.0f, cosYaw);
    glm::mat3 rotatePitch = glm::mat3(1.0f, 0.0f, 0.0f, 0.0f, cosPitch, sinPitch, 0.0f, -sinPitch, cosPitch);

    Ray ray;
    ray.o = this->position;
    ray.d = rotateYaw * (rotatePitch * rayDir);

    return ray;
}
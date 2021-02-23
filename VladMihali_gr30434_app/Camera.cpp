#include "Camera.hpp"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_double4x4.hpp>

namespace gps {

    
    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        Position = cameraPosition;
        WorldUp = cameraUp;
        Target = cameraTarget;
        Yaw = 0.0f;
        Pitch = 0.0f;
        MovementSpeed = 2.5f;
        MouseSensitivity = 0.1f;
        Zoom = 45.0f;
        updateCameraVectors();
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        //TODO

        return glm::lookAt(Position, Position + Front, Up);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        float velocity = MovementSpeed * speed;
        if (direction == MOVE_FORWARD)
            Position += Front * velocity;
        if (direction == MOVE_BACKWARD)
            Position -= Front * velocity;
        if (direction == MOVE_LEFT)
            Position -= Right * velocity;
        if (direction == MOVE_RIGHT)
            Position += Right * velocity;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        pitch *= MouseSensitivity;
        yaw *= MouseSensitivity;

        Yaw += pitch;
        Pitch += yaw;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    void Camera::updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }

    glm::vec3 Camera::getCameraTarget()
    {
        return Target;
    }

    glm::vec3 Camera::getCameraPosition() {
        return Position;
    }
}
#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp >

#include <vector>
#include <stdio.h>
#include <stdlib.h>

const double PI = 3.141592653589793238463;


enum Direction_Rotation {
    THETA_UP,
    THETA_DOWN,
    PHI_LEFT,
    PHI_RIGHT
};

class ShipMovement 
{
public:
    glm::vec3 ShipPos;
    glm::vec3 ShipTangent;
    glm::vec3 ShipBinormal;
    glm::mat4 Transform;
    glm::vec2 MousePos;
    float Fovy;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    float Phi;
    float Theta;
    // camera options
    float MovementSpeed;
    float rotationSpeed;
    float MouseSensitivity;
    float rotSensitivity;
    bool rotDrag;
    bool verticalDrag;
    float height;
    float verticalSensitivty;
    float screenOffset;

    // constructor with vectors
    ShipMovement(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f), float theta = 50.0f) :
        MovementSpeed(2.5f),
        rotationSpeed(60.0f),
        Phi(40.0f), Fovy(45.0f),
        Front(glm::vec3(1.0f, 0.0f, 0.0f)),
        Right(glm::vec3(0.0f, 1.0f, 0.0f)),
        MousePos(glm::vec2(0.0f)),
        verticalDrag(false),
        rotDrag(false),
        rotSensitivity(30.0f),
        verticalSensitivty(1.0f)
    {
        screenOffset = -0.5;
        ShipPos = pos;
        WorldUp = up;
        Theta = theta;
        height = 1.0f;
        updateCameraVectors();
    }

    void setPos(glm::vec3 newPos) {
        ShipPos = newPos;
    }

    void setTangent(glm::vec3 tangent) {
        ShipTangent = tangent;
    }

    void setBinormal(glm::vec3 binormal) {
        ShipBinormal = binormal;
    }

    void setTransform(glm::mat4 transform) {
        Transform = transform;
    }

    glm::mat4 GetViewMatrix()
    {
        updateCameraVectors();
        return glm::lookAt(ShipPos, ShipPos + Front, Up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboardRotation(Direction_Rotation direction, float deltaTime)
    {
        float velocity = rotationSpeed * deltaTime;
        if (direction == THETA_UP)
            Theta -= velocity;
        if (direction == THETA_DOWN)
            Theta += velocity;
        if (direction == PHI_LEFT)
            Phi -= velocity;
        if (direction == PHI_RIGHT)
            Phi += velocity;

        if (Theta > 179.0f)
            Theta = 179.0f;
        if (Theta < 01.0f)
            Theta = 01.0f;

    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        if (MousePos.x > screenOffset)
        {
            Fovy -= (float)yoffset * 1.0f;
            if (Fovy < 1.0f)
                Fovy = 1.0f;
            if (Fovy > 90.0f)
                Fovy = 90.0f;
        }

    }

    void SetRotDrag(bool value)
    {
        rotDrag = value;
    }

    void SetVerticalDrag(bool value)
    {
        verticalDrag = value;
    }


    void SetCurrentMousePos(float xPos, float yPos)
    {
        glm::vec2 pos2d{ xPos, yPos };
        if (rotDrag && (xPos > screenOffset))
        {
            glm::vec2 delta = (pos2d - MousePos);
            Phi -= delta.x * rotSensitivity;
            Theta += delta.y * rotSensitivity;
        }
        if (Theta > 179.0f)
            Theta = 179.0f;
        if (Theta < 01.0f)
            Theta = 01.0f;

        if (verticalDrag && (yPos > screenOffset))
        {
            glm::vec2 delta = (pos2d - MousePos) * -1.0f;
            height += delta.y * verticalSensitivty;
        }

        MousePos.x = xPos;
        MousePos.y = yPos;
    }

    glm::vec3 GerstnerWave(glm::vec4 wave, glm::vec3 p, glm::vec3& tangent, glm::vec3& binormal, float gravity, float time)
    {
        float steepness = wave.z;
        float waveLength = wave.w;
        float k = 2 * PI / waveLength;
        float c = glm::sqrt(gravity / k);
        glm::vec2 d = glm::normalize(glm::vec2(wave.x, wave.y));
        float f = k * (glm::dot(d, glm::vec2(p.x, p.y)) - c * time);
        float a = steepness / k;
        tangent.x = 1 - d.x * d.x * (steepness * glm::sin(f));
        tangent.y = -d.x * d.y * (steepness * glm::sin(f));
        tangent.z = d.x * (steepness * glm::cos(f));

        binormal.x = -d.x * d.y * (steepness * glm::sin(f));
        binormal.y = 1 - d.y * d.y * (steepness * glm::sin(f));
        binormal.z = d.y * (steepness * glm::cos(f));
        return glm::vec3(
            d.x * (a * glm::cos(f)),
            d.y * (a * glm::cos(f)),
            a * glm::sin(f)
        );
    }

    glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest) {
        start = glm::normalize(start);
        dest = glm::normalize(dest);

        float cosTheta = glm::dot(start, dest);
        glm::vec3 rotationAxis;

        if (cosTheta < -1 + 0.001f) {
            // special case when vectors in opposite directions:
            // there is no "ideal" rotation axis
            // So guess one; any will do as long as it's perpendicular to start
            rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
            if (glm::length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
                rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

            rotationAxis = glm::normalize(rotationAxis);
            return glm::angleAxis(glm::radians(180.0f), rotationAxis);
        }

        rotationAxis = glm::cross(start, dest);

        float s = glm::sqrt((1 + cosTheta) * 2);
        float invs = 1 / s;

        return glm::quat(
            s * 0.5f,
            rotationAxis.x * invs,
            rotationAxis.y * invs,
            rotationAxis.z * invs
        );

    }

    private:
        // calculates the front vector from the Camera's (updated) Euler Angles
        void updateCameraVectors()
        {
            glm::vec4 normal = Transform * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            glm::vec4 right = Transform * glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
            glm::mat4 tmpT = glm::rotate(glm::mat4(1.0f), glm::radians(Phi), glm::vec3(normal.x, normal.y, normal.z));
            tmpT = glm::rotate(tmpT, glm::radians(Theta), glm::vec3(right.x, right.y, right.z));
            glm::vec4 front = tmpT * Transform * glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f);
            Front = glm::vec3(front.x, front.y, front.z);
            // Front = glm::normalize(Front); 
            Up = glm::vec3(normal.x, normal.y, normal.z);
            // also re-calculate the Right and Up vector
            Right = glm::normalize(glm::cross(Up, Front));  
            glm::mat4 tmpN = glm::rotate(glm::mat4(1.0f), glm::radians(Phi), glm::vec3(Up.x, Up.y, Up.z));
            tmpN = glm::rotate(glm::mat4(1.0f), glm::radians(Theta), glm::vec3(Right.x, Right.y, Right.z));
            normal = tmpN * Transform * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            Up = glm::vec3(normal.x, normal.y, normal.z);

            ShipPos = ShipPos + Up * height;
        }
};
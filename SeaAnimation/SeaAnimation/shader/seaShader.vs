#version 330 core
#define M_PI 3.1415926535897932384626433832795

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float gravity;
uniform float time;

uniform vec4 waveA;
uniform vec4 waveB;
uniform vec4 waveC;

vec3 GerstnerWave(vec4 wave, vec3 p, inout vec3 tangent, inout vec3 binormal)
{
    float steepness = wave.z;
    float waveLength = wave.w;
    float k = 2 * M_PI / waveLength;
    float c = sqrt(gravity / k);
    vec2 d = normalize(wave.xy);
    float f = k * (dot(d, p.xy) - c * time);
    float a = steepness / k;

    tangent += vec3(
    1 - d.x * d.x * (steepness * sin(f)), 
    -d.x * d.y * (steepness * sin(f)), 
    d.x * (steepness * cos(f))
    );
    binormal += vec3(
    -d.x * d.y * (steepness * sin(f)),
    1 - d.y * d.y * (steepness * sin(f)),
    d.y * (steepness * cos(f))
    );
    return vec3(
        d.x * (a * cos(f)),
        d.y * (a * cos(f)),
        a * sin(f)
    );
}

void main()
{   
    vec3 point = aPos;
    vec3 tangent = vec3(0.0f, 0.0f, 0.0f);
    vec3 binormal = vec3(0.0f, 0.0f, 0.0f);
    vec3 p = point;
    p += GerstnerWave(waveA, point, tangent, binormal);
    p += GerstnerWave(waveB, point, tangent, binormal);
    p += GerstnerWave(waveC, point, tangent, binormal);
    vec3 aNormal = normalize(cross(tangent, binormal));

    FragPos = vec3(model * vec4(p, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; 
    TexCoords = aTexCoords;    
    gl_Position = projection * view * model * vec4(p, 1.0);
}
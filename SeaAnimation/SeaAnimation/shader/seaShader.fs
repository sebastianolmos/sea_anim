#version 330 core
out vec4 FragColor;

struct Material {
    vec3 color;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
    float shininess;
}; 

struct Light {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;

uniform sampler2D texture_tmp;
uniform sampler2D texture_dist;
  
uniform vec3 viewPos;
uniform Material material;
uniform Light light;

uniform float time;
uniform float disASize;
uniform vec2 disADir;
uniform float disASpeed;
uniform float disAStrenght;
uniform vec3 disAColor;
uniform vec2 disADiscard;
uniform bool disAInside;

uniform float disBSize;
uniform vec2 disBDir;
uniform float disBSpeed;
uniform float disBStrenght;
uniform vec3 disBColor;
uniform vec2 disBDiscard;
uniform bool disBInside;

uniform float disCSize;
uniform vec2 disCDir;
uniform float disCSpeed;
uniform float disCStrenght;
uniform vec3 disCColor;
uniform vec2 disCDiscard;
uniform bool disCInside;

vec2 getDistortion(vec2 tx_coords, float speed, float unit)
{
    vec2 displaceCoords = vec2(0.0f);
    displaceCoords.x = tx_coords.x + time * speed;
    displaceCoords.y = tx_coords.y + time * speed;

    vec4 displaceValues = texture(texture_dist, displaceCoords);
    float dv = (displaceValues[0] * 2.0f) - 1.0f;
    float fx = tx_coords[0] + dv * unit;
    float fy = tx_coords[1] + dv * unit *-1.0f;

    return vec2(fx, fy);
}

vec3 getNoise(sampler2D tx_sampler, vec2 direction, float speed, float strenght, float size, vec3 color, vec2 discrd, bool inside)
{
    vec2 coords = getDistortion(TexCoords, 0.04f, 0.005f);
    vec2 nDir = normalize(direction);
    coords += nDir * speed * time;
    vec4 tx_color = texture(tx_sampler, coords * size);
    float p = (
    ( (tx_color.x > discrd.x && tx_color.x < discrd.y) && inside) || 
    ( (tx_color.x < discrd.x || tx_color.x > discrd.y) && !inside)
    ) ? tx_color.x: 0.0f; 
    return p * color * strenght;
}


vec3 getDistortedNoise(vec2 direction, float speed, float strenght, float size, vec3 color, vec2 discrd, bool inside)
{
    vec2 coords = TexCoords;
    vec2 nDir = normalize(direction);
    coords += nDir * speed * time;
    float distorted = texture(texture_tmp, getDistortion(coords * size, 0.04f, 0.02f)).x;
    float p = (
    ( (distorted > discrd.x && distorted < discrd.y) && inside) || 
    ( (distorted < discrd.x || distorted > discrd.y) && !inside)
    ) ? distorted: 0.0f; 
    return p * color * strenght;
}

void main()
{
    // ambient
    vec3 ambient = light.ambient * material.ambient;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    //vec3 lightDir = normalize(light.position - FragPos);
    vec3 lightDir = normalize(-light.direction); 
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
        
    vec3 result = ambient + diffuse + specular;
    vec3 newColor = material.color;
    newColor += getNoise(texture_tmp, disADir, disASpeed, disAStrenght, disASize, disAColor, disADiscard, disAInside);
    newColor += getNoise(texture_tmp, disBDir, disBSpeed, disBStrenght, disBSize, disBColor, disBDiscard, disBInside);
    newColor = result * newColor + getDistortedNoise(disCDir, disCSpeed, disCStrenght, disCSize, disCColor, disCDiscard, disCInside);
    FragColor = vec4(newColor, 1.0) ;
} 
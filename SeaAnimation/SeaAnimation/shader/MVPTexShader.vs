#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 pos;

vec3 bilboard()
{
	vec3 camRight = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 camUp = vec3(view[0][1], view[1][1], view[2][1]);

	vec3 vertexPos = pos + camRight * aPos.x * 5.0f + camUp * aPos.y * 5.0f ;
	return vertexPos;
}


void main()
{
	gl_Position = projection * view * model * vec4(bilboard(), 1.0f);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
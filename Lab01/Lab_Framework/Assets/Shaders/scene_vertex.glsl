#version 330 core

uniform vec3 view_position;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aColor;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform mat4 light_space_matrix;

out vec3 fragment_normal;
out vec3 fragment_position;
out vec4 fragment_position_light_space;
out vec2 vertexUV;
out vec3 vertexColor;

void main()
{
	vertexColor = aColor;
    fragment_normal = mat3(model_matrix) * normals;
    fragment_position = vec3(model_matrix * vec4(position, 1.0));
    fragment_position_light_space = light_space_matrix * vec4(fragment_position, 1.0);
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position.x, position.y, position.z, 1.0);
	vertexUV = aUV;
}

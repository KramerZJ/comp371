//
// COMP 371 Labs Framework
//
// Created by Nicolas Bergeron on 20/06/2019.
//
// Inspired by the following tutorials:
// - https://learnopengl.com/Getting-started/Hello-Window
// - https://learnopengl.com/Getting-started/Hello-Triangle

#include <iostream>
#include <algorithm>

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
// initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FreeImageIO.h>

#include "shaderloader.h"
#include "OBJloader.h"  //For loading .obj files
#include "OBJloaderV2.h"

using namespace std;

const char* getVertexShaderSource()
{
	// For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
	return
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;"
		"layout (location = 1) in vec3 aColor;"
		""
		"uniform mat4 worldMatrix = mat4(1.0f);"
		"uniform mat4 viewMatrix = mat4(1.0f);"
		"uniform mat4 projectionMatrix = mat4(1.0f);"
		"out vec3 vertexColor;"
		"void main()"
		"{"
		"   vertexColor = aColor;"
		"   gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
		"}";
}


const char* getFragmentShaderSource()
{
	return
		"#version 330 core\n"
		"in vec3 vertexColor;"
		"out vec4 FragColor;"
		"void main()"
		"{"
		"   FragColor = vec4(vertexColor.r, vertexColor.g, vertexColor.b, 1.0f);"
		"}";
}
const char* getTexturedVertexShaderSource()
{
	// For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
	return
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;"
		"layout (location = 1) in vec3 aColor;"
		"layout (location = 2) in vec2 aUV;"
		""
		"uniform mat4 worldMatrix;"
		"uniform mat4 viewMatrix = mat4(1.0);"  // default value for view matrix (identity)
		"uniform mat4 projectionMatrix = mat4(1.0);"
		""
		"out vec3 vertexColor;"
		"out vec2 vertexUV;"
		""
		"void main()"
		"{"
		"   vertexColor = aColor;"
		"   mat4 modelViewProjection = projectionMatrix * viewMatrix * worldMatrix;"
		"   gl_Position = modelViewProjection * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
		"   vertexUV = aUV;"
		"}";
}

const char* getTexturedFragmentShaderSource()
{
	return
		"#version 330 core\n"
		"in vec3 vertexColor;"
		"in vec2 vertexUV;"
		"uniform sampler2D textureSampler;"
		"uniform sampler2D texture0;"
		""
		"out vec4 FragColor;"
		"void main()"
		"{"
		"   vec4 textureColor = texture( textureSampler, vertexUV );"
		"   FragColor = textureColor ;"
		"}";
}


int compileAndLinkShaders(const char* vertexShaderSource, const char* fragmentShaderSource)
{
	// compile and link shader program
	// return shader program id
	// ------------------------------------

	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// link shaders
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}
void setProjectionMatrix(int shaderProgram, glm::mat4 projectionMatrix)
{
	glUseProgram(shaderProgram);
	GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projection_matrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}
void setColorProjectionMatrix(int shaderProgram, glm::mat4 projectionMatrix)
{
	glUseProgram(shaderProgram);
	GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}
void setViewMatrix(int shaderProgram, glm::mat4 viewMatrix)
{
	glUseProgram(shaderProgram);
	GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "view_matrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
}
void setColorViewMatrix(int shaderProgram, glm::mat4 viewMatrix)
{
	glUseProgram(shaderProgram);
	GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
}
void setWorldMatrix(int shaderProgram, glm::mat4 worldMatrix)
{
	glUseProgram(shaderProgram);
	GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
}
void setViewPosition(int shader, glm::vec3 viewPosition) {
	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "view_position"), 1, value_ptr(viewPosition));
}

void setModelMatrix(int shader, glm::mat4 modelMatrix)
{
	glUseProgram(shader);
	GLuint modelMatrixLocation = glGetUniformLocation(shader, "model_matrix");
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
}

void setObjectColor(int shader, glm::vec3 objectColor) {
	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "object_color"), 1, value_ptr(objectColor));
}

void setLightSpaceMatrix(int shader, glm::mat4 lightSpaceMatrix)
{
	glUseProgram(shader);
	GLuint lightSpaceMatrixLocation = glGetUniformLocation(shader, "light_space_matrix");
	glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
}

void setLightPosition(int shader, glm::vec3 lightPosition) {
	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "light_position"), 1, value_ptr(lightPosition));
}

void setLightDirection(int shader, glm::vec3 lightDirection) {
	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "light_direction"), 1, value_ptr(lightDirection));
}

void setLightColor(int shader, glm::vec3 lightColor) {
	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "light_color"), 1, value_ptr(lightColor));
}

void setLightCutoffOuterDegrees(int shader, float lightCutoffOuterDegrees) {
	glUseProgram(shader);
	glUniform1f(glGetUniformLocation(shader, "light_cutoff_outer"), cos(glm::radians(lightCutoffOuterDegrees)));
}

void setLightCutoffInnerDegrees(int shader, float lightCutoffInnerDegrees) {
	glUseProgram(shader);
	glUniform1f(glGetUniformLocation(shader, "light_cutoff_inner"), cos(glm::radians(lightCutoffInnerDegrees)));
}

void setLightNearPlane(int shader, float nearPlane) {
	glUseProgram(shader);
	glUniform1f(glGetUniformLocation(shader, "light_near_plane"), nearPlane);
}

void setLightFarPlane(int shader, float farPlane) {
	glUseProgram(shader);
	glUniform1f(glGetUniformLocation(shader, "light_far_plane"), farPlane);
}

void setShadowMapTexture(int shader, int texture) {
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "shadow_map"), texture);
}
struct TexturedColoredVertex
{
	TexturedColoredVertex( glm::vec3 _position,  glm::vec3 _color,  glm::vec2 _uv)
		: position(_position), color(_color), uv(_uv) {}

	 glm::vec3 position;
	 glm::vec3 color;
	 glm::vec2 uv;
};
TexturedColoredVertex texturedVertexArray[] = {
	TexturedColoredVertex(glm::vec3(1.0f,  0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),//0
	TexturedColoredVertex(glm::vec3(-1.0f, 0.0f,1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
	TexturedColoredVertex(glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
	TexturedColoredVertex(glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
	TexturedColoredVertex(glm::vec3(-1.0f, 0.0f,-1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
	TexturedColoredVertex(glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),//6-1
};
int createTexturedCubeVertexBufferObject()
{
	// Create a vertex array
	GLuint texturedvertexArrayObject;
	glGenVertexArrays(1, &texturedvertexArrayObject);
	glBindVertexArray(texturedvertexArrayObject);

	// Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
	GLuint vertexBufferObject;
	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texturedVertexArray), texturedVertexArray, GL_STATIC_DRAW);

	glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
		3,                   // size
		GL_FLOAT,            // type
		GL_FALSE,            // normalized?
		sizeof(TexturedColoredVertex), // stride - each vertex contain 2 vec3 (position, color)
		(void*)0             // array buffer offset
	);
	glEnableVertexAttribArray(0);


	glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(TexturedColoredVertex),
		(void*)sizeof(glm::vec3)      // color is offseted a vec3 (comes after position)
	);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2,                            // attribute 2 matches aUV in Vertex Shader
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(TexturedColoredVertex),
		(void*)(2 * sizeof(glm::vec3))      // uv is offseted by 2 vec3 (comes after position and color)
	);
	glEnableVertexAttribArray(2);

	return vertexBufferObject;
}
int createVertexArrayObject()
{
	// A vertex is a point on a polygon, it contains positions and other data (eg: colors)
	glm::vec3 vertexArray[] = {
		glm::vec3(-1.0f, 0.0f, 0.0f),//0
		glm::vec3(1.0f,  1.0f, 0.0f),//  
		glm::vec3(1.0f, 0.0f, 0.0f), //1
		glm::vec3(1.0f,  1.0f, 0.0f),// yellow  grip 
		glm::vec3(0.0f, 0.0f, 0.0f), //2
		glm::vec3(0.0f,  0.0f, 1.0f),//
		glm::vec3(0.0f, 0.0f, 5.0f), //3
		glm::vec3(0.0f,  0.0f, 1.0f),// Z axis  index 3
		glm::vec3(0.0f, 0.0f, 0.0f),// index 4
		glm::vec3(0.0f,  1.0f, 0.0f),//
		glm::vec3(0.0f, 5.0f, 0.0f), //5
		glm::vec3(0.0f,  1.0f, 0.0f),// Y axis index 5
		glm::vec3(0.0f, 0.0f, 0.0f),// index 6
		glm::vec3(1.0f,  0.0f, 0.0f),//
		glm::vec3(5.0f, 0.0f, 0.0f),//7
		glm::vec3(1.0f,  0.0f, 0.0f),// X axis index 7
		
		glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f), //left - white  8
		glm::vec3(-0.5f,-0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),

		glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),//11
		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(-0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),

		glm::vec3(0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f), // far - white 14
		glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(-0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),

		glm::vec3(0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),

		glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), // bottom - white 20
		glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),

		glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(-0.5f,-0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),

		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), // near - white 26
		glm::vec3(-0.5f,-0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),

		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),

		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), // right - white 32
		glm::vec3(0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),

		glm::vec3(0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),

		glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 1.0f, 1.0f), // top - white 38
		glm::vec3(-0.5f, 0.5f,0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.5f, 0.5f,0.5f), glm::vec3(1.0f, 1.0f, 1.0f),

		glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 1.0f, 1.0f),//41
		glm::vec3(-0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f),//44

		glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f), //New black cube left - red  45 + 36
		glm::vec3(-0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),

		glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),//
		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f, 0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, 0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f), // far - black 
		glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f, 0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, 0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),

		glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), // bottom - black 
		glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),

		glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),

		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), // near - black 
		glm::vec3(-0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), // right - black 
		glm::vec3(0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, 0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),

		glm::vec3(0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),

		glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 0.0f), // top - black 
		glm::vec3(-0.5f, 0.5f,0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, 0.5f,0.5f), glm::vec3(0.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 0.0f),//
		glm::vec3(-0.5f, 0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),//81

		glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), //New Red cube left - red  82 +36
		glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),//
		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), // far - Red 
		glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), // bottom - Red 
		glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), // near - Red 
		glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), // right - Red 
		glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), // top - Red 
		glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),//
		glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),//118
	};

	// Create a vertex array
	GLuint vertexArrayObject;
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);


	// Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
	GLuint vertexBufferObject;
	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

	glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
		3,                   // size
		GL_FLOAT,            // type
		GL_FALSE,            // normalized?
		2 * sizeof(glm::vec3), // stride - each vertex contain 2 vec3 (position, color)
		(void*)0             // array buffer offset
	);
	glEnableVertexAttribArray(0);


	glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
		3,
		GL_FLOAT,
		GL_FALSE,
		2 * sizeof(glm::vec3),
		(void*)sizeof(glm::vec3)      // color is offseted a vec3 (comes after position)
	);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	return vertexBufferObject;
}
int loadTexture(char* imagepath)
{
	// Load image using the Free Image library
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(imagepath, 0);
	FIBITMAP* image = FreeImage_Load(format, imagepath);
	FIBITMAP* image32bits = FreeImage_ConvertTo32Bits(image);

	// Get an available texture index from OpenGL
	GLuint texture = 0;
	glGenTextures(1, &texture);
	assert(texture != 0);

	// Set OpenGL filtering properties (bi-linear interpolation)
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Retrieve width and hight
	int width = FreeImage_GetWidth(image32bits);
	int height = FreeImage_GetHeight(image32bits);

	// This will upload the texture to the GPU memory
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
		0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(image32bits));

	// Free images
	FreeImage_Unload(image);
	FreeImage_Unload(image32bits);

	return texture;
}
GLuint setupModelVBO(string path, int& vertexCount) {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> UVs;

	//read the vertex data from the model's OBJ file
	loadOBJ(path.c_str(), vertices, normals, UVs);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); //Becomes active VAO
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).

	//Vertex VBO setup
	GLuint vertices_VBO;
	glGenBuffers(1, &vertices_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Normals VBO setup
	GLuint normals_VBO;
	glGenBuffers(1, &normals_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	//UVs VBO setup
	GLuint uvs_VBO;
	glGenBuffers(1, &uvs_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs, as we are using multiple VAOs)
	vertexCount = vertices.size();
	return VAO;
}
GLuint setupModelEBO(string path, int& vertexCount)
{
	vector<int> vertexIndices; //The contiguous sets of three indices of vertices, normals and UVs, used to make a triangle
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec2> UVs;

	//read the vertices from the cube.obj file
	//We won't be needing the normals or UVs for this program
	loadOBJ2(path.c_str(), vertexIndices, vertices, normals, UVs);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); //Becomes active VAO
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).

	//Vertex VBO setup
	GLuint vertices_VBO;
	glGenBuffers(1, &vertices_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Normals VBO setup
	GLuint normals_VBO;
	glGenBuffers(1, &normals_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	//UVs VBO setup
	GLuint uvs_VBO;
	glGenBuffers(1, &uvs_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	//EBO setup
	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(int), &vertexIndices.front(), GL_STATIC_DRAW);

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
	vertexCount = vertexIndices.size();
	return VAO;
}

int main(int argc, char*argv[])
{
	// Initialize GLFW and OpenGL version
	glfwInit();

#if defined(PLATFORM_OSX)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	// On windows, we set OpenGL version to 2.1, to support more hardware
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

	// Create Window and rendering context using GLFW, resolution is 1024*768
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Comp371 - HomeWork", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);


	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to create GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	

	// Compile and link shaders here ...
	int colarShaderProgram = compileAndLinkShaders(getVertexShaderSource(), getFragmentShaderSource());
	//int lightingShaderProgram= compileAndLinkShaders(lightVertexShaderSource(), lightFragmentShaderSource());
	int textureShaderProgram = compileAndLinkShaders(getTexturedVertexShaderSource(), getTexturedFragmentShaderSource());

	// Define and upload geometry to the GPU here ...
	int vbo = createVertexArrayObject();

	// Camera variables
	glm::vec3 cameraPosition(0.0f, 20.0f, 30.0f);
	glm::vec3 cameraLookAt(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
	float angle = 0;
	float rotationSpeed = 180.0f;  // 180 degrees per second
	float lastFrameTime = glfwGetTime();
	float leftRight = 0.0f;
	float forwardBack = 0.0f;
	float upDown = 0.0f;
	float speed = 2.0f;
	float cameraHorizontalAngle = 90.0f;
	float cameraVerticalAngle = -30.0f;

	double dx = 0.0f;
	double dy = 0.0f;
	float a = 0.01f;
	float b = 1.0f;
	double mousePosX, mousePosY, unusedMouse;
	// olf variables
	float olfX = 0.0f;
	float olfY = 0.0f;
	float olfZ = 0.0f;
	float olfSize = 1.0f;
	float olfSize2 = 1.0f;
	float fullRota = 0.0f;

	//bools
	bool SpaceRelease = true;

	// For frame time
	int lastMouseLeftState = GLFW_RELEASE;
	double lastMousePosX = 0.0f, lastMousePosY = 0.0f;

	bool isTrian = true;
	bool isPoint = false;
	bool isLine = false;
	float radiusLeftClick = 0.0f;//for camera leftclick zoom in

	std::string shaderPathPrefix = "../Assets/Shaders/";

	GLuint shaderScene = loadSHADER(shaderPathPrefix + "scene_vertex.glsl", shaderPathPrefix + "scene_fragment.glsl");

	GLuint shaderShadow = loadSHADER(shaderPathPrefix + "shadow_vertex.glsl", shaderPathPrefix + "shadow_fragment.glsl");


	string spherePath = "../Assets/Models/sphere.obj";
	string groundPath = "../Assets/Models/ground.obj";

	GLuint snowTextureID = loadTexture((char*)"../Assets/Textures/snow.jpg");
	GLuint carrotTextureID = loadTexture((char*)"../Assets/Textures/carrot.jpg");

	int sphereVertices;
	int groundVertices;

	GLuint sphereVAO = setupModelVBO(spherePath, sphereVertices);
	GLuint groundVAO = setupModelVBO(groundPath, groundVertices);
	
	//depthMap
	GLuint depthMapFBO;
	// Get the framebuffer
	glGenFramebuffers(1, &depthMapFBO);
	// Dimensions of the shadow texture
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	// Variable storing index to texture used for shadow mapping
	GLuint depthMap;
	// Get the texture
	glGenTextures(1, &depthMap);
	// Bind the texture so the next glTex calls affect it
	glBindTexture(GL_TEXTURE_2D, depthMap);
	// Create the texture and specify it's attributes, including widthn height, components (only depth is stored, no color information)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	// Set texture sampler parameters.
	// The two calls below tell the texture sampler inside the shader how to upsample and downsample the texture. Here we choose the nearest filtering option, which means we just use the value of the closest pixel to the chosen image coordinate.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// The two calls below tell the texture sampler inside the shader how it should deal with texture coordinates outside of the [0, 1] range. Here we decide to just tile the image.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Bind the framebuffer so the next glFramebuffer calls affect it
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	// Attach the depth map texture to the depth map framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	// Set texture unit # for shadow map
	setShadowMapTexture(shaderScene, 0);
	//*/
	glm::mat4 projectionMatrix = glm::perspective(70.0f, 1024.0f / 768.0f, 0.01f, 100.0f);

	GLuint projectionMatrixLocation = glGetUniformLocation(colarShaderProgram, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	//view Matrix
	glm::mat4 viewMatrix = lookAt(cameraPosition,  // eye
		cameraPosition + cameraLookAt,  // center
		cameraUp);
	
	setColorProjectionMatrix(colarShaderProgram, projectionMatrix);
	setColorProjectionMatrix(textureShaderProgram, projectionMatrix);


	setColorViewMatrix(colarShaderProgram, viewMatrix);
	setColorViewMatrix(textureShaderProgram, viewMatrix);
	// Set projection matrix on both shaders
	setProjectionMatrix(shaderScene, projectionMatrix);
	setProjectionMatrix(shaderShadow, projectionMatrix);


	setModelMatrix(shaderScene, glm::mat4(1.0));
	setModelMatrix(shaderShadow, glm::mat4(1.0));
	// Set view matrix on both shaders


	setViewMatrix(shaderScene, viewMatrix);
	setViewMatrix(shaderShadow, viewMatrix);


	//enable backface culling*/
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	//ther aer other ways to do it and we can specify the rotation of the vertics like couter clock or clock 
	// Entering Main Loop
	
	while (!glfwWindowShouldClose(window))
	{
		// Each frame, reset color of each pixel to glClearColor
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw geometry --------------------------------------------------------------------------------------------------------------
		glUseProgram(colarShaderProgram);
		glBindVertexArray(vbo);
		/*glbindbuffer(gl_array_buffer, vbo);*/

		float dt = glfwGetTime() - lastFrameTime;
		lastFrameTime += dt;

		//view matrix
		const float cameraAngularSpeed = 60.0f;
		cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
		cameraVerticalAngle -= dy * cameraAngularSpeed * dt;

		// Clamp vertical angle to [-85, 85] degrees
		cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
		if (cameraHorizontalAngle > 360)
		{
			cameraHorizontalAngle -= 360;
		}
		else if (cameraHorizontalAngle < -360)
		{
			cameraHorizontalAngle += 360;
		}

		float theta = glm::radians(cameraHorizontalAngle);
		float phi = glm::radians(cameraVerticalAngle);

		//cameraLookAt = glm::vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta));
		//glm::vec3 cameraSideVector = glm::cross(cameraLookAt, glm::vec3(0.0f, 1.0f, 0.0f));

		//glm::normalize(cameraSideVector);
		////view setting
		//glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.5f, -5.0f));
		//glm::mat4 viewTransform = glm::translate(viewMatrix, glm::vec3(leftRight, upDown, forwardBack));
		//viewMatrix = viewTransform * viewMatrix;
		//GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
		//glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);


		//projection setting
		//projectionMatrix = glm::perspective(70.0f, 1024.0f / 768.0f, 0.01f, 100.0f);

		GLuint projectionMatrixLocation = glGetUniformLocation(colarShaderProgram, "projectionMatrix");
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		//view Matrix
		//viewMatrix = lookAt(cameraPosition,  // eye
		//	cameraPosition + cameraLookAt,  // center
		//	cameraUp);
		float xz = pow(cameraPosition[0], 2.0f) + pow(cameraPosition[2], 2.0f);
		float xzy = xz + pow(cameraPosition[1], 2.0f);
		glm::vec3 olfPos(olfX, olfY + 3.0f, olfZ);
		float radius = sqrt(xzy) - radiusLeftClick;
		//cout.precision(5);
		//cout<< radius <<endl;
		glm::vec3 Pos = olfPos - glm::vec3(radius * cosf(phi) * cosf(theta),
			radius * sinf(phi),
			-radius * cosf(phi) * sinf(theta));
		viewMatrix = glm::lookAt(Pos, olfPos, cameraUp);//viewSwitch
		GLuint viewMatrixLocation = glGetUniformLocation(colarShaderProgram, "viewMatrix");
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

		glm::mat4 groundWorldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.01f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 0.02f, 100.0f));
		
		setModelMatrix(shaderScene, groundWorldMatrix);
		setModelMatrix(shaderShadow, groundWorldMatrix);
		setViewMatrix(shaderScene, viewMatrix);
		setViewMatrix(shaderShadow, viewMatrix);


		setViewPosition(shaderScene, cameraPosition);



		glm::vec3 lightPosition2(olfX, olfY + 30.0f, olfZ);// the location of the light in 3D space
		glm::vec3 lightFocus2(olfX, olfY, olfZ);        // the point in 3D space the light "looks" at
		glm::vec3 lightDirection2 = normalize(lightFocus2 - lightPosition2);
		float lightAngleOuter = 50.0;
		float lightAngleInner = 20.0;
		float lightNearPlane = 5.0f;
		float lightFarPlane = 80.0f;
		glm::mat4 lightProjectionMatrix2 = glm::perspective(20.0f, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, lightNearPlane, lightFarPlane);
		glm::mat4 lightViewMatrix2 = glm::lookAt(lightPosition2, lightFocus2, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 lightSpaceMatrix2 = lightProjectionMatrix2 * lightViewMatrix2;


		// Set light space matrix on both shaders
		setLightSpaceMatrix(shaderShadow, lightSpaceMatrix2);
		setLightSpaceMatrix(shaderScene, lightSpaceMatrix2);

		// Set light far and near planes on scene shader
		setLightNearPlane(shaderScene, lightNearPlane);
		setLightFarPlane(shaderScene, lightFarPlane);

		// Set light position on scene shader
		setLightPosition(shaderScene, lightPosition2);

		// Set light direction on scene shader
		setLightDirection(shaderScene, lightDirection2);

		// Set light cutoff angles on scene shader
		setLightCutoffInnerDegrees(shaderScene, lightAngleInner);
		setLightCutoffOuterDegrees(shaderScene, lightAngleOuter);

		// Set light color on scene shader
		setLightColor(shaderScene, glm::vec3(1.0, 1.0, 1.0));

		// Set object color on scene shader
		setObjectColor(shaderScene, glm::vec3(1.0, 1.0, 1.0));
		//*/

		//grip matrix
		glm::mat4 gripMatrix = glm::mat4(1.0f);
		glm::mat4 gripTranslateMatrix = glm::mat4(1.0f);
		glm::mat4 gripScalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 0.0f, 0.0f));
		glm::mat4 gripRotateMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		GLuint worldMatrixLoaction = glGetUniformLocation(colarShaderProgram, "worldMatrix");
		GLuint worldMatrix = glGetUniformLocation(textureShaderProgram, "worldMatrix");
		glm::mat4 mainTransformationWorldMatrix= glm::translate(mainTransformationWorldMatrix, glm::vec3(olfX, olfY, olfZ));
		glm::mat4 upScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(olfSize2, olfSize2, olfSize2));
		upScaleMatrix =  upScaleMatrix;

		//olaf center matrix
		glm::mat4 olfTranslationMatrix = glm::mat4(1.0f);
		glm::mat4 olfscaleMatrix = glm::mat4(1.0f);
		glm::mat4 olfRotationMatrix = glm::mat4(1.0f);
		glm::mat4 olfTranslateMatrix = glm::mat4(1.0f);
		olfscaleMatrix= glm::scale(olfscaleMatrix, glm::vec3(olfSize2, olfSize2, olfSize2));
		olfRotationMatrix= glm::rotate(olfRotationMatrix, glm::radians(fullRota), glm::vec3(0.0f, 1.0f, 0.0f));
		olfTranslateMatrix= glm::translate(olfTranslateMatrix, glm::vec3(olfX, olfY, olfZ));
		olfTranslationMatrix = olfTranslationMatrix * olfTranslateMatrix * olfRotationMatrix * olfscaleMatrix;



		//olaf Head matrix
		glm::mat4 olfHeadWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfHeadTransformationWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfHeadScaleWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfHeadRotationWorldMatrix = glm::mat4(1.0f);
		olfHeadTransformationWorldMatrix = glm::translate(olfHeadTransformationWorldMatrix, glm::vec3(  0.0f,   3.4f,   0.0f));
		olfHeadScaleWorldMatrix = glm::scale(olfHeadScaleWorldMatrix, glm::vec3(olfSize * 0.4f, olfSize * 0.4f, olfSize * 0.4f));
		olfHeadRotationWorldMatrix = glm::rotate(olfHeadRotationWorldMatrix, glm::radians(fullRota), glm::vec3(0.0f, 1.0f, 0.0f));
		olfHeadWorldMatrix = olfTranslationMatrix *olfHeadTransformationWorldMatrix * olfHeadRotationWorldMatrix  * olfHeadScaleWorldMatrix;
		//eye , nose and hair
		glm::mat4 olfEyesWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfEyesTransformationWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfEyesScaleWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfEyesRotationWorldMatrix = glm::mat4(1.0f);
		olfEyesTransformationWorldMatrix = glm::translate(olfEyesTransformationWorldMatrix, glm::vec3(  0.0f,   3.4f,   0.0f));
		olfEyesScaleWorldMatrix = glm::scale(olfEyesScaleWorldMatrix, glm::vec3(olfSize * 0.2f, olfSize * 0.2f, olfSize * 0.2f));
		olfEyesRotationWorldMatrix = glm::rotate(olfEyesRotationWorldMatrix, glm::radians(fullRota), glm::vec3(0.0f, 1.0f, 0.0f));
		olfEyesWorldMatrix = olfTranslationMatrix*olfEyesTransformationWorldMatrix * olfEyesRotationWorldMatrix  * olfEyesScaleWorldMatrix;
		//eyes offset
		glm::mat4 olfLeftEyeTransformationWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfRightEyeTransformationWorldMatrix = glm::mat4(1.0f);
		olfLeftEyeTransformationWorldMatrix= glm::translate(olfLeftEyeTransformationWorldMatrix, glm::vec3( -1.3f, 0.3f, 2.0f));
		olfRightEyeTransformationWorldMatrix = glm::translate(olfRightEyeTransformationWorldMatrix, glm::vec3(1.3f, 0.3f, 2.0f));

		glm::mat4 olfNoseWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfNoseTransformationWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfNoseScaleWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfNoseRotationWorldMatrix = glm::mat4(1.0f);
		olfNoseTransformationWorldMatrix = glm::translate(olfNoseTransformationWorldMatrix, glm::vec3(  0.0f,   3.4f,   0.0f));
		olfNoseScaleWorldMatrix = glm::scale(olfNoseScaleWorldMatrix, glm::vec3(olfSize * 0.2f, olfSize * 0.2f, olfSize * 0.4f));
		olfNoseRotationWorldMatrix = glm::rotate(olfNoseRotationWorldMatrix, glm::radians(fullRota), glm::vec3(0.0f, 1.0f, 0.0f));
		olfNoseWorldMatrix = olfTranslationMatrix * olfNoseTransformationWorldMatrix * olfNoseRotationWorldMatrix * olfNoseScaleWorldMatrix;
		//nose offset
		glm::mat4 olfNoseOffsetWorldMatrix = glm::mat4(1.0f);
		olfNoseOffsetWorldMatrix= glm::translate(olfNoseOffsetWorldMatrix, glm::vec3( 0.0f,-0.5f, 1.2f));

		glm::mat4 olfHairWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfHairTransformationWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfHairScaleWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfHairRotationWorldMatrix = glm::mat4(1.0f);
		olfHairTransformationWorldMatrix = glm::translate(olfHairTransformationWorldMatrix, glm::vec3(  0.0f,   4.0f,   0.0f));
		olfHairScaleWorldMatrix = glm::scale(olfHairScaleWorldMatrix, glm::vec3(olfSize * 0.05f, olfSize * 0.65f, olfSize * 0.05f));
		olfHairRotationWorldMatrix = glm::rotate(olfHairRotationWorldMatrix, glm::radians(fullRota), glm::vec3(0.0f, 1.0f, 0.0f));
		olfHairWorldMatrix = olfTranslationMatrix * olfHairTransformationWorldMatrix * olfHairRotationWorldMatrix  * olfHairScaleWorldMatrix;
		//hair offsets
		glm::mat4 olfHairOffset1WorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfHairOffset2WorldMatrix = glm::mat4(1.0f);
		olfHairOffset1WorldMatrix = glm::translate(olfHairOffset1WorldMatrix, glm::vec3(2.0f,  0.0f, 0.0f));
		olfHairOffset2WorldMatrix = glm::translate(olfHairOffset2WorldMatrix, glm::vec3(-2.0f, 0.0f, 0.0f));
		//olaf midBody matrix
		glm::mat4 olfMidBodyWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfMidBodyTransformationWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfMidBodyScaleWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfMidBodyRotationWorldMatrix = glm::mat4(1.0f);
		olfMidBodyTransformationWorldMatrix = glm::translate(olfMidBodyTransformationWorldMatrix, glm::vec3(  0.0f,   2.8f,   0.0f));
		olfMidBodyScaleWorldMatrix = glm::scale(olfMidBodyScaleWorldMatrix, glm::vec3(olfSize * 0.5f, olfSize * 0.5f, olfSize * 0.5f));
		olfMidBodyRotationWorldMatrix = glm::rotate(olfMidBodyRotationWorldMatrix, glm::radians(fullRota), glm::vec3(0.0f, 1.0f, 0.0f));
		olfMidBodyWorldMatrix = olfTranslationMatrix * olfMidBodyTransformationWorldMatrix * olfMidBodyRotationWorldMatrix * olfMidBodyScaleWorldMatrix;
		

		//olaf Body matrix
		glm::mat4 olfBodyWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfBodyTransformationWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfBodyScaleWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfBodyRotationWorldMatrix = glm::mat4(1.0f);
		olfBodyTransformationWorldMatrix = glm::translate(olfBodyTransformationWorldMatrix, glm::vec3(  0.0f,   1.5f,   0.0f));
		olfBodyScaleWorldMatrix = glm::scale(olfBodyScaleWorldMatrix, glm::vec3(olfSize * 0.8f, olfSize * 0.8f, olfSize * 0.8f));
		olfBodyRotationWorldMatrix = glm::rotate(olfBodyRotationWorldMatrix, glm::radians(fullRota), glm::vec3(0.0f, 1.0f, 0.0f));
		olfBodyWorldMatrix = olfTranslationMatrix * olfBodyTransformationWorldMatrix * olfBodyRotationWorldMatrix  * olfBodyScaleWorldMatrix;
		//olfWorldMatrix = glm::rotate(olfWorldMatrix, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		
		
		//olaf arms matrix
		glm::mat4 olfArmsWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfArmsTransformationWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfArmsScaleWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfArmsRotationWorldMatrix = glm::mat4(1.0f);
		olfArmsTransformationWorldMatrix = glm::translate(olfArmsTransformationWorldMatrix, glm::vec3(  0.0f,   2.8f,   0.0f));
		olfArmsScaleWorldMatrix = glm::scale(olfArmsScaleWorldMatrix, glm::vec3(olfSize * 2.0f, olfSize * 0.2f, olfSize * 0.2f));
		olfArmsRotationWorldMatrix = glm::rotate(olfArmsRotationWorldMatrix, glm::radians(fullRota), glm::vec3(0.0f, 1.0f, 0.0f));
		olfArmsWorldMatrix = olfTranslationMatrix * olfArmsTransformationWorldMatrix * olfArmsRotationWorldMatrix * olfArmsScaleWorldMatrix;

		//Arms Offset matrix
		glm::mat4 olfLeftArmTransformationWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfRightArmTransformationWorldMatrix = glm::mat4(1.0f);
		olfLeftArmTransformationWorldMatrix = glm::translate(olfLeftArmTransformationWorldMatrix, glm::vec3(-0.65f, 0.0f, 0.0f));
		olfRightArmTransformationWorldMatrix = glm::translate(olfRightArmTransformationWorldMatrix, glm::vec3(0.65f, 0.0f, 0.0f));

		//olaf feet matrix
		glm::mat4 olffeetWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olffeetTransformationWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olffeetScaleWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olffeetRotationWorldMatrix = glm::mat4(1.0f);
		olffeetTransformationWorldMatrix = glm::translate(olffeetTransformationWorldMatrix, glm::vec3(  0.0f,   0.25f,   0.0f));
		olffeetScaleWorldMatrix = glm::scale(olffeetScaleWorldMatrix, glm::vec3(olfSize * 0.5f, olfSize * 0.5f, olfSize * 0.5f));
		olffeetRotationWorldMatrix = glm::rotate(olffeetRotationWorldMatrix, glm::radians(fullRota), glm::vec3(0.0f, 1.0f, 0.0f));
		olffeetWorldMatrix = olfTranslationMatrix * olffeetTransformationWorldMatrix * olffeetRotationWorldMatrix* olffeetScaleWorldMatrix;
		

		//feet offsets
		glm::mat4 olfLeftfootTransformationWorldMatrix = glm::mat4(1.0f);
		glm::mat4 olfRightfootTransformationWorldMatrix = glm::mat4(1.0f);
		olfLeftfootTransformationWorldMatrix= glm::translate(olfLeftfootTransformationWorldMatrix, glm::vec3(-0.75f, 0.0f, 0.0f));
		olfRightfootTransformationWorldMatrix = glm::translate(olfRightfootTransformationWorldMatrix, glm::vec3(0.75f, 0.0f, 0.0f));
		
		
		
		//Olaf draw
		if (isTrian) {
			/*
			//Olaf Eyes
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfEyesWorldMatrix*olfLeftEyeTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfEyesWorldMatrix * olfRightEyeTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			//Olaf Nose
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfNoseWorldMatrix*olfNoseOffsetWorldMatrix)[0][0]);
			glDrawArrays(GL_TRIANGLES, 80, 36);
			//Olaf Hair
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &olfHairWorldMatrix[0][0]);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfHairWorldMatrix*olfHairOffset1WorldMatrix)[0][0]);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfHairWorldMatrix * olfHairOffset2WorldMatrix)[0][0]);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			

			//Olaf feet
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olffeetWorldMatrix * olfLeftfootTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_TRIANGLES, 8, 36);
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olffeetWorldMatrix * olfRightfootTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_TRIANGLES, 8, 36);
			//Olaf Arms
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfArmsWorldMatrix * olfLeftArmTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_TRIANGLES, 8, 36);
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfArmsWorldMatrix * olfRightArmTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_TRIANGLES, 8, 36);


			glBindVertexArray(sphereVAO);
			//Olaf Head
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfHeadWorldMatrix)[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, sphereVertices);
			//Olaf midBody
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &olfMidBodyWorldMatrix[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, sphereVertices);
			//Olaf Body
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &olfBodyWorldMatrix[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, sphereVertices);
			glBindVertexArray(0);
			// Draw geometry
			glUseProgram(textureShaderProgram);
			glActiveTexture(GL_TEXTURE0);
			GLuint textureLocation = glGetUniformLocation(textureShaderProgram, "textureSampler");
			glBindTexture(GL_TEXTURE_2D, carrotTextureID);
			glUniform1i(textureLocation, 0);
			glBindVertexArray(groundVAO);
			setWorldMatrix(textureShaderProgram, groundWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glBindVertexArray(0);//*/

			//===================================================================
						// Render shadow map
			
		// Use proper shader
			glUseProgram(shaderShadow);
			// Use proper image output size
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			// Bind depth map texture as output framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			// Clear depth data on the framebuffer
			glClear(GL_DEPTH_BUFFER_BIT);

			// Bind geometry
			glBindVertexArray(sphereVAO);
			//Olaf Head
			setModelMatrix(shaderShadow, olfHeadWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 0, sphereVertices);
			//Olaf midBody
			setModelMatrix(shaderShadow, olfMidBodyWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 0, sphereVertices);
			//Olaf Body
			setModelMatrix(shaderShadow, olfBodyWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 0, sphereVertices);
			glBindVertexArray(0);
			// Bind geometry
			glBindVertexArray(vbo);
			//eyes
			setModelMatrix(shaderShadow, olfEyesWorldMatrix * olfLeftEyeTransformationWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			setModelMatrix(shaderShadow, olfEyesWorldMatrix * olfRightEyeTransformationWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			//nose
			setModelMatrix(shaderShadow, olfNoseWorldMatrix * olfNoseOffsetWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 80, 36);
			//Olaf Hair
			setModelMatrix(shaderShadow,olfHairWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			setModelMatrix(shaderShadow, olfHairWorldMatrix * olfHairOffset1WorldMatrix);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			setModelMatrix(shaderShadow, olfHairWorldMatrix * olfHairOffset2WorldMatrix);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			//Olaf feet
			setModelMatrix(shaderShadow, olffeetWorldMatrix * olfLeftfootTransformationWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 8, 36);
			setModelMatrix(shaderShadow, olffeetWorldMatrix * olfRightfootTransformationWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 8, 36);
			//Olaf Arms
			setModelMatrix(shaderShadow, olfArmsWorldMatrix* olfLeftArmTransformationWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 8, 36);
			setModelMatrix(shaderShadow, olfArmsWorldMatrix* olfRightArmTransformationWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 8, 36);
			glBindVertexArray(0);

			glBindVertexArray(groundVAO);
			// Draw geometry
			setModelMatrix(shaderShadow, groundWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 0, groundVertices);
			// Unbind geometry
			glBindVertexArray(0);


			//*/
			//================================================================================================================render scene trian`
			glUseProgram(shaderScene);
			// Use proper image output size
			// Side note: we get the size from the framebuffer instead of using WIDTH and HEIGHT because of a bug with highDPI displays
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			glViewport(0, 0, width, height);
			// Bind screen as output framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// Clear color and depth data on framebuffer
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// Bind depth map texture
			glActiveTexture(GL_TEXTURE0);
			// Bind geometry
			glBindVertexArray(vbo);
			//eyes
			setModelMatrix(shaderScene, olfEyesWorldMatrix* olfLeftEyeTransformationWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			setModelMatrix(shaderScene, olfEyesWorldMatrix* olfRightEyeTransformationWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			//nose
			setModelMatrix(shaderScene, olfNoseWorldMatrix* olfNoseOffsetWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 80, 36);
			//Olaf Hair
			setModelMatrix(shaderScene, olfHairWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			setModelMatrix(shaderScene, olfHairWorldMatrix* olfHairOffset1WorldMatrix);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			setModelMatrix(shaderScene, olfHairWorldMatrix* olfHairOffset2WorldMatrix);
			glDrawArrays(GL_TRIANGLES, 44, 36);
			//Olaf feet
			setModelMatrix(shaderScene, olffeetWorldMatrix* olfLeftfootTransformationWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 8, 36);
			setModelMatrix(shaderScene, olffeetWorldMatrix* olfRightfootTransformationWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 8, 36);
			//Olaf Arms
			setModelMatrix(shaderScene, olfArmsWorldMatrix* olfLeftArmTransformationWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 8, 36);
			setModelMatrix(shaderScene, olfArmsWorldMatrix* olfRightArmTransformationWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 8, 36);
			//glBindTexture(GL_TEXTURE_2D, depthMap);
			//GLuint textureLocation = glGetUniformLocation(textureShaderProgram, "shado_wmap");
			//glUniform1i(textureLocation, 0);
			
			/*
			mat4 cubeWorldMatrix = translate(mat4(1.0f), vec3(lightPosition.x, lightPosition.y, lightPosition.z))*scale(mat4(1.0f), vec3(0.1f, 0.1f, 0.1f));
			glBindVertexArray(cubeVAO);
			setModelMatrix(shaderScene, cubeWorldMatrix);
			glDrawElements(GL_TRIANGLES, cubeVertices, GL_UNSIGNED_INT, 0);*/
			glBindVertexArray(sphereVAO);
			//Olaf Head
			glActiveTexture(GL_TEXTURE1);
			GLuint textureLocation = glGetUniformLocation(shaderScene, "texture0");
			//setWorldMatrix(textureShaderProgram, carWorldMatrix);
			glBindTexture(GL_TEXTURE_2D, snowTextureID);
			glUniform1i(textureLocation, 1);
			setModelMatrix(shaderScene, olfHeadWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 0, sphereVertices);
			//Olaf midBody
			setModelMatrix(shaderScene, olfMidBodyWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 0, sphereVertices);
			//Olaf Body
			setModelMatrix(shaderScene, olfBodyWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 0, sphereVertices);
			// Unbind geometry
			glBindVertexArray(0);
			glBindVertexArray(groundVAO);
			setModelMatrix(shaderScene, groundWorldMatrix);
			glBindTexture(GL_TEXTURE_2D, snowTextureID);
			// Draw geometry
			glDrawArrays(GL_TRIANGLES, 0, groundVertices);
			glBindTexture(GL_TEXTURE_2D, 0);
			//========================================*/




		}else if (isLine) {
			//Olaf Head
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &olfHeadWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 8, 36);
			//Olaf Eyes
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfEyesWorldMatrix * olfLeftEyeTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_LINES, 44, 36);
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfEyesWorldMatrix * olfRightEyeTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_LINES, 44, 36);
			//Olaf Nose
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfNoseWorldMatrix * olfNoseOffsetWorldMatrix)[0][0]);
			glDrawArrays(GL_LINES, 80, 36);
			//Olaf Hair
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &olfHairWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 44, 36);
			//Olaf midBody
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &olfMidBodyWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 8, 36);
			//Olaf Body
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &olfBodyWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 8, 36);

			//Olaf feet
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olffeetWorldMatrix * olfLeftfootTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_LINES, 8, 36);
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olffeetWorldMatrix * olfRightfootTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_LINES, 8, 36);
			//Olaf Arms
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfArmsWorldMatrix * olfLeftArmTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_LINES, 8, 36);
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfArmsWorldMatrix * olfRightArmTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_LINES, 8, 36);

			glBindVertexArray(0);
		}else if (isPoint) {
			//Olaf Head
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &olfHeadWorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 8, 36);
			//Olaf Eyes
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfEyesWorldMatrix* olfLeftEyeTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_POINTS, 44, 36);
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfEyesWorldMatrix* olfRightEyeTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_POINTS, 44, 36);
			//Olaf Nose
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfNoseWorldMatrix* olfNoseOffsetWorldMatrix)[0][0]);
			glDrawArrays(GL_POINTS, 80, 36);
			//Olaf Hair
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &olfHairWorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 44, 36);
			//Olaf midBody
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &olfMidBodyWorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 8, 36);
			//Olaf Body
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &olfBodyWorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 8, 36);

			//Olaf feet
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olffeetWorldMatrix * olfLeftfootTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_POINTS, 8, 36);
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olffeetWorldMatrix * olfRightfootTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_POINTS, 8, 36);
			//Olaf Arms
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfArmsWorldMatrix * olfLeftArmTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_POINTS, 8, 36);
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &(olfArmsWorldMatrix * olfRightArmTransformationWorldMatrix)[0][0]);
			glDrawArrays(GL_POINTS, 8, 36);
			
			glBindVertexArray(0);
		}
		
		setColorViewMatrix(colarShaderProgram, viewMatrix);
		setColorProjectionMatrix(colarShaderProgram, projectionMatrix);
		glUseProgram(colarShaderProgram);

		//draw grip
		for (int i = 0; i < 100; i++) {
			gripTranslateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.02f, -50.0f + i));
			gripMatrix = gripTranslateMatrix * gripScalingMatrix;
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &gripMatrix[0][0]);
			glDrawArrays(GL_LINES, 0, 2);
			gripTranslateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f + i, -0.02f, 0.0f));
			gripMatrix = gripTranslateMatrix * gripRotateMatrix * gripScalingMatrix;
			glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &gripMatrix[0][0]);
			glDrawArrays(GL_LINES, 0, 2);
		}
		//draw axis

		//x
		glm::mat4 axisMatrix = glm::mat4(1.0f);//axis matrix
		glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &axisMatrix[0][0]);
		glDrawArrays(GL_LINES, 6, 2);
		//y
		glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &axisMatrix[0][0]);
		glDrawArrays(GL_LINES, 4, 2);
		//z
		glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &axisMatrix[0][0]);
		glDrawArrays(GL_LINES, 2, 2);
		glBindVertexArray(0);
		glUseProgram(0);
		// End Frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
		// Handle inputs
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);


		


		//if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		//{
		//	forwardBack += 0.1f*speed;
		//}
		//if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		//{
		//	forwardBack -= 0.1f*speed;
		//}
		//if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		//{
		//	leftRight += 0.1f*speed;
		//}
		//if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		//{
		//	leftRight -= 0.1f*speed;
		//}
		if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			//OwnX -= 0.1f;
			olfX -= 0.1f * sin(glm::radians(90 + fullRota));
			olfZ -= 0.1f * cos(glm::radians(90 + fullRota));
		}else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{


			fullRota += 0.4f*cameraAngularSpeed* dt;

		}

		if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			//OwnX += 0.1f;
			olfX += 0.1f * sin(glm::radians(90 + fullRota));
			olfZ += 0.1f * cos(glm::radians(90 + fullRota));
			/*
			cout << "cos(radians(carRotate)):";
			cout << cos(radians(carRotate)) << endl;
			cout << "radians(carRotate):";
			cout << radians(carRotate) << endl;
			cout << "carRotate:";
			cout << carRotate << endl;*/
		}else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			fullRota -= 0.4f*cameraAngularSpeed* dt;
		}

		if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			//carOwnZ -= 0.1f;

			/*olfX -= 0.1f * sin(glm::radians(-180 + fullRota));
			olfZ -= 0.1f * cos(glm::radians(-180 + fullRota));*/
			olfY += 0.1f;
			
		}
		else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			olfX -= 0.1f * sin(glm::radians(-180 + fullRota));
			olfZ -= 0.1f * cos(glm::radians(-180 + fullRota));
		}
		if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			//carOwnZ += 0.1f;

			/*olfX += 0.1f * sin(glm::radians(-180 + fullRota));
			olfZ += 0.1f * cos(glm::radians(-180 + fullRota));*/
			olfY -= 0.1f;
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			olfX += 0.1f * sin(glm::radians(-180 + fullRota));
			olfZ += 0.1f * cos(glm::radians(-180 + fullRota));
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
		{
			upDown -= 0.1f*speed;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			upDown += 0.1f*speed;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			if (SpaceRelease) {
				float r1 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 50.0f));
				float r2 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 50.0f));
				float r3 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
				float r4 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
				if (r3 > 1.0f) {
					olfX = r1;
				}
				else {
					olfX = -r1;
				}
				if (r4 > 1.0f) {
					olfZ = r2;
				}
				else {
					olfZ = -r2;
				}
			}
			SpaceRelease = false;

		}
		else {
			SpaceRelease = true;
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {

			glfwGetCursorPos(window, &mousePosX, &unusedMouse);

			dx = mousePosX - lastMousePosX;

			lastMousePosX = mousePosX;
			lastMousePosY = unusedMouse;

		}
		else {
			
				dx = 0.0f;
			
		}
		float camRota = 0.0f;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {



			camRota = 5.0f;

			olfRotationMatrix = glm::rotate(olfRotationMatrix, glm::radians(camRota), glm::vec3(0.0f, 1.0f, 0.0f));



		}
		
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {



			camRota = 5.0f;

			olfRotationMatrix = glm::rotate(olfRotationMatrix, glm::radians(camRota), glm::vec3(0.0f, -1.0f, 0.0f));



		}
		
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {

			camRota = 5.0f;

			olfRotationMatrix = glm::rotate(olfRotationMatrix, glm::radians(camRota), glm::vec3(1.0f,0.0f,0.0f));



		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {

			camRota = 5.0f;

			olfRotationMatrix = glm::rotate(olfRotationMatrix, glm::radians(camRota), glm::vec3(-1.0f, 0.0f, 0.0f));



		}


		if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
			olfSize2 += 0.1;
			
			//upScaleMatrix = glm::scale(upScaleMatrix, glm::vec3(olfSize2 , olfSize2 , olfSize2));
		}
		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
			olfSize2 -= 0.1;
			
			//upScaleMatrix = glm::scale(upScaleMatrix, glm::vec3(olfSize2 , olfSize2 , olfSize2));
			
		}
		
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
			glfwGetCursorPos(window, &unusedMouse, &mousePosY);

			dy = mousePosY - lastMousePosY;

			lastMousePosY = mousePosY;
			lastMousePosX = unusedMouse;
		}
		else {
			
				dy = 0.0f;
			
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

			glfwGetCursorPos(window, &unusedMouse, &mousePosY);
			b = (mousePosY - lastMousePosY) * a;
			//cout << b;
			//cout << "B's value" << endl;
			xz = pow(cameraPosition[0], 2.0f) + pow(cameraPosition[2], 2.0f);
			xzy = xz + pow(cameraPosition[1], 2.0f);
			radius = (sqrt(xzy) - radiusLeftClick);
			//cout.precision(5);
			//cout<< mousePosY <<endl;
			 
			if (true) {
				if (5.0f < radius&& radius < 50.0f) {
					radiusLeftClick += b;
				}
				else if (radius >= 50.0f)
				{
					if (b > 0.0f) {
						radiusLeftClick += b;
					}

				}
				else {
					if (b < 0.0f) {
						radiusLeftClick += b;
					}
				}



			}
			else {

				if (cameraPosition[0] < 70.2f && cameraPosition[1] < 70.2f && cameraPosition[2] < 70.2f) {
					if (cameraPosition[1] > 1.2f) {

						cameraPosition += cameraLookAt * b * speed;
					}
					else {
						if (b < 0) {
							cameraPosition += cameraLookAt * b * speed;
						}
					}
				}
				else {
					if (b > 0) {
						cameraPosition += cameraLookAt * b * speed;
					}
				}

			}
			lastMousePosX = unusedMouse;
			lastMousePosY = mousePosY;
		}
		else {
			glfwGetCursorPos(window, &unusedMouse, &mousePosY);
			lastMousePosX = unusedMouse;
			lastMousePosY = mousePosY;
		}
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			isPoint = true;
			isLine = false;
			isTrian = false;
		}
		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
			isPoint = false;
			isLine = true;
			isTrian = false;
		}
		if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
			isPoint = false;
			isLine = false;
			isTrian = true;
		}



	}

	// Shutdown GLFW
	glfwTerminate();

	return 0;
}

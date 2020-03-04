#include <iostream>
#include <algorithm>
#include <list>

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // cross-platform interface for creating a graphical context,
// initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FreeImageIO.h>

using namespace glm;
using namespace std;

#include "shaderloader.h"
#include "OBJloader.h"  //For loading .obj files
#include "OBJloaderV2.h"  //For loading .obj files using a polygon list format

const char* getVertexShaderSource()
{
	// For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
	return
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;"
		"layout (location = 1) in vec3 aColor;"
		""
		"uniform mat4 worldMatrix;"
		"uniform mat4 viewMatrix = mat4(1.0);"  // default value for view matrix (identity)
		"uniform mat4 projectionMatrix = mat4(1.0);"
		""
		"out vec3 vertexColor;"
		"void main()"
		"{"
		"   vertexColor = aColor;"
		"   mat4 modelViewProjection = projectionMatrix * viewMatrix * worldMatrix;"
		"   gl_Position = modelViewProjection * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
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

/*const char* lightVertexShaderSource()
{
	// For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
	return
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;"
		"layout (location = 1) in vec3 aNormal;"
		""
		"out vec3 FragPos;"
		"out vec3 VertexNormal;"
		""
		"uniform mat4 worldMatrix;"
		"uniform mat4 viewMatrix = mat4(1.0);"  // default value for view matrix (identity)
		"uniform mat4 projectionMatrix = mat4(1.0);"
		""
		"out vec3 VertexColor;"
		"void main()"
		"{"
		"   vec3 vertexNormal = vec3(worldMatrix * vec4(aPos, 1.0f));"
		"   vec3 FragPos = mat3(transpose(inverse(worldMatrix)))* aNormal; "
		"   gl_Position = modelViewProjection * viewMatrix * worldMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
		"}";
}


const char* lightFragmentShaderSource()
{
	return
		"#version 330 core\n"
		"in vec3 FragPos;"
		"in vec3 VertexNormal;"
		"out vec4 FragColor;"
		""
		"uniform vec3 lightPos;"
		"uniform vec3 viewPos;"
		"uniform vec3 objectColor;"
		"uniform vec3 lightColor;"
		"void main()"
		"{"
		""	//ambient
		"	float ambientStrength = 0.1f;"
		"	vec3 ambient = ambientStrength * lightColor;"
		""	//diffuse
		"	vec3 norm=normalize(VertexNormal);"
		"	vec3 lightDir=normalize(lightPos-FragPos);"
		"	float diff= max(dot(norm,lightDir),0.0);"
		"	vec3 diffuse=diff * lightColor;"
		""	//specular
		"	float specularStrength = 0.5f;"
		"	vec3 viewDir = normalize(viewPos-FragPos);"
		"	vec3 reflectDir = reflect(-lightDir,norm);"
		"	float spec = pow(max(dot(viewDir,reflectDir),0.0),32);"
		"	vec3 specular = specularStrength * spec*lightColor;"
		""
		"   vec3 result = (ambient + diffuse + specular) * objectColor;"
		"	FragColor = vec4(result,1.0f);"
		"}";
}*/

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
struct TexturedColoredVertex
{
	TexturedColoredVertex(vec3 _position, vec3 _color, vec2 _uv)
		: position(_position), color(_color), uv(_uv) {}

	vec3 position;
	vec3 color;
	vec2 uv;
};

TexturedColoredVertex texturedVertexArray[] = {
	TexturedColoredVertex(vec3(1.0f,  0.0f, 1.0f), vec3(1.0f, 0.0f, 1.0f),vec2(1.0f, 1.0f)),//0
	TexturedColoredVertex(vec3(1.0f, 0.0f,-1.0f), vec3(1.0f, 0.0f, 1.0f),vec2(1.0f, 0.0f)),
	TexturedColoredVertex(vec3(-1.0f, 0.0f, -1.0f), vec3(1.0f, 0.0f, 1.0f),vec2(0.0f, 0.0f)),
	TexturedColoredVertex(vec3(1.0f, 0.0f, 1.0f), vec3(1.0f, 0.0f, 1.0f),vec2(1.0f, 1.0f)),
	TexturedColoredVertex(vec3(-1.0f, 0.0f,-1.0f), vec3(1.0f, 0.0f, 1.0f),vec2(0.0f, 0.0f)),
	TexturedColoredVertex(vec3(-1.0f, 0.0f, 1.0f), vec3(1.0f, 0.0f, 1.0f),vec2(0.0f, 1.0f)),//6-1
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
		(void*)sizeof(vec3)      // color is offseted a vec3 (comes after position)
	);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2,                            // attribute 2 matches aUV in Vertex Shader
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(TexturedColoredVertex),
		(void*)(2 * sizeof(vec3))      // uv is offseted by 2 vec3 (comes after position and color)
	);
	glEnableVertexAttribArray(2);

	return vertexBufferObject;
}
int createVertexBufferObject()
{
	// Car model
	vec3 vertexArray[] = {  // position,                            color
	vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f),//0green for Y axis
	vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f),

	vec3(0.0f, 0.0f,0.0f), vec3(0.8f, 0.8f, 0.0f),//2Red for X axis
	vec3(1.0f, 0.0f, 0.0f), vec3(0.8f, 0.8f, 0.0f),

	vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f),//4 Blue for Z axis
	vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f),

	vec3(0.0f, 0.0f, -1.0f), vec3(0.5f, 0.5f, 0.5f),//6for grip Z
	vec3(0.0f, 0.0f, 1.0f), vec3(0.5f, 0.5f, 0.5f),//7
	vec3(-1.0f, 0.0f, 0.0f), vec3(0.5f, 0.5f, 0.5f),//8 X
	vec3(1.0f, 0.0f, 0.0f), vec3(0.5f, 0.5f, 0.5f),//9
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
		2*sizeof(vec3), // stride - each vertex contain 2 vec3 (position, color)
		(void*)0             // array buffer offset
	);
	glEnableVertexAttribArray(0);


	glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
		3,
		GL_FLOAT,
		GL_FALSE,
		2*sizeof(vec3),
		(void*)sizeof(vec3)      // color is offseted a vec3 (comes after position)
	);
	glEnableVertexAttribArray(1);

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
void setProjectionMatrix(int shaderProgram, mat4 projectionMatrix)
{
	glUseProgram(shaderProgram);
	GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projection_matrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}
void setColorProjectionMatrix(int shaderProgram, mat4 projectionMatrix)
{
	glUseProgram(shaderProgram);
	GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}
void setViewMatrix(int shaderProgram, mat4 viewMatrix)
{
	glUseProgram(shaderProgram);
	GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "view_matrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
}
void setColorViewMatrix(int shaderProgram, mat4 viewMatrix)
{
	glUseProgram(shaderProgram);
	GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
}
void setWorldMatrix(int shaderProgram, mat4 worldMatrix)
{
	glUseProgram(shaderProgram);
	GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
}
void setViewPosition(int shader, vec3 viewPosition) {
	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "view_position"), 1, value_ptr(viewPosition));
}

void setModelMatrix(int shader, mat4 modelMatrix)
{
	glUseProgram(shader);
	GLuint modelMatrixLocation = glGetUniformLocation(shader, "model_matrix");
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
}

void setObjectColor(int shader, vec3 objectColor) {
	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "object_color"), 1, value_ptr(objectColor));
}

void setLightSpaceMatrix(int shader, mat4 lightSpaceMatrix)
{
	glUseProgram(shader);
	GLuint lightSpaceMatrixLocation = glGetUniformLocation(shader, "light_space_matrix");
	glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
}

void setLightPosition(int shader, vec3 lightPosition) {
	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "light_position"), 1, value_ptr(lightPosition));
}

void setLightDirection(int shader, vec3 lightDirection) {
	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "light_direction"), 1, value_ptr(lightDirection));
}

void setLightColor(int shader, vec3 lightColor) {
	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "light_color"), 1, value_ptr(lightColor));
}

void setLightCutoffOuterDegrees(int shader, float lightCutoffOuterDegrees) {
	glUseProgram(shader);
	glUniform1f(glGetUniformLocation(shader, "light_cutoff_outer"), cos(radians(lightCutoffOuterDegrees)));
}

void setLightCutoffInnerDegrees(int shader, float lightCutoffInnerDegrees) {
	glUseProgram(shader);
	glUniform1f(glGetUniformLocation(shader, "light_cutoff_inner"), cos(radians(lightCutoffInnerDegrees)));
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

	// Create Window and rendering context using GLFW, resolution is 800x600
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Comp371 - A1", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// @TODO 3 - Disable mouse cursor
	// ...
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

	int vbo = createVertexBufferObject();
	int textureVbo = createTexturedCubeVertexBufferObject();
	// We can set the shader once, since we have only one
	//glUseProgram(shaderProgram);
	std::string shaderPathPrefix = "../Assets/Shaders/";

	GLuint shaderScene = loadSHADER(shaderPathPrefix + "scene_vertex.glsl", shaderPathPrefix + "scene_fragment.glsl");

	GLuint shaderShadow = loadSHADER(shaderPathPrefix + "shadow_vertex.glsl", shaderPathPrefix + "shadow_fragment.glsl");

	//setup models
	string CarPath = "../Assets/Models/CarUV.obj";
	string wheelsPath = "../Assets/Models/wheel.obj";
	string lightsPath = "../Assets/Models/lights.obj";
	string pipePath = "../Assets/Models/pipeUV.obj";
	string cubePath = "../Assets/Models/cube.obj";
	//string heraclesPath = "../Assets/Models/heracles.obj";

	int CarVertices;
	int wheelsVertices;
	int lightsVertices;
	int cubeVertices;
	int pipeVertices;
	//int heraclesVertices;
	//GLuint heraclesVAO = setupModelEBO(heraclesPath, heraclesVertices);
	GLuint CarVAO = setupModelVBO(CarPath, CarVertices);
	GLuint wheelsVAO = setupModelVBO(wheelsPath, wheelsVertices);
	GLuint cubeVAO = setupModelEBO(cubePath, cubeVertices);
	GLuint pipeVAO = setupModelEBO(pipePath, pipeVertices);
	GLuint lightsVAO = setupModelEBO(lightsPath, lightsVertices);


	//int activeVAOVertices = CarVertices;
	//GLuint activeVAO = CarVAO;

	//Load Textures

	GLuint grassTextureID = loadTexture((char*)"../Assets/Textures/grass.jpg");
	GLuint tireTextureID = loadTexture((char*)"../Assets/Textures/wheeltxSin.jpg");
	GLuint carTextureID = loadTexture((char*)"../Assets/Textures/carT1.jpg");
	GLuint lightTextureID = loadTexture((char*)"../Assets/Textures/lights.jpg");

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
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Set texture unit # for shadow map
	setShadowMapTexture(shaderScene, 0);
	// Camera parameters for view transform
	vec3 cameraPosition(0.0f, 20.0f, 30.0f);
	vec3 cameraLookAt(0.0f, 0.0f, 0.0f);
	vec3 cameraUp(0.0f, 1.0f, 0.0f);

	// Other camera parameters
	float cameraSpeed = 1.0f;
	float cameraFastSpeed = 2 * cameraSpeed;
	float cameraHorizontalAngle = 90.0f;
	float cameraVerticalAngle = -30.0f;
	bool  isFirstPerson = false;//press C to switch
	bool  isGrip = true;
	bool  isAboveLight = false;
	bool  isMainLight = true;
	bool  isLeftFrontLight = false;
	bool  isRightFrontLight = false;
	bool  isBackMainLight = false;
	bool  isBumper = false;
	bool  isSet = false;
	// Spinning wheel
	float spinningwheelAngle = 0.0f;

	// Set projection matrix for shader, this won't change
	mat4 projectionMatrix = glm::perspective(70.0f,            // field of view in degrees
		800.0f / 600.0f,  // aspect ratio
		0.01f, 100.0f);   // near and far (near > 0)

	//GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	//glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	// Set initial view matrix
	mat4 viewMatrix = lookAt(cameraPosition,  // eye
		cameraPosition + cameraLookAt,  // center
		cameraUp); // up

	//GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	//glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

	setColorProjectionMatrix(colarShaderProgram, projectionMatrix);
	setColorProjectionMatrix(textureShaderProgram, projectionMatrix);


	setColorViewMatrix(colarShaderProgram, viewMatrix);
	setColorViewMatrix(textureShaderProgram, viewMatrix);
	// Set projection matrix on both shaders
	setProjectionMatrix(shaderScene, projectionMatrix);
	setProjectionMatrix(shaderShadow, projectionMatrix);

	setModelMatrix(shaderScene, mat4(1.0));
	setModelMatrix(shaderShadow, mat4(1.0));
	// Set view matrix on both shaders


	setViewMatrix(shaderScene, viewMatrix);
	setViewMatrix(shaderShadow, viewMatrix);

	//setViewPosition(shaderShadow, cameraPosition);
	//int textureVbo = createTexturedCubeVertexBufferObject(); why? this will not work, but upper works?
	//int vbo = createVertexBufferObject();


	// For frame time
	float lastFrameTime = glfwGetTime();
	int lastMouseLeftState = GLFW_RELEASE;
	double lastMousePosX =0.0f, lastMousePosY=0.0f;

	// Other OpenGL states to set once
	// Enable Backface culling
	glEnable(GL_CULL_FACE);

	// Enable Depth Test
	glEnable(GL_DEPTH_TEST);

	//for car whole movement
	float carX = 0.0f;
	float carY = 0.0f;
	float carZ = 0.0f;
	float carOwnX = 0.0f;
	float carOwnY = 0.0f;
	float carOwnZ = 0.0f;
	//for car scale up and down
	float carSize = 1.0f;

	//for rotation
	float carRotate = 0.0f;
	float wheelRotate = 0.0f;
	bool SpaceRelease = true;
	bool L_release = true;
	bool H_release = true;
	bool N_release = true;
	bool AltRelease = true;
	bool is_C_relese = true;

	bool isTrian = true;
	bool isPoint = false;
	bool isLine = false;
	float radiusLeftClick=0.0f;//for camera leftclick zoom in
	//glBindBuffer(GL_ARRAY_BUFFER, textureVbo);
	double dx = 0.0f;
	double dy = 0.0f;
	float a = 0.01f;
	float b = 1.0f;
	double mousePosX, mousePosY, unusedMouse;

	float currentCameraSpeed = cameraFastSpeed;
	// Entering Main Loop
	while (!glfwWindowShouldClose(window))
	{
		// Frame time calculation
		float dt = glfwGetTime() - lastFrameTime;
		lastFrameTime += dt;

		// Each frame, reset color of each pixel to glClearColor

		// @TODO 1 - Clear Depth Buffer Bit as well
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderScene);
		// ...
		/*
		glUseProgram(lightingShaderProgram);
		GLint objectColorLoc = glGetUniformLocation(lightingShaderProgram, "objectColor");
		GLint lightColorLoc = glGetUniformLocation(lightingShaderProgram, "lightColor");
		GLint lightPosLoc = glGetUniformLocation(lightingShaderProgram, "lightPos");
		GLint viewPosLoc = glGetUniformLocation(lightingShaderProgram, "viewPos");
		glUniform3f(objectColorLoc, 1.0f,0.5f,0.31f);
		glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
		//glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(viewPosLoc, cameraPosition.x, cameraPosition.y,cameraPosition.z);

		GLint worldMatrixLocation = glGetUniformLocation(lightingShaderProgram, "worldMatrix");
		GLint viewMatrixLocation = glGetUniformLocation(lightingShaderProgram, "viewMatrix");
		GLint projectionMatrixLocation = glGetUniformLocation(lightingShaderProgram, "projectionMatrix");
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

		glBindVertexArray(CarVAO);
		glm::mat4 worldMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawArrays(GL_TRIANGLES, 0, CarVertices);
		glBindVertexArray(0);
		*/
		

		mat4 viewMatrix(1.0f);
		
		mat4 groundWorldMatrix = translate(mat4(1.0f), vec3(0.0f, -0.01f, 0.0f)) * scale(mat4(1.0f), vec3(50.0f, 0.02f, 50.0f));
		
		const float cameraAngularSpeed = 60.0f;
		cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
		cameraVerticalAngle -= dy * cameraAngularSpeed * dt;

		// Clamp vertical angle to [-85, 85] degrees
		//cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
		if (cameraHorizontalAngle > 360)
		{
			cameraHorizontalAngle -= 360;
		}
		else if (cameraHorizontalAngle < -360)
		{
			cameraHorizontalAngle += 360;
		}

		float theta = radians(cameraHorizontalAngle);
		float phi = radians(cameraVerticalAngle);

		cameraLookAt = vec3(cosf(phi)*cosf(theta), sinf(phi), -cosf(phi)*sinf(theta));
		vec3 cameraSideVector = glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));

		glm::normalize(cameraSideVector);



		if (isFirstPerson) {
			cameraVerticalAngle = std::max(-30.0f, std::min(30.0f, cameraVerticalAngle));
			//cameraHorizontalAngle = std::max(-50.0f, std::min(50.0f, cameraVerticalAngle));
			//viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
			cameraLookAt = vec3(cosf(phi)*cosf(theta), sinf(phi), -cosf(phi)*sinf(theta));
			vec3 driverPos(- 0.6f,+ 4.0f, + 1.5f);//cameraposition
			vec3 carPos(carX, carY, carZ);
			vec3 newDriverpos = driverPos * carPos;
			vec3 carFront(carX - 5.0f, carY, carX+  0.5f);
			//carFront.x -= 0.1f * sin(radians(-180 + carRotate));
			//carFront.y -= 0.1f * cos(radians(-180 + carRotate));
			mat4 newCarTransformationWorldMatrix = mat4(1.0f);
			newCarTransformationWorldMatrix = glm::translate(newCarTransformationWorldMatrix, vec3(0.0f, 0.0f, 0.0f));
			viewMatrix = lookAt(driverPos+ carPos, carPos+driverPos +cameraLookAt, cameraUp);
			mat4 RotationWorldMatrix = mat4(1.0f);
			RotationWorldMatrix=glm::rotate(viewMatrix, radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
			viewMatrix = glm::translate(viewMatrix, vec3(+carPos.x, +carPos.y, +carPos.z));
			viewMatrix = glm::rotate(viewMatrix, -radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
			viewMatrix = glm::translate(viewMatrix, vec3(-carPos.x, -carPos.y, -carPos.z));
			double mousePosX, mousePosY;			
			glfwGetCursorPos(window, &mousePosX, &mousePosY);
			dx = mousePosX - lastMousePosX;
			dy = mousePosY - lastMousePosY;
			lastMousePosX = mousePosX;
			lastMousePosY = mousePosY;
		}
		else {
			float xz = pow(cameraPosition[0], 2.0f) + pow(cameraPosition[2], 2.0f);
			float xzy = xz + pow(cameraPosition[1], 2.0f);
			vec3 CarPos(carX, carY, carZ);
			float radius = sqrt(xzy) - radiusLeftClick;
			vec3 Pos = CarPos - vec3(radius*cosf(phi)*cosf(theta),
				radius*sinf(phi),
				-radius * cosf(phi)*sinf(theta));
			viewMatrix = lookAt(Pos, CarPos, cameraUp);
		}
		setModelMatrix(shaderScene, groundWorldMatrix);
		setModelMatrix(shaderShadow, groundWorldMatrix);
		setViewMatrix(shaderScene, viewMatrix);
		setViewMatrix(shaderShadow, viewMatrix);


		setViewPosition(shaderScene, cameraPosition);

		mat4 carWorldMatrix = mat4(1.0f);
		mat4 carTransformationWorldMatrix = mat4(1.0f);
		mat4 carScaleWorldMatrix = mat4(1.0f);
		mat4 carRotationWorldMatrix = mat4(1.0f);
		carTransformationWorldMatrix = glm::translate(carTransformationWorldMatrix, vec3(carX + 0.0f, carY + 0.0f, carZ + 0.0f));
		carScaleWorldMatrix = glm::scale(carScaleWorldMatrix, vec3(carSize * 2.0f, carSize * 2.0f, carSize * 2.0f));
		carRotationWorldMatrix = glm::rotate(carRotationWorldMatrix, radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
		carWorldMatrix = carTransformationWorldMatrix * carRotationWorldMatrix *carScaleWorldMatrix;
		//carWorldMatrix =  carRotationWorldMatrix *carScaleWorldMatrix;
		//carWorldMatrix = carTransformationWorldMatrix *carScaleWorldMatrix;
		carWorldMatrix = glm::rotate(carWorldMatrix, radians(90.0f), vec3(0.0f, -1.0f, 0.0f));

		mat4 lightsWorldMatrix = mat4(1.0f);
		mat4 lightsTransformationWorldMatrix = mat4(1.0f);
		mat4 lightsScaleWorldMatrix = mat4(1.0f);
		mat4 lightsRotationWorldMatrix = mat4(1.0f);
		lightsTransformationWorldMatrix = glm::translate(lightsTransformationWorldMatrix, vec3(carX + 0.0f, carY + 0.0f, carZ + 0.0f));
		lightsScaleWorldMatrix = glm::scale(lightsScaleWorldMatrix, vec3(carSize * 2.0f, carSize * 2.0f, carSize * 2.0f));
		lightsRotationWorldMatrix = glm::rotate(lightsRotationWorldMatrix, radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
		lightsWorldMatrix = lightsTransformationWorldMatrix * lightsRotationWorldMatrix *lightsScaleWorldMatrix;
		//lightsWorldMatrix = lightsTransformationWorldMatrix * lightsScaleWorldMatrix;
		//lightsWorldMatrix =  lightsRotationWorldMatrix *lightsScaleWorldMatrix;
		lightsWorldMatrix = glm::rotate(lightsWorldMatrix, radians(90.0f), vec3(0.0f, -1.0f, 0.0f));
		mat4 pipeWorldMatrix = mat4(1.0f);
		mat4 pipeTransformationWorldMatrix = mat4(1.0f);
		mat4 pipeScaleWorldMatrix = mat4(1.0f);
		mat4 pipeRotationWorldMatrix = mat4(1.0f);
		mat4 pipeCarTransformationWorldMatrix = mat4(1.0f);
		pipeCarTransformationWorldMatrix=glm::translate(pipeCarTransformationWorldMatrix, vec3(carX , carY , carZ ));
		pipeTransformationWorldMatrix = glm::translate(pipeTransformationWorldMatrix, vec3( 1.0f,  1.6f,  - 8.5f));
		pipeScaleWorldMatrix = glm::scale(pipeScaleWorldMatrix, vec3(carSize * 0.5f, carSize * 0.5f, carSize * 0.5f));
		pipeRotationWorldMatrix = glm::rotate(pipeRotationWorldMatrix, radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
		pipeWorldMatrix = pipeCarTransformationWorldMatrix*pipeRotationWorldMatrix *pipeScaleWorldMatrix;
		pipeWorldMatrix = glm::rotate(pipeWorldMatrix, radians(90.0f), vec3(0.0f, -1.0f, 0.0f));
		// Calculate variables for shadow mapping
		vec3 lightPosition(carX - 5.3f, carY + 1.9f, carZ );// the location of the light in 3D space
		vec3 lightFocus(carX - 20.0f, carY, carZ );// the point in 3D space the light "looks" at
		vec3 lightDirection = normalize(lightFocus - lightPosition);

		float lightAngleOuter = 30.0;
		float lightAngleInner = 20.0;
		float lightNearPlane = 5.0f;
		float lightFarPlane = 80.0f;

		mat4 lightProjectionMatrix = perspective(20.0f, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, lightNearPlane, lightFarPlane);
		mat4 lightViewMatrix = lookAt(lightPosition, lightFocus, vec3(0.0f, 0.0f, 1.0f));
		mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
		if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
			if(L_release){
				isMainLight = !isMainLight;
				isBackMainLight = !isBackMainLight;
				L_release = false;
			}	
		}
		else {
			L_release = true;
		}
		if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
			if (N_release) {
				isMainLight = !isMainLight;
				isAboveLight = !isAboveLight;
				N_release = false;
			}
		}
		else {
			N_release = true;
		}
		if(isMainLight){
		

		// Set light space matrix on both shaders
		setLightSpaceMatrix(shaderShadow, lightSpaceMatrix);
		setLightSpaceMatrix(shaderScene, lightSpaceMatrix);

		// Set light far and near planes on scene shader
		setLightNearPlane(shaderScene, lightNearPlane);
		setLightFarPlane(shaderScene, lightFarPlane);

		// Set light position on scene shader
		setLightPosition(shaderScene, lightPosition);

		// Set light direction on scene shader
		setLightDirection(shaderScene, lightDirection);

		// Set light cutoff angles on scene shader
		setLightCutoffInnerDegrees(shaderScene, lightAngleInner);
		setLightCutoffOuterDegrees(shaderScene, lightAngleOuter);

		// Set light color on scene shader
		setLightColor(shaderScene, vec3(1.0, 1.0, 1.0));

		// Set object color on scene shader
		setObjectColor(shaderScene, vec3(1.0, 1.0, 1.0));
		}
		if (isAboveLight) {
			vec3 lightPosition2(carX , carY + 10.0f, carZ );// the location of the light in 3D space
			vec3 lightFocus2(carX , carY, carZ );        // the point in 3D space the light "looks" at
			vec3 lightDirection2 = normalize(lightFocus2 - lightPosition2);

			 lightAngleOuter = 50.0;
			 lightAngleInner = 20.0;
			 lightNearPlane = 5.0f;
			 lightFarPlane = 80.0f;
			mat4 lightProjectionMatrix2 = perspective(20.0f, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, lightNearPlane, lightFarPlane);
			mat4 lightViewMatrix2 = lookAt(lightPosition2, lightFocus2, vec3(0.0f, 0.0f, 1.0f));
			mat4 lightSpaceMatrix2 = lightProjectionMatrix2 * lightViewMatrix2;


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
			setLightColor(shaderScene, vec3(1.0, 1.0, 1.0));

			// Set object color on scene shader
			setObjectColor(shaderScene, vec3(1.0, 1.0, 1.0));
		}
		if (isLeftFrontLight) {
			vec3 lightPosition2(carX - 5.3f, carY + 1.9f, carZ + 2.4f);// the location of the light in 3D space
			vec3 lightFocus2(carX - 20.0f, carY, carZ + 2.4f);        // the point in 3D space the light "looks" at
			vec3 lightDirection2 = normalize(lightFocus2 - lightPosition2);


			mat4 lightProjectionMatrix2 = perspective(20.0f, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, lightNearPlane, lightFarPlane);
			mat4 lightViewMatrix2 = lookAt(lightPosition2, lightFocus2, vec3(0.0f, 0.0f, 1.0f));
			mat4 lightSpaceMatrix2 = lightProjectionMatrix2 * lightViewMatrix2;


			// Set light space matrix on both shaders
			setLightSpaceMatrix(shaderShadow, lightSpaceMatrix);
			setLightSpaceMatrix(shaderScene, lightSpaceMatrix);

			// Set light far and near planes on scene shader
			setLightNearPlane(shaderScene, lightNearPlane);
			setLightFarPlane(shaderScene, lightFarPlane);

			// Set light position on scene shader
			setLightPosition(shaderScene, lightPosition);

			// Set light direction on scene shader
			setLightDirection(shaderScene, lightDirection);

			// Set light cutoff angles on scene shader
			setLightCutoffInnerDegrees(shaderScene, lightAngleInner);
			setLightCutoffOuterDegrees(shaderScene, lightAngleOuter);

			// Set light color on scene shader
			setLightColor(shaderScene, vec3(1.0, 1.0, 1.0));

			// Set object color on scene shader
			setObjectColor(shaderScene, vec3(1.0, 1.0, 1.0));
		}
		
		// Set light space matrix on both shaders

		if(isRightFrontLight){
		vec3 lightPosition2(carX - 5.3f, carY + 1.9f, carZ - 2.4f);// the location of the light in 3D space
		vec3 lightFocus2(carX - 20.0f, carY, carZ - 2.4f);        // the point in 3D space the light "looks" at
		vec3 lightDirection2 = normalize(lightFocus2 - lightPosition2);


		mat4 lightProjectionMatrix2 = perspective(20.0f, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, lightNearPlane, lightFarPlane);
		mat4 lightViewMatrix2 = lookAt(lightPosition2, lightFocus2, vec3(0.0f, 0.0f, 1.0f));
		mat4 lightSpaceMatrix2 = lightProjectionMatrix2 * lightViewMatrix2;

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
		setLightColor(shaderScene, vec3(1.0, 1.0, 1.0));

		// Set object color on scene shader
		setObjectColor(shaderScene, vec3(1.0, 1.0, 1.0));
		}
		if (isBackMainLight) {
			vec3 lightPosition2(carX + 6.3f, carY + 2.9f, carZ );// the location of the light in 3D space
			vec3 lightFocus2(carX + 20.0f, carY, carZ );        // the point in 3D space the light "looks" at
			vec3 lightDirection2 = normalize(lightFocus2 - lightPosition2);


			mat4 lightProjectionMatrix2 = perspective(20.0f, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, lightNearPlane, lightFarPlane);
			mat4 lightViewMatrix2 = lookAt(lightPosition2, lightFocus2, vec3(0.0f, 0.0f, 1.0f));
			mat4 lightSpaceMatrix2 = lightProjectionMatrix2 * lightViewMatrix2;

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
			setLightColor(shaderScene, vec3(1.0, 1.0, 1.0));

			// Set object color on scene shader
			setObjectColor(shaderScene, vec3(1.0, 1.0, 1.0));
		}
		/*
		vec3 carLight1Position(20.0f, 20.0f, 20.0f);
		vec3 carLight1Position2(-carX, carY, carX);// the location of the light in 3D space
		vec3 carLight1Focus(0.0f, 0.0f, 0.0f);        // the point in 3D space the light "looks" at
		vec3 carLight1Focus2(-carX, carY, carZ + 10.f);
		vec3 carLight1Direction = normalize(carLight1Focus - carLight1Position);
		vec3 carLight1Direction2 = normalize(carLight1Focus2 - carLight1Position2);
		mat4 carLight1ProjectionMatrix = perspective(20.0f, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, lightNearPlane, lightFarPlane);
		mat4 carLight1ViewMatrix = lookAt(carLight1Position, carLight1Focus, vec3(0.0f, 0.0f, 1.0f));
		mat4 carLight1SpaceMatrix = carLight1ProjectionMatrix * carLight1ViewMatrix;
		mat4 carLight1ProjectionMatrix2 = perspective(20.0f, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, lightNearPlane, lightFarPlane);
		mat4 carLight1ViewMatrix2 = lookAt(carLight1Position2, carLight1Focus2, vec3(0.0f, 0.0f, 1.0f));
		mat4 carLight1SpaceMatrix2 = carLight1ProjectionMatrix2 * carLight1ViewMatrix2;

		//setLightSpaceMatrix(shaderShadow, carLight1SpaceMatrix);
		//setLightSpaceMatrix(shaderScene, carLight1SpaceMatrix);

		// Set light far and near planes on scene shader
		//setLightNearPlane(shaderScene, lightNearPlane);
		//setLightFarPlane(shaderScene, lightFarPlane);

		// Set light position on scene shader
		GLint lightSpotLoc = glGetUniformLocation(shaderScene, "light_position");
		GLint lightSpotDirLoc = glGetUniformLocation(shaderScene, "light_direction");
		GLint lightSpotCutOffLoc = glGetUniformLocation(shaderScene, "light_cutoff_inner");
		GLint lightSpotOuterCutOffLoc = glGetUniformLocation(shaderScene, "light_cutoff_outer");
		GLint viewPosLoc = glGetUniformLocation(shaderScene, "view_position");
		glUniform3f(lightSpotLoc, carLight1Position.x, carLight1Position.y, carLight1Position.z);
		glUniform3f(lightSpotDirLoc, carLight1Direction.x, carLight1Direction.y, carLight1Direction.z);// Set light direction on scene shader
		glUniform1f(lightSpotCutOffLoc, glm::cos(glm::radians(12.5f)));
		glUniform1f(lightSpotOuterCutOffLoc, glm::cos(glm::radians(37.5f)));
		glUniform3f(viewPosLoc, carLight1Position.x, carLight1Position.y, carLight1Position.z);
		// Set light cutoff angles on scene shader
		//setLightCutoffInnerDegrees(shaderScene, lightAngleInner);
		//setLightCutoffOuterDegrees(shaderScene, lightAngleOuter);
		
		// Set light color on scene shader
		setLightColor(shaderScene, vec3(1.0, 1.0, 1.0));

		// Set object color on scene shader
		setObjectColor(shaderScene, vec3(1.0, 1.0, 1.0));*/
		/*
		setLightSpaceMatrix(shaderShadow, carLight1SpaceMatrix2);
		setLightSpaceMatrix(shaderScene, carLight1SpaceMatrix2);

		// Set light far and near planes on scene shader
		setLightNearPlane(shaderScene, lightNearPlane);
		setLightFarPlane(shaderScene, lightFarPlane);

		// Set light position on scene shader
		setLightPosition(shaderScene, carLight1Position2);

		// Set light direction on scene shader
		setLightDirection(shaderScene, carLight1Direction2);

		// Set light cutoff angles on scene shader
		setLightCutoffInnerDegrees(shaderScene, lightAngleInner);
		setLightCutoffOuterDegrees(shaderScene, lightAngleOuter);

		// Set light color on scene shader
		setLightColor(shaderScene, vec3(1.0, 1.0, 1.0));

		// Set object color on scene shader
		setObjectColor(shaderScene, vec3(1.0, 1.0, 1.0));
		*/

		mat4 wheelsWorldMatrix = mat4(1.0f);
		mat4 wheelsTransformationWorldMatrix = mat4(1.0f);
		mat4 wheelsScaleWorldMatrix = mat4(1.0f);
		mat4 wheelsRotationWorldMatrix = mat4(1.0f);
		mat4 wheelSpin = mat4(1.0f);
		wheelSpin = glm::rotate(wheelSpin, radians(spinningwheelAngle), vec3(1.0f, 0.0f, 0.0f));
		wheelsTransformationWorldMatrix = glm::translate(wheelsTransformationWorldMatrix, vec3(carX + 0.0f, carY + 0.0f, carZ + 0.0f));
		wheelsScaleWorldMatrix = glm::scale(wheelsScaleWorldMatrix, vec3(carSize * 2.0f, carSize * 2.0f, carSize * 2.0f));
		wheelsRotationWorldMatrix = glm::rotate(wheelsRotationWorldMatrix, radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
		wheelsWorldMatrix = wheelsTransformationWorldMatrix * wheelsRotationWorldMatrix * wheelsScaleWorldMatrix;
		//wheelsWorldMatrix = wheelsTransformationWorldMatrix  * wheelsScaleWorldMatrix;
		//wheelsWorldMatrix =  wheelsRotationWorldMatrix * wheelsScaleWorldMatrix;
		wheelsWorldMatrix = glm::rotate(wheelsWorldMatrix, radians(90.0f), vec3(0.0f, -1.0f, 0.0f));
		
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glBindVertexArray(CarVAO);
		// Draw geometry
		//glDrawArrays(GL_TRIANGLES, 0, CarVertices);
		//glBindVertexArray(0);

		//glUseProgram(shaderScene);
		//glUseProgram(shaderProgram);

		//glBindVertexArray(textureVbo);
		//Draw ground

		//glActiveTexture(GL_TEXTURE0);
		//GLuint textureLocation = glGetUniformLocation(shaderScene, "textureSampler");

		//glBindTexture(GL_TEXTURE_2D, grassTextureID);
		//glUniform1i(textureLocation, 0);


		//mat4 groundWorldMatrix = translate(mat4(1.0f), vec3(0.0f, -0.01f, 0.0f)) * scale(mat4(1.0f), vec3(50.0f, 0.02f, 50.0f));
		//setWorldMatrix(shaderScene, groundWorldMatrix);
		//worldMatrixLocation = glGetUniformLocation(textureShaderProgram, "worldMatrix");
		//glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &groundWorldMatrix[0][0]);
		//glDrawArrays(GL_TRIANGLES, 0, 6); // 36 vertices, starting at index 0
		//setModelMatrix(shaderScene, groundWorldMatrix);
		//glBindVertexArray(0);
		



		GLuint world1MatrixLocation = glGetUniformLocation(shaderScene, "model_matrix");
		//mat4 viewMatrix(1.0f);
		//viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
		//setViewMatrix(shaderProgram, viewMatrix);
		spinningwheelAngle += 360.0f * dt;

		mat4 leftfrontwheel = glm::translate(mat4(1.0f), vec3(1.3f, 0.4f, 1.63f));
		//setModelMatrix(shaderScene, wheelsWorldMatrix*leftfrontwheel*wheelSpin);


		mat4 leftbackwheel = glm::translate(mat4(1.0f), vec3(1.3f, 0.4f, -1.05f));
		//setModelMatrix(shaderScene, wheelsWorldMatrix*leftbackwheel*wheelSpin);


		mat4 rightfrontwheel = glm::translate(mat4(1.0f), vec3(-1.3f, 0.4f, 1.63f));
		//setModelMatrix(shaderScene, wheelsWorldMatrix*rightfrontwheel*wheelSpin);

		mat4 rightbackwheel = glm::translate(mat4(1.0f), vec3(-1.3f, 0.4f, -1.05f));
		//wheelsWorldMatrix = glm::rotate(wheelsWorldMatrix, radians(spinningwheelAngle), vec3(1.0f, 0.0f, 0.0f));

		mat4 carOwnTranWorldMatrix = mat4(1.0f);
		carOwnTranWorldMatrix = glm::translate(carTransformationWorldMatrix, vec3(carOwnX + 0.0f, carOwnY + 0.0f, carOwnZ + 0.0f));
		/*
		mat4 baseWorldMatrix = mat4(1.0f);
		mat4 baseTransformationWorldMatrix = mat4(1.0f);
		mat4 baseScaleWorldMatrix = mat4(1.0f);
		mat4 baseRotationWorldMatrix = mat4(1.0f);

		mat4 topWorldMatrix = mat4(1.0f);
		mat4 topTransformationWorldMatrix = mat4(1.0f);
		mat4 topScaleWorldMatrix = mat4(1.0f);
		mat4 topRotationWorldMatrix = mat4(1.0f);

		mat4 frontWorldMatrix = mat4(1.0f);
		mat4 frontTransformationWorldMatrix = mat4(1.0f);
		mat4 frontScaleWorldMatrix = mat4(1.0f);
		mat4 frontRotationWorldMatrix = mat4(1.0f);

		mat4 backWorldMatrix = mat4(1.0f);
		mat4 backTransformationWorldMatrix = mat4(1.0f);
		mat4 backScaleWorldMatrix = mat4(1.0f);
		mat4 backRotationWorldMatrix = mat4(1.0f);

		mat4 wheel1WorldMatrix = mat4(1.0f);
		mat4 wheel1TransformationWorldMatrix = mat4(1.0f);
		mat4 wheelScaleWorldMatrix = mat4(1.0f);
		mat4 wheelRotationWorldMatrix = mat4(1.0f);

		mat4 wheel2WorldMatrix = mat4(1.0f);
		mat4 wheel2TransformationWorldMatrix = mat4(1.0f);

		mat4 wheel3WorldMatrix = mat4(1.0f);
		mat4 wheel3TransformationWorldMatrix = mat4(1.0f);

		mat4 wheel4WorldMatrix = mat4(1.0f);
		mat4 wheel4TransformationWorldMatrix = mat4(1.0f);

		//base default
		baseTransformationWorldMatrix = glm::translate(baseTransformationWorldMatrix, vec3(carX + 0.0f, carY + 1.0f, carZ + 0.0f));
		baseScaleWorldMatrix = glm::scale(baseScaleWorldMatrix, vec3(carSize * 6.0f, carSize * 0.75f, carSize * 4.0f));
		baseRotationWorldMatrix= glm::rotate(baseRotationWorldMatrix, radians(carRotate),vec3(0.0f,1.0f,0.0f));
		baseWorldMatrix = baseTransformationWorldMatrix * baseRotationWorldMatrix *baseScaleWorldMatrix;
		//top
		topTransformationWorldMatrix = glm::translate(topTransformationWorldMatrix, vec3(carX + 0.0f, carY + 1.675f, carZ + 0.0f));
		topScaleWorldMatrix = glm::scale(topScaleWorldMatrix, vec3(carSize * 3.0f, carSize * 1.0f, carSize * 2.0f));
		topWorldMatrix = topTransformationWorldMatrix * topRotationWorldMatrix *  topScaleWorldMatrix *baseRotationWorldMatrix;

		//front
		frontTransformationWorldMatrix = glm::translate(frontTransformationWorldMatrix, vec3(carX - carSize * 4.2f, carY + 0.75f, carZ + 0.0f));
		frontScaleWorldMatrix = glm::scale(frontScaleWorldMatrix, vec3(carSize * 3.0f, carSize * 0.25f, carSize * 2.0f));
		frontRotationWorldMatrix = glm::rotate(frontRotationWorldMatrix, radians(15.0f), vec3(0.0f, 0.0f, 1.0f));
		frontWorldMatrix = frontTransformationWorldMatrix * frontRotationWorldMatrix * frontScaleWorldMatrix *baseRotationWorldMatrix;

		//back
		backTransformationWorldMatrix = glm::translate(backTransformationWorldMatrix, vec3(carX + carSize * 4.2f, carY +  0.75f, carZ + 0.0f));
		backScaleWorldMatrix = glm::scale(backScaleWorldMatrix, vec3(carSize * 3.0f, carSize * 0.25f, carSize * 2.0f));
		backRotationWorldMatrix = glm::rotate(backRotationWorldMatrix, radians(15.0f), vec3(0.0f, 0.0f, -1.0f));
		backWorldMatrix =  backTransformationWorldMatrix * backRotationWorldMatrix* backScaleWorldMatrix *baseRotationWorldMatrix;

		//wheels
		wheel1TransformationWorldMatrix = glm::translate(wheel1TransformationWorldMatrix, vec3(carX+ carSize*2.5f, carY + 0.75f, carZ + carSize*2.3f));
		wheelScaleWorldMatrix = glm::scale(wheelScaleWorldMatrix, vec3(carSize* 2.0f, carSize*2.0f,  carSize * 1.0f));
		wheelRotationWorldMatrix = glm::rotate(wheelRotationWorldMatrix, radians(spinningwheelAngle), vec3(0.0f, 0.0f, 1.0f));
		wheel1WorldMatrix = wheel1TransformationWorldMatrix * wheelRotationWorldMatrix * wheelScaleWorldMatrix *baseRotationWorldMatrix;
		wheel2TransformationWorldMatrix = glm::translate(wheel2TransformationWorldMatrix, vec3(carX  + carSize*2.5f, carY  + 0.75f, carZ - carSize*2.3f));
		wheel2WorldMatrix = wheel2TransformationWorldMatrix * wheelRotationWorldMatrix * wheelScaleWorldMatrix *baseRotationWorldMatrix;
		wheel3TransformationWorldMatrix = glm::translate(wheel3TransformationWorldMatrix, vec3(carX  -carSize*2.5f, carY  + 0.75f, carZ + carSize*2.3f));
		wheel3WorldMatrix = wheel3TransformationWorldMatrix * wheelRotationWorldMatrix * wheelScaleWorldMatrix *baseRotationWorldMatrix;
		wheel4TransformationWorldMatrix = glm::translate(wheel4TransformationWorldMatrix, vec3(carX -carSize*2.5f, carY  + 0.75f, carZ - carSize*2.3f));
		wheel4WorldMatrix = wheel4TransformationWorldMatrix * wheelRotationWorldMatrix * wheelScaleWorldMatrix *baseRotationWorldMatrix;
		*/
		cout.precision(5);
		if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		{
			carSize += 0.1f;
		}

		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS&& carSize>1.0f)
		{
			carSize -= 0.1f;
		}

		if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			//carOwnX -= 0.1f;
			carX -= 0.1f * sin(radians(-180+carRotate));
			carZ -= 0.1f * cos(radians(-180+carRotate));
		}else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{


			carRotate += 0.4f*cameraAngularSpeed* dt;

		}

		if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			//carOwnX += 0.1f;
			carX += 0.1f * sin(radians(-180+carRotate));
			carZ += 0.1f * cos(radians(-180+carRotate));
			/*
			cout << "cos(radians(carRotate)):";
			cout << cos(radians(carRotate)) << endl;
			cout << "radians(carRotate):";
			cout << radians(carRotate) << endl;
			cout << "carRotate:";
			cout << carRotate << endl;*/
		}else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			carRotate -= 0.4f*cameraAngularSpeed* dt;
		}

		if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			//carOwnZ -= 0.1f;
			carX -= 0.1f * sin(radians(90 + carRotate));
			carZ -= 0.1f * cos(radians(90 + carRotate));
			
		}
		if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			//carOwnZ += 0.1f;
			carX += 0.1f * sin(radians(90 + carRotate));
			carZ += 0.1f * cos(radians(90 + carRotate));
		}
		
		
		//if ((!(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) && (!(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)) && (!(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS))) {
			//wheelRotate = 0;
		//}

		if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
			if (H_release) {
				isBumper = !isBumper;
				H_release = false;
			}
		}
		else {
			H_release = true;
		}

		if(isTrian || glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		{ 
			isPoint = false;
			isLine = false;
			isTrian = true;
		/*// Draw base
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &baseWorldMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Draw top
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &topWorldMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 48, 36);

		// Draw front
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &frontWorldMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 84, 36);

		// Draw back
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &backWorldMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 84, 36);

		// Draw Wheels
		
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wheel1WorldMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 120, 36);

				glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wheel2WorldMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 120, 36);

		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wheel3WorldMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 120, 36);

		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wheel4WorldMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 120, 36);*/
			//car
			/*
			glBindVertexArray(CarVAO);

			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &carWorldMatrix[0][0]);
			//setWorldMatrix(textureShaderProgram, carWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 0, CarVertices);*/

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
			glBindVertexArray(CarVAO);
			setModelMatrix(shaderShadow, carWorldMatrix);
			//setWorldMatrix(textureShaderProgram, carWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 0, CarVertices);


			glBindVertexArray(wheelsVAO);
			//glBindTexture(GL_TEXTURE_2D, tireTextureID);
			leftfrontwheel = glm::translate(mat4(1.0f), vec3(1.3f, 0.4f, 1.63f));
			//setModelMatrix(shaderScene, wheelsWorldMatrix*leftfrontwheel*wheelSpin);
			setModelMatrix(shaderShadow, wheelsWorldMatrix*leftfrontwheel*wheelSpin);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

			leftbackwheel = glm::translate(mat4(1.0f), vec3(1.3f, 0.4f, -1.05f));
			//setModelMatrix(shaderScene, wheelsWorldMatrix*leftbackwheel*wheelSpin);
			setModelMatrix(shaderShadow, wheelsWorldMatrix*leftbackwheel*wheelSpin);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

			rightfrontwheel = glm::translate(mat4(1.0f), vec3(-1.3f, 0.4f, 1.63f));
			//setModelMatrix(shaderScene, wheelsWorldMatrix*rightfrontwheel*wheelSpin);
			setModelMatrix(shaderShadow, wheelsWorldMatrix*rightfrontwheel*wheelSpin);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

			rightbackwheel = glm::translate(mat4(1.0f), vec3(-1.3f, 0.4f, -1.05f));
			//setModelMatrix(shaderScene, wheelsWorldMatrix*rightbackwheel*wheelSpin);
			setModelMatrix(shaderShadow, wheelsWorldMatrix*rightbackwheel*wheelSpin);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);
			glBindVertexArray(textureVbo);
			// Draw geometry
			setModelMatrix(shaderShadow, groundWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// Unbind geometry
			glBindVertexArray(0);
			


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
			glBindVertexArray(textureVbo);

			//glBindTexture(GL_TEXTURE_2D, depthMap);
			//GLuint textureLocation = glGetUniformLocation(textureShaderProgram, "shado_wmap");
			//glUniform1i(textureLocation, 0);
			setWorldMatrix(shaderScene, groundWorldMatrix);
			glActiveTexture(GL_TEXTURE1);
			GLuint textureLocation = glGetUniformLocation(shaderScene, "texture0");
			glBindTexture(GL_TEXTURE_2D, grassTextureID);
			glUniform1i(textureLocation, 1);
			// Draw geometry
			glDrawArrays(GL_TRIANGLES, 0, 6);
			/*
			mat4 cubeWorldMatrix = translate(mat4(1.0f), vec3(lightPosition.x, lightPosition.y, lightPosition.z))*scale(mat4(1.0f), vec3(0.1f, 0.1f, 0.1f));
			glBindVertexArray(cubeVAO);
			setModelMatrix(shaderScene, cubeWorldMatrix);
			glDrawElements(GL_TRIANGLES, cubeVertices, GL_UNSIGNED_INT, 0);*/
			glBindVertexArray(CarVAO);
			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &carWorldMatrix[0][0]);
			//setWorldMatrix(textureShaderProgram, carWorldMatrix);
			glBindTexture(GL_TEXTURE_2D, carTextureID);
			glDrawArrays(GL_TRIANGLES, 0, CarVertices);
			// Unbind geometry
			glBindVertexArray(0);
			glBindVertexArray(wheelsVAO);
			glBindTexture(GL_TEXTURE_2D, tireTextureID);
			setModelMatrix(shaderScene, wheelsWorldMatrix*leftfrontwheel*wheelSpin);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

			setModelMatrix(shaderScene, wheelsWorldMatrix*leftbackwheel*wheelSpin);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

			setModelMatrix(shaderScene, wheelsWorldMatrix*rightfrontwheel*wheelSpin);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

			setModelMatrix(shaderScene, wheelsWorldMatrix*rightbackwheel*wheelSpin);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);
			//glBindVertexArray(CarVAO);
			// Draw geometry
			//glDrawArrays(GL_TRIANGLES, 0, CarVertices);
			//glBindVertexArray(0);

			//glBindTexture(GL_TEXTURE_2D, 0);

			//lights
			glBindVertexArray(lightsVAO);
			glBindTexture(GL_TEXTURE_2D, lightTextureID);
			//setWorldMatrix(textureShaderProgram, lightsWorldMatrix);
			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &lightsWorldMatrix[0][0]);
			glDrawElements(GL_TRIANGLES, lightsVertices, GL_UNSIGNED_INT, 0);
			glBindVertexArray(pipeVAO); 
			glBindTexture(GL_TEXTURE_2D, 0); 
			setModelMatrix(shaderScene, pipeWorldMatrix*pipeTransformationWorldMatrix);
			glDrawElements(GL_TRIANGLES, pipeVertices, GL_UNSIGNED_INT, 0);
			
			if(isBumper){
				
				float carAxisX[5];
				float carAxisY[5];
				float carAxisZ[5];
				mat4 carWorldMatrixArray[5];
				mat4 carTransformationWorldMatrixArray[5];
				mat4 carScaleWorldMatrixArray[5];
				mat4 lightsWorldMatrixArray[5];
				mat4 lightsTransformationWorldMatrixArray[5];
				mat4 lightsScaleWorldMatrixArray[5];
				mat4 lightsRotationWorldMatrixArray[5];
				mat4 carRotationWorldMatrixArray[5];
				mat4 pipeWorldMatrixArray[5];
				mat4 pipeTransformationWorldMatrixArray[5];
				mat4 pipeScaleWorldMatrixArray[5];
				mat4 pipeRotationWorldMatrixArray[5];
				mat4 pipeCarTransformationWorldMatrixArray[5];
				mat4 wheelsWorldMatrixArray[5];
				mat4 wheelsTransformationWorldMatrixArray[5];
				mat4 wheelsScaleWorldMatrixArray[5];
				mat4 wheelsRotationWorldMatrixArray[5];
				if(!isSet){
				for (int i = 0; i < 5; i++) {
					float r5 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 50.0f));
					float r6 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 50.0f));
					float r7 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 50.0f));
					carAxisX[i]= r5;
					carAxisY[i]= 0.0f;
					carAxisZ[i]= r6;
				}
				}
				isSet = true;
			for (int i = 0; i < 5; i++) {
				carWorldMatrixArray[i] = mat4(1.0f);
				carTransformationWorldMatrixArray[i] = mat4(1.0f);
				carScaleWorldMatrixArray[i] = mat4(1.0f);
				carRotationWorldMatrixArray[i] = mat4(1.0f);
				carTransformationWorldMatrixArray[i] = glm::translate(carTransformationWorldMatrixArray[i], vec3(carAxisX[i] + 0.0f, carAxisY[i] + 0.0f, carAxisZ[i] + 0.0f));
				carScaleWorldMatrixArray[i] = glm::scale(carScaleWorldMatrixArray[i], vec3(carSize * 2.0f, carSize * 2.0f, carSize * 2.0f));
				carRotationWorldMatrixArray[i] = glm::rotate(carRotationWorldMatrixArray[i], radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
				carWorldMatrixArray[i] = carTransformationWorldMatrixArray[i] * carRotationWorldMatrixArray[i] * carScaleWorldMatrixArray[i];
				carWorldMatrixArray[i] = glm::rotate(carWorldMatrixArray[i], radians(90.0f), vec3(0.0f, -1.0f, 0.0f));
				lightsWorldMatrixArray[i] = mat4(1.0f);
				lightsTransformationWorldMatrixArray[i] = mat4(1.0f);
				lightsScaleWorldMatrixArray[i] = mat4(1.0f);
				lightsRotationWorldMatrixArray[i] = mat4(1.0f);
				lightsTransformationWorldMatrixArray[i] = glm::translate(lightsTransformationWorldMatrixArray[i], vec3(carAxisX[i] + 0.0f, carAxisY[i] + 0.0f, carAxisZ[i] + 0.0f));
				lightsScaleWorldMatrixArray[i] = glm::scale(lightsScaleWorldMatrixArray[i], vec3(carSize * 2.0f, carSize * 2.0f, carSize * 2.0f));
				lightsRotationWorldMatrixArray[i] = glm::rotate(lightsRotationWorldMatrixArray[i], radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
				lightsWorldMatrixArray[i] = lightsTransformationWorldMatrixArray[i] * lightsRotationWorldMatrixArray[i] * lightsScaleWorldMatrixArray[i];
				//lightsWorldMatrixArray[i] = lightsTransformationWorldMatrixArray[i] * lightsScaleWorldMatrixArray[i];
				//lightsWorldMatrixArray[i] =  lightsRotationWorldMatrixArray[i] *lightsScaleWorldMatrixArray[i];
				lightsWorldMatrixArray[i] = glm::rotate(lightsWorldMatrixArray[i], radians(90.0f), vec3(0.0f, -1.0f, 0.0f));
				pipeWorldMatrixArray[i] = mat4(1.0f);
				pipeTransformationWorldMatrixArray[i] = mat4(1.0f);
				pipeScaleWorldMatrixArray[i] = mat4(1.0f);
				pipeRotationWorldMatrixArray[i] = mat4(1.0f);
				pipeCarTransformationWorldMatrixArray[i] = mat4(1.0f);
				pipeCarTransformationWorldMatrixArray[i] = glm::translate(pipeCarTransformationWorldMatrixArray[i], vec3(carAxisX[i], carAxisY[i], carAxisZ[i]));
				pipeTransformationWorldMatrixArray[i] = glm::translate(pipeTransformationWorldMatrixArray[i], vec3(1.0f, 1.6f, -8.5f));
				pipeScaleWorldMatrixArray[i] = glm::scale(pipeScaleWorldMatrixArray[i], vec3(carSize * 0.5f, carSize * 0.5f, carSize * 0.5f));
				pipeRotationWorldMatrixArray[i] = glm::rotate(pipeRotationWorldMatrixArray[i], radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
				pipeWorldMatrixArray[i] = pipeCarTransformationWorldMatrixArray[i] * pipeRotationWorldMatrixArray[i] * pipeScaleWorldMatrixArray[i];
				pipeWorldMatrixArray[i] = glm::rotate(pipeWorldMatrixArray[i], radians(90.0f), vec3(0.0f, -1.0f, 0.0f));
				wheelsWorldMatrixArray[i] = mat4(1.0f);
				wheelsTransformationWorldMatrixArray[i] = mat4(1.0f);
				wheelsScaleWorldMatrixArray[i] = mat4(1.0f);
				wheelsRotationWorldMatrixArray[i] = mat4(1.0f);
				wheelSpin = glm::rotate(wheelSpin, radians(spinningwheelAngle), vec3(1.0f, 0.0f, 0.0f));
				wheelsTransformationWorldMatrixArray[i] = glm::translate(wheelsTransformationWorldMatrixArray[i], vec3(carAxisX[i] + 0.0f, carAxisY[i] + 0.0f, carAxisZ[i] + 0.0f));
				wheelsScaleWorldMatrixArray[i] = glm::scale(wheelsScaleWorldMatrixArray[i], vec3(carSize * 2.0f, carSize * 2.0f, carSize * 2.0f));
				wheelsRotationWorldMatrixArray[i] = glm::rotate(wheelsRotationWorldMatrixArray[i], radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
				wheelsWorldMatrixArray[i] = wheelsTransformationWorldMatrixArray[i] * wheelsRotationWorldMatrixArray[i] * wheelsScaleWorldMatrixArray[i];
				//wheelsWorldMatrixArray[i] = wheelsTransformationWorldMatrixArray[i]  * wheelsScaleWorldMatrixArray[i];
				//wheelsWorldMatrixArray[i] =  wheelsRotationWorldMatrixArray[i] * wheelsScaleWorldMatrixArray[i];
				wheelsWorldMatrixArray[i] = glm::rotate(wheelsWorldMatrixArray[i], radians(90.0f), vec3(0.0f, -1.0f, 0.0f));
			}
			for (int i = 0; i < 5; i++) {
				glBindVertexArray(CarVAO);
				setModelMatrix(shaderScene, carWorldMatrixArray[i]);
				//setWorldMatrix(textureShaderProgram, carWorldMatrix);
				glBindTexture(GL_TEXTURE_2D, carTextureID);
				glDrawArrays(GL_TRIANGLES, 0, CarVertices);
				// Unbind geometry
				glBindVertexArray(0);
				glBindVertexArray(wheelsVAO);
				glBindTexture(GL_TEXTURE_2D, tireTextureID);
				setModelMatrix(shaderScene, wheelsWorldMatrixArray[i]*leftfrontwheel*wheelSpin);
				glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

				setModelMatrix(shaderScene, wheelsWorldMatrixArray[ i] *leftbackwheel*wheelSpin);
				glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

				setModelMatrix(shaderScene, wheelsWorldMatrixArray[ i] *rightfrontwheel*wheelSpin);
				glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

				setModelMatrix(shaderScene, wheelsWorldMatrixArray[ i] *rightbackwheel*wheelSpin);
				glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);
				//glBindVertexArray(CarVAO);
				// Draw geometry
				//glDrawArrays(GL_TRIANGLES, 0, CarVertices);
				//glBindVertexArray(0);

				//glBindTexture(GL_TEXTURE_2D, 0);

				//lights
				glBindVertexArray(lightsVAO);
				glBindTexture(GL_TEXTURE_2D, lightTextureID);
				//setWorldMatrix(textureShaderProgram, lightsWorldMatrix);
				setModelMatrix(shaderScene,lightsRotationWorldMatrixArray[i]);
				glDrawElements(GL_TRIANGLES, lightsVertices, GL_UNSIGNED_INT, 0);
				glBindVertexArray(pipeVAO);
				glBindTexture(GL_TEXTURE_2D, 0);
				setModelMatrix(shaderScene, pipeWorldMatrixArray[i] *pipeTransformationWorldMatrix);
				glDrawElements(GL_TRIANGLES, pipeVertices, GL_UNSIGNED_INT, 0);
			}
			
			}
			glUniform1i(textureLocation, 0);
			//wheels
			/*glUseProgram(shaderScene);

			glActiveTexture(GL_TEXTURE1);
			glBindVertexArray(wheelsVAO);
			glBindTexture(GL_TEXTURE_2D, tireTextureID);
			mat4 leftfrontwheel = glm::translate(mat4(1.0f), vec3( 1.3f,  0.4f,  1.63f));
			setWorldMatrix(shaderScene, wheelsWorldMatrix*leftfrontwheel*wheelSpin);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

			mat4 leftbackwheel = glm::translate(mat4(1.0f), vec3( 1.3f,  0.4f,  - 1.05f));
			setWorldMatrix(shaderScene, wheelsWorldMatrix*leftbackwheel*wheelSpin);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

			mat4 rightfrontwheel = glm::translate(mat4(1.0f), vec3( - 1.3f,  0.4f,  1.63f));
			setWorldMatrix(shaderScene, wheelsWorldMatrix*rightfrontwheel*wheelSpin);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

			mat4 rightbackwheel = glm::translate(mat4(1.0f), vec3( - 1.3f,  0.4f,  - 1.05f));
			setWorldMatrix(shaderScene, wheelsWorldMatrix*rightbackwheel*wheelSpin);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);
			
			glBindVertexArray(0);
			*/
		}

		if (isPoint || glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			isPoint = true;
			isLine = false;
			isTrian = false;
			/*// Draw base
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &baseWorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 0, 36);

			// Draw top
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &topWorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 48, 36);
			// Draw front
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &frontWorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 84, 36);
			// Draw back
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &backWorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 84, 36);
			// Draw Wheels
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wheel1WorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 120, 36);

			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wheel2WorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 120, 36);

			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wheel3WorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 120, 36);

			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wheel4WorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 120, 36);*/
			glUseProgram(shaderShadow);
			// Use proper image output size
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			// Bind depth map texture as output framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			// Clear depth data on the framebuffer
			glClear(GL_DEPTH_BUFFER_BIT);
			// Bind geometry
			glBindVertexArray(textureVbo);
			// Draw geometry
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// Unbind geometry
			glBindVertexArray(0);



			//================================================================================================================render scene
			glUseProgram(shaderScene);
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
			glBindVertexArray(textureVbo);

			//glBindTexture(GL_TEXTURE_2D, depthMmap);
			//GLuint textureLocation = glGetUniformLocation(textureShaderProgram, "textureSampler");
			//glUniform1i(textureLocation, 0);
			setWorldMatrix(shaderScene, groundWorldMatrix);
			glActiveTexture(GL_TEXTURE1);
			GLuint textureLocation = glGetUniformLocation(shaderScene, "texture0");
			glBindTexture(GL_TEXTURE_2D, grassTextureID);
			glUniform1i(textureLocation, 1);
			// Draw geometry
			glDrawArrays(GL_TRIANGLES, 0, 6);
			//car
			glBindVertexArray(CarVAO);

			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &carWorldMatrix[0][0]);
			glBindTexture(GL_TEXTURE_2D, carTextureID);
			glDrawArrays(GL_POINTS, 0, CarVertices);
			glBindVertexArray(0);
			//lights
			glBindVertexArray(lightsVAO);

			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &lightsWorldMatrix[0][0]);
			glBindTexture(GL_TEXTURE_2D, lightTextureID);
			glDrawElements(GL_POINTS, lightsVertices, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			//wheels
			glBindVertexArray(wheelsVAO);

			glBindTexture(GL_TEXTURE_2D, tireTextureID);
			setWorldMatrix(textureShaderProgram, wheelsWorldMatrix*leftfrontwheel*wheelSpin);
			glDrawArrays(GL_POINTS, 0, wheelsVertices);

			setWorldMatrix(textureShaderProgram, wheelsWorldMatrix*leftbackwheel*wheelSpin);
			glDrawArrays(GL_POINTS, 0, wheelsVertices);

			setWorldMatrix(textureShaderProgram, wheelsWorldMatrix*rightfrontwheel*wheelSpin);
			glDrawArrays(GL_POINTS, 0, wheelsVertices);

			setWorldMatrix(textureShaderProgram, wheelsWorldMatrix*rightbackwheel*wheelSpin);
			glDrawArrays(GL_POINTS, 0, wheelsVertices);

			glBindVertexArray(0);
			glUniform1i(textureLocation, 0);
		}
		if (isLine||glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
			isPoint = false;
			isLine = true;
			isTrian = false;
			/*// Draw base
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &baseWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 0, 36);

			// Draw top
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &topWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 48, 36);
			// Draw front
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &frontWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 84, 36);
			// Draw back
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &backWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 84, 36);
			// Draw Wheels
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wheel1WorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 120, 36);

			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wheel2WorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 120, 36);

			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wheel3WorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 120, 36);

			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wheel4WorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 120, 36);*/
			glUseProgram(shaderShadow);
			// Use proper image output size
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			// Bind depth map texture as output framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			// Clear depth data on the framebuffer
			glClear(GL_DEPTH_BUFFER_BIT);
			// Bind geometry
			glBindVertexArray(textureVbo);
			// Draw geometry
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// Unbind geometry
			glBindVertexArray(0);

			glUseProgram(shaderScene);
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
			glBindVertexArray(textureVbo);

			//glBindTexture(GL_TEXTURE_2D, depthMmap);
			//GLuint textureLocation = glGetUniformLocation(textureShaderProgram, "textureSampler");
			//glUniform1i(textureLocation, 0);
			setWorldMatrix(shaderScene, groundWorldMatrix);
			glActiveTexture(GL_TEXTURE1);
			GLuint textureLocation = glGetUniformLocation(shaderScene, "texture0");
			glBindTexture(GL_TEXTURE_2D, grassTextureID);
			glUniform1i(textureLocation, 1);
			// Draw geometry
			glDrawArrays(GL_TRIANGLES, 0, 6);

			//car
			glBindVertexArray(CarVAO);
			glBindTexture(GL_TEXTURE_2D, carTextureID);
			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &carWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 0, CarVertices);

			glBindVertexArray(0);
			//lights
			glBindVertexArray(lightsVAO);
			glBindTexture(GL_TEXTURE_2D, lightTextureID);
			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &lightsWorldMatrix[0][0]);
			glDrawElements(GL_LINES, lightsVertices, GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);
			//wheels
			glBindVertexArray(wheelsVAO);

			//glBindTexture(GL_TEXTURE_2D, tireTextureID);
			setWorldMatrix(textureShaderProgram, wheelsWorldMatrix*leftfrontwheel*wheelSpin);
			glDrawArrays(GL_LINES, 0, wheelsVertices);

			setWorldMatrix(textureShaderProgram, wheelsWorldMatrix*leftbackwheel*wheelSpin);
			glDrawArrays(GL_LINES, 0, wheelsVertices);

			setWorldMatrix(textureShaderProgram, wheelsWorldMatrix*rightfrontwheel*wheelSpin);
			glDrawArrays(GL_LINES, 0, wheelsVertices);

			setWorldMatrix(textureShaderProgram, wheelsWorldMatrix*rightbackwheel*wheelSpin);
			glDrawArrays(GL_LINES, 0, wheelsVertices);
			glUniform1i(textureLocation, 0);
			glBindVertexArray(0);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
			if (AltRelease) {
				isGrip = !isGrip;
				AltRelease = false;
			}
			

		}
		else {
			AltRelease = true;
		}
		if(isGrip){
		//glPushMatrix();
		setColorViewMatrix(colarShaderProgram, viewMatrix);
		setColorProjectionMatrix(colarShaderProgram, projectionMatrix);
		glBindVertexArray(vbo);
		//draw axis
		mat4 axisYWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)) * scale(mat4(1.0f), vec3(0.0f, 5.0f, 0.0f));
		setWorldMatrix(colarShaderProgram, axisYWorldMatrix);
		glDrawArrays(GL_LINES, 0, 2);

		//draw axis
		mat4 axisXWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)) * scale(mat4(1.0f), vec3(5.0f, 0.0f, 0.0f));
		setWorldMatrix(colarShaderProgram, axisXWorldMatrix);
		glDrawArrays(GL_LINES, 2, 2);

		//draw axis
		mat4 axisZWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)) * scale(mat4(1.0f), vec3(0.0f, 0.0f, 5.0f));
		setWorldMatrix(colarShaderProgram, axisZWorldMatrix);
		glDrawArrays(GL_LINES, 4, 2);

		//draw grips
		for (int i = 0; i < 50; i++) {
			mat4 gripZWorldMatrix = translate(mat4(1.0f), vec3((float)i,  0.0f,0.0f)) * scale(mat4(1.0f), vec3(0.0f, 0.0f, 50.0f));
			setWorldMatrix(colarShaderProgram, gripZWorldMatrix);
			glDrawArrays(GL_LINES, 6, 2);
			mat4 gripZMWorldMatrix = translate(mat4(1.0f), vec3(-(float)i, 0.0f,0.0f)) * scale(mat4(1.0f), vec3(0.0f, 0.0f, 50.0f));
			setWorldMatrix(colarShaderProgram, gripZMWorldMatrix);
			glDrawArrays(GL_LINES, 6, 2);
			mat4 gripXWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, (float)i)) * scale(mat4(1.0f), vec3(50.0f, 0.0f, 0.0f));
			setWorldMatrix(colarShaderProgram, gripXWorldMatrix);
			glDrawArrays(GL_LINES, 8, 2);
			mat4 gripXMWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, -(float)i)) * scale(mat4(1.0f), vec3(50.0f, 0.0f, 0.0f));
			setWorldMatrix(colarShaderProgram, gripXMWorldMatrix);
			glDrawArrays(GL_LINES, 8, 2);
		}
		glBindVertexArray(0);
		}
		// End Frame
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Handle inputs
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);


		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			if (SpaceRelease) {
			float r1 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 50.0f));
			float r2 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 50.0f));
			float r3 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
			float r4 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f));
			if(r3>1.0f){
			carX = r1;
			}
			else {
				carX = -r1;
			}
			if (r4 > 1.0f) {
			carZ = r2;
			}
			else {
				carZ = -r2;
			}
			}
			SpaceRelease = false;
			
		}
		else {
			SpaceRelease= true;
		}


		// This was solution for Lab02 - Moving camera exercise
		// We'll change this to be a first or third person camera
		//bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
		/*
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) // move camera to the left
		{
			isFirstPerson = false;
			dx -= 1.1f;
		}

		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) // move camera to the right
		{
			isFirstPerson = false;
			dx += 1.1f;
		}

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) // move camera up
		{
			isFirstPerson = false;
			dy -= 1.1f;
		}

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // move camera down
		{
			isFirstPerson = false;
			dy += 1.1f;
		}
		*/
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) // move camera home
		{
			if (is_C_relese) {
				isFirstPerson = !isFirstPerson;
				is_C_relese = false;
			}
			
		}
		else {
			is_C_relese = true;
		}
		
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			
			glfwGetCursorPos(window, &mousePosX, &unusedMouse);
			isFirstPerson = false;

			dx = mousePosX - lastMousePosX;

			lastMousePosX = mousePosX;
			lastMousePosY = unusedMouse;

		}
		else {
			if(!isFirstPerson){
			dx = 0.0f;
			}
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
			isFirstPerson = false;
			glfwGetCursorPos(window, &unusedMouse, &mousePosY);

			dy = mousePosY - lastMousePosY;

			lastMousePosY = mousePosY; 
			lastMousePosX = unusedMouse;
		}
		else {
			if (!isFirstPerson) {
				dy = 0.0f;
			}
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

			glfwGetCursorPos(window, &unusedMouse, &mousePosY);
			b = (mousePosY - lastMousePosY) * a;
			//cout << b;
			//cout << "B's value" << endl;
			float xz = pow(cameraPosition[0], 2.0f) + pow(cameraPosition[2], 2.0f);
			float xzy = xz + pow(cameraPosition[1], 2.0f);
			float radius = (sqrt(xzy) - radiusLeftClick);
			if (!isFirstPerson) {
				if (1.0f< radius <50.0f) {
					radiusLeftClick += b;
				}
				else if(radius >= 50.0f)
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
					
				
				
			}else{

			if (cameraPosition[0] < 70.2f&& cameraPosition[1] <70.2f&& cameraPosition[2] < 70.2f) {
				if (cameraPosition[1] > 1.2f) {

					cameraPosition += cameraLookAt * b  * currentCameraSpeed;
				}
				else {
					if (b < 0) {
						cameraPosition += cameraLookAt * b  * currentCameraSpeed;
						}
					}
			}
			else {
					if (b > 0) {
						cameraPosition += cameraLookAt * b  * currentCameraSpeed;
					}	
				}

			}
			lastMousePosX = unusedMouse;
			lastMousePosY = mousePosY;
		}

		// Please understand the code when you un-comment it!

		//double mousePosX, mousePosY;
		//glfwGetCursorPos(window, &mousePosX, &mousePosY);

		//double dx = mousePosX - lastMousePosX;
		//double dy = mousePosY - lastMousePosY;

		//lastMousePosX = mousePosX;
		//lastMousePosY = mousePosY;

		// Convert to spherical coordinates
		
		//GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
		//glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
		//setViewMatrix(colarShaderProgram, viewMatrix);
		//setViewMatrix(textureShaderProgram, viewMatrix);


	}


	// Shutdown GLFW
	glfwTerminate();

	return 0;
}
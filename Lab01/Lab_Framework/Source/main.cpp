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
#include <FreeImageIO.h>

using namespace glm;
using namespace std;

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

const char* lightVertexShaderSource()
{
	// For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
	return
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;"
		"layout (location = 1) in vec3 aNormal;"
		""
		"out vec3 FragPos"
		"out vec3 VertexNormal;"
		""
		"uniform mat4 worldMatrix;"
		"uniform mat4 viewMatrix = mat4(1.0);"  // default value for view matrix (identity)
		"uniform mat4 projectionMatrix = mat4(1.0);"
		""
		"out vec3 VertexColor;"
		"void main()"
		"{"
		"   vertexNormal = vec3(worldMatrix * vec4(aPos, 1.0f));"
		"   FragPos = mat3(transpose(inverse(worldMatrix)))* aNormal; "
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
		"	FragColor = vec4(result,1.0f)"
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
	GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}

void setViewMatrix(int shaderProgram, mat4 viewMatrix)
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
	int lightingShaderProgram= compileAndLinkShaders(lightVertexShaderSource(), lightFragmentShaderSource());
	int textureShaderProgram = compileAndLinkShaders(getTexturedVertexShaderSource(), getTexturedFragmentShaderSource());

	int vbo = createVertexBufferObject();
	int textureVbo = createTexturedCubeVertexBufferObject();
	// We can set the shader once, since we have only one
	//glUseProgram(shaderProgram);

	//setup models
	string CarPath = "../Assets/Models/Car.obj";
	string wheelsPath = "../Assets/Models/wheels.obj";
	string lightsPath = "../Assets/Models/lights.obj";

	int CarVertices;
	int wheelsVertices;
	int lightsVertices;
	GLuint CarVAO = setupModelEBO(CarPath, CarVertices);
	GLuint wheelsVAO = setupModelVBO(wheelsPath, wheelsVertices);
	GLuint lightsVAO = setupModelEBO(lightsPath, lightsVertices);


	//int activeVAOVertices = CarVertices;
	//GLuint activeVAO = CarVAO;

	//Load Textures

	GLuint grassTextureID = loadTexture((char*)"../Assets/Textures/grass.jpg");
	GLuint tireTextureID = loadTexture((char*)"../Assets/Textures/wheelTexture.jpg");
	GLuint carTextureID = loadTexture((char*)"../Assets/Textures/car.jpg");
	GLuint lightTextureID = loadTexture((char*)"../Assets/Textures/lights.jpg");

	// Camera parameters for view transform
	vec3 cameraPosition(0.0f, 20.0f, 30.0f);
	vec3 cameraLookAt(0.0f, 0.0f, 0.0f);
	vec3 cameraUp(0.0f, 1.0f, 0.0f);

	// Other camera parameters
	float cameraSpeed = 1.0f;
	float cameraFastSpeed = 2 * cameraSpeed;
	float cameraHorizontalAngle = 90.0f;
	float cameraVerticalAngle = -30.0f;
	bool  cameraFirstPerson = true; // press 1 or 2 to toggle this variable

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

	setViewMatrix(colarShaderProgram, viewMatrix);
	setViewMatrix(textureShaderProgram, viewMatrix);

	setProjectionMatrix(colarShaderProgram, projectionMatrix);
	setProjectionMatrix(textureShaderProgram, projectionMatrix);
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

	//for car scale up and down
	float carSize = 1.0f;

	//for rotation
	float carRotate = 0.0f;
	float wheelRotate = 0.0f;


	bool isTrian = true;
	bool isPoint = false;
	bool isLine = false;
	float radiusLeftClick=0.0f;//for camera leftclick zoom in
	glBindBuffer(GL_ARRAY_BUFFER, textureVbo);

	// Entering Main Loop
	while (!glfwWindowShouldClose(window))
	{
		// Frame time calculation
		float dt = glfwGetTime() - lastFrameTime;
		lastFrameTime += dt;

		// Each frame, reset color of each pixel to glClearColor

		// @TODO 1 - Clear Depth Buffer Bit as well
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
		glDrawElements(GL_TRIANGLES, CarVertices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		*/


		glUseProgram(textureShaderProgram);
		//glUseProgram(shaderProgram);

		glBindVertexArray(textureVbo);
		//Draw ground

		glActiveTexture(GL_TEXTURE0);
		GLuint textureLocation = glGetUniformLocation(textureShaderProgram, "textureSampler");

		glBindTexture(GL_TEXTURE_2D, grassTextureID);
		glUniform1i(textureLocation, 0);


		mat4 groundWorldMatrix = translate(mat4(1.0f), vec3(0.0f, -0.01f, 0.0f)) * scale(mat4(1.0f), vec3(50.0f, 0.02f, 50.0f));
		setWorldMatrix(textureShaderProgram, groundWorldMatrix);
		//worldMatrixLocation = glGetUniformLocation(textureShaderProgram, "worldMatrix");
		//glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &groundWorldMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 6); // 36 vertices, starting at index 0

		glBindVertexArray(0);


		glUseProgram(colarShaderProgram);



		GLuint world1MatrixLocation = glGetUniformLocation(colarShaderProgram, "worldMatrix");
		//mat4 viewMatrix(1.0f);
		//viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
		//setViewMatrix(shaderProgram, viewMatrix);
		spinningwheelAngle += 360.0f * dt;

		mat4 carWorldMatrix = mat4(1.0f);
		mat4 carTransformationWorldMatrix = mat4(1.0f);
		mat4 carScaleWorldMatrix = mat4(1.0f);
		mat4 carRotationWorldMatrix = mat4(1.0f);
		carTransformationWorldMatrix = glm::translate(carTransformationWorldMatrix, vec3(carX + 0.0f, carY + 0.0f, carZ + 0.0f));
		carScaleWorldMatrix = glm::scale(carScaleWorldMatrix, vec3(carSize * 2.0f, carSize * 2.0f, carSize * 2.0f));
		carRotationWorldMatrix = glm::rotate(carRotationWorldMatrix, radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
		carWorldMatrix = carTransformationWorldMatrix * carRotationWorldMatrix *carScaleWorldMatrix;
		carWorldMatrix = glm::rotate(carWorldMatrix, radians(90.0f), vec3(0.0f, -1.0f, 0.0f));

		mat4 lightsWorldMatrix = mat4(1.0f);
		mat4 lightsTransformationWorldMatrix = mat4(1.0f);
		mat4 lightsScaleWorldMatrix = mat4(1.0f);
		mat4 lightsRotationWorldMatrix = mat4(1.0f);
		lightsTransformationWorldMatrix = glm::translate(lightsTransformationWorldMatrix, vec3(carX + 0.0f, carY + 0.0f, carZ + 0.0f));
		lightsScaleWorldMatrix = glm::scale(lightsScaleWorldMatrix, vec3(carSize * 2.0f, carSize * 2.0f, carSize * 2.0f));
		lightsRotationWorldMatrix = glm::rotate(lightsRotationWorldMatrix, radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
		lightsWorldMatrix = lightsTransformationWorldMatrix * lightsRotationWorldMatrix *lightsScaleWorldMatrix;
		lightsWorldMatrix = glm::rotate(lightsWorldMatrix, radians(90.0f), vec3(0.0f, -1.0f, 0.0f));

		mat4 wheelsWorldMatrix = mat4(1.0f);
		mat4 wheelsTransformationWorldMatrix = mat4(1.0f);
		mat4 wheelsScaleWorldMatrix = mat4(1.0f);
		mat4 wheelsRotationWorldMatrix = mat4(1.0f);
		wheelsTransformationWorldMatrix = glm::translate(wheelsTransformationWorldMatrix, vec3(carX + 0.0f, carY + 0.0f, carZ + 0.0f));
		wheelsScaleWorldMatrix = glm::scale(wheelsScaleWorldMatrix, vec3(carSize * 2.0f, carSize * 2.0f, carSize * 2.0f));
		wheelsRotationWorldMatrix = glm::rotate(wheelsRotationWorldMatrix, radians(carRotate), vec3(0.0f, 1.0f, 0.0f));
		wheelsWorldMatrix = wheelsTransformationWorldMatrix * wheelsRotationWorldMatrix *carScaleWorldMatrix;
		wheelsWorldMatrix = glm::rotate(wheelsWorldMatrix, radians(90.0f), vec3(0.0f, -1.0f, 0.0f));
		//wheelsWorldMatrix = glm::rotate(wheelsWorldMatrix, radians(spinningwheelAngle), vec3(1.0f, 0.0f, 0.0f));

		spinningwheelAngle += 360.0f * dt;
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
			carX -= 0.1f;
		}else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{


			carRotate += 0.333333f;
			//wheelRotate += 0.03333f;
			//wheel1WorldMatrix = glm::rotate(wheel1WorldMatrix, radians(carRotate), glm::vec3(0.0f, 1.0f, 0.0f));
		}

		if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			carX += 0.1f;
		}else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			carRotate -= 0.333333f;
			//wheelRotate -= 0.03333f;
		}

		if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			carZ -= 0.1f;
		}
		if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			carZ += 0.1f;
		}


		if ((!(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) && (!(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)) && (!(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS))) {
			//wheelRotate = 0;
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
			glBindVertexArray(CarVAO);

			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &carWorldMatrix[0][0]);
			glDrawElements(GL_TRIANGLES, CarVertices, GL_UNSIGNED_INT, 0);


			//lights
			glBindVertexArray(lightsVAO);

			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &lightsWorldMatrix[0][0]);
			glDrawElements(GL_TRIANGLES, lightsVertices, GL_UNSIGNED_INT, 0);

			//wheels
			glUseProgram(textureShaderProgram);

			glActiveTexture(GL_TEXTURE0);
			glBindVertexArray(wheelsVAO);
			glBindTexture(GL_TEXTURE_2D, tireTextureID);
			setWorldMatrix(textureShaderProgram, wheelsWorldMatrix);
			glDrawArrays(GL_TRIANGLES, 0, wheelsVertices);

			glBindVertexArray(0);
			glUseProgram(colarShaderProgram);
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
			//car
			glBindVertexArray(CarVAO);

			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &carWorldMatrix[0][0]);
			glDrawElements(GL_POINTS, CarVertices, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			//lights
			glBindVertexArray(lightsVAO);

			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &lightsWorldMatrix[0][0]);
			glDrawElements(GL_POINTS, lightsVertices, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			//wheels
			glBindVertexArray(wheelsVAO);

			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &wheelsWorldMatrix[0][0]);
			glDrawArrays(GL_POINTS, 0, wheelsVertices);

			glBindVertexArray(0);
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
			//car
			glBindVertexArray(CarVAO);

			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &carWorldMatrix[0][0]);
			glDrawElements(GL_LINES, CarVertices, GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);
			//lights
			glBindVertexArray(lightsVAO);

			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &lightsWorldMatrix[0][0]);
			glDrawElements(GL_LINES, lightsVertices, GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);
			//wheels
			glBindVertexArray(wheelsVAO);

			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &wheelsWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 0, wheelsVertices);

			glBindVertexArray(0);
		}
		
		//glPushMatrix();

		glBindVertexArray(vbo);
		//draw axis
		mat4 axisYWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)) * scale(mat4(1.0f), vec3(0.0f, 5.0f, 0.0f));
		glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &axisYWorldMatrix[0][0]);
		glDrawArrays(GL_LINES, 0, 2);

		//draw axis
		mat4 axisXWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)) * scale(mat4(1.0f), vec3(5.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &axisXWorldMatrix[0][0]);
		glDrawArrays(GL_LINES, 2, 2);

		//draw axis
		mat4 axisZWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)) * scale(mat4(1.0f), vec3(0.0f, 0.0f, 5.0f));
		glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &axisZWorldMatrix[0][0]);
		glDrawArrays(GL_LINES, 4, 2);

		//draw grips
		for (int i = 0; i < 50; i++) {
			mat4 gripZWorldMatrix = translate(mat4(1.0f), vec3((float)i,  0.0f,0.0f)) * scale(mat4(1.0f), vec3(0.0f, 0.0f, 50.0f));
			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &gripZWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 6, 2);
			mat4 gripZMWorldMatrix = translate(mat4(1.0f), vec3(-(float)i, 0.0f,0.0f)) * scale(mat4(1.0f), vec3(0.0f, 0.0f, 50.0f));
			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &gripZMWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 6, 2);
			mat4 gripXWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, (float)i)) * scale(mat4(1.0f), vec3(50.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &gripXWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 8, 2);
			mat4 gripXMWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, -(float)i)) * scale(mat4(1.0f), vec3(50.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(world1MatrixLocation, 1, GL_FALSE, &gripXMWorldMatrix[0][0]);
			glDrawArrays(GL_LINES, 8, 2);
		}
		glBindVertexArray(0);

		// End Frame
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Handle inputs
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);


		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
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


		// This was solution for Lab02 - Moving camera exercise
		// We'll change this to be a first or third person camera
		//bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
		float currentCameraSpeed = cameraFastSpeed;

		double dx = 0.0f;
		double dy = 0.0f;
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) // move camera to the left
		{
			cameraFirstPerson = false;
			dx -= 1.1f;
		}

		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) // move camera to the right
		{
			cameraFirstPerson = false;
			dx += 1.1f;
		}

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) // move camera up
		{
			cameraFirstPerson = false;
			dy -= 1.1f;
		}

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // move camera down
		{
			cameraFirstPerson = false;
			dy += 1.1f;
		}

		if (glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS) // move camera home
		{
			cameraFirstPerson = true;
			cameraHorizontalAngle = 90.0f;
			cameraVerticalAngle = -30.0f;
			cameraPosition[0] = 0.0f;
			cameraPosition[1] = 20.0f;
			cameraPosition[2] = 30.0f;
			cameraLookAt[0] = 0.0f;
			cameraLookAt[1] = 0.0f;
			cameraLookAt[2] = 0.0f;
			cameraUp[0] = 0.0f;
			cameraUp[1] = 1.0f;
			cameraUp[2] = 0.0f;
			viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
		}
		double mousePosX, mousePosY;
		float a = 0.01f;
		float b = 1.0f;
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			
			glfwGetCursorPos(window, &mousePosX, &mousePosY);
			cameraFirstPerson = false;

			dx = mousePosX - lastMousePosX;

			lastMousePosX = mousePosX;

		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
			cameraFirstPerson = false;
			glfwGetCursorPos(window, &mousePosX, &mousePosY);

			dy = mousePosY - lastMousePosY;

			lastMousePosY = mousePosY;  
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

			glfwGetCursorPos(window, &mousePosX, &mousePosY);
			b = (mousePosY - lastMousePosY) * a;
			//cout << b;
			//cout << "B's value" << endl;
			float xz = pow(cameraPosition[0], 2.0f) + pow(cameraPosition[2], 2.0f);
			float xzy = xz + pow(cameraPosition[1], 2.0f);
			float radius = (sqrt(xzy) - radiusLeftClick);
			if (!cameraFirstPerson) {
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
			//lastMousePosX = mousePosX;
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

		float theta = radians(cameraHorizontalAngle);
		float phi = radians(cameraVerticalAngle);

		cameraLookAt = vec3(cosf(phi)*cosf(theta), sinf(phi), -cosf(phi)*sinf(theta));
		vec3 cameraSideVector = glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));

		glm::normalize(cameraSideVector);


		
		mat4 viewMatrix(1.0f);
		if(cameraFirstPerson){
        viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp );
		}
		else {
			float xz=pow(cameraPosition[0],2.0f)+ pow(cameraPosition[2], 2.0f);
			float xzy = xz + pow(cameraPosition[1], 2.0f);
			vec3 CarPos(carX,carY,carZ);
			float radius = sqrt(xzy)-radiusLeftClick;
			vec3 Pos = CarPos - vec3(radius*cosf(phi)*cosf(theta),
				radius*sinf(phi),
				-radius * cosf(phi)*sinf(theta));
			viewMatrix = lookAt(Pos, CarPos, cameraUp);
		}
		//GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
		//glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
		setViewMatrix(colarShaderProgram, viewMatrix);
		setViewMatrix(textureShaderProgram, viewMatrix);


	}


	// Shutdown GLFW
	glfwTerminate();

	return 0;
}
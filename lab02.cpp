//
// COMP 371 Labs Framework
//
// Created by Nicolas Bergeron on 20/06/2019.
//
// Inspired by the following tutorials:
// - https://learnopengl.com/Getting-started/Hello-Window
// - https://learnopengl.com/Getting-started/Hello-Triangle

#include <iostream>


#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices

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


int compileAndLinkShaders()
{
    // compile and link shader program
    // return shader program id
    // ------------------------------------

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSource = getVertexShaderSource();
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
    const char* fragmentShaderSource = getFragmentShaderSource();
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

int createVertexArrayObject()
{
    // A vertex is a point on a polygon, it contains positions and other data (eg: colors)
    glm::vec3 vertexArray[] = {
        glm::vec3( 0.0f,  0.5f, 0.03f),  // top center position
        glm::vec3( 1.0f,  0.0f, 0.0f),  // top center color (red)
        glm::vec3( 0.5f, -0.5f, 0.03f),  // bottom right
        glm::vec3( 0.0f,  1.0f, 0.0f),  // bottom right color (green)
        glm::vec3(-0.5f, -0.5f, 0.03f),  // bottom left
        glm::vec3( 0.0f,  0.0f, 1.0f),  // bottom left color (blue)
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
                          2*sizeof(glm::vec3), // stride - each vertex contain 2 vec3 (position, color)
                          (void*)0             // array buffer offset
                          );
    glEnableVertexAttribArray(0);


    glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          2*sizeof(glm::vec3),
                          (void*)sizeof(glm::vec3)      // color is offseted a vec3 (comes after position)
                          );
    glEnableVertexAttribArray(1);

	glBindVertexArray(0);
    return vertexBufferObject;
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

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow* window = glfwCreateWindow(800, 600, "Comp371 - Lab 02", NULL, NULL);
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
    int shaderProgram = compileAndLinkShaders();
    
    // Define and upload geometry to the GPU here ...
    int vbo = createVertexArrayObject();
    
    // Variables to be used later in tutorial
    float angle = 0;
    float rotationSpeed = 180.0f;  // 180 degrees per second
    float lastFrameTime = glfwGetTime();
	float leftRight = 0.0f;
	float forwardBack = 0.0f;
	float speed = 1;
	//enable backface culling
	glEnable(GL_CULL_FACE);
	//ther aer other ways to do it and we can specify the rotation of the vertics like couter clock or clock 
    // Entering Main Loop
    while(!glfwWindowShouldClose(window))
    {
        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Draw geometry
        glUseProgram(shaderProgram);
		glBindVertexArray(vbo);
        /*glbindbuffer(gl_array_buffer, vbo);*/

		float dt = glfwGetTime() - lastFrameTime;
		lastFrameTime += dt;

		angle = (angle + rotationSpeed * dt);
		glm::mat4 rotateMatrix=glm::rotate(glm::mat4(1.0f),glm::radians(angle),glm::vec3(0.0f, 1.0f,0.0f));
		
		GLuint worldMatrixLoaction = glGetUniformLocation(shaderProgram, "worldMatrix");
		glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &rotateMatrix[0][0]);//1 is the number of matrix you are passing in
		//glm::value_ptr(rotationMatrix); #include  <glm/.../type_ptr>


        glDrawArrays(GL_TRIANGLES, 0, 3); // 3 vertices, starting at index 0


		glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, -0.25f, 0.25f));
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f));

		glm::mat4 worldMatrix = translationMatrix * scalingMatrix;

		glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &worldMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 3);


		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, 0.0f));

		worldMatrix = translationMatrix * scalingMatrix;

		glUniformMatrix4fv(worldMatrixLoaction, 1, GL_FALSE, &worldMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindVertexArray(0);

        // End Frame
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Handle inputs
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

		glm::mat4 viewMatrix = glm::mat4(1.0f);
		glm::mat4 viewTransform= glm::translate(viewMatrix,glm::vec3(leftRight*speed, 0.0f, forwardBack*speed));
		viewMatrix = viewTransform* viewMatrix;
		GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		{
			viewMatrix = glm::mat4(1.0f);

			GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
			glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
		}

		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		{
			viewMatrix = glm::lookAt(glm::vec3(-0.5f,0.0f,0.0f),glm::vec3(-0.5f, 0.0f, -1.0f),glm::vec3(0.0f, 1.0f, 0.0f));

			GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
			glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
		}

		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		{
			glm::mat4 projectionMatrix = glm::perspective(70.0f,800.0f/600.0f,0.01f,100.0f);

			GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
			glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		}

		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		{
			glm::mat4 projectionMatrix = glm::ortho(-4.0f,4.0f,-3.0f,3.0f,-100.0f,100.0f);

			GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
			glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		}


		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			forwardBack += 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			forwardBack -= 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			leftRight += 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			leftRight -= 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			speed =2;
		}
		else {
			speed=1;
		}
    }
    
    // Shutdown GLFW
    glfwTerminate();
    
	return 0;
}

// Third Party Libraries
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>

// Cpp Standard Template Libraries (STL)
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

//--------------------------- Error Handling Routines --------------------------------
static void GLClearAllErrors() {
	while (glGetError() != GL_NO_ERROR) {}
}

// returns true if we have an error
static bool GLCheckErrorStatus(const char* function, int line) {
	while (GLenum error = glGetError()) {
		std::cout << 
			"OpenGL Error: " << error << 
			"\tLine: " << line << 
			"\tFunction: " << function << 
			std::endl;
		return true;
	}
	return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);
//------------------------------------------------------------------------------------


//Screen Dimensions
int gScreenHeight = 480;
int gScreenWidth = 680;
SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

// Main loop flag
bool gQuit = false;

//VAO
GLuint gVertexArrayObject = 0;
//VBO
GLuint gVertexBufferObject = 0;
//IBO or EBO
//this is used to store the array of indices that we want to draw from, when we do indexed drawing.
GLuint gIndexBufferObject = 0;

//program object for our shader
GLuint gGraphicsPipelineShaderProgram = 0;

float gUOffset = -2.0f;
float gURotate = 0.0f;
float gUScale = 0.5f;

GLuint CompileShader(GLuint type, const std::string& source)
{
	GLuint shaderObject;

	if (type == GL_VERTEX_SHADER)
	{
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	}
	else if (type == GL_FRAGMENT_SHADER)
	{
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* src = source.c_str();
	glShaderSource(shaderObject, 1, &src, nullptr);
	glCompileShader(shaderObject);

	int result;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		char* errorMessages = new char[length];
		glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

		if (type == GL_VERTEX_SHADER)
		{
			std::cout << "ERROR: GL_VERTEX_SHADER compiliation failed!\n" << errorMessages << "\n";
		}
		else if (type == GL_FRAGMENT_SHADER)
		{
			std::cout << "ERROR: GL_FRAGMENT_SHADER compiliation failed!\n" << errorMessages << "\n";
		}
		delete[] errorMessages;
		glDeleteShader(shaderObject);
		return 0;
	}

	return shaderObject;
}

std::string LoadShaderAsString(const std::string& filename)
{
	std::string result = "";
	std::string line = "";
	std::ifstream myFile(filename.c_str());

	if (myFile.is_open() )
	{
		while (std::getline(myFile, line))
		{
			result += line + '\n';
		}
		myFile.close();
	}

	return result;
}

int main(int argc, char* args[])
{
	printf("Hello OpenGL!\n");
	//Initialization: Setup the graphics program
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			printf("SDL2 video subsystem could not be initialized!\n");
			exit(1);
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);


		gGraphicsApplicationWindow = SDL_CreateWindow(
			"OpenGL Window", 
			50, 
			50, 
			gScreenWidth, 
			gScreenHeight, 
			SDL_WINDOW_OPENGL
		);

		if ( nullptr == gGraphicsApplicationWindow )
		{
			printf("SDL window was not able to be created.\n");
			exit(1);
		}

		gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);

		if (nullptr == gOpenGLContext)
		{
			printf("OpenGL context couldn't be created.\n");
			exit(1);
		}

		if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
		{
			printf("glad was not initialized\n");
			exit(1);
		}

		printf("Vendor: %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("Version: %s\n", glGetString(GL_VERSION));
		printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	}

	//vertex specification: Setup our geometry
	{
		//lives on CPU
		const std::vector<GLfloat> vertexData
		{
			 // 0 - vertex
			-0.5f, -0.5f, 0.0f, //left vertex position
			 1.0f,  0.0f, 0.0f, //color
			 // 1 - vertex
			 0.5f, -0.5f, 0.0f, //right vertex position
			 0.0f,  1.0f, 0.0f, //color
			 // 2 - vertex
			-0.5f,  0.5f, 0.0f, //top-left vertext position
			 0.0f,  0.0f, 1.0f, //color
			 // 3 - vertex
			 0.5f,  0.5f, 0.0f, //top-right vertex position
			 0.0f,  0.0f, 1.0f, //color
		};

		//we start setting things up on the GPU
		glGenVertexArrays(1, &gVertexArrayObject);
		glBindVertexArray(gVertexArrayObject);

		glGenBuffers(1, &gVertexBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
		glBufferData(
			GL_ARRAY_BUFFER,
			vertexData.size() * sizeof(GL_FLOAT),
			vertexData.data(),
			GL_STATIC_DRAW
		);

		const std::vector<GLuint> indexBufferData{ 2,0,1, 3,2,1 };

		//setup the index (element) buffer object (IBO i.e. EBO)
		glGenBuffers(1, &gIndexBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferObject);

		//populate our index buffer
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER, 
			indexBufferData.size() * sizeof(GLuint), 
			indexBufferData.data(), 
			GL_STATIC_DRAW
		);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			sizeof(GL_FLOAT)*6,//stribe
			(void*)0
		);

		// linking up the attributes in our VAO
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,
			3, //r,g,b
			GL_FLOAT,
			GL_FALSE,
			sizeof(GL_FLOAT)*6,
			(GLvoid*)(sizeof(GL_FLOAT)*3)
		);

		glBindVertexArray(0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}

	//create graphic pipeline
	//	- At a minimum, this means the vertex and fragment shader
	{
		//create shader program
		{
			const std::string vertexShaderSource = LoadShaderAsString(".\\shaders\\vert.glsl");
			const std::string fragmentShaderSource = LoadShaderAsString(".\\shaders\\frag.glsl");

			GLuint programObject = glCreateProgram();

			GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
			GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
			glAttachShader(programObject, myVertexShader);
			glAttachShader(programObject, myFragmentShader);
			glLinkProgram(programObject);

			// validate our program
			glValidateProgram(programObject);

			// delete the individual shaders once we are done
			glDetachShader(programObject, myVertexShader);
			glDetachShader(programObject, myFragmentShader);

			glDeleteShader(myVertexShader);
			glDeleteShader(myFragmentShader);

			gGraphicsPipelineShaderProgram = programObject;
		}
	}

	//application main loop
	{
		while (!gQuit)
		{
			//input
			{
				SDL_Event e;

				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						printf("Goodbye!\n");
						gQuit = true;
					}
				}

				const Uint8* state = SDL_GetKeyboardState(NULL);
				if (state[SDL_SCANCODE_UP]) {
					gUOffset += 0.001f;
					std::cout << "gUOffset: " << gUOffset << std::endl;
				}
				if (state[SDL_SCANCODE_DOWN]) {
					gUOffset -= 0.001f;
					std::cout << "gUOffset: " << gUOffset << std::endl;
				}
				if (state[SDL_SCANCODE_LEFT]) {
					gURotate -= 0.01f;
					std::cout << "gURotate: " << gURotate << std::endl;
				}
				if (state[SDL_SCANCODE_RIGHT]) {
					gURotate += 0.01f;
					std::cout << "gURotate: " << gURotate << std::endl;
				}
			}

			//pre draw
			{
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);

				glViewport(0, 0, gScreenWidth, gScreenHeight);
				glClearColor(1.f, 1.f, 0.1f, 1.f);
				glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
				glUseProgram(gGraphicsPipelineShaderProgram);
				
				//Model transformation by translating our object into world space.
				glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(gURotate), glm::vec3(0.0f, 0.1f, 0.0f));
				
				//Update our model matrix by applying a rotation after our translation.
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, gUOffset));

				model = glm::scale(model, glm::vec3(gUScale, gUScale, gUScale));
				// Retrive our location of our Model Matrix
				GLint uModelMatrixLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_ModelMatrix");

				if (uModelMatrixLocation >= 0) {
					//std::cout << "location of u_offset:" << location << std::endl;
					glUniformMatrix4fv(uModelMatrixLocation, 1, GL_FALSE, &model[0][0]);
				}
				else {
					std::cout << "Could not find u_ModelMatrix. maybe a mispelling?" << std::endl;
					exit(EXIT_FAILURE);
				}

				//Projection matrix (in perspective)
				glm::mat4 projection = glm::perspective(
					glm::radians(45.0f),
					(float)gScreenWidth/(float)gScreenHeight,
					0.1f,
					10.0f);

				// Retrieve our location of our perspective matrix uniform
				GLint uProjectionLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_Projection");

				if (uProjectionLocation >= 0) {
					//std::cout << "location of u_offset:" << location << std::endl;
					glUniformMatrix4fv(uProjectionLocation, 1, GL_FALSE, &projection[0][0]);
				}
				else {
					std::cout << "Could not find u_Perspective. maybe a mispelling?" << std::endl;
					exit(EXIT_FAILURE);
				}
			}

			//draw
			{
				glBindVertexArray(gVertexArrayObject);
				glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
				//glDrawArrays(GL_TRIANGLES, 0, 6);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

				// stop using our current graphics pipeline
				// Note: this is not necessary if we only have one graphics pipeline.
				glUseProgram(0);
			}

			//update the screen
			SDL_GL_SwapWindow(gGraphicsApplicationWindow);
		}
	}

	//clean up: call the cleanup function when our program terminates
	{
		SDL_DestroyWindow(gGraphicsApplicationWindow);
		SDL_Quit();
	}
	return 0;
}
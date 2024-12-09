#include <stdio.h>
#include <SDL.h>
#include <glad/glad.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

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
GLuint gVertexBufferObject2 = 0;

//program object for our shader
GLuint gGraphicsPipelineShaderProgram = 0;

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
		const std::vector<GLfloat> vertexPosition
		{
			-0.8f, -0.8f, 0.0f, //left vertex position
			 0.8f, -0.8f, 0.0f, //right vertex position
			 0.0f,  0.8f, 0.0f // top vertext position
		};

		const std::vector<GLfloat> vertexColors
		{
			 1.0f,  0.0f, 0.0f, //left vertex position
			 0.0f,  1.0f, 0.0f, //right vertex position
			 0.0f,  0.0f, 1.0f // top vertext position
		};

		//we start setting things up on the GPU
		glGenVertexArrays(1, &gVertexArrayObject);
		glBindVertexArray(gVertexArrayObject);

		glGenBuffers(1, &gVertexBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
		glBufferData(
			GL_ARRAY_BUFFER,
			vertexPosition.size() * sizeof(GL_FLOAT),
			vertexPosition.data(),
			GL_STATIC_DRAW
		);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		// setting up our colors
		glGenBuffers(1, &gVertexBufferObject2);
		glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject2);
		glBufferData(GL_ARRAY_BUFFER,
			vertexColors.size() * sizeof(GL_FLOAT),
			vertexColors.data(),
			GL_STATIC_DRAW
		);

		// linking up the attributes in our VAO
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,
			3, //r,g,b
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
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
			}

			//pre draw
			{
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);

				glViewport(0, 0, gScreenWidth, gScreenHeight);
				glClearColor(1.f, 1.f, 0.1f, 1.f);
				glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
				glUseProgram(gGraphicsPipelineShaderProgram);
			}

			//draw
			{
				glBindVertexArray(gVertexArrayObject);
				glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
				glDrawArrays(GL_TRIANGLES, 0, 3);

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
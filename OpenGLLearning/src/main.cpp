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

// Our libraries
#include "Camera.hpp"

//--------------------------- Error Handling Routines --------------------------------
/// <summary>
/// Returns the location of an uniform variable based on its name.
/// </summary>
/// <param name="pipeline"></param>
/// <param name="name"></param>
/// <returns></returns>
static int FindUniformLocation(GLuint pipeline, const GLchar* name)
{
	GLint location = glGetUniformLocation(pipeline, name);
	if (location < 0) {
		std::cout << "Could not find " << name << ", maybe a mispelling? " << std::endl;
		exit(EXIT_FAILURE);
	}

	//TODO: Think of some better error code, if we don't want to exit.
	return location;
}

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

struct App {
	//Screen Dimensions
	int				mScreenHeight					= 480;
	int				mScreenWidth					= 680;
	SDL_Window*		mGraphicsApplicationWindow		= nullptr;
	SDL_GLContext	mOpenGLContext					= nullptr;
	// Main loop flag
	bool			mQuit							= false;
	//program object for our shader
	GLuint			mGraphicsPipelineShaderProgram	= 0;
	/// <summary>
	/// A single global camera.
	/// </summary>
	Camera			mCamera;
};

struct Transform {
	glm::mat4 mModelMatrix{ glm::mat4(1.0f) };
};

struct Mesh3D {
	//VAO
	GLuint mVertexArrayObject	= 0;
	//VBO
	GLuint mVertexBufferObject	= 0;
	//IBO or EBO
	//this is used to store the array of indices that we want to draw from, when we do indexed drawing.
	GLuint mIndexBufferObject	= 0;

	/// <summary>
	///This is the graphic pipeline used for this mesh.
	/// </summary>
	GLuint mPipeline			= 0;

	Transform mTransform;
	float mURotate				= 0.0f;
	float mUScale				= 0.5f;
};


App gApp; //Global application
Mesh3D gMesh1;
Mesh3D gMesh2;

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

/// <summary>
/// Initialization: Setup the graphics program
/// </summary>
/// <param name="app"></param>
void InitializeProgram(App* app)
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


	app->mGraphicsApplicationWindow = SDL_CreateWindow(
		"OpenGL Window",
		50,
		50,
		app->mScreenWidth,
		app->mScreenHeight,
		SDL_WINDOW_OPENGL
	);

	if (nullptr == app->mGraphicsApplicationWindow)
	{
		printf("SDL window was not able to be created.\n");
		exit(1);
	}

	app->mOpenGLContext = SDL_GL_CreateContext(app->mGraphicsApplicationWindow);

	if (nullptr == app->mOpenGLContext)
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

/// <summary>
/// vertex specification: Setup our geometry
/// </summary>
/// <param name="mesh"></param>
void MeshCreate(Mesh3D* mesh)
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
	glGenVertexArrays(1, &mesh->mVertexArrayObject);
	glBindVertexArray(mesh->mVertexArrayObject);

	glGenBuffers(1, &mesh->mVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->mVertexBufferObject);
	glBufferData(
		GL_ARRAY_BUFFER,
		vertexData.size() * sizeof(GLfloat),
		vertexData.data(),
		GL_STATIC_DRAW
	);

	const std::vector<GLuint> indexBufferData{ 2,0,1, 3,2,1 };

	//setup the index (element) buffer object (IBO i.e. EBO)
	glGenBuffers(1, &mesh->mIndexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->mIndexBufferObject);

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
		false,
		sizeof(GLfloat) * 6,//stribe
		(void*)0
	);

	// linking up the attributes in our VAO
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		3, //r,g,b
		GL_FLOAT,
		false,
		sizeof(GLfloat) * 6,
		(GLvoid*)(sizeof(GLfloat) * 3)
	);

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void MeshDelete(Mesh3D* mesh)
{
	glDeleteBuffers(1, &mesh->mVertexBufferObject);
	glDeleteVertexArrays(1, &mesh->mVertexArrayObject);
}

/// <summary>
/// MeshSetPipeline
/// Needs to set the graphic pipeline before we draw. 
/// </summary>
/// <param name="pipeline"></param>
/// <param name=""></param>
void MeshSetPipeline(Mesh3D* mesh, GLuint pipeline)
{
	mesh->mPipeline = pipeline;
}

/// <summary>
/// Draw Mesh
/// 
/// Note: We per mesh, choose the graphics pipleine that we want to use.
/// Generally, this is not very efficient, to 'change state' (pipelines)
/// very frequently. (But, for learning purposes or flexibility, this is useful)
/// </summary>
void DrawMesh(Mesh3D* mesh)
{
	if (mesh == nullptr)
	{
		return;
	}

	// Setup which graphics pipeline we are going to use
	glUseProgram(mesh->mPipeline);

	glUniformMatrix4fv(
		FindUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_ModelMatrix"),
		1,
		false,
		&mesh->mTransform.mModelMatrix[0][0]
	);

	//NOTE: Often times we combine the model and the view matrix together to just send in 1 uniform.
	//Updating our view matrix
	{
		glm::mat4 view = gApp.mCamera.GetViewMatrix();
		glUniformMatrix4fv(
			FindUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_ViewMatrix"),
			1,
			false,
			&view[0][0]
		);
	}

	//Updating our projection matrix (in perspective)
	{
		glm::mat4 perspective = gApp.mCamera.GetProjectionMatrix();

		// Retrieve our location of our perspective matrix uniform
		glUniformMatrix4fv(
			FindUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_Projection"),
			1,
			false,
			&perspective[0][0]
		);
	}

	glBindVertexArray(mesh->mVertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->mVertexBufferObject);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// stop using our current graphics pipeline
	// Note: this is not necessary if we only have one graphics pipeline.
	glUseProgram(0);
}

/// <summary>
/// Translates a mesh -- updating the model matrix.
/// </summary>
/// <param name="mesh"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="z"></param>
void MeshTranslate(Mesh3D* mesh, float x, float y, float z)
{
	mesh->mURotate -= 0.01f;
	std::cout << "gURotate: " << mesh->mURotate << std::endl;
	mesh->mTransform.mModelMatrix = glm::translate(mesh->mTransform.mModelMatrix,glm::vec3(x,y,z));
	// Retrive our location of our Model Matrix
}

/// <summary>
/// Rotates a mesh about an arbitrary axis.
/// </summary>
/// <param name="mesh"></param>
/// <param name="angle"></param>
/// <param name="axis"></param>
void MeshRotate(Mesh3D* mesh, float angle, glm::vec3 axis)
{
	//Model transformation by translating our object into world space.
	mesh->mTransform.mModelMatrix = glm::rotate(mesh->mTransform.mModelMatrix, glm::radians(angle), axis);
}

/// <summary>
/// Scales a mesh by a given scale factor.
/// </summary>
/// <param name="mesh"></param>
/// <param name="scale"></param>
void MeshScale(Mesh3D* mesh, glm::vec3 scale)
{
	mesh->mTransform.mModelMatrix = glm::scale(mesh->mTransform.mModelMatrix, scale);
}

int main(int argc, char* args[])
{
	printf("Hello OpenGL!\n");
	
	InitializeProgram(&gApp);

	//setup our camera
	gApp.mCamera.SetProjectionMatrix(
		glm::radians(45.0f), 
		(float)gApp.mScreenWidth/gApp.mScreenHeight, 
		0.1f, 
		100.0f
	);

	MeshCreate(&gMesh1);
	MeshTranslate(&gMesh1, 0.0f, 0.0f, -2.0f);
	MeshScale(&gMesh1, glm::vec3(1.0f, 1.0f, 1.0f));
	

	MeshCreate(&gMesh2);
	MeshTranslate(&gMesh2, 0.0f, 0.0f, -4.0f);
	MeshScale(&gMesh2, glm::vec3(1.0f, 2.0f, 1.0f));

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

			gApp.mGraphicsPipelineShaderProgram = programObject;
		}
	}

	MeshSetPipeline(&gMesh1, gApp.mGraphicsPipelineShaderProgram);
	MeshSetPipeline(&gMesh2, gApp.mGraphicsPipelineShaderProgram);

	//application main loop
	{
		SDL_WarpMouseInWindow(gApp.mGraphicsApplicationWindow, gApp.mScreenWidth/2, gApp.mScreenHeight/2);
		SDL_SetRelativeMouseMode(SDL_TRUE);
		while (!gApp.mQuit)
		{
			//input
			{
				SDL_Event e;
				static int mouseX = gApp.mScreenWidth/2; 
				static int mouseY = gApp.mScreenHeight/2;
				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						printf("Goodbye!\n");
						gApp.mQuit = true;
					}
					else if (e.type == SDL_MOUSEMOTION)
					{
						mouseX += e.motion.xrel;
						mouseY += e.motion.yrel;
						gApp.mCamera.MouseLook(mouseX, mouseY);
					}
				}
				// TODO: use some other key to move our object
				//gUOffset += 0.001f;
				//std::cout << "gUOffset: " << gUOffset << std::endl;
				const Uint8* state = SDL_GetKeyboardState(NULL);
				float speed = 0.005f;
				if (state[SDL_SCANCODE_UP]) {
					gApp.mCamera.MoveForward(speed);
				}
				if (state[SDL_SCANCODE_DOWN]) {
					gApp.mCamera.MoveBackward(speed);
					//gUOffset -= 0.001f;
					//std::cout << "gUOffset: " << gUOffset << std::endl;
				}
				if (state[SDL_SCANCODE_LEFT]) {
					gApp.mCamera.MoveLeft(speed);
				}
				if (state[SDL_SCANCODE_RIGHT]) {
					gApp.mCamera.MoveRight(speed);
				}
				if (state[SDL_SCANCODE_ESCAPE])
				{
					gApp.mQuit = true;
				}
			}

			// Clear up the screen
			{
				// Disable depth test and face culling.
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);

				// Initialize clear color
				// This is the background of the screen.
				glViewport(0, 0, gApp.mScreenWidth, gApp.mScreenHeight);
				glClearColor(1.f, 1.f, 0.1f, 1.f);

				// Clear the color and depth buffers.
				glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			}

			static float rotate = 0.01f;
			MeshRotate(&gMesh1, rotate, glm::vec3(0.0f, 0.1f, 0.0f));
			MeshRotate(&gMesh2, -rotate, glm::vec3(0.0f, 0.1f, 0.0f));

			DrawMesh(&gMesh1);
			DrawMesh(&gMesh2);

			//update the screen
			SDL_GL_SwapWindow(gApp.mGraphicsApplicationWindow);
		}
	}

	//clean up: call the cleanup function when our program terminates
	{
		SDL_DestroyWindow(gApp.mGraphicsApplicationWindow);
		gApp.mGraphicsApplicationWindow = nullptr;

		MeshDelete(&gMesh1);

		glDeleteProgram(gApp.mGraphicsPipelineShaderProgram);

		SDL_Quit();
	}
	return 0;
}
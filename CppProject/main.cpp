#include<vector>
#include<chrono>
#include<thread>
#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>


#include"shaderClass.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"


class TimeClass
{
private:
	double cpy;

public:
	float deltaTime();
	void frameCap(float frameRate, float frameEnd);
};


class Square
{
public:
	float x = 0.0f;
	float y = 0.0f;
	void update(float xPos, float yPos);
private:
};

float TimeClass::deltaTime()
{
	cpy = glfwGetTime();
	glfwSetTime(0.0f);
	return cpy;
}

void TimeClass::frameCap(float frameRate, float frameEnd) {
	

	if (frameEnd < 0.1f / frameRate) {
		bool sleep = true;
		while (sleep)
		{
			auto start = std::chrono::high_resolution_clock::now();

			if (frameEnd > 0.1f / frameRate)
				sleep = false;

			auto end = std::chrono::high_resolution_clock::now();

			frameEnd += std::chrono::duration<float>(end - start).count();
		}
	}

	

	//std::cout << end << "ms (" << "fps: " << 1.0f / end << ")" << std::endl;
}


GLfloat vertices[] =
{ //               COORDINATES                  /     COLORS           //
	 0.0f,  0.0f, 0.0f,                            0.2f, 0.2f,  0.2f, // Top left
	 0.05f,  0.0f, 0.0f,                            0.2f, 0.2f,  0.2f, // Top right
	 0.0f, -0.05f, 0.0f,                            0.2f, 0.2f,  0.2f, // Bottom left
	 0.05f, -0.05f, 0.0f,                            0.1f, 0.1f, 0.1f // Bottom Right
};


void Square::update(float xPos, float yPos)
{	
	vertices[0] = (GLfloat)xPos;
	vertices[1] = (GLfloat)yPos;
	vertices[6] = (GLfloat)0.05f + xPos;
	vertices[7] = (GLfloat)yPos;
	vertices[12] = (GLfloat)xPos;
	vertices[13] = (GLfloat)-0.05f + yPos;

	vertices[18] = (GLfloat)0.05f + xPos;
	vertices[19] = (GLfloat)-0.05f + yPos;

	/*
	int size = (sizeof(vertices[0] * 24) / sizeof(vertices[0])) + sizeof(GLfloat);
	GLfloat *b = (float*)malloc(size);
	if (b != NULL) {
		memcpy(b, vertices, size);
	}*/
}

float velX = 0.0f;
float velY = 0.0f;
float jumpPower = 2.0f;

void jump_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		velY += jumpPower;
}

float frameRate = 144.0f;
float localX = 0.0f;
float localY = 0.0f;



float gravity = 2.0f;
float driftFloor = 1.0f;
float drift = 0.3f;
float speed = 5.0f;

double deltaTime = 0.0f;

bool switched = false;

// Indices for vertices order
GLuint indices[] =
{
	0, 1, 2, // Lower left triangle
	1, 2, 3 // Lower right triangle
};






int main()
{
	// Comments are for self.
	glfwInit();


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Only modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	

	// Create a GLFWwindow object of 800 by 800 pixels, naming it "OpenGL"
	GLFWwindow* window = glfwCreateWindow(800, 800, "OpenGL", NULL, NULL);

	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Add window to context lol
	glfwMakeContextCurrent(window);

	//Load GLAD
	gladLoadGL();
	glViewport(0, 0, 800, 800);



	// Gens shaders.
	Shader shaderProgram("default.vert", "default.frag");
	TimeClass timeObj;
	Square squareObj;

	// Gen vao
	VAO VAO1;

	// Bind vao
	VAO1.Bind();

	// Generates VBO and links it to vertices
	VBO VBO1(vertices, sizeof(vertices));
	// Generates EBO and links it to indices
	EBO EBO1(indices, sizeof(indices));

	// Links VBO attributes such as coordinates and colors to VAO
	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 4 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 4 * sizeof(float), (void*)(3 * sizeof(float)));
	// Unbind all to prevent accidentally modifying them
	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

	// Gets ID of uniform called "scale"
	GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");


	float floor = -0.616f;
	float roof = 0.665f;
	float leftWall = -0.666f;
	float rightWall = 0.62f;


	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
		// frame counter
		auto start = std::chrono::high_resolution_clock::now();

		glfwSetKeyCallback(window, jump_callback);

		// Physics

		if (glfwGetKey(window, GLFW_KEY_A)) velX -= speed * deltaTime;

		if (glfwGetKey(window, GLFW_KEY_S)) velY -= jumpPower * deltaTime;

		if (glfwGetKey(window, GLFW_KEY_D)) velX += speed * deltaTime;

		if(localY >= floor) {
			if (localY < roof) {
				velY -= gravity * deltaTime;
			}
			else
			{
				localY = roof;
				velY = -velY * 0.6f;
			}
		}
		else
		{
			localY = floor;
			velY = -velY * 0.6f;
		}

		if (localX >= rightWall) {
			localX = rightWall;
			velX = -velX * 0.6f;
		}
		else
		{
			if (localX <= leftWall) {
				localX = leftWall;
				velX = -velX * 0.6f;
			}
		}

		if (velX > 0) {
			if (localY < floor + 0.005f) {
				velX -= driftFloor * deltaTime;
			}
			else
			{
				velX -= drift * deltaTime;
			}
		}
		else
		{
			if (localY < floor + 0.005f) {
				velX += driftFloor * deltaTime;
			}
			else
			{
				velX += drift * deltaTime;
			}
			
		}


		localX += velX * deltaTime;
		localY += velY * deltaTime;
		
		// Physics


		// Gets ID of uniform called "scale"
		//GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

		squareObj.update(localX, localY);

		VAO VAO1;
		VAO1.Bind();

		// Generates Vertex Buffer Object and links it to vertices
		VBO VBO1(vertices, sizeof(vertices));
		// Generates Element Buffer Object and links it to indices
		EBO EBO1(indices, sizeof(indices));

		// Links VBO attributes such as coordinates and colors to VAO
		VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
		VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		// Unbind all to prevent accidentally modifying them
		VAO1.Unbind();
		VBO1.Unbind();
		EBO1.Unbind();

		// Specify the color of the background
		// glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
		// Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT);
		// Tell OpenGL which Shader Program we want to use
		shaderProgram.Activate();
		// Assigns a value to the uniform; NOTE: Must always be done after activating the Shader Program
		glUniform1f(uniID, 0.5f);
		// Bind the VAO so OpenGL knows to use it
		VAO1.Bind();
		// Draw primitives, number of indices, datatype of indices, index of indices
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();

		
		deltaTime = timeObj.deltaTime();

		auto end = std::chrono::high_resolution_clock::now();
		float endf = std::chrono::duration<float>(end - start).count();

		timeObj.frameCap(frameRate, endf);
	}



	// Delete all the objects we've created
	VAO1.Delete();
	VBO1.Delete();
	EBO1.Delete();
	shaderProgram.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}
/*
 * File: mycraft.cpp
 * Project: CCSEP 2019 S2 Assignment
 * Created Date: 23/10/19 - 16:09:14
 * Author: Kevin Le - 19472960
 * Contact: kevin.le2@student.curtin.edu.au
 * -----
 * Purpose: Main game

 Code segments and implementation based on Computer Graphics 2019 S2 
 Practical 1 – OpenGL Sample 2. – Curtin University.

 Code segments, implementation, header files, image loading functions, 
 and camera header based on and sourced from LearnOpenGL. https://learnopengl.com/ 
 by Joey de Vries. Accessed 18/10/19.
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h> //Sourced from LearnOpenGL.
#include <learnopengl/camera.h> //Sourced from LearnOpenGL.

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height); //For adapting to window size change
void mouseListener(GLFWwindow* window, double xpos, double ypos); //Retrieve mouse input for camera 
unsigned int loadTexture(const char* path); //Function used to load textures. 
void processInput(GLFWwindow* window); //Process keyboard input
void delayCountdown(); //Used for input timeouts preventing repeat input. Based on Sample 2 CCSEP Practical 1
void restart(); //Restart game 

//Check used to get player position
float calculateAngle(); //Function used to calculate angle from player (camera) to sheep. 
bool checkDead(); //Function to check if player dead. 
bool checkEscaped(); //Function to check if player has made it to exit. 

//Window Size
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

//Load Camera position and settings. 
glm::vec3 initPlayerPos = glm::vec3(-2.0f, 0.8f, 13.0f);

Camera camera(initPlayerPos);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//Timing to control camera speed
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Sheep movement speed 
float SHEEP_SPEED = 0.005f;

//Environmental sizes 
const float boxWidth = 15.0f;
const float boxLength = 30.0f;
const float wallHeight = 1.0f;

const float treeHeight = 2.0f;
const float torchHeight = 0.3f;
const float tableHeight = 0.6f;

const float svenHeight = 0.27f;
const float sheepHeight = 0.4f;

const float mineSize = 0.6f;

//Object Locations 
float svenX = 4.0f;
float svenY = 0.0f;
float svenZ = -12.0f;

float sheepX = -6.0f;
float sheepY = 0.0f;
float sheepZ = -4.0f;

float torchX = 5.0f;
float torchY = torchHeight / 2;
float torchZ = 13.0f;

float lampY = tableHeight + torchHeight;

//Lighting Constants 
float torchIntensity = 1.0f;
int attenIdx = 7; 

//Values retrieved from LearnOpenGL/Lighting/Light Casters. https://learnopengl.com/Lighting/Light-casters
const float linValues[12] = {0.0014f, 0.007f, 0.014f, 0.022f, 0.027f, 0.045f, 0.07f, 0.09f, 0.14f, 0.22f, 0.35f, 0.7f};
const float quadValues[12] = {0.000007f, 0.0002f, 0.0007f, 0.0019f, 0.0028f, 0.0075f, 0.017f, 0.032f, 0.07f, 0.20f, 0.44f, 1.8f};

//Game States
bool TORCH_PICKED_UP = false;
bool SVEN_PICKED_UP = false;
bool LIGHT_ON = false;

bool DEAD = false;
bool ESCAPED = false; 

bool ORTHO = false;
int BUTTON_DELAY = 0; 
int AIR_TIME = 0;
bool JUMPED = false;

//Landmine locations
glm::vec3 mineTrans[] = {
	glm::vec3(0.0f, 0.0f, -12.0f),
	glm::vec3(-3.0f, 0.0f, -8.0f),
	glm::vec3(-5.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 4.0f),
	glm::vec3(4.0f, 0.0f, 3.0f),
	glm::vec3(-5.0f, 0.0f, 10.0f),
};

int main()
{
	//Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//Create GLFW Window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "19472960-CG-Assignment", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouseListener);

	//Capture and hide mouse 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Check if GLAD initialized 
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Set Z depth detection
	glEnable(GL_DEPTH_TEST);

	//Compile shaders for box and light
	Shader boxShader("box.vs", "box.fs");
	Shader lightShader("light.vs", "light.fs");

	//Vertex data
	float box[] = {
		//Positions          //Normals           //Texture Coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	//Setup cube VBO/ VAO
	unsigned int cubeVBO, cubeVAO;

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//Setup light VBO/ VAO
	unsigned int lightVAO;

	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Load textures
	unsigned int grass = loadTexture("textures/grass.jpg");
	unsigned int stone = loadTexture("textures/stone.png");
	unsigned int wood = loadTexture("textures/wood.jpg");
	unsigned int wood2 = loadTexture("textures/wood.png");
	unsigned int sky = loadTexture("textures/stars.jpg");
	unsigned int skyday = loadTexture("textures/sky.jpg");
	unsigned int leaves = loadTexture("textures/leaf.jpg");
	unsigned int red = loadTexture("textures/red.jpg");
	unsigned int cactus = loadTexture("textures/cactus.png");
	unsigned int ceramic = loadTexture("textures/ceramic.png");
	unsigned int cobble = loadTexture("textures/cobble1.png");
	unsigned int door = loadTexture("textures/door.png");
	unsigned int torch = loadTexture("textures/torch.png");

	unsigned int fur = loadTexture("textures/fur.jpg");
	unsigned int wool = loadTexture("textures/wool.jpg");

	unsigned int wolf = loadTexture("textures/wolf.png");
	unsigned int sheep = loadTexture("textures/sheep.png");
	unsigned int sheepspec = loadTexture("textures/sheepspec.png");

	unsigned int nospec = loadTexture("textures/nospec.jpg");
	unsigned int midspec = loadTexture("textures/midspec.jpg");
	unsigned int spec = loadTexture("textures/spec.jpg");

	unsigned int gameOver = loadTexture("textures/gameover.jpg");
	unsigned int gameWon = loadTexture("textures/gamewon.png");

	//Initialize box
	boxShader.use();
	boxShader.setInt("material.diffuse", 0);
	boxShader.setInt("material.specular", 1);

	//Model matrices
	glm::mat4 boxModel, lampModel;

	//Tree translations 
	glm::vec3 treeTrans[] = {
		glm::vec3(-5.0f, treeHeight / 2, 13.0f),
		glm::vec3(-5.0f, treeHeight / 2, -11.0f),
		glm::vec3(-4.0f, treeHeight / 2, -4.0f),
		glm::vec3(-4.0f, treeHeight / 2, 7.0f),
		glm::vec3(-2.0f, treeHeight / 2, 1.0f),
		glm::vec3(1.0f, treeHeight / 2, -8.0f),
		glm::vec3(1.0f, treeHeight / 2, -2.0f),
		glm::vec3(1.0f, treeHeight / 2, 9.0f),
		glm::vec3(5.0f, treeHeight / 2, -10.0f),
		glm::vec3(5.0f, treeHeight / 2, -3.0f),
		glm::vec3(3.0f, treeHeight / 2, 5.0f),
	};

	//Leaf layer 1 translations
	glm::vec3 leaf1[] = {
		glm::vec3(-5.0f, treeHeight + 0.4f, 13.0f),
		glm::vec3(-5.0f, treeHeight + 0.4f, -11.0f),
		glm::vec3(-4.0f, treeHeight + 0.4f, -4.0f),
		glm::vec3(-4.0f, treeHeight + 0.4f, 7.0f),
		glm::vec3(-2.0f, treeHeight + 0.4f, 1.0f),
		glm::vec3(1.0f, treeHeight + 0.4f, -8.0f),
		glm::vec3(1.0f, treeHeight + 0.4f, -2.0f),
		glm::vec3(1.0f, treeHeight + 0.4f, 9.0f),
		glm::vec3(5.0f, treeHeight + 0.4f, -10.0f),
		glm::vec3(5.0f, treeHeight + 0.4f, -3.0f),
		glm::vec3(3.0f, treeHeight + 0.4f, 5.0f),
	};

	//Leaf layer 2 translations
	glm::vec3 leaf2[] = {
		glm::vec3(-5.0f, treeHeight + 1.2f, 13.0f),
		glm::vec3(-5.0f, treeHeight + 1.2f, -11.0f),
		glm::vec3(-4.0f, treeHeight + 1.2f, -4.0f),
		glm::vec3(-4.0f, treeHeight + 1.2f, 7.0f),
		glm::vec3(-2.0f, treeHeight + 1.2f, 1.0f),
		glm::vec3(1.0f, treeHeight + 1.2f, -8.0f),
		glm::vec3(1.0f, treeHeight + 1.2f, -2.0f),
		glm::vec3(1.0f, treeHeight + 1.2f, 9.0f),
		glm::vec3(5.0f, treeHeight + 1.2f, -10.0f),
		glm::vec3(5.0f, treeHeight + 1.2f, -3.0f),
		glm::vec3(3.0f, treeHeight + 1.2f, 5.0f),
	};

	//Table leg translations 
	glm::vec3 tableLegTrans[] = {
		glm::vec3(4.0f, tableHeight / 2, 12.0f),
		glm::vec3(6.0f, tableHeight / 2, 12.0f),
		glm::vec3(4.0f, tableHeight / 2, 13.0f),
		glm::vec3(6.0f, tableHeight / 2, 13.0f),
	};
	
	//Wall translations
	glm::vec3 wallTrans[] = {
		glm::vec3(0.0f, wallHeight / 2, 15.25f),
		glm::vec3(0.0f, wallHeight / 2, -15.25f),
		glm::vec3(7.75f, wallHeight / 2, 0.0f),
		glm::vec3(-7.75f, wallHeight / 2, 0.0f),
	};	
		
	//Sheep translations
	glm::vec3 sheepTrans[] = 
	{
		glm::vec3(-0.25f, sheepHeight / 2, -0.1f), //Leg1
		glm::vec3(-0.25f, sheepHeight / 2, 0.1f), //Leg2		
		glm::vec3(0.25f, sheepHeight / 2, -0.1f), //Leg3
		glm::vec3(0.25f, sheepHeight / 2, 0.1f), //Leg4
		glm::vec3(0.0f, sheepHeight, 0.0f), //Body
		glm::vec3(-0.24f, sheepHeight + 0.1f, 0.0f), //Collar 
	};

	//Wall scales 
	glm::vec3 wallScale[] = {
		glm::vec3(boxWidth, wallHeight, 0.5f),
		glm::vec3(boxWidth, wallHeight, 0.5f),
		glm::vec3(0.5f, wallHeight, boxLength),
		glm::vec3(0.5f, wallHeight, boxLength),
	};
	
	//Sheep scales
	glm::vec3 sheepScale[] = {
		glm::vec3(0.17f, sheepHeight, 0.17f), //Leg1
		glm::vec3(0.17f, sheepHeight, 0.17f), //Leg2
		glm::vec3(0.17f, sheepHeight, 0.17f), //Leg3
		glm::vec3(0.17f, sheepHeight, 0.17f), //Leg4
		glm::vec3(0.7f, 0.4f, 0.4f), //Body
		glm::vec3(0.3f, 0.3f, 0.3f), //Collar
	};

	//Sven scales 
	glm::vec3 svenScale[] = {
		glm::vec3(0.07f, svenHeight, 0.07f), //Leg1
		glm::vec3(0.07f, svenHeight, 0.07f), //Leg2
		glm::vec3(0.07f, svenHeight, 0.07f), //Leg3
		glm::vec3(0.07f, svenHeight, 0.07f), //Leg4
		glm::vec3(0.5f, 0.23f, 0.2f), //Body
		glm::vec3(0.4f, 0.05f, 0.05f), //Tail
		glm::vec3(0.2f, 0.26f, 0.23f), //Collar
	};

	//Render loop 
	while (!glfwWindowShouldClose(window))
	{
		glm::mat4 projection; //To be modified by selected projection mode. 

		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		delayCountdown(); //Decrement countdown for airtime and button timeout

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (ORTHO) //If player has selected orthographic projection mode. 
		{
			projection = glm::ortho(-10.0f, 10.0f, 0.0f, 10.0f, -30.0f, 300.0f);
		}
		else //Use perspective projection
		{
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 300.0f); 
		}

		//Lighting values 
		boxShader.use();
		boxShader.setMat4("projection", projection); //Set projection 
		boxShader.setVec3("light.position", glm::vec3(torchX, torchY, torchZ)); //Set light position 
		boxShader.setVec3("viewPos", camera.Position); //Set current camera position 

		boxShader.setVec3("light.diffuse", 0.4f, 0.4f, 0.4f); //Set diffuse values. 
		boxShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f); //Set specular values.
		boxShader.setFloat("material.shininess", 64.0f); //Set material shininess values. 

		//Attenuation values 
		boxShader.setFloat("light.constant", 1.0f); //Lighting constant 
		boxShader.setFloat("light.linear", linValues[attenIdx]); //Chosen linear value 
		boxShader.setFloat("light.quadratic", quadValues[attenIdx]); //Chosen quadratic value 

		glm::mat4 view; //View matrix, can be 2 modes. Regular camera or death camera (sideways) 

		if (DEAD) //If player is dead 
		{
			//Make death camera for position died. 
			Camera deathCam(glm::vec3(camera.Position.x, 0.2f, camera.Position.z), glm::vec3(0.0f, 0.0f, 1.0f));

			//Face camera the position the player died. 
			if (camera.Front.z < 0.0)
			{
				deathCam.Front.z = -1.0f; 
			}
			else
			{
				deathCam.Front.z = 1.0f;
			}

			//Set view to the deathcam. 
			view = deathCam.GetViewMatrix(); 

			//Game Over Message Box
			boxShader.setVec3("light.ambient", 0.8f, 0.8f, 0.8f);

			glBindVertexArray(cubeVAO);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gameOver); 	
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, nospec); 

			boxModel = glm::mat4(1.0f); 
			boxModel = glm::translate(boxModel, glm::vec3(deathCam.Position.x + deathCam.Front.x, deathCam.Position.y, deathCam.Position.z + deathCam.Front.z)); 
			boxModel = glm::scale(boxModel, glm::vec3(0.5f, 0.5f, 0.5f));

			boxShader.setMat4("model", boxModel);

			glDrawArrays(GL_TRIANGLES, 0, 36);

		}
		else //Player not dead 
		{
			//Use standard view.
			view = camera.GetViewMatrix();
		}

		//Set view 
		boxShader.setMat4("view", view);

		//Used to control player airtime and time when they should be brought back down to ground 
		if(AIR_TIME == 0 && JUMPED)
		{
			camera.JumpDown();
			JUMPED = false;
		}

		//Change game to day when user opts or when user wins. 
		if (LIGHT_ON || ESCAPED)
		{
			boxShader.setVec3("light.ambient", 0.8f, 0.8f, 0.8f);
		}
		else
		{
			boxShader.setVec3("light.ambient", 0.08f, 0.08f, 0.08f);
		}

		//If user picked up torch 
		if (TORCH_PICKED_UP) //Update torch coords to travel with player. 
		{
			torchX = camera.Position.x + (camera.Front.x + 0.4f);
			torchY = 0.0f;
			torchZ = camera.Position.z + camera.Front.z;

			lampY = torchY + 2 * torchHeight + 0.15f;
		}

		//If user picked up Sven - activate hostiles
		if (SVEN_PICKED_UP) //Update sven coords to travel with player. 
		{
			svenX = camera.Position.x + (camera.Front.x - 0.4f);
			svenY = 0.3f;
			svenZ = camera.Position.z + camera.Front.z;

			if (!DEAD) //If the player isn't dead, make the sheep move towards the player 
			{
				if (camera.Position.x < sheepX) 
				{
					sheepX -= SHEEP_SPEED;
				}
				else
				{
					sheepX += SHEEP_SPEED;
				}

				sheepY = 0.0f;

				if (camera.Position.z < sheepZ)
				{
					sheepZ -= SHEEP_SPEED;
				}
				else
				{
					sheepZ += SHEEP_SPEED;
				}
			}

			//Show the escape door/ exit. 
			glBindVertexArray(cubeVAO);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, door); //Apply door texture				
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, nospec); //Apply specular map for door (no specular)	

			boxModel = glm::mat4(1.0f); 
			boxModel = glm::translate(boxModel, glm::vec3(-2.0f, 1.0f, 14.9f)); 
			boxModel = glm::scale(boxModel, glm::vec3(1.0f, 2.0f, 0.2f));

			boxShader.setMat4("model", boxModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			if (!ESCAPED) //If player hasn't escaped yet 
			{
				if (checkDead()) //Check if they have died (sheep or landmine)
				{
					DEAD = true; //Set dead to true 
					svenY = 0.0f; //Drop sven. 
				}
			}

			if (checkEscaped()) //If player has escaped 
			{
				ESCAPED = true; //Set escaped to true. 
			}
		}

		//Sven translations. (In render loop as Svens position is modified)
		glm::vec3 svenTrans[] = 
		{
			glm::vec3(svenX - 0.15f, svenY + svenHeight / 2, svenZ - 0.07f), //Leg1
			glm::vec3(svenX - 0.15f, svenY + svenHeight / 2, svenZ + 0.06f), //Leg2		
			glm::vec3(svenX + 0.15f, svenY + svenHeight / 2, svenZ - 0.07f), //Leg3
			glm::vec3(svenX + 0.15f, svenY + svenHeight / 2, svenZ + 0.06f), //Leg4
			glm::vec3(svenX, svenY + svenHeight, svenZ), //Body
			glm::vec3(svenX + 0.25f, svenY + svenHeight, svenZ), //Tail 
			glm::vec3(svenX - 0.15f, svenY + svenHeight, svenZ), //Collar 
		};
		
		if (ESCAPED) //If they have escaped 
		{
			//Show win message 
			glBindVertexArray(cubeVAO);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gameWon); 			
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, nospec); 	

			boxModel = glm::mat4(1.0f); 
			boxModel = glm::translate(boxModel, glm::vec3(-2.0f, 1.0f, 12.0f)); 
			boxModel = glm::scale(boxModel, glm::vec3(2.0f, 2.0f, 0.1f));

			boxShader.setMat4("model", boxModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Sky
		glBindVertexArray(cubeVAO);

		glActiveTexture(GL_TEXTURE0);
		if (LIGHT_ON || ESCAPED) //If player has opted to turn on lights or they have escaped, make the sky day. 
		{
			glBindTexture(GL_TEXTURE_2D, skyday); 
		}
		else //Use night sky 
		{
			glBindTexture(GL_TEXTURE_2D, sky);
		}

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, nospec); 	

		boxModel = glm::mat4(1.0f); 
		boxModel = glm::scale(boxModel, glm::vec3(200.0f, 200.0f, 200.0f));

		boxShader.setMat4("model", boxModel);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Grass
		glBindVertexArray(cubeVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grass); 	
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, nospec); 

		boxModel = glm::mat4(1.0f); 
		boxModel = glm::translate(boxModel, glm::vec3(0.0f, 0.0f, 0.0f)); 
		boxModel = glm::scale(boxModel, glm::vec3(boxWidth, 0.0f, boxLength));

		boxShader.setMat4("model", boxModel);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Trees
		for (int ii = 0; ii < sizeof(treeTrans) / sizeof(treeTrans[0]); ii++)
		{
			//Trunk
			glBindVertexArray(cubeVAO);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, wood); 
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, nospec); 

			boxModel = glm::mat4(1.0f);
			boxModel = glm::translate(boxModel, treeTrans[ii]);
			boxModel = glm::scale(boxModel, glm::vec3(0.8f, treeHeight, 0.8f));

			boxShader.setMat4("model", boxModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			//Leaves (2 layers) 
			glBindVertexArray(cubeVAO);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, leaves); 
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, midspec); 

			boxModel = glm::mat4(1.0f);
			boxModel = glm::translate(boxModel, leaf1[ii]);
			boxModel = glm::scale(boxModel, glm::vec3(3.0f, 0.8f, 3.0f));

			boxShader.setMat4("model", boxModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			glBindVertexArray(cubeVAO);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, leaves); 
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, midspec); 

			boxModel = glm::mat4(1.0f);
			boxModel = glm::translate(boxModel, leaf2[ii]);
			boxModel = glm::scale(boxModel, glm::vec3(1.5f, 0.8f, 1.5f));

			boxShader.setMat4("model", boxModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Landmines
		if (!ESCAPED)
		{
			for (int kk = 0; kk < sizeof(mineTrans) / sizeof(mineTrans[0]); kk++)
			{
				//Stone plate 
				glBindVertexArray(cubeVAO);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, stone);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, nospec);

				boxModel = glm::mat4(1.0f);
				boxModel = glm::translate(boxModel, mineTrans[kk]);
				boxModel = glm::scale(boxModel, glm::vec3(mineSize, 0.1f, mineSize));

				boxShader.setMat4("model", boxModel);
				glDrawArrays(GL_TRIANGLES, 0, 36);

				glBindVertexArray(cubeVAO);

				//Red button 
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, red);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, spec);

				boxModel = glm::mat4(1.0f);
				boxModel = glm::translate(boxModel, mineTrans[kk]);
				boxModel = glm::scale(boxModel, glm::vec3(mineSize / 3, 0.2f, mineSize / 2));

				boxShader.setMat4("model", boxModel);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}
	
		//Table Legs
		for (int jj = 0; jj < sizeof(tableLegTrans) / sizeof(tableLegTrans[0]); jj++)
		{
			glBindVertexArray(cubeVAO);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, wood2); 		
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, midspec); 

			boxModel = glm::mat4(1.0f);
			boxModel = glm::translate(boxModel, tableLegTrans[jj]);
			boxModel = glm::scale(boxModel, glm::vec3(0.2f, tableHeight, 0.2f));

			boxShader.setMat4("model", boxModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Walls
		for (int pp = 0; pp < sizeof(wallTrans) / sizeof(wallTrans[0]); pp++)
		{
			glBindVertexArray(cubeVAO);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, cobble); 		
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, nospec); 

			boxModel = glm::mat4(1.0f);
			boxModel = glm::translate(boxModel, wallTrans[pp]);
			boxModel = glm::scale(boxModel, wallScale[pp]);

			boxShader.setMat4("model", boxModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Table Top 
		glBindVertexArray(cubeVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wood2); 		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, midspec); 

		boxModel = glm::mat4(1.0f);
		boxModel = glm::translate(boxModel, glm::vec3(5.0f, tableHeight, 12.5f));
		boxModel = glm::scale(boxModel, glm::vec3(2.5f, 0.1f, 1.5f));

		boxShader.setMat4("model", boxModel);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Pot
		glBindVertexArray(cubeVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ceramic); 
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, spec); 

		boxModel = glm::mat4(1.0f);
		boxModel = glm::translate(boxModel, glm::vec3(4.2f, tableHeight + 0.1f, 12.3f));
		boxModel = glm::rotate(boxModel, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		boxModel = glm::scale(boxModel, glm::vec3(0.18f, 0.18f, 0.18f));

		boxShader.setMat4("model", boxModel);
		glDrawArrays(GL_TRIANGLES, 0, 36);		
		
		//Cactus
		glBindVertexArray(cubeVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cactus); 
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, nospec); 

		boxModel = glm::mat4(1.0f);
		boxModel = glm::translate(boxModel, glm::vec3(4.2f, tableHeight + 0.3f, 12.3f));
		boxModel = glm::rotate(boxModel, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		boxModel = glm::scale(boxModel, glm::vec3(0.1f, 0.3f, 0.1f));

		boxShader.setMat4("model", boxModel);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Sven Body
		for (int jj = 0; jj < sizeof(svenTrans) / sizeof(svenTrans[0]); jj++)
		{
			glBindVertexArray(cubeVAO);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fur); 		
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, nospec); 

			boxModel = glm::mat4(1.0f);
			boxModel = glm::translate(boxModel, svenTrans[jj]); //Translate sven to correct locations 
			boxModel = glm::scale(boxModel, svenScale[jj]); //Scale sven to correct size 

			boxShader.setMat4("model", boxModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Sven Head 
		glBindVertexArray(cubeVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wolf); 		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, nospec); 

		boxModel = glm::mat4(1.0f);
		
		boxModel = glm::translate(boxModel, glm::vec3(svenX - 0.16f, svenY + svenHeight, svenZ));
		boxModel = glm::scale(boxModel, glm::vec3(0.2f, 0.2f, 0.2f));

		boxShader.setMat4("model", boxModel);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Get angle of sheep from player
		float angle = calculateAngle();

		if (!ESCAPED && !DEAD) //If the player hasn't escaped or isn't dead yet. Show the sheep. 
		{
			//Sheep Body
			for (int jj = 0; jj < sizeof(sheepTrans) / sizeof(sheepTrans[0]); jj++)
			{
				glBindVertexArray(cubeVAO);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, wool); 		
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, nospec); 

				boxModel = glm::mat4(1.0f);
				boxModel = glm::translate(boxModel, glm::vec3(sheepX, sheepY, sheepZ)); //Move sheep parts to desired location 
				boxModel = glm::rotate(boxModel, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)); //Rotate to face player 
				boxModel = glm::translate(boxModel, sheepTrans[jj]); //Translate sheep parts to correct positions 
				boxModel = glm::scale(boxModel, sheepScale[jj]); //Resize sheep parts to correct positions 

				boxShader.setMat4("model", boxModel);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}

			//Sheep Head 
			glBindVertexArray(cubeVAO);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, sheep); 		
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, sheepspec); 

			boxModel = glm::mat4(1.0f);
			boxModel = glm::translate(boxModel, glm::vec3(sheepX, sheepY, sheepZ));
			boxModel = glm::rotate(boxModel, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
			boxModel = glm::translate(boxModel, glm::vec3(-0.26f, sheepHeight + 0.1f, 0.0f));
			boxModel = glm::scale(boxModel, glm::vec3(0.28f, 0.28f, 0.28f));

			boxShader.setMat4("model", boxModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Torch
		glBindVertexArray(cubeVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, torch);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, midspec); 

		boxModel = glm::mat4(1.0f);
		boxModel = glm::translate(boxModel, glm::vec3(torchX, tableHeight + torchY, torchZ));
		boxModel = glm::scale(boxModel, glm::vec3(0.1f, torchHeight, 0.1f));

		boxShader.setMat4("model", boxModel);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightShader.use();
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view);
		lightShader.setFloat("intensity", torchIntensity);

		lampModel = glm::mat4(1.0f);
		lampModel = glm::translate(lampModel, glm::vec3(torchX, lampY + 0.05f, torchZ));
		lampModel = glm::scale(lampModel, glm::vec3(0.1f)); //Reduce cube size

		lightShader.setMat4("model", lampModel);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//De-Allocate Resources 
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &cubeVBO);

	//Finish and clear resources 
	glfwTerminate();
	return 0;
}

/**
 * Import: Reference towindow 
 * 
 * Purpose: Process user input and performs actions according to each key. 
 */
void processInput(GLFWwindow* window)
{
	//Exit application (ESC) 
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//Movement (WASD + Space) Actions if player hasn't died. 
	if (!DEAD)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) //Jump
		{
			if (AIR_TIME == 0) //If on ground 
			{
				BUTTON_DELAY = 40;
				AIR_TIME = 40;

				camera.JumpUp();

				JUMPED = true;
			}
		}
	}
	
	//Restart game (R) 
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		restart();

	//If button timeout has passed 
	if (BUTTON_DELAY == 0)
	{
		//Pick up torch (F)
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		{
			BUTTON_DELAY = 30;

			glm::vec3 torchPos = glm::vec3(torchX, torchY, torchZ);
			if (glm::length(camera.Position - torchPos) < 1.5f && !TORCH_PICKED_UP)
			{
				TORCH_PICKED_UP = true;
			}
			else if (TORCH_PICKED_UP)
			{
				TORCH_PICKED_UP = false;

				torchY = -0.4f; //Put on ground
				lampY = torchY + 2 * torchHeight + 0.15f;
			}
		}		
		
		//Pick up Sven (E) 
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			BUTTON_DELAY = 30;

			glm::vec3 svenPos = glm::vec3(svenX, svenY, svenZ);
			if (glm::length(camera.Position - svenPos) < 1.5f && !SVEN_PICKED_UP)
			{
				SVEN_PICKED_UP = true;
			}
			else if (SVEN_PICKED_UP)
			{
				SVEN_PICKED_UP = false;
				svenY = 0.0f; 
			}
		}

		//Reduce Torch Bright Radius (K)
		if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) 
		{
			BUTTON_DELAY = 30;

			if (!(attenIdx + 1 > 11))
			{
				attenIdx += 1; 
			}
		}

		//Increase Torch Bright Radius (L)
		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) 
		{
			BUTTON_DELAY = 30;

			if (!(attenIdx - 1 < 0))
			{
				attenIdx -= 1;
			}
		}

		//Make daytime and bright (O)
		if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) 
		{
			BUTTON_DELAY = 30;

			if (LIGHT_ON)
			{
				LIGHT_ON = false;
			}
			else
			{
				LIGHT_ON = true;
			}
		}
		
		//Switch projection mode orthographic/ projection. (P)
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) 
		{
			BUTTON_DELAY = 30;

			if (ORTHO)
			{
				ORTHO = false; 
			}
			else
			{
				ORTHO = true;
			}
		}
	}
}

/**
 * Exports: Boolean representing if player has died
 * 
 * Purpose: Determine if player has died based on their location to hostiles. 
 */
bool checkDead()
{
	//If player is touched by the sheep. 
	if (glm::length(camera.Position - glm::vec3(sheepX, sheepY, sheepZ)) < 1.0f)
	{
		return true; 
	}

	//If player has stood on a mine. 
	for (int nn = 0; nn < sizeof(mineTrans) / sizeof(mineTrans[0]); nn++)
	{
		if (glm::length(camera.Position - mineTrans[nn]) < 1.0f)
		{
			return true; 
		}
	}

	//if none above true, they haven't died yet. 
	return false; 
}

/**
 * Exports: Boolean representing if player has escaped 
 * 
 * Purpose: Determine if player has made it to the door. 
 */
bool checkEscaped()
{
	//If player touches door 
	if (glm::length(camera.Position - glm::vec3(-2.0f, 1.0f, 14.9f)) < 1.0f)
		return true; 

	//If they havent touched door, not escaped. 
	return false;
}

/**
 * Purpose: Performs countdown for time user spends in air when jumping or
 * timeout to prevent repeated button press. 

   References: Implementation based on Computer Graphics 2019 S2 Practical 1 – OpenGL Sample 2. – Curtin University.
 */
void delayCountdown()
{
	if (BUTTON_DELAY > 0) BUTTON_DELAY -= 1;
	if (AIR_TIME > 0) AIR_TIME -= 1;
}

/**
 * Exports: Float representing angle of player from sheep in degrees 
 * 
 * Purpose: Calculate angle of player from sheep in order to make sheep face player. 
 */
float calculateAngle()
{
	//Get difference of X and Z of player from Sheep 
	float xDiff = camera.Position.x - sheepX; 
	float zDiff = camera.Position.z - sheepZ; 
	
	float rad = atan(xDiff / zDiff); //Use inverse tan to calculate angle  
	float angle = glm::degrees(atan2(xDiff, zDiff)); //Convert to degrees 

	if (angle < 0.0f)
	{
		angle += 360.0f; //Convert arc tan to 0-360 deg 
	}

	angle += 90.0f; //Face the player 

	return angle; 
}

/**
 * Purpose: Restarts game by setting all game states and data to initial.
 */
void restart()
{
	TORCH_PICKED_UP = false;
	SVEN_PICKED_UP = false;
	LIGHT_ON = false;
	DEAD = false;
	ESCAPED = false; 
	ORTHO = false;
	BUTTON_DELAY = 0; 
	AIR_TIME = 0;
	JUMPED = false;

	//Original Positions 
	svenX = 4.0f;
	svenY = 0.0f;
	svenZ = -12.0f;

	sheepX = -6.0f;
	sheepY = 0.0f;
	sheepZ = -4.0f;

	torchX = 5.0f;
	torchY = torchHeight / 2;
	torchZ = 13.0f;

	lampY = tableHeight + torchHeight;

	camera.Position = initPlayerPos; 
}

/**
 * Purpose: Adapt to window resize. 
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

/**
 * Purpose: Change camera position according to mouse 
 */
void mouseListener(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos;

	lastX = (float)xpos;
	lastY = (float)ypos;

	if (!DEAD)
	{
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}

/**
 * Import: File path 
 * Export: Integer
 * 
 * Purpose: Load texture from file path
 * References: Sourced from LearnOpenGL 
 */
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format{};
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include <string>
#include <iostream>


int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048*8;
const unsigned int SHADOW_HEIGHT = 2048*8;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
	glm::vec3(0.0f, 0.1f, 5.5f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));

gps::Camera myCamera2(
	glm::vec3(0.0f, 0.1f, 5.5f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.02f;
float rotateSpeed = 5.0f;

bool firstMouse = true;
static float lastX = 0.0f, lastY = 0.0f;


bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;
GLfloat signAngle = 0.0f;
GLfloat acceleration = 0.0f;
bool isSwing = false;
bool isAddAngle = true;
bool isFog = true;
bool isPointLight = true;
bool isSandStorm = false;
bool isAutoRotateRight = false;
bool isAutoRotateLeft = false;
bool isAutoMoveForward = false;
bool isAnimationCamera = false;

float rotateSum = 0.0f;
float rotateBound = 900.0f;

float animationSum = 0.0f;
float animationBound = 3.0f;

float animationSum2 = 0.0f;
float animationBound2 = 8.0f;


gps::Model3D nanosuit;
gps::Model3D ground;
gps::Model3D well;
gps::Model3D sign;
gps::Model3D signFrame;
gps::Model3D water;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D particleModel;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader waterShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

bool showDepthMap;

struct Particle {
	glm::vec3 Position;
	/*float angle;
	glm::vec3 Rotate;*/
};

unsigned int nr_particles = 10000;
Particle sandParticles[10000];
float boundLeft = -10.0f;
float boundRight = 10.0f;
float boundTop = 2.0f;
float boundBottom =-1.0f;
float boundFront = 2.0f;
float boundBack = -6.0f;
float particleSpeed = 0.1f;

void initParticles() {
	for (int i = 0; i < nr_particles; i++) {
		float x = (rand() % (int)(boundRight - boundLeft) + boundLeft) + (float)rand()/RAND_MAX;
		float y = (rand() % (int)(boundTop - boundBottom) + boundBottom)+ (float)rand() / RAND_MAX;
		float z = (rand() % (int)(boundFront - boundBack) + boundBack)+ (float)rand() / RAND_MAX;
		/*sandParticles[i].angle = rand() % 180;
		switch (i % 3) {
		case 0:
			sandParticles[i].Rotate = glm::vec3(1.0f, 0.0f, 0.0f);
			break;
		case 1:
			sandParticles[i].Rotate = glm::vec3(0.0f, 1.0f, 0.0f);
			break;
		case 2:
			sandParticles[i].Rotate = glm::vec3(0.0f, 0.0f, 1.0f);
			break;
		}*/
		sandParticles[i].Position = glm::vec3(x,y,z);
	}
}

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	

	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		isSwing = !isSwing;

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
		isFog = !isFog;

	if (key == GLFW_KEY_C && action == GLFW_PRESS)
		isPointLight = !isPointLight;

	if (key == GLFW_KEY_V && action == GLFW_PRESS)
		isSandStorm = !isSandStorm;

	if (key == GLFW_KEY_1 && action == GLFW_PRESS && !isAnimationCamera) {
		isAutoRotateRight = !isAutoRotateRight;
		isAutoRotateLeft = false;
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS && !isAnimationCamera) {
		isAutoRotateLeft = !isAutoRotateLeft;
		isAutoRotateRight = false;
	}

	if (key == GLFW_KEY_3 && action == GLFW_PRESS && !isAnimationCamera)
		isAutoMoveForward = !isAutoMoveForward;

	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		myCamera = myCamera2;
		myCamera.rotate(-900.0f, 0.0f);
		isAnimationCamera = true;
		isAutoRotateLeft = false;
		isAutoRotateRight = false;
		isAutoMoveForward = true;

		rotateSum = 0.0f;
		rotateBound = 900.0f;

		animationSum = 0.0f;
		animationBound = 3.0f;

		animationSum2 = 0.0f;
		animationBound2 = 8.0f;

	}

	if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
		isAnimationCamera = false;
		isAutoRotateLeft = false;
		isAutoRotateRight = false;
		isAutoMoveForward = false;

		rotateSum = 0.0f;
		rotateBound = 900.0f;

		animationSum = 0.0f;
		animationBound = 3.0f;

		animationSum2 = 0.0f;
		animationBound2 = 8.0f;

	}


	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (!isAutoMoveForward && !isAutoRotateLeft && !isAutoRotateRight) {
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}
		auto xoffset = xpos - lastX;
		lastX = xpos;

		auto yoffset = lastY - ypos;
		lastY = ypos;

		myCamera.rotate(xoffset, yoffset);
	}

}

float zoom = 45.0f;
gps::MOVE_DIRECTION mouseScrollDirection;
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	zoom -= (float)yoffset; // amount scrolled vertically

	if (zoom < 1.0f)
		zoom = 1.0f;
	if (zoom > 45.0f)
		zoom = 45.0f;

	if (xoffset < yoffset)
		mouseScrollDirection = gps::MOVE_BACKWARD;
	else
		mouseScrollDirection = gps::MOVE_FORWARD;

	myCamera.move(mouseScrollDirection, 0.1f);
}

float angle = 0.0f;
void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W] && !isAutoMoveForward && !isAutoRotateLeft && !isAutoRotateRight) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S] && !isAutoMoveForward && !isAutoRotateLeft && !isAutoRotateRight) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A] && !isAutoMoveForward && !isAutoRotateLeft && !isAutoRotateRight) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D] && !isAutoMoveForward && !isAutoRotateLeft && !isAutoRotateRight) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_G]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_H]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//if (pressedKeys[GLFW_KEY_Z]) {
	//	isSwing = true;
	//}

	//if (pressedKeys[GLFW_KEY_X]) {
	//	isSwing = false;
	//}

	if (glfwGetKey(glWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(glWindow, true);
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetScrollCallback(glWindow, scrollCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	nanosuit.LoadModel("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/objects/nanosuit/nanosuit.obj");
	ground.LoadModel("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/objects/western/gpproject.obj");
	//ground.LoadModel("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/objects/ground/ground.obj");
	well.LoadModel("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/objects/well/old_well.obj");
	sign.LoadModel("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/objects/sign/sign.obj");
	signFrame.LoadModel("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/objects/sign/signFrame.obj");
	water.LoadModel("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/objects/water/water.obj");
	lightCube.LoadModel("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/objects/cube/cube.obj");
	screenQuad.LoadModel("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/objects/quad/quad.obj");
	particleModel.LoadModel("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/objects/sandParticle/sandParticle.obj");
}

void initShaders() {
	myCustomShader.loadShader("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/shaderStart2.vert", "C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/shaderStart2.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/lightCube.vert", "C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/screenQuad.vert", "C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/simpleDepthMap.vert", "C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/simpleDepthMap.frag");
	waterShader.loadShader("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/shaderStart.vert", "C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/shaderStart.frag");
	waterShader.useShaderProgram();
	//skyboxShader.loadShader("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/skyboxShader.vert", "C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/skyboxShader.frag");
	//skyboxShader.useShaderProgram();

}

void initUniforms(gps::Shader myCustomShadery) {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(10.0f, 10.0f, 10.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos1"), 1, glm::value_ptr(glm::vec3(0.3f, 0.7f, 4.2f)));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFaces() {
	std::vector<const GLchar*> faces;
	faces.push_back("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/textures/skybox/posx.tga");
	faces.push_back("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/textures/skybox/negx.tga");
	faces.push_back("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/textures/skybox/posy.tga");
	faces.push_back("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/textures/skybox/negy.tga");
	faces.push_back("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/textures/skybox/posz.tga");
	faces.push_back("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/textures/skybox/negz.tga");

	mySkyBox.Load(faces);
	skyboxShader.loadShader("C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/skyboxShader.vert", "C:/Users/vladm/source/repos/OpenGLproject8/OpenGLproject8/shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	/*const GLfloat near_plane = 1.0f, far_plane = 30.0f;*/

	const GLfloat near_plane = 0.1f, far_plane = 30.0f;
	glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void swingAnimation() {
	acceleration = cos(signAngle * 3.14f / 180.0f) * 9.82f * 1.0f / 24.0f;
	if (isSwing) {
		if (signAngle > 30.0f) {
			isAddAngle = false;
		}
		else if (signAngle < -30.0f) {
			isAddAngle = true;
		}
		if (isAddAngle) {
			signAngle += acceleration;
			//signAngle += 1.0f;

		}
		else {
			signAngle -= acceleration;
			//signAngle -= 1.0f;
		}
	}
}

void cameraAuto() {
	if (isAutoMoveForward) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (isAutoRotateLeft) {
		myCamera.rotate(-rotateSpeed, 0.0f);
	}

	if (isAutoRotateRight) {
		myCamera.rotate(rotateSpeed, 0.0f);
	}
}

void cameraAnimation() {
	cameraAuto();
	if (isAnimationCamera) {
		if (animationSum > animationBound) {
			isAutoMoveForward = false;
			isAutoRotateLeft = true;
			if (rotateSum > rotateBound) {
				isAutoMoveForward = true;
				isAutoRotateLeft = false;
				if (animationSum2 > animationBound2) {
					isAutoMoveForward = false;
					isAutoRotateLeft = false;
					isAnimationCamera = false;
				}
				else {
					animationSum2 += cameraSpeed;
				}
			}
			else {
				rotateSum += rotateSpeed;
			}
		}
		else {
			animationSum += cameraSpeed;
		}
	}
}

void renderParticles(gps::Shader shader) {
	/*myCustomShader.useShaderProgram();*/
	for (int i = 0; i < nr_particles; i++) {
		if (sandParticles[i].Position.x < boundLeft) {
			/*float x = rand() % (int)(boundRight - boundLeft) + boundLeft;
			float y = rand() % (int)(boundTop - boundBottom) + boundBottom;
			float z = rand() % (int)(boundFront - boundBack) + boundBack;
			sandParticles[i].Position = glm::vec3(x, y, z);*/
			sandParticles[i].Position.x = boundRight;
		}
		sandParticles[i].Position.x -= particleSpeed;
		//model = glm::rotate(glm::mat4(1.0f), glm::radians(sandParticles[i].angle), sandParticles[i].Rotate);
		model = glm::translate(glm::mat4(1.0f), sandParticles[i].Position);
		model = glm::scale(model, glm::vec3(0.1f));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(shader.shaderProgram, "isReflect"), false);
		particleModel.Draw(shader);
	}
}

void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform1i(glGetUniformLocation(shader.shaderProgram, "isFog"), isFog);
		glUniform1i(glGetUniformLocation(shader.shaderProgram, "isPointLight"), isPointLight);
	}
	else {
		glUniform1i(glGetUniformLocation(shader.shaderProgram, "isReflect"), false);
		
	}

	nanosuit.Draw(shader);


	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(10.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	else {
		glUniform1i(glGetUniformLocation(shader.shaderProgram, "isReflect"), false);
	}

	ground.Draw(shader);


	model = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.10f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	else {
		glUniform1i(glGetUniformLocation(shader.shaderProgram, "isReflect"), false);
	}

	well.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, -5.5f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.20f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	else {
		glUniform1i(glGetUniformLocation(shader.shaderProgram, "isReflect"), false);
	}

	sign.Draw(shader);

	swingAnimation();
	//printf("angle = %f\n", signAngle);
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.1f, -5.5f));
	model = glm::rotate(model, glm::radians(signAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f, -1.1f, 0.0f));
	//model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));
	model = glm::scale(model, glm::vec3(0.20f));
	
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	else {
		glUniform1i(glGetUniformLocation(shader.shaderProgram, "isReflect"), false);
	}
	signFrame.Draw(shader);

	//waterShader.useShaderProgram();
	//
	model = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.10f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	else {
		glUniform1i(glGetUniformLocation(shader.shaderProgram, "isReflect"), true);
	}
	water.Draw(shader);

	if (!depthPass && isSandStorm) {
		renderParticles(shader);
	}

}

void renderScene() {

	cameraAnimation();

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	//glCullFace(GL_BACK);
	drawObjects(depthMapShader, true);
	//glCullFace(GL_FRONT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		
		//draw a white cube around the light

		lightShader.useShaderProgram();
		glUniform1i(glGetUniformLocation(lightShader.shaderProgram, "isPointLight"), isPointLight);
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.3f, 0.7f, 4.2f));
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader);
		mySkyBox.Draw(skyboxShader, view, projection);
	}
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}
	initParticles();
	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms(myCustomShader);
	initUniforms(waterShader);
	initFBO();
	initFaces();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}

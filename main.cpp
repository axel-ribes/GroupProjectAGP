#include <iostream>
#include <stdlib.h>
#include <stack>

#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "rt3d.h"
#include "rt3dObjLoader.h"

using namespace std;

#define DEG_TO_RADIAN 0.017453293

GLuint meshIndexCount = 0;
GLuint meshObjects;
GLuint texture;

//shaders
GLuint shaderProgram;
GLuint spotlightProgram;

GLfloat r = 0.0f;

glm::vec3 eye(0.0f, 1.0f, 8.0f);
glm::vec3 at(0.0f, 1.0f, -1.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

// Blue Light
rt3d::lightStruct lightBlue = {
	{0.3f, 0.3f, 0.3f, 1.0f}, // ambient
	{0.0f, 0.0f, 1.0f, 1.0f}, // diffuse
	{0.0f, 0.0f, 1.0f, 1.0f}, // specular
	{-6.0f, 5.0f, 14.0f, 1.0f}, // position
};
glm::vec4 lightBluePos(-6.0f, 5.0f, 14.0f, 1.0f);

// Yellow Light
rt3d::lightStruct lightYellow = {
	{0.3f, 0.3f, 0.3f, 1.0f}, // ambient
	{1.0f, 1.0f, 0.0f, 1.0f}, // diffuse
	{1.0f, 1.0f, 0.0f, 1.0f}, // specular
	{6.0f, 5.0f, 14.0f, 1.0f}, // position
};
glm::vec4 lightYellowPos(6.0f, 5.0f, 14.0f, 1.0f);

// material
rt3d::materialStruct materialMap = {
	{0.9f, 0.9f, 0.9f, 1.0f},
	{0.95f, 0.95f, 0.95f, 1.0f},
	{1.0f, 1.0f, 1.0f, 1.0f},
	1.0f
};

stack<glm::mat4> drawStack;
float rotationBlueAngle =0.0f;
float rotationYellowAngle =0.0f;

// rotation angle of the reflectors
glm::vec3 rotationBluePlane(0.0f, 0.0f, 0.0f);
glm::vec3 rotationYellowPlane(0.0f, 0.0f, 0.0f);
// reflector Normal of the reflectors
glm::vec3 reflectorBlueNormal(0.0f, 0.0f, 0.0f);
glm::vec3 reflectorYellowNormal(0.0f, 0.0f, 0.0f);

SDL_Window* setupSDL(SDL_GLContext& context) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		cout << "SDL_Init Error: " << SDL_GetError() << endl;
		exit(1);
	}

	// set OpenGL version 3
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// set double buffer on
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);// antialiasing

	// create window
	SDL_Window* window = SDL_CreateWindow("Class test Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
		SDL_Quit();
		exit(1);
	}

	context = SDL_GL_CreateContext(window);// create context

	return window;
}

GLuint loadTexture(const char* fileName) {
	GLuint textureID;
	glGenTextures(1, &textureID);

	// load file - using core SDL library
	SDL_Surface* tmpSurface;
	tmpSurface = SDL_LoadBMP(fileName);
	if (tmpSurface == nullptr) {
		std::cout << "Error loading bitmap" << std::endl;
	}

	// bind texture and set parameters
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tmpSurface->w, tmpSurface->h, 0,
		GL_BGR, GL_UNSIGNED_BYTE, tmpSurface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(tmpSurface);
	return textureID;
}

void init() {
	shaderProgram = rt3d::initShaders("src/phongShader.vert", "src/phongShader.frag");

	spotlightProgram = rt3d::initShaders("src/spotlightPhongShader.vert", "src/spotlightPhongShader.frag");

	vector<GLfloat> verts;
	vector<GLfloat> norms;
	vector<GLfloat> tex_coords;
	vector<GLuint> indices;
	rt3d::loadObj("src/cube.obj", verts, norms, tex_coords, indices);
	meshIndexCount = indices.size();
	texture = loadTexture("src/Red_bricks.bmp");
	meshObjects = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), tex_coords.data(), meshIndexCount, indices.data());

	glEnable(GL_DEPTH_TEST);
}

glm::vec3 moveForward(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d * std::sin(r * DEG_TO_RADIAN), pos.y, pos.z - d * std::cos(r * DEG_TO_RADIAN));
}

glm::vec3 moveRight(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d * std::cos(r * DEG_TO_RADIAN), pos.y, pos.z + d * std::sin(r * DEG_TO_RADIAN));
}

void movement() {
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_W]) eye = moveForward(eye, r, 0.1f);
	if (keys[SDL_SCANCODE_S]) eye = moveForward(eye, r, -0.1f);
	if (keys[SDL_SCANCODE_D]) eye = moveRight(eye, r, 0.1f);
	if (keys[SDL_SCANCODE_A]) eye = moveRight(eye, r, -0.1f);
	if (keys[SDL_SCANCODE_R]) eye.y += 0.1f;
	if (keys[SDL_SCANCODE_F]) eye.y -= 0.1f;
	if (keys[SDL_SCANCODE_COMMA]) r -= 0.5f;
	if (keys[SDL_SCANCODE_PERIOD]) r += 0.5f;

	if (keys[SDL_SCANCODE_UP]) {
		if (rotationBluePlane.x >= -1) {
			rotationBluePlane.x -= 0.01;
		}
	}
	if (keys[SDL_SCANCODE_RIGHT]) {
		if (rotationBluePlane.z >= -1) {
			rotationBluePlane.z -= 0.01;
		}
	}
	if (keys[SDL_SCANCODE_LEFT]) {
		if (rotationBluePlane.z <= 1) {
			rotationBluePlane.z += 0.01;
		}
	}
	if (keys[SDL_SCANCODE_DOWN]) {
		if (rotationBluePlane.x <= 1) {
			rotationBluePlane.x += 0.01;
		}
	}
	reflectorBlueNormal.z = ((abs(rotationBluePlane.x) * 45.0f) * 1) / 180;
	if (rotationBluePlane.x < 0)
		reflectorBlueNormal.z += 1;
	else
		reflectorBlueNormal.z -= 1;

	reflectorBlueNormal.x = ((rotationBluePlane.z * 45.0f) * 0.5) / 90;
	if (rotationBluePlane.x < 0)
		reflectorBlueNormal.x = -reflectorBlueNormal.x;

	if (abs(rotationBluePlane.x) < abs(rotationBluePlane.z))
		rotationBlueAngle = abs(rotationBluePlane.z) * 45.0f;
	else
		rotationBlueAngle = abs(rotationBluePlane.x) * 45.0f;
	if (rotationBlueAngle > 45.0f)
		rotationBlueAngle = 45.0f;


	if (keys[SDL_SCANCODE_I]) {
		if (rotationYellowPlane.x >= -1) {
			rotationYellowPlane.x -= 0.01;
		}
	}
	if (keys[SDL_SCANCODE_L]) {
		if (rotationYellowPlane.z >= -1) {
			rotationYellowPlane.z -= 0.01;
		}
	}
	if (keys[SDL_SCANCODE_J]) {
		if (rotationYellowPlane.z <= 1) {
			rotationYellowPlane.z += 0.01;
		}
	}
	if (keys[SDL_SCANCODE_K]) {
		if (rotationYellowPlane.x <= 1) {
			rotationYellowPlane.x += 0.01;
		}
	}
	reflectorYellowNormal.z = ((abs(rotationYellowPlane.x) * 45.0f) * 1) / 180;
	if (rotationYellowPlane.x < 0)
		reflectorYellowNormal.z += 1;
	else
		reflectorYellowNormal.z -= 1;

	reflectorYellowNormal.x = ((rotationYellowPlane.z * 45.0f) * 0.5) / 90;
	if (rotationYellowPlane.x < 0)
		reflectorYellowNormal.x = -reflectorYellowNormal.x;

	if (abs(rotationYellowPlane.x) < abs(rotationYellowPlane.z))
		rotationYellowAngle = abs(rotationYellowPlane.z) * 45.0f;
	else
		rotationYellowAngle = abs(rotationYellowPlane.x) * 45.0f;
	if (rotationYellowAngle > 45.0f)
		rotationYellowAngle = 45.0f;
}

void draw(SDL_Window* window) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 model(1.0);
	drawStack.push(model);

	at = moveForward(eye, r, 1.0f);
	glm::mat4 view = glm::lookAt(eye, at, up);
	rt3d::setUniformMatrix4fv(shaderProgram, "view", glm::value_ptr(view));

	glm::mat4 projection = glm::perspective(float(60.0f * DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 150.0f);

	glm::vec4 tmpBlue = drawStack.top() * lightBluePos;
	lightBlue.position[0] = tmpBlue.x;
	lightBlue.position[1] = tmpBlue.y;
	lightBlue.position[2] = tmpBlue.z;

	glm::vec4 tmpYellow = drawStack.top() * lightYellowPos;
	lightYellow.position[0] = tmpYellow.x;
	lightYellow.position[1] = tmpYellow.y;
	lightYellow.position[2] = tmpYellow.z;

	// Wall
	glUseProgram(spotlightProgram);

	// Set blue light
	int uniformIndex = glGetUniformLocation(spotlightProgram, "lightBlue.ambient");
	glUniform4fv(uniformIndex, 1, lightBlue.ambient);
	uniformIndex = glGetUniformLocation(spotlightProgram, "lightBlue.diffuse");
	glUniform4fv(uniformIndex, 1, lightBlue.diffuse);
	uniformIndex = glGetUniformLocation(spotlightProgram, "lightBlue.specular");
	glUniform4fv(uniformIndex, 1, lightBlue.specular);
	uniformIndex = glGetUniformLocation(spotlightProgram, "lightBluePosition");
	glUniform4fv(uniformIndex, 1, lightBlue.position);

	// Set yellow light
	uniformIndex = glGetUniformLocation(spotlightProgram, "lightYellow.ambient");
	glUniform4fv(uniformIndex, 1, lightYellow.ambient);
	uniformIndex = glGetUniformLocation(spotlightProgram, "lightYellow.diffuse");
	glUniform4fv(uniformIndex, 1, lightYellow.diffuse);
	uniformIndex = glGetUniformLocation(spotlightProgram, "lightYellow.specular");
	glUniform4fv(uniformIndex, 1, lightYellow.specular);
	uniformIndex = glGetUniformLocation(spotlightProgram, "lightYellowPosition");
	glUniform4fv(uniformIndex, 1, lightYellow.position);

	rt3d::setUniformMatrix4fv(spotlightProgram, "view", glm::value_ptr(view));

	rt3d::setUniformMatrix4fv(spotlightProgram, "projection", glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(spotlightProgram, "generalBlueLightPos"), 1, glm::value_ptr(tmpBlue));
	glUniform3fv(glGetUniformLocation(spotlightProgram, "generalYellowLightPos"), 1, glm::value_ptr(tmpYellow));

	glUniform3f(glGetUniformLocation(spotlightProgram, "viewPos"), eye.x, eye.y, eye.z);

	glUniform3f(glGetUniformLocation(spotlightProgram, "reflectorPositionBlue"), -8.0f, -2.0f, -3.0f);
	glUniform3f(glGetUniformLocation(spotlightProgram, "reflectorPositionYellow"), 8.0f, -2.0f, -3.0f);

	glUniform3fv(glGetUniformLocation(spotlightProgram, "reflectorBlueNormal"), 1, glm::value_ptr(reflectorBlueNormal));
	glUniform3fv(glGetUniformLocation(spotlightProgram, "reflectorYellowNormal"), 1, glm::value_ptr(reflectorYellowNormal));

	glUniform1f(glGetUniformLocation(spotlightProgram, "lightCutOff"), glm::cos(glm::radians(5.5f)));

	glBindTexture(GL_TEXTURE_2D, texture);
	drawStack.push(drawStack.top());
	drawStack.top() = glm::translate(drawStack.top(), glm::vec3(0.0f, 7.0f, -20.0f));
	drawStack.top() = glm::scale(drawStack.top(), glm::vec3(20.0f, 10.0f, 3.0f));
	rt3d::setUniformMatrix4fv(spotlightProgram, "model", glm::value_ptr(drawStack.top()));
	rt3d::setMaterial(spotlightProgram, materialMap);
	rt3d::drawIndexedMesh(meshObjects, meshIndexCount, GL_TRIANGLES);
	drawStack.pop();

	// Reflector light blue
	glUseProgram(shaderProgram);
	rt3d::setLight(shaderProgram, lightBlue);
	rt3d::setLightPos(shaderProgram, glm::value_ptr(tmpBlue));

	glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(eye));

	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));

	drawStack.push(drawStack.top());
	drawStack.top() = glm::translate(drawStack.top(), glm::vec3(-6.0f, -2.0f, -3.0f));
	if (rotationBluePlane.x != 0 || rotationBluePlane.z != 0 )
		drawStack.top() = glm::rotate(drawStack.top(), float(rotationBlueAngle * DEG_TO_RADIAN), rotationBluePlane);
	drawStack.top() = glm::scale(drawStack.top(), glm::vec3(3.0f, 0.2f, 5.0f));
	rt3d::setUniformMatrix4fv(shaderProgram, "model", glm::value_ptr(drawStack.top()));
	rt3d::setMaterial(shaderProgram, materialMap);
	rt3d::drawIndexedMesh(meshObjects, meshIndexCount, GL_TRIANGLES);
	drawStack.pop();

	// blue light position
	drawStack.push(drawStack.top());
	drawStack.top() = glm::translate(drawStack.top(), glm::vec3(lightBluePos[0], lightBluePos[1], lightBluePos[2]));
	drawStack.top() = glm::scale(drawStack.top(), glm::vec3(1.0f, 1.0f, 1.0f));
	rt3d::setUniformMatrix4fv(shaderProgram, "model", glm::value_ptr(drawStack.top()));
	rt3d::setMaterial(shaderProgram, materialMap);
	rt3d::drawIndexedMesh(meshObjects, meshIndexCount, GL_TRIANGLES);
	drawStack.pop();

	// Yellow light
	rt3d::setLight(shaderProgram, lightYellow);
	rt3d::setLightPos(shaderProgram, glm::value_ptr(tmpYellow));

	glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(eye));

	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));

	drawStack.push(drawStack.top());
	drawStack.top() = glm::translate(drawStack.top(), glm::vec3(6.0f, -2.0f, -3.0f));
	if (rotationYellowPlane.x != 0 || rotationYellowPlane.z != 0)
		drawStack.top() = glm::rotate(drawStack.top(), float(rotationYellowAngle * DEG_TO_RADIAN), rotationYellowPlane);
	drawStack.top() = glm::scale(drawStack.top(), glm::vec3(3.0f, 0.2f, 5.0f));
	rt3d::setUniformMatrix4fv(shaderProgram, "model", glm::value_ptr(drawStack.top()));
	rt3d::setMaterial(shaderProgram, materialMap);
	rt3d::drawIndexedMesh(meshObjects, meshIndexCount, GL_TRIANGLES);
	drawStack.pop();

	SDL_GL_SwapWindow(window);
}

int main(int argc, char* argv[]) {
	SDL_Window* window;
	SDL_GLContext context;
	window = setupSDL(context);

	GLenum glew(glewInit());
	if (glew != GLEW_OK) {
		cout << "glewInit() Error: " << glewGetErrorString(glew) << endl;
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	init();
	bool finish = false;
	SDL_Event events;
	while (!finish) {
		while (SDL_PollEvent(&events)) {
			if (events.type == SDL_QUIT)
				finish = true;
		}

		movement();
		draw(window);
	}

	// destroy context and window 
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
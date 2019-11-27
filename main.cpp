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

GLuint textures[2];

// light
rt3d::lightStruct light = {
	{0.4f, 0.4f, 0.4f, 1.0f}, // ambient
	{1.0f, 1.0f, 1.0f, 1.0f}, // diffuse
	{1.0f, 1.0f, 1.0f, 1.0f}, // specular
	{0.0f, 5.0f, 0.0f, 1.0f}, // position
};
glm::vec4 lightPos(0.0f, 5.0f, 14.0f, 1.0f);

// material
rt3d::materialStruct materialMap = {
	{0.9f, 0.9f, 0.9f, 1.0f},
	{0.95f, 0.95f, 0.95f, 1.0f},
	{1.0f, 1.0f, 1.0f, 1.0f},
	1.0f
};

stack<glm::mat4> drawStack;
float rotationAngle =0.0f;

glm::vec3 rotationPlane(0.0f, 0.0f, 0.0f);
glm::vec3 reflectorNormal(0.0f, 0.0f, 0.0f);

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

void calculateTangents(vector<GLfloat>& tangents, vector<GLfloat>& verts, vector<GLfloat>& normals, vector<GLfloat>& tex_coords, vector<GLuint>& indices) {

	// Code taken from http://www.terathon.com/code/tangent.html and modified slightly to use vectors instead of arrays
	// Lengyel, Eric. “Computing Tangent Space Basis Vectors for an Arbitrary Mesh”. Terathon Software 3D Graphics Library, 2001. 

	// This is a little messy because my vectors are of type GLfloat:
	// should have made them glm::vec2 and glm::vec3 - life, would be much easier!

	vector<glm::vec3> tan1(verts.size() / 3, glm::vec3(0.0f));
	vector<glm::vec3> tan2(verts.size() / 3, glm::vec3(0.0f));
	int triCount = indices.size() / 3;
	for (int c = 0; c < indices.size(); c += 3)
	{
		int i1 = indices[c];
		int i2 = indices[c + 1];
		int i3 = indices[c + 2];

		glm::vec3 v1(verts[i1 * 3], verts[i1 * 3 + 1], verts[i1 * 3 + 2]);
		glm::vec3 v2(verts[i2 * 3], verts[i2 * 3 + 1], verts[i2 * 3 + 2]);
		glm::vec3 v3(verts[i3 * 3], verts[i3 * 3 + 1], verts[i3 * 3 + 2]);

		glm::vec2 w1(tex_coords[i1 * 2], tex_coords[i1 * 2 + 1]);
		glm::vec2 w2(tex_coords[i2 * 2], tex_coords[i2 * 2 + 1]);
		glm::vec2 w3(tex_coords[i3 * 2], tex_coords[i3 * 2 + 1]);

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0F / (s1 * t2 - s2 * t1);
		glm::vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);
		glm::vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}

	for (int a = 0; a < verts.size(); a += 3)
	{
		glm::vec3 n(normals[a], normals[a + 1], normals[a + 2]);
		glm::vec3 t = tan1[a / 3];

		glm::vec3 tangent;
		tangent = (t - n * glm::dot(n, t));
		tangent = glm::normalize(tangent);

		// handedness
		GLfloat w = (glm::dot(glm::cross(n, t), tan2[a / 3]) < 0.0f) ? -1.0f : 1.0f;

		tangents.push_back(tangent.x);
		tangents.push_back(tangent.y);
		tangents.push_back(tangent.z);
		tangents.push_back(w);

	}

}

void init() {
	shaderProgram = rt3d::initShaders("../GroupProjectAGP/phongShader.vert", "../GroupProjectAGP/phongShader.frag");
	rt3d::setLight(shaderProgram, light);

	spotlightProgram = rt3d::initShaders("../GroupProjectAGP/spotlightPhongShader.vert", "../GroupProjectAGP/spotlightPhongShader.frag");
	rt3d::setLight(spotlightProgram, light);

	vector<GLfloat> verts;
	vector<GLfloat> norms;
	vector<GLfloat> tex_coords;
	vector<GLuint> indices;

	

	rt3d::loadObj("../GroupProjectAGP/cube.obj", verts, norms, tex_coords, indices);
	meshIndexCount = indices.size();

	texture = loadTexture("../GroupProjectAGP/Red_bricks.bmp");
	meshObjects = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), tex_coords.data(), meshIndexCount, indices.data());

	// also need for normal mapping a VBO for the bitangents
	vector<GLfloat> tangents;
	calculateTangents(tangents, verts, norms, tex_coords, indices);

	textures[0] = loadTexture("../GroupProjectAGP/Red_Bricks_normalMap.bmp"); 

	glBindVertexArray(meshObjects);
	GLuint VBO;
	glGenBuffers(1, &VBO);
	// VBO for tangent data
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(GLfloat), tangents.data(), GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)4, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(4);
	glBindVertexArray(0);

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
		if (rotationPlane.x >= -1) {
			rotationPlane.x -= 0.01;
		}
	}
	if (keys[SDL_SCANCODE_RIGHT]) {
		if (rotationPlane.z >= -1) {
			rotationPlane.z -= 0.01;
		}
	}
	if (keys[SDL_SCANCODE_LEFT]) {
		if (rotationPlane.z <= 1) {
			rotationPlane.z += 0.01;
		}
	}
	if (keys[SDL_SCANCODE_DOWN]) {
		if (rotationPlane.x <= 1) {
			rotationPlane.x += 0.01;
		}
	}
	reflectorNormal.z = ((abs(rotationPlane.x) * 45.0f) * 1) / 180;
	if (rotationPlane.x < 0)
		reflectorNormal.z += 1;
	else
		reflectorNormal.z -= 1;

	reflectorNormal.x = ((rotationPlane.z * 45.0f) * 0.5) / 90;
	if (rotationPlane.x < 0)
		reflectorNormal.x = -reflectorNormal.x;

	if (abs(rotationPlane.x) < abs(rotationPlane.z))
		rotationAngle = abs(rotationPlane.z) * 45.0f;
	else
		rotationAngle = abs(rotationPlane.x) * 45.0f;
	if (rotationAngle > 45.0f)
		rotationAngle = 45.0f;

	if (keys[SDL_SCANCODE_I]) lightPos[2] -= 0.1;
	if (keys[SDL_SCANCODE_J]) lightPos[0] -= 0.1;
	if (keys[SDL_SCANCODE_K]) lightPos[2] += 0.1;
	if (keys[SDL_SCANCODE_L]) lightPos[0] += 0.1;
	if (keys[SDL_SCANCODE_P]) lightPos[1] += 0.1f;
	if (keys[SDL_SCANCODE_SEMICOLON]) lightPos[1] -= 0.1f;
}

void draw(SDL_Window* window) {
	glClearColor(0.1f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 model(1.0);
	drawStack.push(model);

	at = moveForward(eye, r, 1.0f);
	glm::mat4 view = glm::lookAt(eye, at, up);
	rt3d::setUniformMatrix4fv(shaderProgram, "view", glm::value_ptr(view));

	glm::mat4 projection = glm::perspective(float(60.0f * DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 150.0f);

	glm::vec4 tmp = drawStack.top() * lightPos;
	light.position[0] = tmp.x;
	light.position[1] = tmp.y;
	light.position[2] = tmp.z;

	glUseProgram(spotlightProgram);
	
	rt3d::setUniformMatrix4fv(spotlightProgram, "view", glm::value_ptr(view));
	rt3d::setUniformMatrix4fv(spotlightProgram, "projection", glm::value_ptr(projection));

	glm::vec4 tmpSpotlightPos = drawStack.top() * glm::vec4(0.0f, -2.0f, -3.0f, 1.0f);
	light.position[0] = tmpSpotlightPos.x;
	light.position[1] = tmpSpotlightPos.y;
	light.position[2] = tmpSpotlightPos.z;
	rt3d::setLightPos(spotlightProgram, glm::value_ptr(tmpSpotlightPos));

	glUniform3fv(glGetUniformLocation(spotlightProgram, "generalLightPos"), 1, glm::value_ptr(tmp));
	glUniform3f(glGetUniformLocation(spotlightProgram, "viewPos"), eye.x, eye.y, eye.z);
	glUniform3f(glGetUniformLocation(spotlightProgram, "reflectorPosition"), 0.0f, -2.0f, -3.0f);
	glUniform3fv(glGetUniformLocation(spotlightProgram, "reflectorNormal"), 1, glm::value_ptr(reflectorNormal));


	glUniform1f(glGetUniformLocation(spotlightProgram, "lightCutOff"), glm::cos(glm::radians(12.5f)));

	GLuint uniformIndex = glGetUniformLocation(spotlightProgram, "normalMap");
	glUniform1i(uniformIndex, 1);
	uniformIndex = glGetUniformLocation(spotlightProgram, "textureUnit0"); //texMap in reality
	glUniform1i(uniformIndex, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[0]);



	drawStack.push(drawStack.top());
	drawStack.top() = glm::translate(drawStack.top(), glm::vec3(0.0f, 7.0f, -20.0f));
	drawStack.top() = glm::scale(drawStack.top(), glm::vec3(20.0f, 10.0f, 0.5f));
	rt3d::setUniformMatrix4fv(spotlightProgram, "model", glm::value_ptr(drawStack.top()));
	rt3d::setMaterial(spotlightProgram, materialMap);
	rt3d::drawIndexedMesh(meshObjects, meshIndexCount, GL_TRIANGLES);
	drawStack.pop();

	glUseProgram(shaderProgram);
	rt3d::setLightPos(shaderProgram, glm::value_ptr(tmp));

	glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(eye));
	rt3d::setUniformMatrix4fv(shaderProgram, "view", glm::value_ptr(view));
	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));

	drawStack.push(drawStack.top());
	drawStack.top() = glm::translate(drawStack.top(), glm::vec3(0.0f, -2.0f, -3.0f));
	if (rotationPlane.x != 0 || rotationPlane.z != 0 )
		drawStack.top() = glm::rotate(drawStack.top(), float(rotationAngle * DEG_TO_RADIAN), rotationPlane);
	drawStack.top() = glm::scale(drawStack.top(), glm::vec3(3.0f, 0.2f, 5.0f));
	rt3d::setUniformMatrix4fv(shaderProgram, "model", glm::value_ptr(drawStack.top()));
	rt3d::setMaterial(shaderProgram, materialMap);
	rt3d::drawIndexedMesh(meshObjects, meshIndexCount, GL_TRIANGLES); //platform
	drawStack.pop();

	drawStack.push(drawStack.top());
	drawStack.top() = glm::translate(drawStack.top(), glm::vec3(lightPos[0], lightPos[1], lightPos[2]));
	drawStack.top() = glm::scale(drawStack.top(), glm::vec3(1.0f, 1.0f, 1.0f));
	rt3d::setUniformMatrix4fv(shaderProgram, "model", glm::value_ptr(drawStack.top()));
	rt3d::setMaterial(shaderProgram, materialMap);
	rt3d::drawIndexedMesh(meshObjects, meshIndexCount, GL_TRIANGLES); // box forlight
	drawStack.pop();

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
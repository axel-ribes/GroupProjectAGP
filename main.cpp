// MD2 animation renderer
// This demo will load and render an animated MD2 model, an OBJ model and a skybox
// Most of the OpenGL code for dealing with buffer objects, etc has been moved to a 
// utility library, to make creation and display of mesh objects as simple as possible

// Windows specific: Uncomment the following line to open a console window for debug output
#if _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#endif

#include "rt3d.h"
#include "rt3dObjLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stack>
#include "md2model.h"

//#include <SDL_ttf.h>

using namespace std;

#define DEG_TO_RADIAN 0.017453293

// Globals
// Real programs don't use globals :-D

bool myShaderCheck = false;

float attenuationConstant = 1.0f;
float attenuationLinear = 0.01f;
float attenuationQuadratic = 0.01f;

GLuint meshIndexCount = 0;
GLuint md2VertCount = 0;
GLuint meshObjects[2];
GLuint phongShader;
GLuint gouraudShader;
GLuint toonShader;
GLuint myShader;
GLuint skyboxProgram;

GLfloat r = 0.0f;

glm::vec3 eye(0.0f, 1.0f, 0.0f);
glm::vec3 at(0.0f, 1.0f, -1.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

stack<glm::mat4> mvStack;

// TEXTURE STUFF
GLuint textures[4];
GLuint skybox[5];
GLuint labels[5];

rt3d::lightStruct light0 = {
	{0.3f, 0.3f, 0.3f, 1.0f}, // ambient
	{1.0f, 1.0f, 1.0f, 1.0f}, // diffuse
	{1.0f, 1.0f, 1.0f, 1.0f}, // specular
	{-10.0f, 10.0f, 10.0f, 1.0f}  // position
};
//glm::vec4 lightPos(-10.0f, 10.0f, 10.0f, 1.0f); //light position

rt3d::materialStruct material0 = {
	{0.2f, 0.4f, 0.2f, 1.0f}, // ambient
	{0.5f, 1.0f, 0.5f, 1.0f}, // diffuse
	{0.0f, 0.1f, 0.0f, 1.0f}, // specular
	2.0f  // shininess
};
rt3d::materialStruct material1 = {
	{0.4f, 0.4f, 1.0f, 1.0f}, // ambient
	{0.8f, 0.8f, 1.0f, 1.0f}, // diffuse
	{0.8f, 0.8f, 0.8f, 1.0f}, // specular
	1.0f  // shininess
};
rt3d::materialStruct myMaterial;


// md2 stuff
md2model tmpModel;
int currentAnim = 0;

//shader
GLuint myshader;

//float originalPosx, oPosy, oPosZ, oPosE = 0.0;
//glm::vec4 lightPos1(originalPosx, oPosy, oPosZ, 1.0); //light position

glm::vec4 lightPos(0.0, 4.0, -5.0, 1.0);

rt3d::lightStruct light1 = {
	{0.5f, 0.5f, 0.5f, 1.0f}, // ambient
	{1.0f, 1.0f, 1.0f, 1.0f}, // diffuse
	{1.0f, 1.0f, 1.0f, 1.0f}, // specular
	{0.0f, 0.0f, 0.0f, 1.0f}  // position
};

//Shadow mapping
unsigned int depthMapFBO;
glGenFramebuffers(1, &depthMapFBO);
glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
void RenderScene();
void BindForWriting();
void BindForReading(GLenum TextureUnit);
GLuint m_fbo;
GLuint m_shadowMap;


// Set up rendering context
SDL_Window * setupRC(SDL_GLContext &context) {
	SDL_Window * window;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize video
		rt3d::exitFatalError("Unable to initialize SDL");

	// Request an OpenGL 3.0 context.

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // double buffering on
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // 8 bit alpha buffering
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Turn on x4 multisampling anti-aliasing (MSAA)

	// Create 800x600 window
	window = SDL_CreateWindow("SDL/GLM/OpenGL Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!window) // Check window was created OK
		rt3d::exitFatalError("Unable to create window");

	context = SDL_GL_CreateContext(window); // Create opengl context and attach to window
	SDL_GL_SetSwapInterval(1); // set swap buffers to sync with monitor's vertical refresh rate
	return window;
}

// A simple texture loading function
// lots of room for improvement - and better error checking!
GLuint loadBitmap(char *fname) {
	GLuint texID;
	glGenTextures(1, &texID); // generate texture ID

	// load file - using core SDL library
	SDL_Surface *tmpSurface;
	tmpSurface = SDL_LoadBMP(fname);
	if (!tmpSurface) {
		std::cout << "Error loading bitmap" << std::endl;
	}

	// bind texture and set parameters
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	SDL_PixelFormat *format = tmpSurface->format;

	GLuint externalFormat, internalFormat;
	if (format->Amask) {
		internalFormat = GL_RGBA;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGBA : GL_BGRA;
	}
	else {
		internalFormat = GL_RGB;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGB : GL_BGR;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, tmpSurface->w, tmpSurface->h, 0,
		externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(tmpSurface); // texture loaded, free the temporary buffer
	return texID;	// return value of texture ID
}


void init(void) {
	phongShader = rt3d::initShaders("../phong-tex.vert", "../phong-tex.frag");
	rt3d::setLight(phongShader, light1);
	rt3d::setMaterial(phongShader, material0);
	GLuint uniformIndex = glGetUniformLocation(phongShader, "attenuationConstant");
	glUniform1f(uniformIndex, attenuationConstant);
	uniformIndex = glGetUniformLocation(phongShader, "attenuationLinear");
	glUniform1f(uniformIndex, attenuationLinear);
	uniformIndex = glGetUniformLocation(phongShader, "attenuationQuadratic");
	glUniform1f(uniformIndex, attenuationQuadratic);

	//Gouraud
	gouraudShader = rt3d::initShaders("../gouraud.vert", "../gouraud.frag");
	rt3d::setLight(gouraudShader, light1);
	rt3d::setMaterial(gouraudShader, material0);
	GLuint uniformIndex2 = glGetUniformLocation(gouraudShader, "attenuationConstant");
	glUniform1f(uniformIndex2, attenuationConstant);
	uniformIndex2 = glGetUniformLocation(gouraudShader, "attenuationLinear");
	glUniform1f(uniformIndex2, attenuationLinear);
	uniformIndex2 = glGetUniformLocation(gouraudShader, "attenuationQuadratic");
	glUniform1f(uniformIndex2, attenuationQuadratic);

	//Toon
	toonShader = rt3d::initShaders("../toon.vert", "../toon.frag");
	rt3d::setLight(toonShader, light1);
	rt3d::setMaterial(toonShader, material0);
	GLuint uniformIndex3 = glGetUniformLocation(toonShader, "attenuationConstant");
	glUniform1f(uniformIndex3, attenuationConstant);
	uniformIndex2 = glGetUniformLocation(toonShader, "attenuationLinear");
	glUniform1f(uniformIndex3, attenuationLinear);
	uniformIndex2 = glGetUniformLocation(toonShader, "attenuationQuadratic");
	glUniform1f(uniformIndex3, attenuationQuadratic);


	

	skyboxProgram = rt3d::initShaders("../textured.vert", "../textured.frag");

	vector<GLfloat> verts;
	vector<GLfloat> norms;
	vector<GLfloat> tex_coords;
	vector<GLuint> indices;
	rt3d::loadObj("../resources/cube.obj", verts, norms, tex_coords, indices);
	GLuint size = indices.size();
	meshIndexCount = size;
	textures[0] = loadBitmap("../resources/fabric.bmp");
	meshObjects[0] = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), tex_coords.data(), size, indices.data());

	textures[1] = loadBitmap("../resources/hobgoblin2.bmp");
	meshObjects[1] = tmpModel.ReadMD2Model("../resources/tris.MD2");
	md2VertCount = tmpModel.getVertDataCount();

	textures[2] = loadBitmap("../resources/colormap.bmp");

	textures[3] = loadBitmap("../resources/Black.bmp");

	verts.clear(); 
	norms.clear(); 
	tex_coords.clear();
	indices.clear();
	
	rt3d::loadObj("../resources/bunny-5000.obj", verts, norms, tex_coords, indices);
	size = indices.size();
	meshIndexCount = size;
	meshObjects[2] = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), nullptr, size, indices.data());

	

	skybox[0] = loadBitmap("../resources/Town-skybox/Town_ft.bmp");
	skybox[1] = loadBitmap("../resources/Town-skybox/Town_bk.bmp");
	skybox[2] = loadBitmap("../resources/Town-skybox/Town_lf.bmp");
	skybox[3] = loadBitmap("../resources/Town-skybox/Town_rt.bmp");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

glm::vec3 moveForward(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d * std::sin(r*DEG_TO_RADIAN), pos.y, pos.z - d * std::cos(r*DEG_TO_RADIAN));
}

glm::vec3 moveRight(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d * std::cos(r*DEG_TO_RADIAN), pos.y, pos.z + d * std::sin(r*DEG_TO_RADIAN));
}


//Depth map texture
//const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

/*unsigned int depthMap;
//glGenTextures(1, &depthMap);
glBindTexture(GL_TEXTURE_2D, depthMap);
glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
	SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/

void update(void) {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_I]) eye = moveForward(eye, r, 0.1f);
	if (keys[SDL_SCANCODE_J]) eye = moveForward(eye, r, -0.1f);
	if (keys[SDL_SCANCODE_K]) eye = moveRight(eye, r, -0.1f);
	if (keys[SDL_SCANCODE_L]) eye = moveRight(eye, r, 0.1f);

	if (keys[SDL_SCANCODE_COMMA]) r -= 1.0f;
	if (keys[SDL_SCANCODE_PERIOD]) r += 1.0f;

	//light movement
	if (keys[SDL_SCANCODE_W]) lightPos[2] -= 0.1f;
	if (keys[SDL_SCANCODE_A]) lightPos[0] -= 0.1f;
	if (keys[SDL_SCANCODE_S]) lightPos[2] += 0.1f;
	if (keys[SDL_SCANCODE_D]) lightPos[0] += 0.1f;

	//shader control
	if (keys[SDL_SCANCODE_2])
	{
		myMaterial = material0;
		myShader = phongShader;
		material0.specular[0] = 0.1;
		material0.specular[1] = 0.1;
		material0.specular[2] = 0.1;
		material0.diffuse[0] = 3.0;
		material0.diffuse[1] = 3.0;
		material0.diffuse[2] = 3.0;
		material0.shininess = 0.2;
	}

	if (keys[SDL_SCANCODE_3])
	{
		myMaterial = material0;
		myShader = phongShader;
		material0.specular[0] = 3.0;
		material0.specular[1] = 3.0;
		material0.specular[2] = 3.0;
		material0.diffuse[0] = 0.1;
		material0.diffuse[1] = 0.1;
		material0.diffuse[2] = 0.1;
		material0.shininess = 2.0;
	}

	if (keys[SDL_SCANCODE_4])
	{
		myMaterial = material0;
		myShader = gouraudShader;
		material0.specular[0] = 3.0;
		material0.specular[1] = 3.0;
		material0.specular[2] = 3.0;
		material0.diffuse[0] = 0.1;
		material0.diffuse[1] = 0.1;
		material0.diffuse[2] = 0.1;
		material0.shininess = 2.0;
	}

	if (keys[SDL_SCANCODE_3])
	{
		myMaterial = material0;
		myShader = toonShader;
		material0.specular[0] = 3.0;
		material0.specular[1] = 3.0;
		material0.specular[2] = 3.0;
		material0.diffuse[0] = 0.1;
		material0.diffuse[1] = 0.1;
		material0.diffuse[2] = 0.1;
		material0.shininess = 2.0;
	}


	
	if (keys[SDL_SCANCODE_U]) lightPos[1] += 0.1f;
		if (keys[SDL_SCANCODE_H]) lightPos[1] -= 0.1f;

	if (keys[SDL_SCANCODE_1]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);
	}
	if (keys[SDL_SCANCODE_2]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
	}
	if (keys[SDL_SCANCODE_Z]) {
		if (--currentAnim < 0) currentAnim = 19;
		cout << "Current animation: " << currentAnim << endl;
	}
	if (keys[SDL_SCANCODE_X]) {
		if (++currentAnim >= 20) currentAnim = 0;
		cout << "Current animation: " << currentAnim << endl;
	}
}

//Render Scene for shadow mapping
void RenderScene()
{
	//Depth map texture
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	//unsigned int depthMap;
//glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMapFBO);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapFBO, 0);

	glDrawBuffer(GL_NONE);

	//shadow mapping code
//First render
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	ConfigureShaderAndMatrices(); //ConfigureShaderMatrices();
	RenderScene();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//render with shadow mapping
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ConfigureShaderAndMatrices();
	glBindTexture(GL_TEXTURE_2D, depthMapFBO);
	RenderScene();

	glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);

	// Compute the MVP matrix from the light's point of view
	glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
	glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 depthModelMatrix = glm::mat4(1.0);
	glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

	// Send our transformation to the currently bound shader,
	// in the "MVP" uniform
	glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthMVP[0][0]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);

	//Not a clue where this goes
	glm::mat4 biasMatrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
	glm::mat4 depthBiasMVP = biasMatrix * depthMVP;

	
}


void draw(SDL_Window * window) 
{
	if (myShaderCheck == false)
	{
		myShader = phongShader;
		myMaterial = material0;
		myShaderCheck = true;
	}
	

	// clear the screen
	glEnable(GL_CULL_FACE);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection(1.0);
	projection = glm::perspective(float(60.0f*DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 150.0f);
	
	
	GLfloat scale(1.0f); // just to allow easy scaling of complete scene

	glm::mat4 modelview(1.0); // set base position for scene
	mvStack.push(modelview);

	at = moveForward(eye, r, 1.0f);
	mvStack.top() = glm::lookAt(eye, at, up);


	// draw a skybox
	glUseProgram(skyboxProgram);
	rt3d::setUniformMatrix4fv(skyboxProgram, "projection", glm::value_ptr(projection));

	glDepthMask(GL_FALSE); // make sure depth test is off
	glm::mat3 mvRotOnlyMat3 = glm::mat3(mvStack.top());
	mvStack.push(glm::mat4(mvRotOnlyMat3));

	// front
	mvStack.push(mvStack.top());
	glBindTexture(GL_TEXTURE_2D, skybox[0]);
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(2.0f, 2.0f, 2.0f));
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, 0.0f, -2.0f));
	rt3d::setUniformMatrix4fv(skyboxProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// back
	mvStack.push(mvStack.top());
	glBindTexture(GL_TEXTURE_2D, skybox[1]);
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(2.0f, 2.0f, 2.0f));
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, 0.0f, 2.0f));
	rt3d::setUniformMatrix4fv(skyboxProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// left
	mvStack.push(mvStack.top());
	glBindTexture(GL_TEXTURE_2D, skybox[2]);
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(2.0f, 2.0f, 2.0f));
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-2.0f, 0.0f, 0.0f));
	rt3d::setUniformMatrix4fv(skyboxProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// right
	mvStack.push(mvStack.top());
	glBindTexture(GL_TEXTURE_2D, skybox[3]);
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(2.0f, 2.0f, 2.0f));
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(2.0f, 0.0f, 0.0f));
	rt3d::setUniformMatrix4fv(skyboxProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();

	mvStack.pop();

	// back to remainder of rendering
	glDepthMask(GL_TRUE); // make sure depth test is on

	glUseProgram(toonShader);																				//||
	rt3d::setUniformMatrix4fv(toonShader, "projection", glm::value_ptr(projection));						//||

	//second light
	glm::vec4 tmp1 = mvStack.top() * lightPos;
	light1.position[0] = tmp1.x;
	light1.position[1] = tmp1.y;
	light1.position[2] = tmp1.z;
	rt3d::setLightPos(toonShader, glm::value_ptr(tmp1));														//||
	
	// draw a cube for ground plane
	glUseProgram(phongShader);																				//||
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-10.0f, -0.1f, -10.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, 0.1f, 20.0f));
	rt3d::setUniformMatrix4fv(myShader, "modelview", glm::value_ptr(mvStack.top()));						//||
	rt3d::setMaterial(phongShader, myMaterial);																//||
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// cube for toon stuff
	glUseProgram(toonShader);																				//||
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, 4.0f, -10.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(1.0f, 1.0f, 1.0f));
	rt3d::setUniformMatrix4fv(toonShader, "modelview", glm::value_ptr(mvStack.top()));						//||
	rt3d::setMaterial(toonShader, myMaterial);																//||
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();

	//bunny
	glUseProgram(toonShader);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0, 0.0, -5.0));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(15.0, 15.0, 15.0));
	rt3d::setUniformMatrix4fv(toonShader, "modelview", glm::value_ptr(mvStack.top()));						//||
	rt3d::setMaterial(toonShader, myMaterial);																//||
	rt3d::drawIndexedMesh(meshObjects[2], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();

	//bunny 2
	glCullFace(GL_FRONT); //Front face cull
	glUseProgram(toonShader); 
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0, 0.0, -5.0));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(16.0, 16.0, 16.0));
	rt3d::setUniformMatrix4fv(toonShader, "modelview", glm::value_ptr(mvStack.top()));						//||
	rt3d::setMaterial(toonShader, myMaterial);																//||
	rt3d::drawIndexedMesh(meshObjects[2], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();
	glCullFace(GL_BACK); //backface cull
	glDisable(GL_DEPTH_TEST); //disable z buffer 

		
	glUseProgram(phongShader);//Use the texture shader again to draw the labelled cube							//||
	rt3d::setUniformMatrix4fv(phongShader, "projection", glm::value_ptr(projection));							//||
	// draw a cube block on top of ground plane
	// with text texture 
	//Keep the texture-only shader
	glDepthMask(GL_FALSE); // make sure writing to update depth test is off
	glBindTexture(GL_TEXTURE_2D, labels[0]);
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(lightPos.x, lightPos.y, lightPos.z));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(1.0f, 1.0f, 0.0f));	
	rt3d::setUniformMatrix4fv(phongShader, "modelview", glm::value_ptr(mvStack.top()));						//||
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();

	//shadow mapping code
	//First render
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	ConfigureShaderAndMatrices(); //ConfigureShaderMatrices();
	RenderScene();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//render with shadow mapping
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ConfigureShaderAndMatrices();
	glBindTexture(GL_TEXTURE_2D, depthMap);
	RenderScene();
	//RenderScene()?
	// remember to use at least one pop operation per push...
	mvStack.pop(); // initial matrix
	glDepthMask(GL_TRUE);

	SDL_GL_SwapWindow(window); // swap buffers
}


// Program entry point - SDL manages the actual WinMain entry point for us
int main(int argc, char *argv[]) {
	SDL_Window * hWindow; // window handle
	SDL_GLContext glContext; // OpenGL context handle
	hWindow = setupRC(glContext); // Create window and render context 

	// Required on Windows *only* init GLEW to access OpenGL beyond 1.1
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { // glewInit failed, something is seriously wrong
		std::cout << "glewInit failed, aborting." << endl;
		exit(1);
	}
	cout << glGetString(GL_VERSION) << endl;

	init();

	bool running = true; // set running to true
	SDL_Event sdlEvent;  // variable to detect SDL events
	while (running) {	// the event loop
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT)
				running = false;
		}
		update();
		draw(hWindow); // call the draw function
	}

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(hWindow);
	SDL_Quit();
	return 0;
}
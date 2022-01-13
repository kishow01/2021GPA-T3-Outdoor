#include "src\basic\SceneRenderer.h"
#include <GLFW\glfw3.h>
#include "SceneSetting.h"

#include <glm/gtx/transform.hpp>

#include "object.h"

#define SHADOW_MAP_SIZE 4096
#define PI 3.1415
#define deg2rad(x) ((x) * ((PI)/(180.0)))

#pragma comment (lib, "lib-vc2015\\glfw3.lib")
#pragma comment (lib, "assimp\\assimp-vc140-mtd.lib")

int FRAME_WIDTH = 1024;
int FRAME_HEIGHT = 768;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void cursorPosCallback(GLFWwindow* window, double x, double y);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void initializeGL();
void resizeGL(GLFWwindow *window, int w, int h);
void paintGL();
void updateState();
void adjustCameraPositionWithTerrain();
void updateAirplane(const glm::mat4 &viewMatrix);
void initScene();

bool m_leftButtonPressed = false;
bool m_rightButtonPressed = false;
double cursorPos[2];

SceneRenderer *m_renderer = nullptr;

glm::vec3 m_lookAtCenter;
glm::vec3 m_lookAtCenter_t;
glm::vec3 m_eye;
glm::vec3 m_eye_t;
glm::vec3 m_eye0ffset;

void vsyncEnabled(GLFWwindow *window);
void vsyncDisabled(GLFWwindow *window);

PlantManager *m_plantManager = nullptr;
Terrain *m_terrain = nullptr;

Object *plane = nullptr;

glm::vec3 plane_direction = glm::vec3(0.0f, 0.0f, -1.0f);

// the airplane's transformation has been handled
glm::vec3 m_airplanePosition;
glm::mat4 m_airplaneRotMat;

Object *house1 = nullptr;
Object *house2 = nullptr;

bool nm_mapping_enable = false;

glm::vec3 house1_position = glm::vec3(631, 130, 468);
glm::vec3 house2_position = glm::vec3(656, 135, 483);

float house1_rotation_angle = 60.0f;
float house2_rotation_angle = 15.0f;

Object *grass0 = nullptr;
Object *grass1 = nullptr;
Object *tree0_trunk = nullptr;
Object *tree0_leaves = nullptr;
Object *tree1_trunk = nullptr;
Object *tree1_leaves = nullptr;

// Directional light shadow
struct {
	GLuint fbo;
	GLuint depthMap;
} shadowBuffer;

struct {
	struct {
		GLuint   mvp;
		GLuint   type; 
	} light;
} uniforms;

// glm::vec3 light_position = glm::vec3(636.48, 134.79, 495.98);
glm::vec3 light_position = glm::vec3(0.2, 0.6, 0.5);

Shader *depthShader = nullptr;

int render_option = 0;

// Texture parameters
glm::vec3 PLANE_KA = glm::vec3(1.000000, 1.000000, 1.000000);
glm::vec3 PLANE_KD = glm::vec3(0.800000, 0.800000, 0.800000);
glm::vec3 PLANE_KS = glm::vec3(0.500000, 0.500000, 0.500000);

glm::vec3 HOUSE_KA = glm::vec3(1.000000, 1.000000, 1.000000);
glm::vec3 HOUSE_KD = glm::vec3(0.800000, 0.800000, 0.800000);
glm::vec3 HOUSE_KS = glm::vec3(0.500000, 0.500000, 0.500000);

glm::vec3 TREE_0_TRUNK_KA = glm::vec3(1.000000, 1.000000, 1.000000);
glm::vec3 TREE_0_TRUNK_KD = glm::vec3(0.640000, 0.640000, 0.640000);
glm::vec3 TREE_0_TRUNK_KS = glm::vec3(0.500000, 0.500000, 0.500000);

glm::vec3 TREE_1_TRUNK_KA = glm::vec3(1.000000, 1.000000, 1.000000);
glm::vec3 TREE_1_TRUNK_KD = glm::vec3(0.640000, 0.640000, 0.640000);
glm::vec3 TREE_1_TRUNK_KS = glm::vec3(0.500000, 0.500000, 0.500000);

glm::vec3 TREE_0_LEAVES_KA = glm::vec3(1.000000, 1.000000, 1.000000);
glm::vec3 TREE_0_LEAVES_KD = glm::vec3(0.640000, 0.640000, 0.640000);
glm::vec3 TREE_0_LEAVES_KS = glm::vec3(0.500000, 0.500000, 0.500000);

glm::vec3 TREE_1_LEAVES_KA = glm::vec3(1.000000, 1.000000, 1.000000);
glm::vec3 TREE_1_LEAVES_KD = glm::vec3(0.640000, 0.640000, 0.640000);
glm::vec3 TREE_1_LEAVES_KS = glm::vec3(0.500000, 0.500000, 0.500000);

glm::vec3 GRASS_0_KA = glm::vec3(1.000000, 1.000000, 1.000000);
glm::vec3 GRASS_0_KD = glm::vec3(0.588000, 0.588000, 0.588000);
glm::vec3 GRASS_0_KS = glm::vec3(0.000000, 0.000000, 0.000000);

glm::vec3 GRASS_1_KA = glm::vec3(1.000000, 1.000000, 1.000000);
glm::vec3 GRASS_1_KD = glm::vec3(0.203922, 0.588235, 0.176471);
glm::vec3 GRASS_1_KS = glm::vec3(0.000000, 0.000000, 0.000000);


int main(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	

	GLFWwindow *window = glfwCreateWindow(FRAME_WIDTH, FRAME_HEIGHT, "rendering", nullptr, nullptr);
	if (window == nullptr){
		std::cout << "failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// load OpenGL function pointer
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetKeyCallback(window, keyCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetFramebufferSizeCallback(window, resizeGL);

	initializeGL();

	//vsyncEnabled(window);

	// be careful v-sync issue
	glfwSwapInterval(0);
	vsyncDisabled(window);

	glfwTerminate();
	return 0;
}
void vsyncDisabled(GLFWwindow *window){
	float periodForEvent = 1.0 / 60.0;
	float accumulatedEventPeriod = 0.0;
	double previousTime = glfwGetTime();
	double previousTimeForFPS = glfwGetTime();
	int frameCount = 0;
	while (!glfwWindowShouldClose(window)){
		// measure speed
		double currentTime = glfwGetTime();
		float deltaTime = currentTime - previousTime;
		frameCount = frameCount + 1;
		if (currentTime - previousTimeForFPS >= 1.0){
			std::cout << "\rFPS: " << frameCount;
			frameCount = 0;
			previousTimeForFPS = currentTime;
		}
		previousTime = currentTime;

		// game loop
		accumulatedEventPeriod = accumulatedEventPeriod + deltaTime;
		if (accumulatedEventPeriod > periodForEvent){
			updateState();
			accumulatedEventPeriod = accumulatedEventPeriod - periodForEvent;
		}

		paintGL();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
void vsyncEnabled(GLFWwindow *window){
	double previousTimeForFPS = glfwGetTime();
	int frameCount = 0;
	while (!glfwWindowShouldClose(window)){
		// measure speed
		double currentTime = glfwGetTime();
		frameCount = frameCount + 1;
		if (currentTime - previousTimeForFPS >= 1.0){
			std::cout << "\rFPS: " << frameCount;
			frameCount = 0;
			previousTimeForFPS = currentTime;
		}
		
		updateState();
		paintGL();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void initializeGL(){
	depthShader = new Shader("src\\shader\\shadow.vs.glsl", "src\\shader\\shadow.fs.glsl");
	uniforms.light.mvp = glGetUniformLocation(depthShader->getProgramID(), "mvp");
	uniforms.light.type = glGetUniformLocation(depthShader->getProgramID(), "render_type");

	// Set Up Depth FBO
	glGenFramebuffers(1, &shadowBuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.fbo);

	glGenTextures(1, &shadowBuffer.depthMap);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer.depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowBuffer.depthMap, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowBuffer.depthMap, 0);

	m_renderer = new SceneRenderer();
	m_renderer->initialize(FRAME_WIDTH, FRAME_HEIGHT, depthShader->getProgramID());

	// m_eye = glm::vec3(512.0, 10.0, 512.0);
	// m_lookAtCenter = glm::vec3(512.0, 0.0, 500.0);
	m_lookAtCenter = glm::vec3(512.0, 185.0, 500.0);

	// m_eye = glm::vec3(512.0, 220.0, 450);
	m_eye0ffset = glm::vec3(0.0f, 35.0f, -50.0f);

	m_eye = m_lookAtCenter + m_eye0ffset;
	
	initScene();

	plane = new Object(0);
	plane->initialize(m_plantManager);

	house1 = new Object(1);
	house1->initialize(m_plantManager);

	house2 = new Object(1);
	house2->initialize(m_plantManager);

	// plant & grass init ...
	tree0_trunk = new Object(2);
	tree0_trunk->initialize(m_plantManager);

	tree0_leaves = new Object(6);
	tree0_leaves->initialize(m_plantManager);

	tree1_trunk = new Object(3);
	tree1_trunk->initialize(m_plantManager);

	tree1_leaves = new Object(7);
	tree1_leaves->initialize(m_plantManager);

	grass0 = new Object(4);
	grass0->initialize(m_plantManager);

	grass1 = new Object(5);
	grass1->initialize(m_plantManager);

	plane->ka = PLANE_KA;
	plane->kd = PLANE_KD;
	plane->ks = PLANE_KS;

	house1->ka = HOUSE_KA;
	house1->kd = HOUSE_KD;
	house1->ks = HOUSE_KS;

	house2->ka = HOUSE_KA;
	house2->kd = HOUSE_KD;
	house2->ks = HOUSE_KS;

	tree0_trunk->ka = TREE_0_TRUNK_KA;
	tree0_trunk->kd = TREE_0_TRUNK_KD;
	tree0_trunk->ks = TREE_0_TRUNK_KS;

	tree1_trunk->ka = TREE_1_TRUNK_KA;
	tree1_trunk->kd = TREE_1_TRUNK_KD;
	tree1_trunk->ks = TREE_1_TRUNK_KS;

	tree0_leaves->ka = TREE_0_LEAVES_KA;
	tree0_leaves->kd = TREE_0_LEAVES_KD;
	tree0_leaves->ks = TREE_0_LEAVES_KS;

	tree1_leaves->ka = TREE_1_LEAVES_KA;
	tree1_leaves->kd = TREE_1_LEAVES_KD;
	tree1_leaves->ks = TREE_1_LEAVES_KS;

	m_renderer->setProjection(glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f));
	plane->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	house1->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	house2->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	tree0_trunk->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	tree0_leaves->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	tree1_trunk->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	tree1_leaves->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	grass0->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	grass1->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
}
void resizeGL(GLFWwindow *window, int w, int h){
	FRAME_WIDTH = w;
	FRAME_HEIGHT = h;
	m_renderer->resize(w, h);
	m_renderer->setProjection(glm::perspective(glm::radians(60.0f), w * 1.0f / h, 0.1f, 1000.0f));
	plane->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	house1->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	house2->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	tree0_trunk->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	tree0_leaves->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	tree1_trunk->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	tree1_leaves->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	grass0->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
	grass1->um4p = glm::perspective(glm::radians(60.0f), FRAME_WIDTH * 1.0f / FRAME_HEIGHT, 0.1f, 1000.0f);
}


void updateState(){	
	// [TODO] update your eye position and look-at center here
	// m_eye = ... ;
	// m_lookAtCenter = ... ;

	// m_lookAtCenter = m_lookAtCenter - plane_direction;
	// m_eye = m_eye - plane_direction;
	if(render_option == 0) {
		m_lookAtCenter = m_lookAtCenter - plane_direction;
		m_eye = m_lookAtCenter + m_eye0ffset;
	}

	/* Testing Normal Mapping */
	if(render_option == 1) {
		m_lookAtCenter_t = m_lookAtCenter_t - plane_direction;
		m_eye_t = m_eye_t - plane_direction;

		m_lookAtCenter = house1_position;
		m_eye = house1_position + glm::vec3(-35.0, 40.0, -35.0);
	}

	
	// adjust camera position with terrain
	adjustCameraPositionWithTerrain();	

	// calculate camera matrix
	glm::mat4 vm = glm::lookAt(m_eye, m_lookAtCenter, glm::vec3(0.0, 1.0, 0.0)); 

	// [IMPORTANT] set camera information to renderer
	m_renderer->setView(vm, m_eye);
	plane->um4v = vm;
	house1->um4v = vm;
	house2->um4v = vm;
	tree0_trunk->um4v = vm;
	tree0_leaves->um4v = vm;
	tree1_trunk->um4v = vm;
	tree1_leaves->um4v = vm;
	grass0->um4v = vm;
	grass1->um4v = vm;

	// update airplane
	updateAirplane(vm);	
}
void paintGL(){
	static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	static const GLfloat ones[] = { 1.0f };

	glClearBufferfv(GL_COLOR, 0, black);
	glClearBufferfv(GL_DEPTH, 0, ones);

	const float shadow_range = 100.0f;
	glm::mat4 light_proj_matrix = glm::ortho(-shadow_range, shadow_range, -shadow_range, shadow_range, 0.1f, 5000.0f);
	glm::mat4 light_view_matrix = glm::lookAt(light_position, m_lookAtCenter, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 light_vp_matrix = light_proj_matrix * light_view_matrix;

	glm::mat4 scale_bias_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	glm::mat4 shadow_sbpv_matrix = scale_bias_matrix * light_vp_matrix;

	plane->um4m = glm::translate(glm::mat4(1.0f), m_airplanePosition) * m_airplaneRotMat;
	// plane->um4m = glm::translate(glm::mat4(1.0f), m_airplanePosition);
	house1->um4m = glm::rotate(glm::translate(glm::mat4(1.0f), house1_position), house1_rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	house2->um4m = glm::rotate(glm::translate(glm::mat4(1.0f), house2_position), house2_rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	
	tree0_trunk->um4m = glm::mat4(1.0f);
	tree0_leaves->um4m = glm::mat4(1.0f);
	tree1_trunk->um4m = glm::mat4(1.0f);
	tree1_leaves->um4m = glm::mat4(1.0f);
	grass0->um4m = glm::mat4(1.0f);
	grass1->um4m = glm::mat4(1.0f);

	///////////// Code: DrawCall #1 Rendering Depth Map From Light View /////////////
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.fbo);
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
	depthShader->useShader();
	glClear(GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	// glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);

	// m_renderer->renderLight(uniforms.light.mvp, light_vp_matrix);

	glUniform1i(uniforms.light.type, 0);
	plane->renderLight(uniforms.light.mvp, light_vp_matrix);
	house1->renderLight(uniforms.light.mvp, light_vp_matrix);
	house2->renderLight(uniforms.light.mvp, light_vp_matrix);
	
	/*
	glUniform1i(uniforms.light.type, 1);
	tree0_trunk->renderLight(uniforms.light.mvp, light_vp_matrix);
	tree0_leaves->renderLight(uniforms.light.mvp, light_vp_matrix);
	tree1_trunk->renderLight(uniforms.light.mvp, light_vp_matrix);
	tree1_leaves->renderLight(uniforms.light.mvp, light_vp_matrix);
	*/
	/* 植物應該不需要畫陰影 */
	// grass0->renderLight(uniforms.light.mvp, light_vp_matrix);
	// grass1->renderLight(uniforms.light.mvp, light_vp_matrix);

	depthShader->disableShader();
	glDisable(GL_POLYGON_OFFSET_FILL);

	///////////// Code: DrawCall #2 Rendering Object from Camera View /////////////
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	house1->enable_normal_mapping = nm_mapping_enable;
	house2->enable_normal_mapping = nm_mapping_enable;

	m_renderer->renderPass(shadowBuffer.depthMap, shadow_sbpv_matrix);

	if(render_option == 0) {
		plane->shadow_matrix = shadow_sbpv_matrix * plane->um4m;
		plane->renderPass(shadowBuffer.depthMap);
	}
	house1->shadow_matrix = shadow_sbpv_matrix * house1->um4m;
	house1->renderPass(shadowBuffer.depthMap);
	house2->shadow_matrix = shadow_sbpv_matrix * house2->um4m;
	house2->renderPass(shadowBuffer.depthMap);

	tree0_trunk->shadow_matrix = shadow_sbpv_matrix * tree0_trunk->um4m;
	tree0_trunk->renderPass(shadowBuffer.depthMap);

	tree0_leaves->shadow_matrix = shadow_sbpv_matrix * tree0_leaves->um4m;
	tree0_leaves->renderPass(shadowBuffer.depthMap);

	tree1_trunk->shadow_matrix = shadow_sbpv_matrix * tree1_trunk->um4m;
	tree1_trunk->renderPass(shadowBuffer.depthMap);

	tree1_leaves->shadow_matrix = shadow_sbpv_matrix * tree1_leaves->um4m;
	tree1_leaves->renderPass(shadowBuffer.depthMap);

	// 植物應該不需要畫陰影，可以忽略 shadow matrix 直接畫 
	// grass0->shadow_matrix = shadow_sbpv_matrix * grass0->um4m;
	grass0->renderPass(shadowBuffer.depthMap);
	// grass1->shadow_matrix = shadow_sbpv_matrix * grass1->um4m;
	grass1->renderPass(shadowBuffer.depthMap);
	
}

////////////////////////////////////////////////
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods){}
void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {}
void cursorPosCallback(GLFWwindow* window, double x, double y){
	cursorPos[0] = x;
	cursorPos[1] = y;
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(key == 'Q' || key == 'q') {
		glm::mat4 m = glm::rotate(glm::mat4(1.0f), 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
		plane_direction = glm::normalize(glm::vec3(m * glm::vec4(plane_direction, 1.0f)));
		
		// Rotate m_eye around m_lookAtCenter
		m = glm::rotate(glm::mat4(1.0f), 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
		m_eye0ffset = glm::vec3(m * glm::vec4(m_eye0ffset, 1.0f));
		
	} else if(key == 'E' || key == 'e') {
		glm::mat4 m = glm::rotate(glm::mat4(1.0f), -0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
		plane_direction = glm::normalize(glm::vec3(m * glm::vec4(plane_direction, 1.0f)));

		// Rotate m_eye around m_lookAtCenter
		m = glm::rotate(glm::mat4(1.0f), -0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
		m_eye0ffset = glm::vec3(m * glm::vec4(m_eye0ffset, 1.0f));
	} else if((key == 'Z' || key == 'z') && action == GLFW_PRESS)
		nm_mapping_enable = !nm_mapping_enable;
	else if ((key == 'C' || key == 'c') && action == GLFW_PRESS) {
		if (render_option == 0) {
			m_lookAtCenter_t = m_lookAtCenter;
			m_eye_t = m_eye;
		} else if(render_option == 1) {
			m_lookAtCenter = m_lookAtCenter_t;
			m_eye = m_eye_t;
		}


		render_option = (render_option + 1) % 2;
	}
}

////////////////////////////////////////////////
// The following functions don't need to change
void initScene() {
	m_plantManager = new PlantManager();		
	m_terrain = new Terrain();
	m_renderer->appendTerrain(m_terrain);
}
void updateAirplane(const glm::mat4 &viewMatrix) {
	// apply yaw
	glm::mat4 rm = viewMatrix;
	rm = glm::transpose(rm);
	glm::vec3 tangent(-1.0 * rm[2].x, 0.0, -1.0 * rm[2].z);
	tangent = glm::normalize(tangent);
	glm::vec3 bitangent = glm::normalize(glm::cross(glm::vec3(0.0, 1.0, 0.0), tangent));
	glm::vec3 normal = glm::normalize(glm::cross(tangent, bitangent));

	m_airplaneRotMat[0] = glm::vec4(bitangent, 0.0);
	m_airplaneRotMat[1] = glm::vec4(normal, 0.0);
	m_airplaneRotMat[2] = glm::vec4(tangent, 0.0);
	m_airplaneRotMat[3] = glm::vec4(0.0, 0.0, 0.0, 1.0);

	m_airplanePosition = m_lookAtCenter;
}

// adjust camera position and look-at center with terrain
void adjustCameraPositionWithTerrain() {
	// adjust camera height
	glm::vec3 cp = m_lookAtCenter;
	float ty = m_terrain->getHeight(cp.x, cp.z);
	if (cp.y < ty + 3.0) {
		m_lookAtCenter.y = ty + 3.0;
		m_eye.y = m_eye.y + (ty + 3.0 - cp.y);
	}
	cp = m_eye;
	ty = m_terrain->getHeight(cp.x, cp.z);
	if (cp.y < ty + 3.0) {
		m_lookAtCenter.y = ty + 3.0;
		m_eye.y = m_eye.y + (ty + 3.0 - cp.y);
	}
}
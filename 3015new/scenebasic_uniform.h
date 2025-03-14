#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/Texture.h"
#include "helper/Camera.h"
#include "helper/SimpleModel.h"
#include <GLFW/glfw3.h>

class SceneBasic_Uniform : public Scene
{
private:
    GLuint vaoHandle;
    GLSLProgram prog;
    float angle;

	// frame stats
	float gFrameRate = 60.0f;

	// scene content
	GLSLProgram gNormalMapShader;	// shader program object
	GLSLProgram gBasicLightingShader;	// shader program object
	GLSLProgram gCubemapShader;
	GLSLProgram gColorShader;
	GLuint gVBO = 0;		// vertex buffer object identifier
	GLuint gVAO = 0;		// vertex array object identifier
	GLuint lineVAO = 0;
	GLuint lineVBO = 0;

	glm::mat4 gViewMatrix;			// view matrix
	glm::mat4 gProjectionMatrix;	// projection matrix
	std::map<std::string, glm::mat4> gModelMatrix;			// object matrix

	glm::mat4 gOrthoMatrix;

	Light gLight;					// light properties
	Material gMaterial;				// material properties
	std::map<std::string, Texture> gTexture;	// texture objects

	float rotateAngle = 0.0f;

	// controls
	bool gWireframe = false;	// wireframe control

	bool enableMultipleViews = false;

	Camera topRightCamera{ glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -90.0f };
	Camera bottomLeftCamera{ glm::vec3(-0.5f, 1.5f, 2.5f), glm::vec3(0.0f, 1.0f, 0.0f) };

	SimpleModel gTorusModel;		// scene object model
	SimpleModel gCubeModel;		// scene object model

	Texture gCubeEnvMap;			// cube environment map

	GLFWwindow* window;

	float frameTime = 0.0f;

	float lastFrame = 0.0f;
public:
	glm::vec2 lastMousePosition;

	Camera mainCamera{ glm::vec3(0.0f, 5.0f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -45.0f };

	bool rightMouseButtonDown = false;

	float rotateSpeed = 90.0f;

	float cubemapBlendFactor = 1.0f;

	bool animateLight = false;
private:

    void compile();

	void updateLigthPosition();

	void updateFPS();

	void drawWall(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, Texture& texture, Texture& normalMap);
	void drawModel(GLSLProgram& shader, SimpleModel& model, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, Texture& texture, Texture& normalMap);

	void render_scene(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
public:
    SceneBasic_Uniform();

    void initScene(GLFWwindow* inWindow);
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H

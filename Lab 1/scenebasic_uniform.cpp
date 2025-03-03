#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include "helper/utilities.h"

using glm::vec3;

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f) {}

void SceneBasic_Uniform::initScene(GLFWwindow* inWindow)
{
	window = inWindow;

	glEnable(GL_DEPTH_TEST);	// enable depth buffer test

	glDepthFunc(GL_LESS);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetWindowUserPointer(window, this);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

    compile();

    std::cout << std::endl;

    prog.printActiveUniforms();

    /////////////////// Create the VBO ////////////////////
    float positionData[] = {
        -0.8f, -0.8f, 0.0f,
         0.8f, -0.8f, 0.0f,
         0.0f,  0.8f, 0.0f };
    float colorData[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f };

    // Create and populate the buffer objects
    GLuint vboHandles[2];
    glGenBuffers(2, vboHandles);
    GLuint positionBufferHandle = vboHandles[0];
    GLuint colorBufferHandle = vboHandles[1];

    glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), positionData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), colorData, GL_STATIC_DRAW);

    // Create and set-up the vertex array object
    glGenVertexArrays( 1, &vaoHandle );
    glBindVertexArray(vaoHandle);

    glEnableVertexAttribArray(0);  // Vertex position
    glEnableVertexAttribArray(1);  // Vertex color

    #ifdef __APPLE__
        glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL );

        glBindBuffer(GL_ARRAY_BUFFER, colorBufferHandle);
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL );
    #else
    		glBindVertexBuffer(0, positionBufferHandle, 0, sizeof(GLfloat)*3);
    		glBindVertexBuffer(1, colorBufferHandle, 0, sizeof(GLfloat)*3);

    		glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
    		glVertexAttribBinding(0, 0);
    		glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 0);
    	  glVertexAttribBinding(1, 1);
    #endif
    glBindVertexArray(0);

	// compile and link a vertex and fragment shader pair
	gNormalMapShader.compileShader("shader/normalMap.vert");
	gNormalMapShader.compileShader("shader/normalMap.frag");
	gNormalMapShader.link();

	gBasicLightingShader.compileShader("shader/basicLighting.vert");
	gBasicLightingShader.compileShader("shader/basicLighting.frag");
	gBasicLightingShader.link();

	gCubemapShader.compileShader("shader/lighting.vert");
	gCubemapShader.compileShader("shader/lighting_cubemap.frag");
	gCubemapShader.link();

	gColorShader.compileShader("shader/modelViewProj.vert");
	gColorShader.compileShader("shader/color.frag");
	gColorShader.link();


	// initialise view matrix
	gViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	gViewMatrix = mainCamera.GetViewMatrix();

	// initialise projection matrix
	gProjectionMatrix = glm::perspective(glm::radians(45.0f),
		static_cast<float>(width) / height, 0.1f, 100.0f);

	gOrthoMatrix = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), 0.1f, 10.0f);

	// initialise point light properties
	gLight.pos = glm::vec3(0.0f, 3.0f, 0.0f);
	gLight.dir = glm::vec3(0.3f, -0.7f, -0.5f);
	gLight.La = glm::vec3(0.3f);
	gLight.Ld = glm::vec3(1.0f);
	gLight.Ls = glm::vec3(1.0f);
	gLight.att = glm::vec3(1.0f, 0.0f, 0.0f);

	// initialise material properties
	gMaterial.Ka = glm::vec3(0.2f);
	gMaterial.Kd = glm::vec3(1.0f, 1.0f, 1.0f);
	gMaterial.Ks = glm::vec3(0.2f, 0.7f, 1.0f);
	gMaterial.shininess = 40.0f;

	// initialise model matrices
	gModelMatrix["BackWall1"] = glm::translate(glm::vec3(-2.0f, 0.0f, -3.0f));
	gModelMatrix["BackWall2"] = glm::translate(glm::vec3(0.0f, 0.0f, -3.0f));
	gModelMatrix["BackWall3"] = glm::translate(glm::vec3(2.0f, 0.0f, -3.0f));
	gModelMatrix["BackWall4"] = glm::translate(glm::vec3(-2.0f, 2.0f, -3.0f));
	gModelMatrix["BackWall5"] = glm::translate(glm::vec3(0.0f, 2.0f, -3.0f));
	gModelMatrix["BackWall6"] = glm::translate(glm::vec3(2.0f, 2.0f, -3.0f));

	gModelMatrix["FrontWall1"] = glm::translate(glm::vec3(-2.0f, 0.0f, 3.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["FrontWall2"] = glm::translate(glm::vec3(0.0f, 0.0f, 3.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["FrontWall3"] = glm::translate(glm::vec3(2.0f, 0.0f, 3.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["FrontWall4"] = glm::translate(glm::vec3(-2.0f, 2.0f, 3.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["FrontWall5"] = glm::translate(glm::vec3(0.0f, 2.0f, 3.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["FrontWall6"] = glm::translate(glm::vec3(2.0f, 2.0f, 3.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	gModelMatrix["LeftWall1"] = glm::translate(glm::vec3(-3.0f, 0.0f, -2.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["LeftWall2"] = glm::translate(glm::vec3(-3.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["LeftWall3"] = glm::translate(glm::vec3(-3.0f, 0.0f, 2.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["LeftWall4"] = glm::translate(glm::vec3(-3.0f, 2.0f, -2.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["LeftWall5"] = glm::translate(glm::vec3(-3.0f, 2.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["LeftWall6"] = glm::translate(glm::vec3(-3.0f, 2.0f, 2.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	gModelMatrix["RightWall1"] = glm::translate(glm::vec3(3.0f, 0.0f, -2.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["RightWall2"] = glm::translate(glm::vec3(3.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["RightWall3"] = glm::translate(glm::vec3(3.0f, 0.0f, 2.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["RightWall4"] = glm::translate(glm::vec3(3.0f, 2.0f, -2.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["RightWall5"] = glm::translate(glm::vec3(3.0f, 2.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	gModelMatrix["RightWall6"] = glm::translate(glm::vec3(3.0f, 2.0f, 2.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 1.0f));
	auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	auto translation = glm::translate(glm::vec3(0.0f, -0.5f, 0.0f));

	gModelMatrix["Floor"] = translation * rotation * scale;

	gModelMatrix["Torus"] = glm::translate(glm::vec3(-1.0f, 0.0f, -1.0f));
	gModelMatrix["Cube"] = glm::translate(glm::vec3(1.0f, 0.0f, 1.0f));

	// load texture and normal map
	gTexture["Stone"].generate("./media/images/Fieldstone.bmp");
	gTexture["StoneNormalMap"].generate("./images/FieldstoneBumpDOT3.bmp");

	gTexture["White"].generate("./media/images/White.bmp");
	gTexture["WhiteNormalMap"].generate("./media/images/WhiteBumpDOT3.bmp");

	gTexture["Crate"].generate("./media/images/diffuse.bmp");

	// load cube environment map texture
	gCubeEnvMap.generate(
		"./media/images/cm_front.bmp", "./media/images/cm_back.bmp",
		"./media/images/cm_left.bmp", "./media/images/cm_right.bmp",
		"./media/images/cm_top.bmp", "./media/images/cm_bottom.bmp");

	// load model
	gTorusModel.loadModel("./media/models/torus.obj", false);
	gCubeModel.loadModel("./media/models/cube.obj", true);

	// vertex positions, normals, tangents and texture coordinates
	std::vector<GLfloat> vertices =
	{
		-1.0f, -1.0f, 0.0f,	// vertex 0: position
		0.0f, 0.0f, 1.0f,	// vertex 0: normal
		1.0f, 0.0f, 0.0f,	// vertex 0: tangent
		0.0f, 0.0f,			// vertex 0: texture coordinate
		1.0f, -1.0f, 0.0f,	// vertex 1: position
		0.0f, 0.0f, 1.0f,	// vertex 1: normal
		1.0f, 0.0f, 0.0f,	// vertex 1: tangent
		1.0f, 0.0f,			// vertex 1: texture coordinate
		-1.0f, 1.0f, 0.0f,	// vertex 2: position
		0.0f, 0.0f, 1.0f,	// vertex 2: normal
		1.0f, 0.0f, 0.0f,	// vertex 2: tangent
		0.0f, 1.0f,			// vertex 2: texture coordinate
		1.0f, 1.0f, 0.0f,	// vertex 3: position
		0.0f, 0.0f, 1.0f,	// vertex 3: normal
		1.0f, 0.0f, 0.0f,	// vertex 3: tangent
		1.0f, 1.0f,			// vertex 3: texture coordinate
	};

	// create VBO
	glGenBuffers(1, &gVBO);					// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO);			// generate unused VAO identifier
	glBindVertexArray(gVAO);				// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);	// bind the VBO

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, position)));		// specify format of position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, normal)));		// specify format of colour data
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, tangent)));		// specify format of tangent data
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, texCoord)));		// specify format of texture coordinate data

	glEnableVertexAttribArray(0);	// enable vertex attributes
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	float lineVertices[] = {
		// lines
		0.0f, 600.0f, 0.0f,		// line 1 vertex 0: position
		1.0f, 1.0f, 1.0f,		// line 1 vertex 0: colour
		1600.0f, 600.0f, 0.0f,	// line 1 vertex 1: position
		1.0f, 1.0f, 1.0f,		// line 1 vertex 1: colour
		800.0f, 0.0f, 0.0f,		// line 2 vertex 0: position
		1.0f, 1.0f, 1.0f,		// line 2 vertex 0: colour
		800.0f, 1200.0f, 0.0f,	// line 2 vertex 1: position
		1.0f, 1.0f, 1.0f,		// line 2 vertex 1: colour
	};

	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &lineVAO);			// generate unused VAO identifier
	glBindVertexArray(lineVAO);				// create VAO

	// create VBO
	glGenBuffers(1, &lineVBO);					// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor),
		reinterpret_cast<void*>(offsetof(VertexColor, position)));		// specify format of position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor),
		reinterpret_cast<void*>(offsetof(VertexColor, color)));		// specify format of colour data

	glEnableVertexAttribArray(0);	// enable vertex attributes
	glEnableVertexAttribArray(1);
}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update( float t )
{
	//update your angle here
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(Camera_Movement::FORWARD, gFrameTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(Camera_Movement::BACKWARD, gFrameTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(Camera_Movement::LEFT, gFrameTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(Camera_Movement::RIGHT, gFrameTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(Camera_Movement::DOWN, gFrameTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		mainCamera.ProcessKeyboard(Camera_Movement::UP, gFrameTime);

	rotateAngle += rotateSpeed * gFrameTime;
	gViewMatrix = mainCamera.GetViewMatrix();
}

void SceneBasic_Uniform::drawWall(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, Texture& texture, Texture& normalMap)
{
	// calculate matrices
	glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));

	// set uniform variables
	gNormalMapShader.setUniform("uModelViewProjectionMatrix", MVP);
	gNormalMapShader.setUniform("uModelMatrix", modelMatrix);
	gNormalMapShader.setUniform("uNormalMatrix", normalMatrix);

	// set texture and normal map
	gNormalMapShader.setUniform("uTextureSampler", 0);
	gNormalMapShader.setUniform("uNormalSampler", 1);

	glActiveTexture(GL_TEXTURE0);
	texture.bind();

	glActiveTexture(GL_TEXTURE1);
	normalMap.bind();

	glBindVertexArray(gVAO);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertices
}

void SceneBasic_Uniform::drawModel(GLSLProgram& shader, SimpleModel& model, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, Texture& texture, Texture& normalMap)
{
	// calculate matrices
	glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));

	// set uniform variables
	shader.setUniform("uModelViewProjectionMatrix", MVP);
	shader.setUniform("uModelMatrix", modelMatrix);
	shader.setUniform("uNormalMatrix", normalMatrix);

	// set texture and normal map
	shader.setUniform("uTextureSampler", 0);
	shader.setUniform("uNormalSampler", 1);

	glActiveTexture(GL_TEXTURE0);
	texture.bind();

	glActiveTexture(GL_TEXTURE1);
	normalMap.bind();

	model.drawModel();
}

void SceneBasic_Uniform::render_scene(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
	// 正常渲染场景
	// use the shaders associated with the shader program
	gNormalMapShader.use();

	// set light properties
	gNormalMapShader.setUniform("uLight.pos", gLight.pos);
	gNormalMapShader.setUniform("uLight.La", gLight.La);
	gNormalMapShader.setUniform("uLight.Ld", gLight.Ld);
	gNormalMapShader.setUniform("uLight.Ls", gLight.Ls);
	gNormalMapShader.setUniform("uLight.att", gLight.att);

	// set material properties
	gNormalMapShader.setUniform("uMaterial.Ka", gMaterial.Ka);
	gNormalMapShader.setUniform("uMaterial.Kd", gMaterial.Kd);
	gNormalMapShader.setUniform("uMaterial.Ks", gMaterial.Ks);
	gNormalMapShader.setUniform("uMaterial.shininess", gMaterial.shininess);

	// set viewing position
	gNormalMapShader.setUniform("uViewpoint", glm::vec3(0.0f, 0.0f, 4.0f));

	Texture& floorTexture = gTexture["White"];
	Texture& floorNormalMap = gTexture["WhiteNormalMap"];

	Texture& wallTexture = gTexture["Stone"];
	Texture& wallNormalMap = gTexture["StoneNormalMap"];

	drawWall(gModelMatrix["BackWall1"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["BackWall2"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["BackWall3"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["BackWall4"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["BackWall5"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["BackWall6"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);

	drawWall(gModelMatrix["FrontWall1"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["FrontWall2"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["FrontWall3"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["FrontWall4"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["FrontWall5"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["FrontWall6"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);

	drawWall(gModelMatrix["LeftWall1"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["LeftWall2"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["LeftWall3"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["LeftWall4"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["LeftWall5"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["LeftWall6"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);

	drawWall(gModelMatrix["RightWall1"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["RightWall2"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["RightWall3"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["RightWall4"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["RightWall5"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);
	drawWall(gModelMatrix["RightWall6"], viewMatrix, projectionMatrix, wallTexture, wallNormalMap);

	// use the shaders associated with the shader program
	gBasicLightingShader.use();

	// set light properties
	gBasicLightingShader.setUniform("uLight.pos", gLight.pos);
	gBasicLightingShader.setUniform("uLight.La", gLight.La);
	gBasicLightingShader.setUniform("uLight.Ld", gLight.Ld);
	gBasicLightingShader.setUniform("uLight.Ls", gLight.Ls);
	gBasicLightingShader.setUniform("uLight.att", gLight.att);

	// set material properties
	gBasicLightingShader.setUniform("uMaterial.Ka", gMaterial.Ka);
	gBasicLightingShader.setUniform("uMaterial.Kd", gMaterial.Kd);
	gBasicLightingShader.setUniform("uMaterial.Ks", gMaterial.Ks);
	gBasicLightingShader.setUniform("uMaterial.shininess", gMaterial.shininess);

	// set viewing position
	gBasicLightingShader.setUniform("uViewpoint", glm::vec3(0.0f, 0.0f, 4.0f));

	auto modelMatrix = glm::translate(glm::vec3(1.0f, 1.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

	Texture& crateTexture = gTexture["Crate"];

	drawModel(gBasicLightingShader, gCubeModel, modelMatrix, viewMatrix, projectionMatrix, crateTexture, crateTexture);

	// use the shaders associated with the shader program
	gCubemapShader.use();

	// set light properties
	gCubemapShader.setUniform("uLight.dir", gLight.dir);

	gCubemapShader.setUniform("uLight.La", gLight.La);
	gCubemapShader.setUniform("uLight.Ld", gLight.Ld);
	gCubemapShader.setUniform("uLight.Ls", gLight.Ls);
	gCubemapShader.setUniform("uLight.att", gLight.att);

	// set material properties
	gCubemapShader.setUniform("uMaterial.Ka", gMaterial.Ka);
	gCubemapShader.setUniform("uMaterial.Kd", gMaterial.Kd);
	gCubemapShader.setUniform("uMaterial.Ks", gMaterial.Ks);
	gCubemapShader.setUniform("uMaterial.shininess", gMaterial.shininess);

	// set viewing position
	gCubemapShader.setUniform("uViewpoint", glm::vec3(0.0f, 0.0f, 4.0f));

	// set cube environment map
	gCubemapShader.setUniform("uEnvironmentMap", 0);

	modelMatrix = glm::translate(glm::vec3(-1.0f, 1.0f, -1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.8f));

	auto rotation = glm::rotate(glm::radians(rotateAngle), glm::vec3(1.0f, 0.0f, 0.0f));

	modelMatrix *= rotation;

	gCubemapShader.setUniform("cubemapBlendFactor", cubemapBlendFactor);

	// render model
	drawModel(gCubemapShader, gTorusModel, modelMatrix, viewMatrix, projectionMatrix, gCubeEnvMap, gCubeEnvMap);

	// 绘制地板
	gNormalMapShader.use();

	drawWall(gModelMatrix["Floor"], viewMatrix, projectionMatrix, floorTexture, floorNormalMap);

	// flush the graphics pipeline
	glFlush();
}

// key press or release callback function
void SceneBasic_Uniform::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// close the window when the ESCAPE key is pressed
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
}

// mouse movement callback function
void SceneBasic_Uniform::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	auto app = reinterpret_cast<SceneBasic_Uniform*>(glfwGetWindowUserPointer(window));

	auto deltaX = static_cast<float>(xpos - app->lastMousePosition.x);
	auto deltaY = static_cast<float>(app->lastMousePosition.y - ypos);

	if (app->rightMouseButtonDown)
	{
		app->mainCamera.ProcessMouseMovement(deltaX, deltaY);
	}

	app->lastMousePosition.x = static_cast<float>(xpos);
	app->lastMousePosition.y = static_cast<float>(ypos);
}

// mouse button callback function
void SceneBasic_Uniform::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	auto app = reinterpret_cast<SceneBasic_Uniform*>(glfwGetWindowUserPointer(window));

	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
	{
		app->rightMouseButtonDown = true;
	}

	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE)
	{
		app->rightMouseButtonDown = false;
	}
}

void SceneBasic_Uniform::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glViewport(0, 0, width, height);

	render_scene(mainCamera.GetViewMatrix(), gProjectionMatrix);			// render the scene

    glBindVertexArray(0);
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0,0,w,h);
}

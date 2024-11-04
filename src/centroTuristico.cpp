/*
* Código del proyecto
*/

#include <iostream>
#include <stdlib.h>

// GLAD: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator
// https://glad.dav1d.de/
#include <glad/glad.h>

// GLFW: https://www.glfw.org/
#include <GLFW/glfw3.h>

// GLM: OpenGL Math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Model loading classes (clases creadas por el profesor que proporcionan recursos para camara, dibujado, animacion e iluminacion)
#include <shader_m.h>
#include <camera.h>
#include <model.h>
#include <animatedmodel.h>
#include <material.h>
#include <light.h>
#include <cubemap.h>

#include <irrKlang.h>
using namespace irrklang;

// Functions 
bool Start(); //(se declara la función que inicializa las variables, metódos, dependecias, etc)
bool Update(); //(la función Update es el ciclo de renderizado)

// Definición de callbacks (Funciones que procesan la interacción con el puerto de vista, gestión del la ventana y con los periféricos)
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// Globals
GLFWwindow* window;

// Tamaño en pixeles de la ventana
const unsigned int SCR_WIDTH = 1920;//10124
const unsigned int SCR_HEIGHT = 1080;//768

// Definición de cámaras distintas (posición en XYZ)
Camera camera(glm::vec3(0.0f, 15.0f, 0.0f));
Camera cameraair(glm::vec3(0.0f, 0.0f, 0.0f));
float orthoSize = 200.0f; // Ajusta este valor para abarcar más área sin cambiar el nivel de zoom
float aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
// selección de cámara
bool    activeCamera = 1; // activamos la primera cámara

// Controladores para el movimiento del mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Variables para la velocidad de reproducción de las animaciones
float deltaTime = 0.0f; //Camara
float lastFrame = 0.0f;
float elapsedTime = 0.0f;
glm::vec3 position(0.0f,0.0f, 0.0f);
glm::vec3 forwardView(0.0f, 0.0f, 1.0f);
float     trdpersonOffset = 1.5f;
float     scaleV = 0.025f;
float     rotateCharacter = 0.0f;

//Animación simple
float	  door_rotation2 = 0.0f;
float	  door_rotation = 0.0f;

// Shaders
Shader *mLightsShader;
Shader *proceduralShader;
Shader* wavesShader;
Shader *cubemapShader;
Shader *dynamicShader;
Shader* staticShader;


// Apuntadores a la información de los modelo en fbx (carga la información del modelo)
Model	*terrain;
Model	*river;
Model	*sun;
Model	*cascade;
Model	*lamp;
Model   *decores;
Model	*puerta1;
Model	*puerta2;

// Apuntadores a la información del modelos animados con KeyFrames, ya que requieren una clase distinta (carga la información del modelo)
AnimatedModel   *character01;
//Se desconoce para que se usan estas variables
float tradius = 10.0f;
float theta = 0.0f;
float alpha = 0.0f;

// Cubemap (apuntador para cargar un cubeMap y su información, es decir, texturas)
CubeMap *mainCubeMap;

// Materiales
Material material01;
Material material;
Light    light;
Light    artificial;

float proceduralTime = 0.0f;
float wavesTime = 0.0f;
float riverTime = 0.0f;

// Audio
ISoundEngine *SoundEngine = createIrrKlangDevice();


// Entrada a función principal
int main()
{
	if (!Start()) //Si no se puede inicializar la función, se regresa -1
		return -1;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window)) //Ejecuta el ciclo de renderizado hasta que se cierra la ventana
	{
		if (!Update())
			break;
	}

	glfwTerminate(); //Termina dependencias de GLFW al cerrar el programa y regresa 0 en terminal
	return 0;

}

//FUNCIÓN QUE CARGA, CONFIGURA E INICIALIZA LA ESCENA
bool Start() {
	// Inicialización de GLFW

	//Metodos para prepara la ventana (no estoy seguro)
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creación de la ventana con GLFW
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Animation", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Ocultar el cursor mientras se rota la escena
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: Cargar todos los apuntadores
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	// Activación de buffer de profundidad
	glEnable(GL_DEPTH_TEST);

	// Compilación y enlace de shaders
	mLightsShader = new Shader("shaders/11_BasicPhongShader.vs", "shaders/11_BasicPhongShader.fs");
	proceduralShader = new Shader("shaders/12_ProceduralAnimation.vs", "shaders/12_ProceduralAnimation.fs");
	wavesShader = new Shader("shaders/13_wavesAnimation.vs", "shaders/13_wavesAnimation.fs");
	cubemapShader = new Shader("shaders/10_vertex_cubemap.vs", "shaders/10_fragment_cubemap.fs");
	dynamicShader = new Shader("shaders/10_vertex_skinning-IT.vs", "shaders/10_fragment_skinning-IT.fs");
	staticShader = new Shader("shaders/10_vertex_simple.vs", "shaders/10_fragment_simple.fs");

	// Máximo número de huesos: 100
	dynamicShader->setBonesIDs(MAX_RIGGING_BONES);

	// Dibujar en malla de alambre
	// glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	//Se dirige los apuntadores a la información de los modelos, mediante la ruta a estos y el constrcutor
	terrain = new Model("models/escena3.fbx");
	river = new Model("models/river.fbx");
	sun = new Model("models/sol.fbx");
	cascade = new Model("models/cascade.fbx");
	character01 = new AnimatedModel("models/empanim.fbx");
	lamp = new Model("models/lampcolo.fbx");
	decores = new Model("models/resinilu.fbx");
	puerta1 = new Model("models/puerta1.fbx");
	puerta2 = new Model("models/puerta2.fbx");

	// Cubemap (Se inizializa el cubemap y se pasan las rutas de las texturas del mismo )
	vector<std::string> faces
	{
		"textures/cubemap/02/posx.png",
		"textures/cubemap/02/negx.png",
		"textures/cubemap/02/posy.png",
		"textures/cubemap/02/negy.png",
		"textures/cubemap/02/posz.png",
		"textures/cubemap/02/negz.png"
	};
	mainCubeMap = new CubeMap();
	mainCubeMap->loadCubemap(faces);

	//Configuracuón de una de las camaras al inicializar la escena
	cameraair.SetAerialView(300.0f);

	// Configuración para la luz direccional (sol)
	light.Position = glm::vec3(0.0f, 300.0f, 0.0f);
	light.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Power = glm::vec4(60.0f, 60.0f, 60.0f, 1.0f);
	light.alphaIndex = 10;
	light.distance = 5.0f;   

	// Configuración de lampara
	artificial.Position = glm::vec3(0.0f, 3.5f, -90.0f);
	artificial.Color = glm::vec4(1.35f, 1.1f, 1.0f, 1.0f);
	artificial.Power = glm::vec4(10.0f, 10.0f, 10.0f, 1.0f);
	artificial.alphaIndex = 10;
	artificial.distance = 3.0f;

	//Configuración del audio global de la escena al inicializarla
	// SoundEngine->play2D("sound/EternalGarden.mp3", true);

	return true;
}

//¿Funciones que procesan los vectores de iluminación?
void SetLightUniformInt(Shader* shader, const char* propertyName, size_t lightIndex, int value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shader->setInt(uniformName.c_str(), value);
}
void SetLightUniformFloat(Shader* shader, const char* propertyName, size_t lightIndex, float value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shader->setFloat(uniformName.c_str(), value);
}
void SetLightUniformVec4(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec4 value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shader->setVec4(uniformName.c_str(), value);
}
void SetLightUniformVec3(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec3 value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shader->setVec3(uniformName.c_str(), value);
}

//CICLO DE RENDERIZADO
bool Update() {
	// Cálculo del framerate
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// Procesa la entrada del teclado o mouse
	processInput(window);

	// Renderizado R - G - B - A
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection;
	glm::mat4 view;

	//ESTRUCTURA DE CONTROL PARA DECIDIR QUE CAMARA USAR EN LA ESCENA
	if (activeCamera) {
		// Cámara en primera persona
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		view = camera.GetViewMatrix();
	}
	else {
		// cámara aerea
		projection = glm::ortho(-orthoSize * aspectRatio, orthoSize * aspectRatio, -orthoSize, orthoSize, 0.1f,10000.0f);
		view = cameraair.GetViewMatrix();
	}
	// Cubemap (dibujado del fondo en cada ciclo)
	{
		mainCubeMap->drawCubeMap(*cubemapShader, projection, view);
	}
	
	//-------------------------------------------------------------------------------DIBUJADO Y TRANSFORMACIONES DE MODELO--------------------------------------------------------------------------------------------

	//DIBUJADO DEL TERRENO EN LA ESCENA
	{
		mLightsShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		mLightsShader->setMat4("projection", projection);
		mLightsShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -90.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		mLightsShader->setMat4("model", model);

		// Configuramos propiedades de fuentes de luz
		mLightsShader->setVec4("LightColor", light.Color);
		mLightsShader->setVec4("LightPower", light.Power);
		mLightsShader->setInt("alphaIndex", light.alphaIndex);
		mLightsShader->setFloat("distance", light.distance);
		mLightsShader->setVec3("lightPosition", light.Position);
		mLightsShader->setVec3("lightDirection", light.Direction);
		mLightsShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales
		mLightsShader->setVec4("MaterialAmbientColor", material.ambient);
		mLightsShader->setVec4("MaterialDiffuseColor", material.diffuse);
		mLightsShader->setVec4("MaterialSpecularColor", material.specular);
		mLightsShader->setFloat("transparency", material.transparency);

		terrain->Draw(*mLightsShader);
	}
	glUseProgram(0);

	{
		//DIBUJADO DEL RIO
		wavesShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		wavesShader->setMat4("projection", projection);
		wavesShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -90.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		wavesShader->setMat4("model", model);

		wavesShader->setFloat("time", riverTime);
		wavesShader->setFloat("radius", 5.0f);
		wavesShader->setFloat("height", 5.0f);

		river->Draw(*wavesShader);

		riverTime += 0.01;
	}
	glUseProgram(0);

	// DIBUJADO DEL MODELO DE CASCADA
	{
		// Activamos el shader de Phong
		wavesShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		wavesShader->setMat4("projection", projection);
		wavesShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 450.0f, -90.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		wavesShader->setMat4("model", model);

		wavesShader->setFloat("time", wavesTime);
		wavesShader->setFloat("radius", 5.0f);
		wavesShader->setFloat("height", 5.0f);

		cascade->Draw(*wavesShader);
		wavesTime += 0.0075;

	}
	glUseProgram(0);
	
	//DUMMY PARA LA FUENTE DE ILUMINACIÓN PRINCIPAL DE LA ESCENA
	{
		staticShader->use();

		staticShader->setMat4("projection", projection);
		staticShader->setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, light.Position);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		staticShader->setMat4("model", model);

		sun->Draw(*staticShader);
	}
	glUseProgram(0);

	//SEGUNDA FUENTE DE ILUMINACION
	{
		mLightsShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		mLightsShader->setMat4("projection", projection);
		mLightsShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, artificial.Position); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		mLightsShader->setMat4("model", model);

		// Configuramos propiedades de fuentes de luz
		mLightsShader->setVec4("LightColor", artificial.Color);
		mLightsShader->setVec4("LightPower", artificial.Power);
		mLightsShader->setInt("alphaIndex", artificial.alphaIndex);
		mLightsShader->setFloat("distance", artificial.distance);
		mLightsShader->setVec3("lightPosition", artificial.Position);
		mLightsShader->setVec3("lightDirection", artificial.Direction);
		mLightsShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales
		mLightsShader->setVec4("MaterialAmbientColor", material01.ambient);
		mLightsShader->setVec4("MaterialDiffuseColor", material01.diffuse);
		mLightsShader->setVec4("MaterialSpecularColor", material01.specular);
		mLightsShader->setFloat("transparency", material01.transparency);
		//DIBUJADO DEL DUMMY
		lamp->Draw(*mLightsShader);

		//DIBUJAJDO DEL INTERIOR ILUMINADO CON ESTA FUENTE
		model = glm::mat4(1.0f);

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -90.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		mLightsShader->setMat4("model", model);
		
		decores->Draw(*mLightsShader);

	//DIBUJADO DE PUERTAS ANIMADAS POR ESPECIFICACION DIRECTA

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -90.0f + door_rotation)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		mLightsShader->setMat4("model", model);

		puerta1->Draw(*mLightsShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-0.3f, 0.0f, -90.0f + door_rotation2)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		mLightsShader->setMat4("model", model);

		puerta2->Draw(*mLightsShader);

	}
	glUseProgram(0);

	// EMPLEADO QUE LLAMA A LA PIRAMIDE
	{
		character01->UpdateAnimation(deltaTime);

		// Activación del shader del personaje
		dynamicShader->use();

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		dynamicShader->setMat4("projection", projection);
		dynamicShader->setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(240.0f, 0.0f, -250.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down

		dynamicShader->setMat4("model", model);

		dynamicShader->setMat4("gBones", MAX_RIGGING_BONES, character01->gBones);

		// Dibujamos el modelo
		character01->Draw(*dynamicShader);
	}

	glUseProgram(0); 
	// glfw: swap buffers 
	glfwSwapBuffers(window);
	glfwPollEvents();

	return true;
} //FIN DEL CICLO DE RENDERIZADO

// PROCESAMIENTO DE ENTRADAS DEL TECLADO
void processInput(GLFWwindow* window)
{
	//CERRAR VENATANA
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//CONTROL DE LAS CAMARAS
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
		activeCamera = 0;
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
		activeCamera = 1;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		if (activeCamera == 1)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		else
			cameraair.ProcessKeyboardair(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		if (activeCamera == 1)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		else
			cameraair.ProcessKeyboardair(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		if (activeCamera == 1)
			camera.ProcessKeyboard(LEFT, deltaTime);
		else
			cameraair.ProcessKeyboardair(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		if (activeCamera == 1)
			camera.ProcessKeyboard(RIGHT, deltaTime);
		else
			cameraair.ProcessKeyboardair(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.Position.y += 1.0f;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.Position.y -= 1.0f;

	//DEBUG DE LAS GEOMETRÍAS
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	//Abrir y cerrar puertas restaurante
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		if (door_rotation < 10.0f and door_rotation2 > -10.0f) {
			door_rotation += 1.f;
			door_rotation2 -= 1.f;
		}
		else {
			door_rotation = door_rotation;
			door_rotation2 = door_rotation2;
		}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)

		if (door_rotation >= 0.1f and door_rotation2 <= 0.1f) {
			door_rotation -= 1.f;
			door_rotation2 += 1.f;
		}
		else {
			door_rotation = door_rotation;
			door_rotation2 = door_rotation2;
		}
}

// glfw: Actualizamos el puerto de vista si hay cambios del tamaño
// de la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: Callback del movimiento y eventos del mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
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

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: Complemento para el movimiento y eventos del mouse
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}

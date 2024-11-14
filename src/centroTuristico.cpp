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
Model	*tazaKF;

// Apuntadores a la información del modelos animados con KeyFrames, ya que requieren una clase distinta (carga la información del modelo)
AnimatedModel   *character01;
AnimatedModel	*character02;
AnimatedModel	*character03;
AnimatedModel	*character04;
AnimatedModel	*character05;
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

//DATOS DE KEYFRAMES
int key,action;
//taza
float
movTazaX = 0.0f, incMovTazaX = 0.0f,
movTazaY = 0.0f, incMovTazaY = 0.0f,
movTazaZ = 0.0f, incMovTazaZ= 0.0f,
rotTazaY = 0.0f, incRotTazaY = 0.0f,
myVariable = 0.0f;

#define MAX_FRAMES 10
int i_max_steps = 60;
int i_curr_steps = 0;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float movTazaX, movTazaY, movTazaZ, rotTazaY;

}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 10;			//introducir número en caso de tener Key guardados
bool play = false;
int playIndex = 0;

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
	terrain = new Model("models/escenafinal.fbx");
	river = new Model("models/river.fbx");
	sun = new Model("models/sol.fbx");
	cascade = new Model("models/cascade.fbx");
	lamp = new Model("models/lampcolo.fbx");
	decores = new Model("models/resinilu.fbx");
	puerta1 = new Model("models/puerta1.fbx");
	puerta2 = new Model("models/puerta2.fbx");
	tazaKF = new Model("models/tazaKF.fbx");
	//Personajes de animaciones
	character01 = new AnimatedModel("models/empanim.fbx");
	character02 = new AnimatedModel("models/personSit.fbx");
	character03 = new AnimatedModel("models/personcCook.fbx");
	character04 = new AnimatedModel("models/personSup.fbx");
	character05 = new AnimatedModel("models/personWalking.fbx");


	//INICIALIZACION DE KEYFRAMES
	//Inicialización de KeyFrames, quitar esto y codear los frames para tener una animación guardada con keyFrames.
	KeyFrame[0].movTazaX = -245;
	KeyFrame[0].movTazaY= 14;
	KeyFrame[0].movTazaZ = -91;
	KeyFrame[0].rotTazaY = 0;

	KeyFrame[1].movTazaX = -241;
	KeyFrame[1].movTazaY = 14;
	KeyFrame[1].movTazaZ = -91;
	KeyFrame[1].rotTazaY = 0;

	KeyFrame[2].movTazaX = -241;
	KeyFrame[2].movTazaY = 14;
	KeyFrame[2].movTazaZ = -91;
	KeyFrame[2].rotTazaY = 0;

	KeyFrame[3].movTazaX = -239;
	KeyFrame[3].movTazaY = 14;
	KeyFrame[3].movTazaZ = -91;
	KeyFrame[3].rotTazaY = 0;

	KeyFrame[4].movTazaX = -239;
	KeyFrame[4].movTazaY = 14;
	KeyFrame[4].movTazaZ = -92;
	KeyFrame[4].rotTazaY = -6;

	KeyFrame[5].movTazaX = -239;
	KeyFrame[5].movTazaY = 14;
	KeyFrame[5].movTazaZ = -94;
	KeyFrame[5].rotTazaY = -15;

	KeyFrame[6].movTazaX = -238;
	KeyFrame[6].movTazaY = 14;
	KeyFrame[6].movTazaZ = -95;
	KeyFrame[6].rotTazaY = -15;

	KeyFrame[7].movTazaX = -238;
	KeyFrame[7].movTazaY = 14;
	KeyFrame[7].movTazaZ = -95;
	KeyFrame[7].rotTazaY = -20;

	KeyFrame[8].movTazaX = -236;
	KeyFrame[8].movTazaY = 14;
	KeyFrame[8].movTazaZ = -95;
	KeyFrame[8].rotTazaY = -45;

	KeyFrame[9].movTazaX = -236;
	KeyFrame[9].movTazaY = 0;
	KeyFrame[9].movTazaZ = -93;
	KeyFrame[9].rotTazaY = -180;


	// Cubemap (Se inizializa el cubemap y se pasan las rutas de las texturas del mismo )
	vector<std::string> faces
	{
		"textures/cubemap/01/posx.png",
		"textures/cubemap/01/negx.png",
		"textures/cubemap/01/posy.png",
		"textures/cubemap/01/negy.png",
		"textures/cubemap/01/posz.png",
		"textures/cubemap/01/negz.png"
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
	artificial.Position = glm::vec3(0.0f, 3.5f, -80.0f);
	artificial.Color = glm::vec4(1.75f, 1.25f, 1.0f, 1.0f);
	artificial.Power = glm::vec4(12.0f, 12.0f, 12.0f, 1.0f);
	artificial.alphaIndex = 10;
	artificial.distance = 3.0f;

	//Configuración del audio global de la escena al inicializarla
	SoundEngine->play2D("sound/1-02 Main Theme.mp3", true);

	return true;
}

//Funciones que procesan los vectores de iluminación
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

//FUNCIONES DE KEYFRAMES
void saveFrame(void)
{
	printf("frameindex %d\n", FrameIndex);
	std::cout << "Frame Index = " << FrameIndex << std::endl;

	KeyFrame[FrameIndex].movTazaX = movTazaX;
	KeyFrame[FrameIndex].movTazaY = movTazaY;
	KeyFrame[FrameIndex].movTazaZ = movTazaZ;
	KeyFrame[FrameIndex].rotTazaY = rotTazaY;
	std::cout << "X: = " << movTazaX << std::endl;
	std::cout << "Y: = " << movTazaY << std::endl;
	std::cout << "Z: = " << movTazaZ << std::endl;
	std::cout << "rY = " << rotTazaY << std::endl;

	FrameIndex++;
}

void resetElements(void)
{
	movTazaX = KeyFrame[0].movTazaX;
	movTazaY = KeyFrame[0].movTazaY;
	movTazaZ = KeyFrame[0].movTazaZ;
	rotTazaY = KeyFrame[0].rotTazaY;
}

void interpolation(void)
{
	incMovTazaX = (KeyFrame[playIndex + 1].movTazaX - KeyFrame[playIndex].movTazaX) / (i_max_steps);
	incMovTazaY = (KeyFrame[playIndex + 1].movTazaY - KeyFrame[playIndex].movTazaY) / (i_max_steps);
	incMovTazaZ = (KeyFrame[playIndex + 1].movTazaZ - KeyFrame[playIndex].movTazaZ) / (i_max_steps);
	incRotTazaY = (KeyFrame[playIndex + 1].rotTazaY - KeyFrame[playIndex].rotTazaY) / (i_max_steps);
}

void animate(void)
{
	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				std::cout << "Animation ended" << std::endl;
				//printf("termina anim\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
				//Interpolation
				interpolation();
			}
		}
		else
		{
			//Draw animation
			movTazaX += incMovTazaX;
			movTazaY += incMovTazaY;
			movTazaZ += incMovTazaZ;
			rotTazaY += incRotTazaY;

			i_curr_steps++;
		}
	}
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

	//ANIMACIÓN PROGRAMADA POR KEYFRAMES
	animate();
	
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
		
		//TAZA CON KEYFRAMES
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(movTazaX, movTazaY, movTazaZ)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(rotTazaY), glm::vec3(0.0f, 0.0f, 1.0f));
		//model = glm::translate(model, glm::vec3(-245.0f, 14.0f, -91.0f)); // se ajusta a donde empieza la animacion
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -90.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
		mLightsShader->setMat4("model", model);

		tazaKF->Draw(*mLightsShader);
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
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -80.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		mLightsShader->setMat4("model", model);
		
		decores->Draw(*mLightsShader);

	//DIBUJADO DE PUERTAS ANIMADAS POR ESPECIFICACION DIRECTA

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(174.0f, 0.0f, -90.0f + door_rotation)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.8f, 1.3f, 1.3f));	// it's a bit too big for our scene, so scale it down
		mLightsShader->setMat4("model", model);

		puerta1->Draw(*mLightsShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(173.5f, 0.0f, -90.0f + door_rotation2)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.8f, 1.3f, 1.3f));	// it's a bit too big for our scene, so scale it down
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

	//PERSONA SENTADA
	{
		character02->UpdateAnimation(deltaTime);

		// Activación del shader del personaje
		dynamicShader->use();

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		dynamicShader->setMat4("projection", projection);
		dynamicShader->setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::translate(model, glm::vec3(-110.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 100.0f)); // translate it down so it's at the center of the scene
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -90.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down

		dynamicShader->setMat4("model", model);

		dynamicShader->setMat4("gBones", MAX_RIGGING_BONES, character02->gBones);
		character02->Draw(*dynamicShader);
	}
	glUseProgram(0);

	//PERSONA COCINA
	{
		character03->UpdateAnimation(deltaTime);

		// Activación del shader del personaje
		dynamicShader->use();

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		dynamicShader->setMat4("projection", projection);
		dynamicShader->setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-226.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 83.0f)); // translate it down so it's at the center of the scene
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -90.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down

		dynamicShader->setMat4("model", model);

		dynamicShader->setMat4("gBones", MAX_RIGGING_BONES, character03->gBones);
		character03->Draw(*dynamicShader);
	}
	glUseProgram(0);

	//PERSONA SORPRENDIDA
	///*
	{
		character04->UpdateAnimation(deltaTime);

		// Activación del shader del personaje
		dynamicShader->use();

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		dynamicShader->setMat4("projection", projection);
		dynamicShader->setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(175.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 130.0f)); // translate it down so it's at the center of the scene
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -90.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down

		dynamicShader->setMat4("model", model);

		dynamicShader->setMat4("gBones", MAX_RIGGING_BONES, character04->gBones);
		character04->Draw(*dynamicShader);
	}
	glUseProgram(0);
	//*/

	//PERSONA CAMINA
	{
		character05->UpdateAnimation(deltaTime);

		// Activación del shader del personaje
		dynamicShader->use();

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		dynamicShader->setMat4("projection", projection);
		dynamicShader->setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 38.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -90.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down

		dynamicShader->setMat4("model", model);

		dynamicShader->setMat4("gBones", MAX_RIGGING_BONES, character05->gBones);
		character05->Draw(*dynamicShader);
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
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		if (door_rotation < 10.0f and door_rotation2 > -10.0f) {
			door_rotation += 1.f;
			door_rotation2 -= 1.f;
		}
		else {
			door_rotation = door_rotation;
			door_rotation2 = door_rotation2;
		}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)

		if (door_rotation >= 0.1f and door_rotation2 <= 0.1f) {
			door_rotation -= 1.f;
			door_rotation2 += 1.f;
		}
		else {
			door_rotation = door_rotation;
			door_rotation2 = door_rotation2;
		}

	//TECLAS PARA KEYFRAMES
		//To Configure Model
	/*
	* 	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
		movTazaY++;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		movTazaY--;
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		movTazaX--;
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		movTazaX++;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		movTazaZ--;
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		movTazaZ++;
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		rotTazaY += 3.0f;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		rotTazaY -= 3.0f;
	*/



	//To play KeyFrame animation 
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		if (play == false && (FrameIndex > 1))
		{
			std::cout << "Play animation" << std::endl;
			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
			std::cout << "Not enough Key Frames" << std::endl;
		}
	}

	//To Save a KeyFrame
	/*
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		if (FrameIndex < MAX_FRAMES)
		{
			saveFrame();
		}
	}
	*/
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
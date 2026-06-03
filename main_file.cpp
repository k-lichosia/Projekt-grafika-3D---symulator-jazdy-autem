#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#define GLEW_STATIC  
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"
#include "map_geometry.h"
#include "city_map.h"
#include "Car.h"
#include "samochodyOBJ.h"
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")

ShaderProgram* spMap; 
float dist = 0.0f;    
float speed_x = 0;
float speed_y = 0;
float aspectRatio = 1;

ShaderProgram* sp;

Car autoGracza(0.0f, 0.5f, -2.0f);
std::vector<Car> inneAuta;
float spawnTimer = 0.0f;

ObjModel modelSamochodu;

extern GLuint texAsphalt; 
extern GLuint texBuilding;
extern GLuint texGrass;

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_LEFT) speed_x = -PI / 2;
        if (key == GLFW_KEY_RIGHT) speed_x = PI / 2;
        if (key == GLFW_KEY_UP) speed_y = PI / 2;
        if (key == GLFW_KEY_DOWN) speed_y = -PI / 2;

        if (key == GLFW_KEY_D) autoGracza.toggleLeftIndicator();
        if (key == GLFW_KEY_A) autoGracza.toggleRightIndicator();
        if (key == GLFW_KEY_S) autoGracza.toggleHazardLights();
    }
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT) speed_x = 0;
        if (key == GLFW_KEY_RIGHT) speed_x = 0;
        if (key == GLFW_KEY_UP) speed_y = 0;
        if (key == GLFW_KEY_DOWN) speed_y = 0;
    }
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

GLuint readTexture(const char* filename) {
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	std::vector<unsigned char> image;
	unsigned width, height;

	unsigned error = lodepng::decode(image, width, height, filename);

	if (error) {
		printf("Blad wczytywania tekstury %s: %s\n", filename, lodepng_error_text(error));
		return 0;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
	return tex;
}

void initOpenGLProgram(GLFWwindow* window) {
	glClearColor(0.15f, 0.25f, 0.45f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");
	spMap = new ShaderProgram("v_map.glsl", NULL, "f_map.glsl");

	if (!modelSamochodu.load("Car.obj")) {
		printf("UWAGA: Model nie zostal wczytany!\n");
	}

	GLfloat light_position[] = { 10.0f, 20.0f, 10.0f, 0.0f };

	GLfloat light_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	glShadeModel(GL_SMOOTH);

	texChodnik = readTexture("chodnik2.png");
	texAsphalt = readTexture("asfalt.png");
	texBuilding = readTexture("kamienica.png");
	texGrass = readTexture("trawa.png");

	if (texChodnik == 0) {
		printf("UWAGA: Tekstura nie zostala wczytana!\n");
	}

	glBindTexture(GL_TEXTURE_2D, texChodnik);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void freeOpenGLProgram(GLFWwindow* window) {
	delete sp;
	delete spMap;
}

void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_NORMALIZE);

	glm::mat4 V = glm::lookAt(glm::vec3(0, 3, -10), glm::vec3(0, 0, 10), glm::vec3(0, 1, 0));
	glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 0.01f, 200.0f);

	spMap->use();
	glUniformMatrix4fv(spMap->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(spMap->u("V"), 1, false, glm::value_ptr(V));
	renderCity(spMap, dist);

	glUseProgram(0);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(P));

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(V));

	GLfloat light_position[] = { 10.0f, 20.0f, 10.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glPushMatrix();
	glTranslatef(autoGracza.x, autoGracza.y, autoGracza.z);

	float katSkretu = speed_x * 15.0f;
	glRotatef(90.0f - katSkretu, 0.0f, 1.0f, 0.0f);

	glScalef(2.5f, 2.5f, 2.5f);
	glTranslatef(-1.0f, 0.0f, -0.4f);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);

	glColor3f(0.2f, 0.4f, 1.0f);

	GLfloat mat_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat mat_shininess[] = { 50.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	autoGracza.draw_model_only();

	glDisable(GL_NORMALIZE);
	glDisable(GL_LIGHTING);
	glPopMatrix();

	for (int i = 0; i < inneAuta.size(); i++) {
		glPushMatrix();

		glTranslatef(inneAuta[i].x, inneAuta[i].y, inneAuta[i].z);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

		glScalef(1.0f, 1.0f, 1.0f);
		glTranslatef(-1.0f, 0.0f, -0.4f);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_NORMALIZE);

		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

		modelSamochodu.draw(inneAuta[i].colorR, inneAuta[i].colorG, inneAuta[i].colorB);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
		glDisable(GL_LIGHTING);           

		glBegin(GL_QUADS);

		float npcRoadY = -0.388f;

		glColor4f(1.0f, 1.0f, 0.8f, 0.45f);
		glVertex3f(-0.9f, npcRoadY, 1.2f); 
		glVertex3f(0.9f, npcRoadY, 1.2f); 

		glColor4f(1.0f, 1.0f, 0.8f, 0.0f);
		glVertex3f(3.0f, npcRoadY, 12.0f); 
		glVertex3f(-3.0f, npcRoadY, 12.0f);

		glEnd();

		glEnable(GL_LIGHTING);
		glDisable(GL_BLEND);

		glDisable(GL_NORMALIZE);
		glPopMatrix();
	}
	glfwSwapBuffers(window);
}

int main(void)
{
	srand(time(NULL));
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);

	if (!window)
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window);

	glfwSetTime(0);

	PlaySound(TEXT("muzyka.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

	while (!glfwWindowShouldClose(window)) {
		float deltaTime = glfwGetTime();
		glfwSetTime(0);

		dist += 10.0f * deltaTime;

		autoGracza.x -= speed_x * deltaTime * 5.0f;
		autoGracza.z += speed_y * deltaTime * 5.0f;
		autoGracza.wheelAngle += 200.0f * deltaTime;

		spawnTimer += deltaTime;
		if (spawnTimer > 3.5f) {
			float lewyPas = -3.0f;
			float prawyPas = 1.7f;

			float laneX = (rand() % 100 < 50) ? lewyPas : prawyPas;

			Car npc(laneX, 0.5f, 40.0f);

			float paletaBarw[6][3] = {
				{0.0f, 0.8f, 0.2f},
				{0.1f, 0.5f, 1.0f}, 
				{1.0f, 0.4f, 0.7f}, 
				{1.0f, 0.8f, 0.0f}, 
				{0.9f, 0.1f, 0.1f}, 
				{1.0f, 0.5f, 0.0f} 
			};

			int losowyIndeks = rand() % 6;

			npc.colorR = paletaBarw[losowyIndeks][0];
			npc.colorG = paletaBarw[losowyIndeks][1];
			npc.colorB = paletaBarw[losowyIndeks][2];

			inneAuta.push_back(npc);

			spawnTimer = 0.0f;
		}

		for (int i = 0; i < inneAuta.size(); i++) {
			inneAuta[i].z -= 25.0f * deltaTime;
			inneAuta[i].wheelAngle -= 200.0f * deltaTime; 

			if (inneAuta[i].z < -10.0f) {
				inneAuta.erase(inneAuta.begin() + i);
				i--; 
			}
		}

		drawScene(window, 0, 0);

		glfwPollEvents();
	}

	freeOpenGLProgram(window);
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

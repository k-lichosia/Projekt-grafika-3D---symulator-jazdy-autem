/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#define GLEW_STATIC  // To musi być PIERWSZE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"
#include "map_geometry.h"

#include "city_map.h"
#include "Car.h"

#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")
ShaderProgram* spMap; // Nowy wskaźnik na Twoje shadery
float dist = 0.0f;    // Zmienna przejechanego dystansu

float speed_x=0;
float speed_y=0;
float aspectRatio=1;

ShaderProgram *sp;

Car autoGracza(0.0f, 0.5f, -2.0f);
//Odkomentuj, żeby rysować kostkę
float* vertices = myCubeVertices;
float* normals = myCubeNormals;
float* texCoords = myCubeTexCoords;
float* colors = myCubeColors;
int vertexCount = myCubeVertexCount;

//Odkomentuj, żeby rysować czajnik
//float* vertices = myTeapotVertices;
//float* normals = myTeapotNormals;
//float* texCoords = myTeapotTexCoords;
//float* colors = myTeapotColors;
//int vertexCount = myTeapotVertexCount;



//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}


void keyCallback(GLFWwindow* window,int key,int scancode,int action,int mods) {
    if (action==GLFW_PRESS) {
        if (key==GLFW_KEY_LEFT) speed_x=-PI/2;
        if (key==GLFW_KEY_RIGHT) speed_x=PI/2;
        if (key==GLFW_KEY_UP) speed_y=PI/2;
        if (key==GLFW_KEY_DOWN) speed_y=-PI/2;
    }
    if (action==GLFW_RELEASE) {
        if (key==GLFW_KEY_LEFT) speed_x=0;
        if (key==GLFW_KEY_RIGHT) speed_x=0;
        if (key==GLFW_KEY_UP) speed_y=0;
        if (key==GLFW_KEY_DOWN) speed_y=0;
    }
}

void windowResizeCallback(GLFWwindow* window,int width,int height) {
    if (height==0) return;
    aspectRatio=(float)width/(float)height;
    glViewport(0,0,width,height);
}

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Jasny błękit nieba
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window,windowResizeCallback);
	glfwSetKeyCallback(window,keyCallback);

	sp=new ShaderProgram("v_simplest.glsl",NULL,"f_simplest.glsl");
	// Zainicjuj swoje shadery mapy
	spMap = new ShaderProgram("v_map.glsl", NULL, "f_map.glsl");
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************

    delete sp;
}




void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Kamera patrzy z (0, 3, -10) na (0, 0, 10)
	glm::mat4 V = glm::lookAt(glm::vec3(0, 3, -10), glm::vec3(0, 0, 10), glm::vec3(0, 1, 0));
	glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 0.01f, 200.0f);

	// 1. RYSOWANIE MAPY
	spMap->use();
	glUniformMatrix4fv(spMap->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(spMap->u("V"), 1, false, glm::value_ptr(V));
	renderCity(spMap, dist);

	// 2. RYSOWANIE SAMOCHODU
	glUseProgram(0);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(P));

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(V));

	glPushMatrix();
	// 1. Przesunięcie do pozycji auta na drodze
	glTranslatef(autoGracza.x, autoGracza.y, autoGracza.z);

	// ========================================================
	// 2. SKRĘCANIE I PRZECHYLANIE (Efekt "ukosu")
	// Mnożnik określa jak mocno auto reaguje. 
	// Wartość speed_x przy wciśnięciu to około 1.5 lub -1.5
	// ========================================================

	// Skręt "nosa" samochodu w lewo/prawo (Yaw)
	float katSkretu = speed_x * 15.0f;
	glRotatef(90.0f - katSkretu, 0.0f, 1.0f, 0.0f);

	// Opcjonalny przechył karoserii na boki (Body Roll)
	// Jeśli auto przechyla się w złą stronę, usuń znak minusa!
	//float przechyl = -speed_x * 5.0f;
	//glRotatef(przechyl, 1.0f, 0.0f, 0.0f);

	// ========================================================
	// 3. POWIĘKSZENIE SAMOCHODU (Skalowanie)
	// Wartość 1.5f oznacza powiększenie o 50%. 
	// Jeśli chcesz 2 razy większe auto, wpisz 2.0f, 2.0f, 2.0f
	// ========================================================
	glScalef(2.5f, 2.5f, 2.5f);

	// 4. Wyśrodkowanie modelu przed wykonaniem powyższych operacji
	glTranslatef(-1.0f, 0.0f, -0.4f);

	autoGracza.draw_model_only();
	glPopMatrix();
	glfwSwapBuffers(window);
}


int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window);

	glfwSetTime(0); // Zeruj timer przed pętlą

	while (!glfwWindowShouldClose(window)) {
		float deltaTime = glfwGetTime();
		glfwSetTime(0); // Zerujemy, żeby w kolejnej klatce deltaTime było czasem trwania TEJ klatki

		dist += 10.0f * deltaTime; // Zwiększamy dystans o prędkość * czas

		// 2. RUCH SAMOCHODU
		// speed_x (strzałki lewo/prawo) modyfikuje pozycję X
		autoGracza.x -= speed_x * deltaTime * 5.0f; // Mnożnik 5.0f to czułość skrętu

		// speed_y (strzałki góra/dół) modyfikuje pozycję Z (przód/tył)
		autoGracza.z += speed_y * deltaTime * 5.0f; // Mnożnik 5.0f to prędkość

		// 3. Kręcenie kołami
		// Koła kręcą się cały czas, bo miasto się przesuwa pod autem
		autoGracza.wheelAngle += 200.0f * deltaTime;
		// TYLKO TO wywołanie rysuje scenę
		drawScene(window, 0, 0);

		glfwPollEvents();
	}

	freeOpenGLProgram(window);
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

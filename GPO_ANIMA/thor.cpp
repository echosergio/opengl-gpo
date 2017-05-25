/************************  GPO_03 ************************************
ATG, 2014
******************************************************************************/
//#include "GpO.h"
#include "../cross-cutting/GpO.h"

char *WINDOW_TITLE = "TEXTURAS en OpenGL (GpO)";
int CurrentWidth = 600, CurrentHeight = 450, WindowHandle = 0; // Tama�o ventana, handle a ventana
unsigned FrameCount = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 330 core\n" #src

const char *vertex_prog = GLSL(
	layout(location = 0) in vec3 pos;
	layout(location = 1) in vec2 color;
	out vec2 col;
	
	uniform mat4 MVP1 = mat4(1.0f);
	uniform mat4 MVP2 = mat4(1.0f);
	uniform mat4 MVP3 = mat4(1.0f);
	uniform float WIDTH = 0.5;

	void main() {
		vec4 pos1 = MVP1 * vec4(pos, 1);
		vec4 pos2 = MVP2 * vec4(pos, 1);
		vec4 pos3 = MVP3 * vec4(pos, 1);

		if (pos.z <= -0.48) {
			gl_Position = pos1;
		} else if (pos.z >= 0.48) {
			gl_Position = pos2;
		} else {
			gl_Position = pos3;
		}

		col = color; // Paso color a fragment shader
	});

const char *fragment_prog = GLSL(
	in vec2 col;
	out vec3 color;
	uniform sampler2D tex;
	void main() {
		color = texture(tex, col).rgb;
	});

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLuint prog;
objeto obj;

void dibujar_indexado(objeto obj)
{
	glBindVertexArray(obj.VAO);										  // Activamos VAO asociado al objeto
	glDrawElements(GL_TRIANGLES, obj.Ni, obj.tipo_indice, (void *)0); // Dibujar (indexado)
	glBindVertexArray(0);
}

// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Preparaci�n de los datos de los objetos a dibujar, envialarlos a la GPU
// Opciones generales de render de OpenGL
void init_scene()
{
	GLuint tex0;

	prog = LinkShaders(vertex_prog, fragment_prog); // Compile shaders, crear programa a usar, Mandar a GPU
	glUseProgram(prog);								// Indicamos que programa vamos a usar

	obj = cargar_modelo("thor.bix");
	tex0 = cargar_textura_from_bmp("thor.bmp", GL_TEXTURE0);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

vec3 pos_obs = vec3(0.0f, 2.0f, 3.0f);
vec3 target = vec3(0.0f, 1.0f, 0.0f);

// Dibujar objetos
// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
	FrameCount++;
	float tt = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // Tiempo en segundos

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);				// Especifica color (RGB+alfa)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Aplica color asignado

	///////// Aqui vendr�a nuestr c�digo para actualizar escena  /////////

	mat4 P = perspective(40.0f, 4.0f / 3.0f, 0.1f, 20.0f); //40� Y-FOV,  4:3 ,  Znear=0.1, Zfar=20
	mat4 V = lookAt(pos_obs, target, vec3(0, 1, 0));	   // Pos camara, Lookat, head up
	mat4 M, T, R, S, R2, R3;

	R = glm::rotate(30.0f * tt, vec3(0, 1, 0));
	M = R;

	transfer_mat4("MVP3", P * V * M);
	transfer_int("tex", 0);

	R2 = glm::rotate(45.0f * (1.0f - cos(3.0f * tt)), vec3(0, 1, 0));

	vec3 p_tmp = vec3(0, 1.45, 0.48);
	vec4 p = vec4(p_tmp, 1);
	vec3 Mp = vec3(M * p);
	mat4 M2 = translate(Mp) * R2 * translate(-Mp) * M;

	transfer_mat4("MVP2", P * V * M2);

	R3 = glm::rotate(-45.0f * (1.0f - cos(3.0f * tt)), vec3(0, 1, 0));

	p_tmp = vec3(0, 1.45, -0.48);
	p = vec4(p_tmp, 1);
	Mp = vec3(M * p);
	mat4 M3 = translate(Mp) * R3 * translate(-Mp) * M;

	transfer_mat4("MVP1", P * V * M3);

	dibujar_indexado(obj);

	////////////////////////////////////////////////////////

	glutSwapBuffers(); // Intercambiamos buffers de refresco y display
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////  INTERACCION  TECLADO RATON
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void keyboard(unsigned char, int, int);
void key_special(int, int, int);
void mouse(int, int, int, int);
void mouse_mov(int, int);

void eventos_teclado_mouse()
{
	glutKeyboardFunc(keyboard); // Caso de pulsar alguna tecla
								//	glutSpecialFunc(key_special);  // Teclas de funci�n, cursores, etc

	//	glutMouseFunc(mouse);           // Eventos del rat�n
	//	glutMotionFunc(mouse_mov); // Mov del rat�n con boton presionado
	//	glutPassiveMotionFunc(mouse_mov); // Mov del rat�n
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:				 // Escape key
		glutLeaveMainLoop(); // Salimos del bucle
		return;
		break;
	}
}

void key_special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F1: // Teclas de Funcion
		break;
	case GLUT_KEY_UP: //Teclas cursor;
		break;
	case GLUT_KEY_DOWN:
		break;
	case GLUT_KEY_LEFT:
		break;
	case GLUT_KEY_RIGHT:
		break;
	}
}

void mouse(int but, int state, int x, int y)
{
}

void mouse_mov(int x, int y)
{
	fprintf(stdout, "Pos (%3d,%3d)\n", x - CurrentWidth / 2, y - CurrentHeight / 2);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GLUT:  CREACION VENTANAS E INTERACCION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Init_Opengl(void);
void Init_Window(int, char *[]);
void cambia_window(int, int);
void TimerFunction(int);
void IdleFunction(void);
void clean_up(void);

int main(int argc, char *argv[])
{
	Init_Window(argc, argv); // Prepara y abre ventana
	Init_Opengl();			 // Inicializa OpenGL, comprueba versi�n.
	init_scene();			 // Preppara escena
	glutMainLoop();			 // Entra en bucle (render_scene se ejecuta constantemente)
	exit(EXIT_SUCCESS);
}

void Init_Opengl(void)
{
	LoadFunctions();
	fprintf(stdout, "OpenGL Version:  ", glGetString(GL_VERSION));
	if (IsVersionGEQ(3, 3))
		printf(">=3.3 => OK.\n");
	else
	{
		printf("Se necesita OpenGL >= 3.3 para estos ejemplos.\n");
		glutDestroyWindow(WindowHandle);
		exit(EXIT_FAILURE);
	}
}

void Init_Window(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutInitWindowSize(CurrentWidth, CurrentHeight);

	fprintf(stdout, "Pantalla  %d x %d pixels\n", glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - CurrentWidth) / 2, (glutGet(GLUT_SCREEN_HEIGHT) - CurrentHeight) / 2);
	// Creamos ventana centrada en pantalla

	WindowHandle = glutCreateWindow("TITULO");
	if (WindowHandle < 1)
	{
		fprintf(stderr, "ERROR: Could not create a new rendering window.\n");
		exit(EXIT_FAILURE);
	}
	else
		fprintf(stderr, "Ventana creada (%d)\n", WindowHandle);

	// Asociar funciones de eventos
	glutDisplayFunc(render_scene);  //render, llamada de forma continua en el bucle
	glutReshapeFunc(cambia_window); // Caso de cambiar tama�o de ventana
	glutIdleFunc(IdleFunction);
	glutTimerFunc(200, TimerFunction, 0);
	glutCloseFunc(clean_up);

	eventos_teclado_mouse();
}

//Called whenever the window is resized.
void cambia_window(int Width, int Height)
{
	CurrentWidth = Width;
	CurrentHeight = Height;

	glViewport(0, 0, CurrentWidth, CurrentHeight);
}

void IdleFunction(void)
{
	glutPostRedisplay();
}

void clean_up(void) // Borrar los objetos creados (programas, objetos graficos)
{
	glDeleteProgram(prog);
}

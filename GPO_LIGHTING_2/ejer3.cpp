/************************  GPO_03 ************************************
ATG, 2014
******************************************************************************/
//#include "GpO.h"
#include "../cross-cutting/GpO.h"

char *WINDOW_TITLE = "ILUMINACION en OpenGL (GpO)";
int CurrentWidth = 600, CurrentHeight = 450, WindowHandle = 0; // Tama�o ventana, handle a ventana
unsigned FrameCount = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 330 core\n" #src

float az = 0.0f;
float elev = 3.14/4;
vec4 coeff = vec4(0.1, 0.6, 0.3, 16);

const char *vertex_prog1 = GLSL( // GOURAD, LUZ LEJANA
	layout(location = 0) in vec3 pos;
	layout(location = 1) in vec3 normal;
	out vec3 ilu;

	uniform mat4 MVP;
	uniform mat4 M_normales;
	uniform mat4 M;

	uniform vec3 lightdir = vec3(1 / sqrt(2.0f), 1 / sqrt(2.0f), 0.0f);
	const vec3 lightcolor = vec3(1.0f, 1.0f, 1.0f);
	uniform vec3 campos = vec3(1.0f, 1.0f, 1.0f);

	void main() {
		float difusa;
		float comp_esp;
		vec3 normal_T;
		vec3 v;
		vec3 r;

		gl_Position = MVP * vec4(pos, 1);

		normal_T = (M_normales * vec4(normal, 0.0f)).xyz; // Modificamos normales por el mov del objeto
		normal_T = normalize(normal_T);					  // Aseguramos de que vectoror normal sea de longitud 1

		v = normalize( campos - (M * vec4(pos,1)).xyz );
		r = reflect(-lightdir, normal_T);
		comp_esp = pow(clamp(dot(r,v),0.0f,1.0f), 16.0f);
		difusa = clamp(dot(lightdir, normal_T), 0.0f, 1.0f); // producto escalar entre vector luz y normal

		ilu = 0.10f + 0.60f * difusa * lightcolor + 0.30f * comp_esp; //Ambiente + difusa + especular
	});


const char *fragment_prog1 = GLSL(
	in vec3 ilu; // Entrada = colores de vertices (interpolados en fragmentos)
	out vec3 color_fragmento;
	void main() {
		color_fragmento = ilu;
	});

//////////////////////////////////////////////////////////////////////////////////

const char *vertex_prog2 = GLSL( // GOURAD, LUZ LEJANA
	layout(location = 0) in vec3 pos;
	layout(location = 1) in vec3 normal;
	out vec3 v;
	out vec3 normal_T;

	uniform mat4 MVP;
	uniform mat4 M_normales;
	uniform mat4 M;

	const vec3 lightcolor = vec3(1.0f, 1.0f, 1.0f);
	uniform vec3 campos = vec3(1.0f, 1.0f, 1.0f);

	void main() {
		gl_Position = MVP * vec4(pos, 1);

		normal_T = (M_normales * vec4(normal, 0.0f)).xyz; // Modificamos normales por el mov del objeto
						  // Aseguramos de que vectoror normal sea de longitud 1

		v = campos - (M * vec4(pos,1)).xyz;
	});

const char *fragment_prog2 = GLSL(
	in vec3 normal_T; // Entrada = colores de vertices (interpolados en fragmentos)
	in vec3 v;
	out vec3 color_fragmento;
	uniform vec3 lightdir = vec3(1 / sqrt(2.0f), 1 / sqrt(2.0f), 0.0f);
    uniform vec4 coeff;
	const vec3 lightcolor = vec3(1.0f, 0.8f, 1.0f);
	void main() {
		float comp_esp;
		float difusa;
		vec3 r;
		vec3 n_T;
		vec3 v2;

		n_T = normalize(normal_T);	
		v2 = normalize(v);

		r = reflect(-lightdir, n_T);
		comp_esp = pow(clamp(dot(r,v2),0.0f,1.0f), coeff[4]);
		difusa = clamp(dot(lightdir, n_T), 0.0f, 1.0f); // producto escalar entre vector luz y normal
		color_fragmento = coeff[0] + coeff[1] * difusa * lightcolor + coeff[2] * comp_esp; //Ambiente + difusa + especular;
	});

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLuint prog1, prog2;

void clean_up(void) // Borrar los objetos creados (programas, objetos graficos)
{
	glDeleteProgram(prog2);
	glDeleteProgram(prog1);
}

// Dibuja objeto indexado
void dibujar_indexado(objeto obj)
{
	glBindVertexArray(obj.VAO);										  // Activamos VAO asociado al objeto
	glDrawElements(GL_TRIANGLES, obj.Ni, obj.tipo_indice, (void *)0); // Dibujar (indexado)
	glBindVertexArray(0);
}

// Variables globales
glm::mat4 Proy, View, M, MVP;
vec3 campos = vec3(0.0f, 0.0f, 4.0f);
vec3 target = vec3(0.0f, 0.0f, 0.0f);

objeto modelo;
bool prog_switch = true;

// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Preparaci�n de los datos de los objetos a dibujar, envialarlos a la GPU
// Opciones generales de render de OpenGL
void init_scene()
{
	prog1 = LinkShaders(vertex_prog1, fragment_prog1); // Compile shaders prog1
	prog2 = LinkShaders(vertex_prog2, fragment_prog2); // Compile shaders prog2
	
	glUseProgram(prog1);

	modelo = cargar_modelo("esfera_520_n.bix");

	Proy = glm::perspective(55.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	View = glm::lookAt(campos, target, glm::vec3(0, 1, 0));
	M = mat4(1.0f);
	MVP = Proy * View * M;

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
	FrameCount++;

	prog_switch ? glUseProgram(prog1) : glUseProgram(prog2);

	vec3 dirlight = vec3(cos(elev)*cos(az), sin(elev), cos(elev)*sin(az));
	transfer_vec3("lightdir", dirlight);

	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float tt = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	glm::mat4 adjunta_M;

	glm::vec3 t = vec3(cos(tt), 0.0f, sin(tt));
	M = translate(t) * rotate(50 * tt, vec3(0.0f, 1.0f, 0.0f)); //Rotate model
	MVP = Proy * View * M;

	transfer_mat4("MVP", MVP);

	transfer_mat4("M", M);
	transfer_vec3("campos", campos);

	adjunta_M = glm::transpose(inverse(M));
	transfer_mat4("M_normales", adjunta_M);

	dibujar_indexado(modelo);
	glutSwapBuffers();
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
	glutSpecialFunc(key_special);  // Teclas de funci�n, cursores, etc

	//	glutMouseFunc(mouse);           // Eventos del rat�n
	//	glutMotionFunc(mouse_mov); // Mov del ra�n
}


void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:				 // Escape key
		glutLeaveMainLoop(); // Salimos del bucle
		return;
		break;
    // Subir valores
    case 'q':
        coeff += vec4(0.05,0.0,0.0,0.0);
        printf("*******( %.2f, %.2f, %.2f, %.2f ) ********\n", coeff[0], coeff[1], coeff[2], coeff[3]);
        break;
    case 'w':
        coeff += vec4(0.0,0.05,0.0,0.0);
        printf("*******( %.2f, %.2f, %.2f, %.2f ) ********\n", coeff[0], coeff[1], coeff[2], coeff[3]);
        break;
    case 'e':
        coeff += vec4(0.0,0.0,0.05,0.0);
        printf("*******( %.2f, %.2f, %.2f, %.2f ) ********\n", coeff[0], coeff[1], coeff[2], coeff[3]);
        break;
    case 'r':
        coeff += vec4(0.0,0.0,0.0,0.05);
        printf("*******( %.2f, %.2f, %.2f, %.2f ) ********\n", coeff[0], coeff[1], coeff[2], coeff[3]);
        break;

    // Bajar valores
    case 'a':
        coeff -= vec4(0.05,0.0,0.0,0.0);
        printf("*******( %.2f, %.2f, %.2f, %.2f ) ********\n", coeff[0], coeff[1], coeff[2], coeff[3]);
        break;
    case 's':
        coeff -= vec4(0.0,0.05,0.0,0.0);
        printf("*******( %.2f, %.2f, %.2f, %.2f ) ********\n", coeff[0], coeff[1], coeff[2], coeff[3]);
        break;
    case 'd':
        coeff -= vec4(0.0,0.0,0.05,0.0);
        printf("*******( %.2f, %.2f, %.2f, %.2f ) ********\n", coeff[0], coeff[1], coeff[2], coeff[3]);
        break;
    case 'f':
        coeff -= vec4(0.0,0.0,0.0,0.05);
        printf("*******( %.2f, %.2f, %.2f, %.2f ) ********\n", coeff[0], coeff[1], coeff[2], coeff[3]);
        break;      
	}
}

void key_special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F1: // Teclas de Funcion
		prog_switch = !prog_switch;
		break;
	case GLUT_KEY_UP: //Teclas cursor;
		elev += 0.02;
		break;
	case GLUT_KEY_DOWN:
		elev -= 0.02;
		break;
	case GLUT_KEY_LEFT:
		az -= 0.02;
		break;
	case GLUT_KEY_RIGHT:
		az += 0.02;
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

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
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

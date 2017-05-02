#include "../cross-cutting/GpO.h"

char *WINDOW_TITLE = "OPENGL (GpO_01)";
int CurrentWidth = 600, CurrentHeight = 450, fig = 0; // Tamaño ventana, handle a ventana
unsigned FrameCount = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 330 core\n" #src

const char *vertex_prog = GLSL(
	layout(location = 0) in vec3 pos;
	layout(location = 1) in vec3 color;
	out vec3 col;
	uniform mat4 MVP = mat4(1.0f);
	void main() {
		gl_Position = MVP * vec4(pos, 1); // Construyo coord homogéneas y aplico matriz transformacion M
		col = color;					  // Paso color a fragment shader
	});

const char *fragment_prog = GLSL(
	in vec3 col;
	out vec4 outputColor;
	void main() {
		outputColor = vec4(col, 1);
	});

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

objeto crear_objeto(void)
{
	objeto obj;
	GLuint VAO;
	GLuint buffer;

	// Vertices triangulo     
	//  XYZ  data              
	//  RGB  data
	GLfloat vertex_data[] = {0.0f, 0.0000f, 1.0f, 1.0f, 0.0f, 0.0f,
							 0.0f, -0.8660f, -0.5f, 0.0f, 1.0f, 0.0f,
							 0.0f, 0.8660f, -0.5f, 0.0f, 0.0f, 1.0f};

	// Creo y enlazo el VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// TRansfiero datos usando un VBO
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

	// Especifico donde encontrar datos del atributo 0 (pos) del vertex shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	// Defino 2º argumento (atributo 1 = color) del vertex shader
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Asignados atributos, podemos desconectar VBO
	glBindVertexArray(0);			  //Cerramos Vertex Array con todo listo para ser pintado

	obj.VAO = VAO;
	obj.Nv = 3; // Devuelvo objeto VAO + número de vertices en estructura obj

	return obj;
}

GLuint prog;
objeto triangulo;

// Preparación de los datos de los objetos a dibujar, envialarlos a la GPU
// Compilación programas a ejecutar en la tarjeta gráfica:  vertex shader, fragment shaders
// Opciones generales de render de OpenGL
void init_scene()
{
	triangulo = crear_objeto();						// Datos del objeto, mandar a GPU
	prog = LinkShaders(vertex_prog, fragment_prog); // Compile shaders, crear programa a usar, Mandar a GPU
	glUseProgram(prog);								// Indicamos que programa vamos a usar
}

vec3 pos_obs = vec3(10.0f, 0.0f, 0.0f);
vec3 target = vec3(0.0f, 0.0f, 0.0f);
vec3 up = vec3(0, 0, 1);

float fov = 35.0f, aspect = 4.0f / 3.0f;

// Actualizar escena: cambiar posición objetos, nuevos objetros, posición cámara, luces, etc.
void render_scene()
{
	FrameCount++;
	float tt = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // Tiempo en segundos

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Especifica color para el fondo (RGB+alfa)
	glClear(GL_COLOR_BUFFER_BIT);		  // Aplica color asignado borrando el buffer

	///////// Actializaciuon matrices M, V, P  /////////

	mat4 P, V, M, T, R, S;
	T = translate(0.0f, 0.0f, 3 * sin(tt));
	M = T;
	P = perspective(fov, aspect, 0.5f, 20.0f); // 40 FOV,  4:3 ,  Znear=0.5, Zfar=20
	V = lookAt(pos_obs, target, up);		   // Pos camara, Lookat, head up

	mat4 Q = P * V * M;
	transfer_mat4("MVP", Q);

	glBindVertexArray(triangulo.VAO);			 // Activamos VAO asociado al objeto
	glDrawArrays(GL_TRIANGLES, 0, triangulo.Nv); // Orden de dibujar (Nv vertices)
												 // glDrawArrays(GL_POINTS, 0, 3);
	glBindVertexArray(0);

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

void asignacion_eventos_teclado_mouse()
{
	glutKeyboardFunc(keyboard); // Caso de pulsar alguna tecla
								// Teclas de función, cursores, etc: glutSpecialFunc(key_special);       
								// Eventos del ratón: glutMouseFunc(mouse);     
								// Movimiento del ratón: glutPassiveMotionFunc(mouse_mov); 
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
	glutReshapeWindow(CurrentWidth, CurrentHeight + 10);
	printf("Estado %d\n", state);
}

void mouse_mov(int x, int y)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PROGRAMA PRINCIPAL
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
	Init_Opengl();			 // Inicializa OpenGL, comprueba versión.
	init_scene();			 // Preppara escena
	glutMainLoop();			 // Entra en bucle (render_scene se ejecuta constantemente)
	exit(EXIT_SUCCESS);
}

void Init_Opengl(void)
{
	LoadFunctions();
	fprintf(stdout, "OpenGL Version: %s ", glGetString(GL_VERSION));
	if (IsVersionGEQ(3, 3))
		printf(">=3.3 => OK.\n--------------------------\n");
	else
	{
		printf("Se necesita OpenGL >= 3.3 para estos ejemplos.\n");
		glutDestroyWindow(fig);
		exit(EXIT_FAILURE);
	}
}

void Init_Window(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutInitWindowSize(CurrentWidth, CurrentHeight);

	fprintf(stdout, "Pantalla  %d x %d pixels\n", glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - CurrentWidth) / 2, (glutGet(GLUT_SCREEN_HEIGHT) - CurrentHeight) / 2);
	// Creamos ventana centrada en pantalla

	fig = glutCreateWindow("TITULO");
	if (fig < 1)
	{
		fprintf(stderr, "ERROR: Could not create a new rendering window.\n");
		exit(EXIT_FAILURE);
	}
	else
		fprintf(stderr, "Ventana creada (%d)\n--------------------------\n", fig);

	// Asociar funciones de eventos
	glutDisplayFunc(render_scene);  // render, llamada de forma continua en el bucle
	glutReshapeFunc(cambia_window); // Caso de cambiar tamaño de ventana
	glutIdleFunc(IdleFunction);
	glutTimerFunc(200, TimerFunction, 0);
	glutCloseFunc(clean_up);
	glutMouseFunc(mouse);

	asignacion_eventos_teclado_mouse();
}

// Called whenever the window is resized.
void cambia_window(int Width, int Height)
{
	CurrentWidth = Width;
	CurrentHeight = Height;
	aspect = (float)CurrentWidth / (float)CurrentHeight;
	fov = (35.0f / 450.0f) * (float)CurrentHeight;
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

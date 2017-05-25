/************************  GPO           ************************************
ATG, 2016
******************************************************************************/
#include "GpO.h"


char* WINDOW_TITLE="Animacion en OpenGL (GpO)";
int CurrentWidth = 600,   CurrentHeight = 450,  fig1 = 0;  // Tamaño ventana, handle a ventana
unsigned FrameCount = 0;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 330 core\n" #src

const char* vertex_prog = GLSL(
layout(location = 0) in vec3 pos; 
layout(location = 1) in vec3 color;
out vec3 col;
out vec4 gl_Position;
uniform mat4 MVP1=mat4(1.0f);
void main()
 {
  vec4 pos1 =  MVP1*vec4(pos, 1);
  gl_Position = pos1;
  col=color;                 // Paso color a fragment shader
 }
);


const char* fragment_prog = GLSL(
in vec3 col;
out vec3 outputColor;
void main() 
 {
   outputColor = col;
 }
);



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLuint prog;
objeto cilindro;


void clean_up(void) // Borrar los objetos creados (programas, objetos graficos)
{
 glDeleteProgram(prog);
 glDeleteVertexArrays(1, &(cilindro.VAO));
}


void dibujar_indexado(objeto obj)
{
  glBindVertexArray(obj.VAO);              // Activamos VAO asociado al objeto
  glDrawElements(GL_TRIANGLES,obj.Ni,obj.tipo_indice,(void*)0);  // Dibujar (indexado)
  glBindVertexArray(0);
}



// Compilación programas a ejecutar en la tarjeta gráfica:  vertex shader, fragment shaders
// Preparación de los datos de los objetos a dibujar, envialarlos a la GPU
// Opciones generales de render de OpenGL
void init_scene()  
{
  cilindro = cargar_modelo("cilindro3.bix");

  prog=LinkShaders(vertex_prog,fragment_prog); // Compile shaders, crear programa a usar, Mandar a GPU

  //glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);   
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

}


float az = 0;
vec3 pos_obs=vec3(8.0f*sin(az),8.0f*cos(az),2.0f);
vec3 target = vec3(0.0f,0.0f,-2.0f);


// Dibujar objetos 
// Actualizar escena: cambiar posición objetos, nuevos objetros, posición cámara, luces, etc.
void render_scene()
{
	FrameCount++;
	float tt = glutGet(GLUT_ELAPSED_TIME)/1000.0f;  // Tiempo en segundos

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // Especifica color (RGB+alfa)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	///////// Aqui vendría nuestr código para actualizar escena  /////////	
    glUseProgram(prog);    // Indicamos que programa vamos a usar 

	pos_obs = vec3(8.0f*sin(az), 8.0f*cos(az), 2.0f);
	mat4 P = perspective(40.0f, 4.0f / 3.0f, 0.1f, 20.0f);  //40º Y-FOV,  4:3 ,  Znear=0.1, Zfar=20
	mat4 V = lookAt(pos_obs, target,  vec3(0,0,1) );  // Pos camara, Lookat, head up

	mat4 M= mat4(1.0f); // Matriz identidad
	
	transfer_mat4("MVP1",P*V*M);  

    dibujar_indexado(cilindro);

	////////////////////////////////////////////////////////

	glutSwapBuffers();  // Intercambiamos buffers de refresco y display
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////  INTERACCION  TECLADO RATON
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void keyboard(unsigned char, int, int);
void key_special(int, int, int);
void mouse(int,int, int, int);
void mouse_mov(int,int);


void eventos_teclado_mouse()
{
	glutKeyboardFunc(keyboard);      // Caso de pulsar alguna tecla
	glutSpecialFunc(key_special);  // Teclas de función, cursores, etc

//	glutMouseFunc(mouse);           // Eventos del ratón
//	glutPassiveMotionFunc(mouse_mov); // Mov del raón
}


void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	  case 27:  // Escape key
		  glutLeaveMainLoop(); // Salimos del bucle  
		  return;
		  break;
     
	}
}

void key_special(int key, int x, int y)
{
  switch (key)
	{
	  case GLUT_KEY_F1:  // Teclas de Funcion
	    break;
	  case GLUT_KEY_UP:  //Teclas cursor;  
       
		break;         
	  case GLUT_KEY_DOWN:

         break;
	  case GLUT_KEY_LEFT:
		  az += 0.05;
	     break;
      case GLUT_KEY_RIGHT:
		  az -= 0.05;
		break;
	}
}

void mouse(int but,int state, int x, int y)
{
}

void mouse_mov(int x, int y)
{
	fprintf(stdout,"Pos (%3d,%3d)\n",x-CurrentWidth/2,y-CurrentHeight/2);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GLUT:  CREACION VENTANAS E INTERACCION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
void Init_Opengl(void);
void Init_Window(int, char*[]);
void cambia_window(int, int);
void TimerFunction(int);
void IdleFunction(void);
void clean_up(void);


int main(int argc, char* argv[])
{
 Init_Window(argc, argv);  // Prepara y abre ventana
 Init_Opengl();            // Inicializa OpenGL, comprueba versión.
 init_scene();             // Preppara escena
 glutMainLoop();           // Entra en bucle (render_scene se ejecuta constantemente)
 exit(EXIT_SUCCESS);
}

void Init_Opengl(void)
{   
   LoadFunctions();
   fprintf(stdout,"OpenGL Version: %s. ",glGetString(GL_VERSION));
   if(IsVersionGEQ(3,3)) printf("Version >=3.3 => OK.\n");
   else
	{
	 printf("Se necesita OpenGL >= 3.3 para estos ejemplos.\n");
	 glutDestroyWindow(fig1);
	 exit(EXIT_FAILURE);
	}
	
}


void Init_Window(int argc, char* argv[])
{
    glutInit(&argc, argv);
	
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitContextVersion(3,3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    glutInitWindowSize(CurrentWidth, CurrentHeight);

	fprintf(stdout,"Pantalla  %d x %d pixels\n",glutGet(GLUT_SCREEN_WIDTH),glutGet(GLUT_SCREEN_HEIGHT));
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)-CurrentWidth)/2,(glutGet(GLUT_SCREEN_HEIGHT)-CurrentHeight)/2);
	// Creamos ventana centrada en pantalla
   

    fig1 = glutCreateWindow("TITULO");
    if(fig1 < 1) 
	  { 
	    fprintf(stderr,"ERROR: Could not create a new rendering window.\n"); exit(EXIT_FAILURE);
      }
	else   fprintf(stderr,"Ventana creada (%d)\n",fig1);


	// Asociar funciones de eventos
	glutDisplayFunc(render_scene);   //render, llamada de forma continua en el bucle
	glutReshapeFunc(cambia_window); // Caso de cambiar tamaño de ventana
    glutIdleFunc(IdleFunction);
    glutTimerFunc(200, TimerFunction, 0);
	glutCloseFunc(clean_up);


	eventos_teclado_mouse();

}

//Called whenever the window is resized. 
void cambia_window(int Width, int Height)
{
    CurrentWidth = Width; CurrentHeight = Height;
	
    glViewport(0, 0, CurrentWidth, CurrentHeight);
}

void IdleFunction(void)
{
    glutPostRedisplay();
}
 

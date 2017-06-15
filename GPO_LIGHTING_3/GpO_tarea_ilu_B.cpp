/************************  GPO_03 ************************************
ATG, 2015
******************************************************************************/
#include "../cross-cutting/GpO.h"


char* WINDOW_TITLE="ILUMINACION en OpenGL (GpO)";
int CurrentWidth = 600,   CurrentHeight = 450,  WindowHandle = 0;  // Tama�o ventana, handle a ventana
unsigned FrameCount = 0;



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 330 core\n" #src

const char* vertex_prog = GLSL(
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 n;
out vec3 n_modif;

uniform mat4 MVP;
uniform mat4 M_adj;
uniform mat4 M;

out vec3 pos_escena;

void main(){	
	gl_Position =  MVP*vec4(pos,1);
	n_modif = (M_adj*vec4(n,0.0)).xyz;
	pos_escena = (M*vec4 (pos, 1)).xyz;
}
);

const char* fragment_prog = GLSL(
in vec3 n_modif;
out vec3 color;
in vec3 pos_escena;

uniform vec3 pos_light;
const vec3 lightcolor=vec3(1.0f,1.0f,0.8f);  // Color luz

void main()
{
	vec3 L;
	float d;
	vec3 aux = pos_light - pos_escena;

	d = length (aux);
	L = normalize (aux);
	float I = 1 / (1 + 0.5 * pow (d, 2));

	vec3 n=normalize(n_modif);
	float ilu=0.1+0.9*clamp(dot(L,n),0,1); 
	color = ilu*lightcolor;
}
);



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLuint prog;
mat4 MVP,Projection,View,M;


float az=1.57f, elev=0.5f;  vec3 lightdir;   // Direcci�n luz lejana

void clean_memory(void)
{
 glDeleteProgram(prog);
}

// Dibuja objeto indexado
void dibujar_indexado(objeto obj)
{
  glBindVertexArray(obj.VAO);              // Activamos VAO asociado al objeto
  glDrawElements(GL_TRIANGLES,obj.Ni,obj.tipo_indice,(void*)0);  // Dibujar (indexado)
  glBindVertexArray(0);
}

objeto modelo;

objeto crear_cuadrado(void)
{
  objeto obj;
  GLuint VAO;
  GLuint buffer,i_buffer;

  // Just one square = 2 triangles = 4 vertex
  
static const GLfloat vertex_data[] = {
	-2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 1.0f,   //0
	2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 1.0f,  //1
	2.0f, -2.0f, 0.0f, 0.0f, 0.0f, 1.0f,  //2
	-2.0f, -2.0f, 0.0f, 0.0f, 0.0f, 1.0f}; //3
	 
    GLbyte indices[]={ 0,1,2, 0,2,3}; 
	obj.Ni=2*3;
	obj.tipo_indice=GL_UNSIGNED_BYTE; 

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

	
	// Especifico 1er argumento (atributo 0) del vertex shader
	glEnableVertexAttribArray(0); 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);

	// Especifico 2� argumento (atributo 1) del vertex shader
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
	
	// Especifico argumentos adicionales del vertex shader


	glBindBuffer(GL_ARRAY_BUFFER, 0);  // Asignados atributos, podemos desconectar BUFFER

	glGenBuffers(1, &i_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),indices, GL_STATIC_DRAW);
			
	glBindVertexArray(0);  //Cerramos Vertex Array con todo lidto para ser pintado

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	obj.VAO=VAO; 
	
	return obj;

}



void init_scene()
{
	prog=LinkShaders(vertex_prog,fragment_prog); // Compile shaders
	glUseProgram(prog);

	Projection = glm::perspective(50.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	View       = glm::lookAt(vec3(4,0,1.0),vec3(0,0,0),vec3(0,0,1));

	modelo=crear_cuadrado(); 

	lightdir=vec3(cos(elev)*cos(az),cos(elev)*sin(az),sin(elev));
	transfer_vec3("L",lightdir);

}

vec3 pos_light = vec3 (0.0f, 0.0f, 0.5f);

void render_scene()
{
	FrameCount++;
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	float tt = glutGet(GLUT_ELAPSED_TIME)/1000.0f;

	mat4 R = rotate(20 * sin(tt), vec3(1,0,0)); 
	M = R;

	MVP        =  Projection * View * M; 
	mat4 M_adj=transpose(inverse(M));


	transfer_mat4("MVP",MVP);
	transfer_mat4("M_adj",M_adj);
	transfer_mat4("M", M);

	transfer_vec3("pos_light", pos_light);

	lightdir = vec3(cos(elev)*cos(az), cos(elev)*sin(az), sin(elev));
	transfer_vec3("L", lightdir);

	dibujar_indexado(modelo);

	glutSwapBuffers();
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
//	glutMouseFunc(mouse);           // Eventos del rat�n
	glutSpecialFunc(key_special);  // Teclas de funci�n, cursores, etc
//	glutPassiveMotionFunc(mouse_mov); // Mov del ra�n
}


void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	  case 27:
		  glutLeaveMainLoop();
		  return;

	}
}

void key_special(int key, int x, int y)
{
  switch (key)
	{
	  case GLUT_KEY_F1:  // Teclas de Funcion
	    break;
	  case GLUT_KEY_F2:  // Teclas de Funcion
	    break;
	  case GLUT_KEY_UP:  //Teclas cursor;  
		  pos_light.z += 0.05f; 
		  break;         
	  case GLUT_KEY_DOWN:
		  pos_light.z -= 0.05f; 
		  break;
	  case GLUT_KEY_LEFT:
		  pos_light.y -= 0.05f; 
		  break;
      case GLUT_KEY_RIGHT:
		  pos_light.y += 0.05f; 
		  break;
	}
  	
   printf("Az %.2f  El %.2f :: ",az*180/3.14,elev*180/3.14); 
   printf("Pos L %.2f %.2f %.2f\n",pos_light.x,pos_light.y,pos_light.z);
   
}


int ini_x,ini_y;

void mouse(int but,int state, int x, int y)
{
 //fprintf(stdout,"But %d  State %d  Pos (%3d,%3d)\n",but,state,x,y);
 
 switch(but)
 {
 case 0: 	  break;
 case 3:      break;
 case 4:      break;
 }

}

void mouse_mov(int x, int y)
{
//	fprintf(stdout,"Pos (%3d,%3d)\n",x-CurrentWidth / 2,y-CurrentHeight / 2);

	//glutWarpPointer(CurrentWidth / 2, CurrentHeight / 2);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GLUT:  CREACION VENTANAS E INTERACCION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
void Initialize(int, char*[]);
void InitWindow(int, char*[]);
void reshape_window(int, int);
void TimerFunction(int);
void IdleFunction(void);



int main(int argc, char* argv[])
{
 Initialize(argc, argv);
 init_scene();
 glutMainLoop();
 clean_memory();
 exit(EXIT_SUCCESS);
}

void Initialize(int argc, char* argv[])
{
   InitWindow(argc, argv);
   
   LoadFunctions();
   fprintf(stdout,"OpenGL Version:  ",glGetString(GL_VERSION));
   if(IsVersionGEQ(3,3)) printf(">=3.3 => OK.\n");
   else
	{
	 printf("Se necesita OpenGL >= 3.3 para estos ejemplos.\n");
	 glutDestroyWindow(WindowHandle);
	 exit(EXIT_FAILURE);
	}
	
}


void InitWindow(int argc, char* argv[])
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
   

    WindowHandle = glutCreateWindow("PRIMERA VENTANA");
    if(WindowHandle < 1) 
	  { 
	    fprintf(stderr,"ERROR: Could not create a new rendering window.\n"); exit(EXIT_FAILURE);
      }
	else   fprintf(stderr,"Ventana creada (%d)\n",WindowHandle);


	// Asociar funciones de eventos
	glutDisplayFunc(render_scene);   //render, llamada de forma continua en el bucle
	glutReshapeFunc(reshape_window); // Caso de cambiar tama�o de ventana
    glutIdleFunc(IdleFunction);
    glutTimerFunc(0, TimerFunction, 0);


	eventos_teclado_mouse();

}

//Called whenever the window is resized. The new window size is given, in pixels.
//This is an opportunity to call glViewport or glScissor to keep up with the change in size.
void reshape_window(int Width, int Height)
{
    CurrentWidth = Width;
    CurrentHeight = Height;
    glViewport(0, 0, CurrentWidth, CurrentHeight);
}

void IdleFunction(void)
{
    glutPostRedisplay();
}
 





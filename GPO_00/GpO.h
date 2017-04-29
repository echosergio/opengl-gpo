//#define LINUX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// GL_LOAD LIB
#include <glload/gl_3_3.h>


#include <glload/gl_load.hpp> 
using namespace glload;



// FREE_GLUT LIB
#include <GL/freeglut.h>

// GLM LIB 
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;


// DEclaraciones  de tipos
typedef struct {GLuint VAO; GLuint Ni; GLuint Nv; GLuint Nt; GLuint tipo_indice; } objeto;



/// DECLARACIONES DE FUNCIONES AUXILIARES

void vuelca_mat4(mat4 M);

GLuint LinkShaders(const char*,const char*);
GLuint cargar_textura_from_bmp(const char *,GLuint);
objeto cargar_modelo(char*);

void transfer_mat4(char*, mat4);
void transfer_mat3(char*, mat3);
void transfer_vec4(char*, vec4);
void transfer_vec3(char*, vec3);
void transfer_vec2(char*, vec2);
void transfer_int(char*, GLuint);
void transfer_float(char*, GLfloat);



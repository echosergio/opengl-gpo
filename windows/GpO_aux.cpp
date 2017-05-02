#include "GpO.h"


////////////////////   

#define MAX_TITLE 128
#define REFRESCO_msec 500  // 500 msec de refresco de titulo en ventana
extern char* WINDOW_TITLE;
extern unsigned FrameCount;
extern int CurrentWidth, CurrentHeight;

void TimerFunction(int Value)
{
  char title[MAX_TITLE];
  int F=1000/REFRESCO_msec;
#ifdef LINUX
  snprintf(title,MAX_TITLE,"%s: %4d FPS @ %d x %d",WINDOW_TITLE, FrameCount*F,CurrentWidth,CurrentHeight);
#else
  sprintf_s(title,MAX_TITLE,"%s: %4d FPS @ %d x %d",WINDOW_TITLE, FrameCount*F,CurrentWidth,CurrentHeight);
#endif
 
  glutSetWindowTitle(title);
       
  FrameCount = 0;
  glutTimerFunc(REFRESCO_msec, TimerFunction, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CARGA, COMPILACION Y LINKADO DE LOS SHADERS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLuint compile_shader(const char* Shader_source, GLuint type)
{

  GLuint ShaderID = glCreateShader(type);  // Create shader object

  //char const * VertexSourcePointer = vertexShader_source;
  glShaderSource(ShaderID, 1, &Shader_source , NULL);    // Compile Shader
  glCompileShader(ShaderID);

  GLint Result = GL_FALSE;
  int InfoLogLength;
  char error[512]; 

  glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
  if (Result==GL_TRUE) fprintf(stdout,"OK\n");  
  else 
  {
   fprintf(stdout,"ERRORES\n");  
   glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
   if(InfoLogLength>512) InfoLogLength=512;
   glGetShaderInfoLog(ShaderID, InfoLogLength, NULL, error);
   fprintf(stdout,"\n%s\n", error);
  }

 return ShaderID;
}

void check_program(GLuint id)
{
 GLint Result = GL_FALSE;
 int InfoLogLength;
 char error[512]; 

 glGetProgramiv(id, GL_LINK_STATUS, &Result);
 if (Result==GL_TRUE) fprintf(stdout,"OK\n"); // Compiled OK
 else 
	{
     fprintf(stdout,"ERRORES\n");  
     glGetProgramiv(id, GL_INFO_LOG_LENGTH, &InfoLogLength);
	 if(InfoLogLength<1) InfoLogLength=1; if(InfoLogLength>512) InfoLogLength=512;
     glGetProgramInfoLog(id, InfoLogLength, NULL, error);
     fprintf(stdout, "\n%s\n",error);
	}
}


GLuint LinkShaders(const char* vertexShader_source,const char*fragmentShader_source)
{
	printf("--------------------------------\n");
	printf("Compiling Vertex Shader :: ");
	GLuint VertexShaderID = compile_shader(vertexShader_source, GL_VERTEX_SHADER);
	printf("Compiling Fragment Shader :: ");
	GLuint FragmentShaderID = compile_shader(fragmentShader_source,GL_FRAGMENT_SHADER);

    // Link the program
    fprintf(stdout, "Linking program en GPU: ");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
	check_program(ProgramID);
    

	glDetachShader(ProgramID,VertexShaderID);
	glDetachShader(ProgramID,FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

	printf("--------------------------------\n");
    return ProgramID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////  AUXILIARES 
/////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char* loadBMP(const char * imagepath, unsigned int* ancho, unsigned int* alto)
{

	printf("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	//FILE * file = fopen(imagepath,"rb");

	FILE* file;

	fopen_s(&file,imagepath,"rb");

	if (!file) {printf("%s could not be opened. Are you in the right directory ?\n", imagepath); getchar(); return 0;}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if ( fread(header, 1, 54, file)!=54 ){ 
		printf("Not a correct BMP file\n");
		return 0;
	}
	// A BMP files always begins with "BM"
	if ( header[0]!='B' || header[1]!='M' ){
		printf("Not a correct BMP file\n");
		return 0;
	}
	// Make sure this is a 24bpp file
	if ( *(int*)&(header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    return 0;}
	if ( *(int*)&(header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    return 0;}

	// Read the information about the image
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);

	printf("BMP:: %d  %d  %d\n",height,width,imageSize);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char [imageSize];

	// Read the actual data from the file into the buffer
	fread(data,1,imageSize,file);

	// Everything is in memory now, the file wan be closed
	fclose (file);

	*ancho=width;
	*alto=height;

	return data;
}

GLuint cargar_textura_from_bmp(const char * imagepath, GLuint tex_unit)
{
 unsigned int width, height;
 unsigned char * data = loadBMP(imagepath,&width,&height);

  glActiveTexture(tex_unit);
  glBindTexture(GL_TEXTURE_2D, 0); 
	
	GLuint textureID;
	glGenTextures(1, &textureID);             // Crear objeto textura
	glBindTexture(GL_TEXTURE_2D, textureID);  // "Bind" la textura creada
	
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);  //Pasa datos a GPU
	glGenerateMipmap(GL_TEXTURE_2D);
	
	delete [] data;  // Libera memoria

	// Opciones de muestreo, magnificación, coordenadas fuera del borde, etc.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	

	//glBindTexture(GL_TEXTURE_2D, 0); 
	// DEvolvemos ID de la textura creada y cargada con la imagen
	return textureID;
}


objeto cargar_modelo(char* fichero)
{
	objeto obj;
	GLuint VAO;
	GLuint buffer,i_buffer;
	unsigned char s_index=4;
	

	GLuint N_vertices, N_caras, N_indices; 

	unsigned char *vertex_data; 
	unsigned char *indices; 


	 FILE* fid;
     fopen_s(&fid,fichero,"rb");

	 if (fid==NULL) { 
		 printf("Error al abrir fichero  %s\n",fichero); 
		 obj.VAO=0; obj.Ni=0; obj.tipo_indice=0;
	     return obj;
	 }

	 fread((void*)&N_caras,4,1,fid);    
	 fread((void*)&N_indices,4,1,fid); 
	 //N_indices=3*N_caras; 
	 fread((void*)&N_vertices,4,1,fid); 

	 fseek(fid,0,SEEK_END);
	 unsigned int s_fichero=ftell(fid);

	

	 fseek(fid,12,SEEK_SET);
	 GLuint tipo=GL_UNSIGNED_INT;
	 if (N_vertices<=65536) { tipo=GL_UNSIGNED_SHORT; s_index=2;}
	 if (N_vertices<=256)   { tipo=GL_UNSIGNED_BYTE; s_index=1;}

	 GLuint bytes_indices=N_indices*s_index;
	 GLuint bytes_data = s_fichero-12-bytes_indices;
	 GLuint datos_per_vertex=((bytes_data/4)/N_vertices);

	 printf("Leyendo modelo de %s: (%d bytes)\n",fichero,s_fichero);
	  printf("%d vertices, %d triangulos. Lista de %d indices\n",N_vertices,N_caras,N_indices);
	// printf("%d vertices, %d triangulos\n",N_vertices,N_caras);
	 printf("Indices guardados en enteros de %d bytes\n",s_index);
	 printf("%d datos por vertice\n",datos_per_vertex);

	 obj.Ni=N_indices;
	 	 obj.Nv=N_vertices;
	 obj.Nt=N_caras;
	 obj.tipo_indice=tipo; 

	 vertex_data=(unsigned char*)malloc(N_vertices*datos_per_vertex*4); 
	 if (vertex_data==NULL) printf("ptr NULL\n"); 
	 indices=(unsigned char*)malloc(N_indices*s_index); 
	 if (indices ==NULL) printf("ptrindices NULL\n"); 

     fread((void*)vertex_data,4,datos_per_vertex*N_vertices,fid);
     fread((void*)indices,s_index,N_indices,fid);

	 fclose(fid);


    glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, N_vertices*datos_per_vertex*4, vertex_data, GL_STATIC_DRAW);

	// Defino 1er argumento (atributo 0) del vertex shader (siempre XYZ)
	glEnableVertexAttribArray(0); 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), 0);

	switch (datos_per_vertex)
	{
	case 3:   break;
	case 5:   // 2º atributo = UV
       	glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), (void*)(3*sizeof(float)));
		break;
	case 6:   // 2º atributo = (nx,ny,nz)
       	glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), (void*)(3*sizeof(float)));
		break;
	case 8:   // 2º atributo = UV, 3º atributo = (nx,ny,nz)
       	glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(2);
	    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), (void*)(5*sizeof(float)));
		break;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);  // Asignados atributos, podemos desconectar BUFFER

	glGenBuffers(1, &i_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, N_indices*s_index,indices, GL_STATIC_DRAW);
	
		
	glBindVertexArray(0);  //Cerramos Vertex Array con todo lidto para ser pintado

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	obj.VAO=VAO; 
	

	//GLuint k;
	//for (k=0;k<12;k++) printf("%.1f ",vertex_data[k]);
	//for (k=0;k<12;k++) printf("%1d ",indices[k]);

	// Una vez transferido datos liberamos memoria en CPU
	free((void*)vertex_data);
	free((void*)indices);


	return obj;

}



objeto cargar_modelo_bad(char* fichero)
{
	objeto obj;
	GLuint VAO;
	GLuint buffer,i_buffer;
	unsigned char s_index=4;
	

	GLuint N_vertices, N_caras, N_indices; 

	unsigned char *vertex_data; 
	unsigned char *indices; 


	 FILE* fid;
         fopen_s(&fid,fichero,"rb");

	 if (fid==NULL) { 
		 printf("Error al abrir fichero  %s\n",fichero); 
		 obj.VAO=0; obj.Ni=0; obj.tipo_indice=0;
	     return obj;
	 }

	 fread((void*)&N_caras,4,1,fid);  
	  fread((void*)&N_indices,4,1,fid); //N_indices=3*N_caras; 
	 fread((void*)&N_vertices,4,1,fid); 
	

	 fseek(fid,0,SEEK_END);
	 unsigned int s_fichero=ftell(fid);

	

	 fseek(fid,12,SEEK_SET);
	 GLuint tipo=GL_UNSIGNED_INT; s_index=4;
	 if (N_vertices<=65536) { tipo=GL_UNSIGNED_SHORT; s_index=2;}
	 if (N_vertices<=256)   { tipo=GL_UNSIGNED_BYTE; s_index=1;}

	 GLuint bytes_indices=N_indices*s_index;
	 GLuint bytes_data = s_fichero-12-bytes_indices;
	 GLuint datos_per_vertex=((bytes_data/4)/N_vertices);

	 printf("Leyendo modelo de %s: (%d bytes)\n",fichero,s_fichero);
	 printf("%d vertices, %d triangulos. Lista de %d indices\n",N_vertices,N_caras,N_indices);
	 printf("Indices guardados en enteros de %d bytes\n",s_index);
	 printf("%d datos por vertice\n",datos_per_vertex);

	 obj.Ni=N_indices;
	 obj.Nv=N_vertices;
	 obj.Nt=N_caras;
	 obj.tipo_indice=tipo; 

	 vertex_data=(unsigned char*)malloc(N_vertices*datos_per_vertex*4); 
	 if (vertex_data==NULL) printf("ptr NULL\n"); 
	 indices=(unsigned char*)malloc(N_indices*s_index); 
	 if (indices ==NULL) printf("ptrindices NULL\n"); 

     fread((void*)vertex_data,4,datos_per_vertex*N_vertices,fid);
     fread((void*)indices,s_index,N_indices,fid);

	 fclose(fid);


    glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, N_vertices*datos_per_vertex*4, vertex_data, GL_STATIC_DRAW);

	// Defino 1er argumento (atributo 0) del vertex shader (siempre XYZ)
	glEnableVertexAttribArray(0); 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), 0);

	switch (datos_per_vertex)
	{
	case 3:   break;
	case 5:   // 2º atributo = UV
       	glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), (void*)(3*sizeof(float)));
		break;
	case 6:   // 2º atributo = (nx,ny,nz)
       	glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), (void*)(3*sizeof(float)));
		break;
	case 8:   // 2º atributo = UV, 3º atributo = (nx,ny,nz)
       	glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(2);
	    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), (void*)(5*sizeof(float)));
		break;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);  // Asignados atributos, podemos desconectar BUFFER

	glGenBuffers(1, &i_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, N_indices*s_index,indices, GL_STATIC_DRAW);
	
		
	glBindVertexArray(0);  //Cerramos Vertex Array con todo lidto para ser pintado

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	obj.VAO=VAO; 
	
	// Una vez transferido datos liberamos memoria en CPU
	free((void*)vertex_data);
	free((void*)indices);

	return obj;

}


void transfer_mat4(char* name, mat4 M)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);

 loc=glGetUniformLocation(prog,name); 
// if(loc==-1) printf("No existe variable %s en programa especificado (%d)\n",name,prog); 
 glUniformMatrix4fv(loc, 1, GL_FALSE, &M[0][0]);
}

void transfer_mat3(char* name, mat3 M)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);

 loc=glGetUniformLocation(prog,name); 
// if(loc==-1) printf("No existe variable %s en programa especificado (%d)\n",name,prog); 
 glUniformMatrix3fv(loc, 1, GL_FALSE, &M[0][0]);
}


void transfer_vec4(char* name, vec4 x)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 // if(loc==-1) printf("No existe variable %s en programa especificado (%d)\n",name,prog); 
 glUniform4fv(loc, 1, &x[0]);
}

void transfer_vec3(char* name, vec3 x)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 // if(loc==-1) printf("No existe variable %s en programa especificado (%d)\n",name,prog); 
 glUniform3fv(loc, 1, &x[0]);
}

void transfer_vec2(char* name, vec2 x)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 // if(loc==-1) printf("No existe variable %s en programa especificado (%d)\n",name,prog); 
 glUniform2fv(loc, 1, &x[0]);
}

void transfer_int(char* name, GLuint valor)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
// if(loc==-1) printf("No existe variable %s en programa especificado (%d)\n",name,prog); 
 glUniform1i(loc,valor);
}


void transfer_float(char* name, GLfloat valor)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
// if(loc==-1) printf("No existe variable %s en programa especificado (%d)\n",name,prog); 
 glUniform1f(loc,valor);
}


void vuelca_mat4(glm::mat4 M)
{
	int j, k;
	printf("--------------------------------------\n");
	for (k = 0; k<4; k++) { for (j = 0; j<4; j++) printf("%6.3f ", M[j][k]); printf("\n"); }
	printf("--------------------------------------\n");
}
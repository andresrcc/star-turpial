#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <string.h>
#include "glm.h"
#include <math.h>

#define INERTIA_DELTA 0.001 //Delta de velocidad en el cual se considera que un objeto se detuvo completamente

//registro que representa un vector de 3 dimensiones
struct vector{
        float x;
        float y;
        float z;
};typedef struct vector vector3D;

//registro que representa a un objeto
struct objeto{
	float time; //tiempo que ha pasado desde la ultima actualizacion
        int state; //estado del objeto: 1. visible, 2.no-visible
        float maxVel; //velocidad maxima que puede tener el objeto
        float inertia; //modulo de aceleracion con que se desacelera el objeto al no verse afectado por fuerzas externas
        float engineAcc; //aceleracio que es proporcionada por un motor
        vector3D pos; //vector posicion
        vector3D vel; //vector velocidad
        vector3D acc; //vectos aceleracion
};typedef struct objeto figura;


int movement_pressed_x = 0; //1. hay una tecla presionada que afecta el movimiento en el eje x, 2.'1' es falso
int movement_pressed_y = 0; //1. hay una tecla presionada que afecta el movimiento en el eje y, 2.'1' es falso
int movement_pressed_z = 0; //1. hay una tecla presionada que afecta el movimiento en el eje z, 2.'1' es falso
GLuint texture; //textura de fondo
int frames = 0;//conteo de frames
int time_a = 0.0;
int rate = 1;
int rate_store = 0;
char rate_str[10];
int time_d = 0.0;
float fps = 0.0;
char *FPS[10];
int puntos = 0;
char puntaje[20];
figura objetos[2]; //arreglo con todos los objetos, no hay razon alguna para este tamano, se arreglara cuando sepamos el numero maximo de objetos
//objetos[0] : Nave
//objetos[10] : bala


/**
 *Objetos
 * Anillos
 * Blancos
 * Turpial Cosmico (Nave)
 */

GLMmodel *modelo_TURPIAL = NULL;
GLMmodel *modelo_BLANCO = NULL;
GLMmodel *modelo_ANILLO = NULL;

//Para guardar objetos quadratic. Estos sirven para
//crear los anillos y blancos con funciones de la libreria GLU
GLUquadricObj *quadratic;


void print_pantalla(int x, int y, char *string)
{
  glPushMatrix();
 // glColor3f( 0, 1, 0 );
  glRasterPos3f(x, y, objetos[0].pos.z);
  int len, i;
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
  }
  glPopMatrix();
}



/*
 * Esta funcion recibe una figura y calcula su posicion y velocidad respecto al tiempo
 */
void movimiento(figura *fig){
	//Si la figura sale del volumen de proyeccion no se calcula nada
	if((*fig).pos.z-objetos[0].pos.z < -30){
		(*fig).state = 0;
		return;
	}
	//Se calcula nTime, el tiempo que ha pasado desde la ultima llamada a esta funcion con este parametro fig
	float nTime = 0-(*fig).time;
	(*fig).time = glutGet(GLUT_ELAPSED_TIME)*0.001;
        nTime += (*fig).time;

        //se calcula el cambio de velocidad debido a la aceleracion
        //para cada caso si la velocidad es menor que el maximo permitido se recalcula,
        //de lo contrario se usa la velocidad maxima.

        //velocidad en x
        if((*fig).vel.x <= (*fig).maxVel && (*fig).vel.x >= -(*fig).maxVel){
                (*fig).vel.x += (*fig).acc.x*nTime;
        }else if((*fig).vel.x > (*fig).maxVel){
                (*fig).vel.x = (*fig).maxVel;
        }else{
		(*fig).vel.x = -(*fig).maxVel;
	}

        //velocidad en y
        if((*fig).vel.y <= (*fig).maxVel && (*fig).vel.y >= -(*fig).maxVel ){
                (*fig).vel.y += (*fig).acc.y*nTime;
        }else if((*fig).vel.y > (*fig).maxVel){
                (*fig).vel.y = (*fig).maxVel;
        }else{
		(*fig).vel.y = -(*fig).maxVel;
	}

        //velocidad en z
        if((*fig).vel.z <= (*fig).maxVel && (*fig).vel.z >= -(*fig).maxVel){
                (*fig).vel.z += (*fig).acc.z*nTime;
        }else if((*fig).vel.z > (*fig).maxVel){
                (*fig).vel.z = (*fig).maxVel;
        }else{
		(*fig).vel.z = -(*fig).maxVel;
	}



        //se calcula el cambio en la posicion debido a la velocidad
        (*fig).pos.x += (*fig).vel.x*nTime;
        (*fig).pos.y += (*fig).vel.y*nTime;
        (*fig).pos.z += (*fig).vel.z*nTime;

}


/*
 * Esta funcion recibe una figura y calcula su posicion respecto al tiempo, considera que la figura puede ser movida por el usuario
 */
void movimiento_nave(figura *fig){
	//Si hay alguna tecla de movimiento presionada que afecte el movimiento en el eje x, se aplica una desaceleracion o
	//o se detiene completamente si el modulo de su velocidad  esta dentro del rango dado por INERTIA_DELTA
        if(movement_pressed_x == 0){
                if((*fig).vel.x >= -INERTIA_DELTA && (*fig).vel.x <= INERTIA_DELTA){
                        (*fig).acc.x = 0;
                        (*fig).vel.x = 0;
                }else if((*fig).vel.x > INERTIA_DELTA){
                        (*fig).acc.x = -(*fig).inertia;
                }else{
                        (*fig).acc.x = (*fig).inertia;
                }

        }

	//Equivalente para teclas que afecten el movimiento en el eje y
	if(movement_pressed_y == 0){
                if((*fig).vel.y >= -INERTIA_DELTA && (*fig).vel.y <= INERTIA_DELTA){
                        (*fig).acc.y = 0;
                        (*fig).vel.y = 0;
                }else if((*fig).vel.y > INERTIA_DELTA){
                        (*fig).acc.y = -(*fig).inertia;
                }else{
                        (*fig).acc.y = (*fig).inertia;
                }
        }
	movimiento(fig);
}


void dibujar_blanco(){
  glPushMatrix();
  //glClear(GL_COLOR_BUFFER_BIT);
  //glLoadIdentity();
  glTranslatef(-2.0,0.0,-3.0);
  glColor3f(0.2,0.1,0.4);
  //gluDisk(quadratic,0.5,3.5,32,32);
  glutSolidTorus(0.05,0.5,20,20);
  glPopMatrix();

  glFlush();
}



/*
 * un cubo con la anchura, altura y profundidad dados
 */
void dibujar_cubo(GLfloat anch, GLfloat alt, GLfloat prof){

	anch = anch/2;
	alt = alt/2;
	prof = prof/2;
	glBegin(GL_QUADS);
		glNormal3f(0.0f,1.0f,0.0f);
		glVertex3f(anch,prof,alt);
		glVertex3f(anch,prof,-alt);
		glVertex3f(-anch,prof,-alt);
		glVertex3f(-anch,prof,alt);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(1.0f,0.0f,0.0f);
		glVertex3f(anch,prof,alt);
		glVertex3f(anch,prof,-alt);
		glVertex3f(anch,-prof,-alt);
		glVertex3f(anch,-prof,alt);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(0.0f,-1.0f,0.0f);
		glVertex3f(anch,-prof,alt);
		glVertex3f(anch,-prof,-alt);
		glVertex3f(-anch,-prof,-alt);
		glVertex3f(-anch,-prof,alt);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(-1.0f,0.0f,0.0f);
		glVertex3f(-anch,-prof,alt);
		glVertex3f(-anch,-prof,-alt);
		glVertex3f(-anch,prof,-alt);
		glVertex3f(-anch,prof,alt);
	glEnd(); 
	glBegin(GL_QUADS);
		glNormal3f(0.0f,0.0f,1.0f);
		glVertex3f(-anch,prof,alt);
		glVertex3f(-anch,-prof,alt);
		glVertex3f(anch,-prof,alt);
		glVertex3f(anch,prof,alt);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(0.0f,0.0f,-1.0f);
		glVertex3f(-anch,prof,-alt);
		glVertex3f(anch,prof,-alt);
		glVertex3f(anch,-prof,-alt);
		glVertex3f(-anch,-prof,-alt);
	glEnd();
}

/*
 * Funcion que dibuja a todos los objetos que se veran en la escena
 */
void dibujar_objetos(){
  int i = 0;
  glPushMatrix();
	//Se dibuja la nave
  	glTranslatef(objetos[0].pos.x,objetos[0].pos.y,objetos[0].pos.z);
  	glRotatef(-180.0,0,1,0);
  //	glRotatef(90.0,1,0,0); 
  	glmUnitize(modelo_TURPIAL);
 	glmFacetNormals(modelo_TURPIAL);
  	glmVertexNormals(modelo_TURPIAL, 90.0);
  	glEnable(GL_COLOR_MATERIAL);
  	glmDraw(modelo_TURPIAL, GLM_SMOOTH | GLM_MATERIAL);
  glPopMatrix();
  //Se sibujan los demas objetos
  for (i =1 ; i < 2 ; i++){
    if(objetos[i].state != 0){
      glPushMatrix();
      glTranslatef(objetos[i].pos.x,objetos[i].pos.y,objetos[i].pos.z);
      dibujar_cubo(0.1,0.5,0.1);
      glPopMatrix();
    }
  }

  dibujar_blanco();
  //se le indica a glut que la escena debe ser redibujada
  glutPostRedisplay();
}


/*
 * Funcion que nos permite cargar en memoria una textura a partir de un archivo en formato BMP
 */
GLuint loadBMP(char * imagepath){
        unsigned char header[54];
        unsigned int dataPos;
        unsigned int width, height;
        unsigned int imageSize;
        unsigned char * data;
        FILE * file = fopen(imagepath, "rb");
        if (fread(header,1,54,file)!=54){
//              return false;
        }
        if (header[0]!='B' || header[1]!='M'){
//              return 0;
        }
        dataPos = *(int*)&(header[0x0A]);
        imageSize = *(int*)&(header[0x22]);
        width = *(int*)&(header[0x12]);
        height = *(int*)&(header[0x16]);
        if(imageSize==0){
                imageSize = width*height*3;
        }
        if(dataPos == 0){
                dataPos = 54;
        }
        data = (unsigned char*)malloc(imageSize*sizeof(unsigned char));
        fread(data,1,imageSize,file);
        fclose(file);
        GLuint textureID;
        glGenTextures(1,&textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_BGR_EXT,GL_UNSIGNED_BYTE,data);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	return textureID;
}


/*
 * Funcion que dibuja el fondo
 */
void drawBackground() {
	glPushMatrix(); 
        	glEnable(GL_TEXTURE_2D);
		float z = objetos[0].pos.z - 30.0; 
        	glBegin( GL_QUADS ); 
        	        glNormal3d(0,0,-1); 
        	        glTexCoord2d(0.0f,0.0f); glVertex3d(-22.0,-14.5,z); 
        	        glTexCoord2d(1.0f,0.0f); glVertex3d(22.0,-14.5,z); 
        	        glTexCoord2d(1.0f,1.0f); glVertex3d(22.0,14.5,z); 
        	        glTexCoord2d(0.0f,1.0f); glVertex3d(-22.0,14.5,z); 
        	glEnd(); 
        	glDisable(GL_TEXTURE_2D); 
        glPopMatrix();



	}







/*
 * Funcion display de glut que dibuja de nuevo la escena
 */
void display(){
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  //se calcula el movimiento de la nave
  movimiento_nave(&objetos[0]);
  //se calcula el movimiento de los demas objetos
  int i;
  for (i = 1 ; i < 2 ; i++){
    figura * f = &objetos[i];
    if((*f).state == 1){
      movimiento(f);
    }
  }

  glClearColor(0.0,0.0,0.0,0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GLfloat lData4[] = {-1,1,0,0.0};
  glLightfv(GL_LIGHT0, GL_POSITION, lData4);
  GLfloat lData5[] = {1,0,0};
  //Camara
  gluLookAt(0.0, //Coordenada X
	    0.0, //Coordenada Y
	    objetos[0].pos.z+15.0, //Coordenada Z
	    0.0, 0.0, objetos[0].pos.z, //Posicion Inicial Camara
	    0.0, 1, 0.0 //Vector UP En este caso, Z
	    );
  glDisable(GL_LIGHTING);
  drawBackground();
  glEnable(GL_LIGHTING);
  dibujar_objetos();
  sprintf(rate_str,"Rate : %d",rate);
  print_pantalla(0,4,rate_str); 
  print_pantalla(-7,-4,FPS);//Se imprime el FPS
  sprintf(puntaje,"Puntos: %d",puntos);
  print_pantalla(4,-4,puntaje);//Se imprime el puntaje
  glutSwapBuffers();
}

/**
 * Acciones a llevar a cabo si se cambia el
 * tamaño de la ventana
 */
void cambios_ventana(int w, int h){

  float aspectratio;
  if (h == 0)
    h=1;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);

  glLoadIdentity();

  aspectratio = (float) w / (float) h;

  gluPerspective(35.0, aspectratio, 1.0, 300.0);

  glutPostRedisplay();

}




/*
 * recibe el input del usuario y mueve la nave
 */

void teclado (unsigned char tecla, int x, int y){
  float acc = objetos[0].engineAcc;
  switch(tecla){
     case 27:
       exit(0);
     case 'w':
	movement_pressed_y = 1;
       objetos[0].acc.y = acc;
       break;
     case 'a':
       //izquierda: 
	movement_pressed_x = 1;
       objetos[0].acc.x = -acc;
       break;
     case 's':
	//abajo:
	movement_pressed_y = 1;
       objetos[0].acc.y = -acc;
       break;
     case 'd':
	//derecha:
	movement_pressed_x = 1;
       objetos[0].acc.x = acc;
       break;
     case 'p':
       if (rate == 0){
         rate = rate_store;
       }else{
         rate_store = rate;
         rate = 0;
       }
       break;
     case '+':
       rate *= 2;
       break;
     case '-':
       if(rate > 1){
         rate /= 2;
       }
  }
}


/*
 * Recibe el input del usuario y mueve la nave
 */
void teclado_up (unsigned char tecla, int x, int y){
  switch(tecla){
	case 'a':
		movement_pressed_x = 0;
		break;
	case 's':
		movement_pressed_y = 0;
		break;
	case 'w':
		movement_pressed_y = 0;
		break;
	case 'd':
		movement_pressed_x = 0;
		break;
	}
}


/*
 * Se realizan las inicializaciones pertinente para poder ejecutar el juego
 */
void init(){
	//objetos[0] = nave;
        //posicion
        objetos[0].pos.x = 0;
        objetos[0].pos.y = 0;
        objetos[0].pos.z = 0;
        //aceleracion
        objetos[0].acc.x = 0;
        objetos[0].acc.y = 0;
        objetos[0].acc.z = 0;
        //velocidad
        objetos[0].vel.x = 0;
        objetos[0].vel.y = 0;
        objetos[0].vel.z = -2.0;
        //estado
        objetos[0].state = 1;
        //limite de velocidad
        objetos[0].maxVel = 2.0;
	//parametro inertia
        objetos[0].inertia = 1.0;
	//parametro engineAcc
        objetos[0].engineAcc = 2.0;



	//objetos[10] = proyectil;
        //posicion
        objetos[1].pos.x = 0;
        objetos[1].pos.y = 0;
        objetos[1].pos.z = 0;
        //aceleracion
        objetos[1].acc.x = 0;
        objetos[1].acc.y = 0;
        objetos[1].acc.z = 0;
        //velocidad
        objetos[1].vel.x = 0;
        objetos[1].vel.y = 0;
        objetos[1].vel.z = -30.0;
        //estado
        objetos[1].state = 0;
        //limite de velocidad
        objetos[1].maxVel = 30.0;
	//parametro inertia
        objetos[1].inertia = 1.0;
	//parametro engineAcc
        objetos[1].engineAcc = 2.0;


	



	texture = loadBMP("stars.bmp");
	glBindTexture( GL_TEXTURE_2D, texture );
}

/*
 * Funcion que maneja los clicks en el mouse
 */
GLvoid mouse_action(GLint button, GLint state, GLint x, GLint y){
        switch(button){
                case GLUT_LEFT_BUTTON:
			if(objetos[1].state != 1){
                        	objetos[1].state = 1;
				objetos[1].pos = objetos[0].pos;
			}
			 break;
        }
}


void calcular_fps(){
	frames++;
	time_d = glutGet(GLUT_ELAPSED_TIME);
	int nTime = time_d - time_a;
	if(nTime > 1000){
		fps = frames / (nTime / 1000.0f);
		sprintf(FPS,"%f FPS",fps);
		time_a = time_d;
		frames = 0;
	} 	

}




/*
 * Funcion que se ejecuta cuando la aplicacion esta idle
 */
void idle(){
	calcular_fps();
}



int main (int argc, char** argv){
  
  GLfloat lData0[] = {1,1,100,0.0f};

  //Inicializamos la ventana
  glutInit(&argc,argv);
  glutInitWindowSize(600,400);
  glutInitWindowPosition(10,50);
  glutCreateWindow("Star Turpial");
 
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  GLfloat lData1[] = {0.05, 0.05, 0.05,1.0f};
  glLightfv(GL_LIGHT0, GL_POSITION, lData0);
  glLightfv(GL_LIGHT0,GL_AMBIENT,lData1);
  GLfloat lData2[] = {1,1,1,1};
  glLightfv(GL_LIGHT0,GL_DIFFUSE,lData2);
  GLfloat lData3[] = {1,1,1,1};
  glLightfv(GL_LIGHT0,GL_SPECULAR,lData3);


  //Podemos convertir esto en una funcion 
  //posicion objeto una vez que tengamos los objetos

  modelo_TURPIAL = glmReadOBJ("objetos/f-16.obj");
  if (!modelo_TURPIAL) exit(0);

  glutDisplayFunc(display);
  glutReshapeFunc(cambios_ventana);
  glutKeyboardFunc(teclado);
  glutKeyboardUpFunc(teclado_up);
  glutMouseFunc(mouse_action);
  glutIdleFunc(idle);
  glEnable(GL_DEPTH_TEST);
  init();
  glutMainLoop();

  return 0;
}

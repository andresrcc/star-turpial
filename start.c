#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "glm.h"

#define INERTIA_DELTA 0.001 //Delta de velocidad en el cual se considera que un objeto se detuvo completamente

//registro que representa un vector de 3 dimensiones
struct vector{
        float x;
        float y;
        float z;
};typedef struct vector vector3D;

//registro que representa a un objeto
struct objeto{
        int state; //estado del objeto: 1. visible, 2.no-visible
        float maxVel; //velocidad maxima que puede tener el objeto
        float inertia; //modulo de aceleracion con que se desacelera el objeto al no verse afectado por fuerzas externas
        float engineAcc; //aceleracio que proporcionada por un motor
        vector3D pos; //vector posicion
        vector3D vel; //vector velocidad
        vector3D acc; //vectos aceleracion
};typedef struct objeto figura;


int movement_pressed_x = 0; //1. hay una tecla presionada que afecta el movimiento en el eje x, 2.'1' es falso
int movement_pressed_y = 0; //1. hay una tecla presionada que afecta el movimiento en el eje y, 2.'1' es falso
int movement_pressed_z = 0; //1. hay una tecla presionada que afecta el movimiento en el eje z, 2.'1' es falso


float time = 0; //tiempo del juego en milisegundos
figura objetos[1]; //arreglo con todos los objetos


/**
 *Objetos
 * Anillos
 * Blancos
 * Turpial Cosmico (Nave)
 */

GLMmodel *modelo_TURPIAL = NULL;
GLMmodel *modelo_BLANCO = NULL;
GLMmodel *modelo_ANILLO = NULL;



//Nave
//figura nave;


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
        objetos[0].vel.z = 0;
        //estado
        objetos[0].state = 1;
        //limite de velocidad
        objetos[0].maxVel = 2.0;
	//parametro inertia
        objetos[0].inertia = 1.0;
	//parametro engineAcc
        objetos[0].engineAcc = 2.0;
}

/*
 * Esta funcion recibe una figura y calcula su posicion respecto al tiempo
 */
void movimiento(figura *fig){
        float nTime = 0-time;
        time = glutGet(GLUT_ELAPSED_TIME)*0.001;
        nTime += time;

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


       // printf("VELOCIDAD\nx: %f\ny: %f\nz: %f\n\n",(*fig).vel.x,(*fig).vel.y,(*fig).vel.z);

        //se calcula el cambio en la posicion debido a la velocidad
        (*fig).pos.x += (*fig).vel.x*nTime;
        (*fig).pos.y += (*fig).vel.y*nTime;
        (*fig).pos.z += (*fig).vel.z*nTime;

        //printf("ACELERACION\nx: %f\ny: %f\nz: %f\n\n",(*fig).acc.x,(*fig).acc.y,(*fig).acc.z);


        //printf("POSICION\nx: %f\ny: %f\nz: %f\n\nnTime: %f\n\n",(*fig).pos.x,(*fig).pos.y,(*fig).pos.z,nTime);
}


/*
 * Esta funcion recibe una figura y calcula su posicion respecto al tiempo, considera que la figura puede ser movida por el usuario
 */
void movimiento_nave(figura *fig){

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

	if(movement_pressed_z == 0){
                if((*fig).vel.z >= -INERTIA_DELTA && (*fig).vel.z <= INERTIA_DELTA){
                        (*fig).acc.z = 0;
                        (*fig).vel.z = 0;
                }else if((*fig).vel.z > INERTIA_DELTA){
                        (*fig).acc.z = -(*fig).inertia;
                }else{
                        (*fig).acc.z = (*fig).inertia;
                }
        }
	movimiento(fig);
}








/*
 * un cubo con la anchura, altura y profundidad dados
 */
void dibujar_cubo(GLfloat a, GLfloat b, GLfloat c, GLfloat anch, GLfloat alt, GLfloat prof){

	anch = anch/2;
	alt = alt/2;
	prof = prof/2;
	//glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glNormal3f(0.0f,1.0f,0.0f);
		//glTexCoord2f(0.0f,1.0f);
		glVertex3f(anch,prof,alt);
		//glTexCoord2f(0.0f,0.0f);
		glVertex3f(anch,prof,-alt);
		//glTexCoord2f(1.0f,0.0f);
		glVertex3f(-anch,prof,-alt);
		//glTexCoord2f(1.0f,1.0f);
		glVertex3f(-anch,prof,alt);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(1.0f,0.0f,0.0f);
		//glTexCoord2f(1.0f,1.0f);
		glVertex3f(anch,prof,alt);
		//glTexCoord2f(1.0f,0.0f);
		glVertex3f(anch,prof,-alt);
		//glTexCoord2f(0.0f,0.0f);
		glVertex3f(anch,-prof,-alt);
		//glTexCoord2f(0.0f,1.0f);
		glVertex3f(anch,-prof,alt);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(0.0f,-1.0f,0.0f);
		//glTexCoord2f(1.0f,1.0f);
		glVertex3f(anch,-prof,alt);
		//glTexCoord2f(1.0f,0.0f);
		glVertex3f(anch,-prof,-alt);
		//glTexCoord2f(0.0f,0.0f);
		glVertex3f(-anch,-prof,-alt);
		//glTexCoord2f(0.0f,1.0f);
		glVertex3f(-anch,-prof,alt);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(-1.0f,0.0f,0.0f);
		//glTexCoord2f(1.0f,1.0f);
		glVertex3f(-anch,-prof,alt);
		//glTexCoord2f(1.0f,0.0f);
		glVertex3f(-anch,-prof,-alt);
		//glTexCoord2f(0.0f,0.0f);
		glVertex3f(-anch,prof,-alt);
		//glTexCoord2f(0.0f,1.0f);
		glVertex3f(-anch,prof,alt);
	glEnd(); 
	glBegin(GL_QUADS);
		glNormal3f(0.0f,0.0f,1.0f);
		//glTexCoord2f(0.0f,1.0f);
		glVertex3f(-anch,prof,alt);
		//glTexCoord2f(0.0f,0.0f);
		glVertex3f(-anch,-prof,alt);
		//glTexCoord2f(1.0f,0.0f);
		glVertex3f(anch,-prof,alt);
		//glTexCoord2f(1.0f,1.0f);
		glVertex3f(anch,prof,alt);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(0.0f,0.0f,-1.0f);
		//glTexCoord2f(0.0f,0.0f);
		glVertex3f(-anch,prof,-alt);
		//glTexCoord2f(1.0f,0.0f);
		glVertex3f(anch,prof,-alt);
		//glTexCoord2f(1.0f,1.0f);
		glVertex3f(anch,-prof,-alt);
		//glTexCoord2f(0.0f,1.0f);
		glVertex3f(-anch,-prof,-alt);
	glEnd();

	//glDisable(GL_TEXTURE_2D);
}

/*
 * Dibuja al objeto creado por nosotros (Joe)
 */
void dibujar_Joe(){

  //in vec2 UV;

  
  //Matriz mayor
  glPushMatrix();
  	//trasladar todo el objeto
  	//Submatrices del objeto
  	//Cabeza
  	glColor3f(0.93f,0.81f,0.81f);
  	glPushMatrix();
 		 glTranslatef(0,0.75,0);
 		 dibujar_cubo(0.0,0.0,0.0,0.5,0.5,0.5);
 	 glPopMatrix();
 	 //torso
 	 //glColor3f(1.0,0,1.0);		
 	 glPushMatrix();	
 		 glTranslatef(0,0,0);
 		 dibujar_cubo(0.0,0.0,0.0,0.3,0.15,1);
 	 glPopMatrix();
 	 //BrazoI
 	 //glColor3f(1.0,1.0,0);		
 	 glPushMatrix();	
 		 glTranslatef(0.35,0.2,-0.15);
 	 	 glRotatef(45.0,0,1.0,0);
 		 dibujar_cubo(0.0,0.0,0.0,0.45,0.1,0.1);
 	 glPopMatrix();
 	 //BrazoD
 	 //glColor3f(1.0,1.0,0);		
 	 glPushMatrix();	
 		 glTranslatef(-0.35,0.2,-0.15);
		 glRotatef(-45.0,0,1.0,0);
	 	 dibujar_cubo(0.0,0.0,0.0,0.45,0.1,0.1);
	  glPopMatrix();
	  //PiernaI
	 //glColor3f(0,1.0,1.0);		
	  glPushMatrix();	
	 	 glTranslatef(0.15,-0.5,0);
	 	 dibujar_cubo(0.0,0.0,0.0,0.1,0.1,0.5);
	  glPopMatrix();
	  //PiernaD
	 //glColor3f(0,1.0,1.0);		
	  glPushMatrix();	
	 	 glTranslatef(-0.15,-0.5,0);
	 	 dibujar_cubo(0.0,0.0,0.0,0.1,0.1,0.5);
	  glPopMatrix();
	
  glPopMatrix();
	
}


void dibujar_objetos(){
  movimiento_nave(&objetos[0]);
  //nave
  glEnable(GL_LIGHTING);

  glPushMatrix();
  glTranslatef(objetos[0].pos.x,objetos[0].pos.y,objetos[0].pos.z);
  glRotatef(-90.0,0,0,1);
  glRotatef(90.0,1,0,0); 
  //printf("%f-%f-%f\n",objetos[0].pos.x,objetos[0].pos.y,objetos[0].pos.z);
  glmUnitize(modelo_TURPIAL);
  glmFacetNormals(modelo_TURPIAL);
  glmVertexNormals(modelo_TURPIAL, 90.0);
  glEnable(GL_COLOR_MATERIAL);
  glmDraw(modelo_TURPIAL, GLM_SMOOTH | GLM_MATERIAL);
  //dibujar_Joe();
  glPopMatrix();

  glutPostRedisplay();
}







void display(){
  movimiento(&objetos[0]);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0,0.0,0.0,0.0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //Camara

  gluLookAt(15.0, //Coordenada X
	    0.0, //Coordenada Y
	    0.0, //Coordenada Z
	    0.0, 0.0, 0.0, //Posicion Inicial Camara
	    0.0, 0.0, 1.0 //Vector UP En este caso, Z
	    );

  //Hay que ver como los dibujamos que vengan hacia la nave
  dibujar_objetos();
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
	movement_pressed_z = 1;
       objetos[0].acc.z = acc;
       break;
     case 'a':
       //izquierda: 
	movement_pressed_y = 1;
       objetos[0].acc.y = -acc;
       break;
     case 's':
	//abajo:
	movement_pressed_z = 1;
       objetos[0].acc.z = -acc;
       break;
     case 'd':
	//derecha:
	movement_pressed_y = 1;
       objetos[0].acc.y = acc;
       break;
  }
}


/*
 * Recibe el input del usuario y mueve la nave
 */
void teclado_up (unsigned char tecla, int x, int y){
  switch(tecla){
	case 'a':
		movement_pressed_y = 0;
		break;
	case 's':
		movement_pressed_z = 0;
		break;
	case 'w':
		movement_pressed_z = 0;
		break;
	case 'd':
		movement_pressed_y = 0;
		break;
	}
}

int main (int argc, char** argv){
  init();
  static const GLfloat ambient[4] = {1.0f,1.0f,1.0f,1.0f};

  //Inicializamos la ventana
  glutInit(&argc,argv);
  glutInitWindowSize(600,400);
  glutInitWindowPosition(10,50);
  glutCreateWindow("Star Turpial");
 
  glShadeModel(GL_SMOOTH);
  glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
  glEnable(GL_LIGHT0);

 
  //Podemos convertir esto en una funcion 
  //posicion objeto una vez que tengamos los objetos

  modelo_TURPIAL = glmReadOBJ("objetos/f-16.obj");
  if (!modelo_TURPIAL) exit(0);

  glutDisplayFunc(display);
  glutReshapeFunc(cambios_ventana);
  glutKeyboardFunc(teclado);
  glutKeyboardUpFunc(teclado_up);
  glEnable(GL_DEPTH_TEST);

  glutMainLoop();

  return 0;
}

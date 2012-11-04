#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "glm.h"

/**
 *Objetos
 * Anillos
 * Blancos
 * Turpial Cosmico (Nave)
 */

GLMmodel *modelo_TURPIAL = NULL;
GLMmodel *modelo_BLANCO = NULL;
GLMmodel *modelo_ANILLO = NULL;

struct objeto{

  float x;
  float y;
  float z;

}; typedef struct objeto figura;


//Arreglo
//Arreglo con objetos. Lista dinamica? Puedo hacer varias instancias de objetos....
//No te metas con listas dinamicas en C. Haremos un arreglo. Puede haber un maximo
//de anillos o blancos al mismo tiempo en un lugar.


//Nave
figura nave;

/*
 *Podemos hacer una funcion dibujar estrella!
 */


void dibujar_objetos(){

  //nave
  
  glPushMatrix();
  glTranslatef(nave.x,nave.y,nave.z);
  glmUnitize(modelo_TURPIAL);
  glmFacetNormals(modelo_TURPIAL);
  glmVertexNormals(modelo_TURPIAL, 90.0);
  glEnable(GL_COLOR_MATERIAL);
  glmDraw(modelo_TURPIAL, GLM_SMOOTH | GLM_MATERIAL);
  glPopMatrix();

  glutPostRedisplay();
}

void display(){
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0,0.0,0.0,0.0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //Camara

  gluLookAt(0.0, //Coordenada X
	    0.0, //Coordenada Y
	    0.0, //Coordenada Z
	    0.0, 0.0, 0.0, //Posicion Inicial Camara
	    0.0, 1.0, 0.0 //Vector UP En este caso, Y
	    );

  //Hay que ver como los dibujamos que vengan hacia la nave
  dibujar_objetos();
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

/**
 * Recibe la entrada del teclado, y 
 * efectua los cambios a la camara
 */

void teclado (unsigned char tecla, int x, int y){

  /* Una fraccion del vector de
   * direccion de la camara 
   * (linea de observacion)
   */
  float k = 0.1;

  switch(tecla){
     case 27:
       exit(0);
     case 'w':
	//subir: 
        nave.y += k;
       break;
     case 'a':
       //izquierda: 
       nave.x -= k;
       break;
     case 's':
	//abajo:
       nave.y += k;
       break;
     case 'd':
	//derecha:
       nave.x += k;
       break;
  }

  glutPostRedisplay();
  
}

int main (int argc, char** argv){

  //Inicializamos la ventana
  glutInit(&argc,argv);
  glutInitWindowSize(600,600);
  glutInitWindowPosition(10,50);
  glutCreateWindow("Star Turpial");
  
  //Posicionamos al turpial cosmico
  nave.x = 0;
  nave.y = 5;
  nave.z = 0;
  //Podemos convertir esto en una funcion 
  //posicion objeto una vez que tengamos los objetos

  modelo_TURPIAL = glmReadOBJ("objetos/HUMBIRD.OBJ");
  if (!modelo_TURPIAL) exit(0);

  glutDisplayFunc(display);
  glutReshapeFunc(display);
  glutKeyboardFunc(teclado);

  glutMainLoop();

  return 0;
}

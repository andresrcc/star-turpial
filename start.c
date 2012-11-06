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

//registro que representa los datos reelevantes para la partida
struct gameData{
	int time_a;//tiempo que ha pasado desde el ultimo calculo de FPS
	int time_d;//tiempo que ha pasado desde que se inicio el calculo del nuevo FPS
	int frames;//conteo de frames
	float fps;//FPS
	char fps_str[50];//representacion en string el FPS
	int puntos;//puntaje
	char puntaje[20];//representacion en string del puntaje
	float rate;//taza con la desarrolla el juego
	float rate_store;//variable en donde se guarda la taza cuando el juego es pausado
	char rate_str[50];//representacion en string de la taza
};typedef struct gameData datosJuego;


int movement_pressed_x = 0; //1. hay una tecla presionada que afecta el movimiento en el eje x, 2.'1' es falso
int movement_pressed_y = 0; //1. hay una tecla presionada que afecta el movimiento en el eje y, 2.'1' es falso
int movement_pressed_z = 0; //1. hay una tecla presionada que afecta el movimiento en el eje z, 2.'1' es falso
GLuint texture; //textura de fondo
datosJuego juego; //datos relevantes para el juego
figura objetos[2]; //objetos[0]: nave, objetos[1]: laser
figura blancos[5]; //arreglo con todos los blanco del escenario
figura anillos[5]; //arreglo con todos los anillos del escenario

//Colores de luces
GLfloat lData4[] = {-1,1,0,0.0};



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


/*
 * Funcion que se encarga de imprimir en la posicion data en panatalla el string pasado como argumento
 */
void print_pantalla(float x, float y, char *string)
{
	glPushMatrix();
		glRasterPos3f(x, y, objetos[0].pos.z+2);
		int len, i;
		len = (int) strlen(string);
		for (i = 0; i < len; i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
		}
	glPopMatrix();
}

/*
 * Funicion que cambia el estado de una figura dependiendo de si esta o no en al area de interes para el juego
 * (area de interes: volumen contenido ente el plano z=nave.z y el plano z=nave.z-30)
 */
void screen_time(figura *fig){
	float guard = (*fig).pos.z-objetos[0].pos.z;
	if(guard < -30 || guard > 0){
		(*fig).state = 0;
	}else{
		(*fig).state = 1;
	}
}


/*
 * Esta funcion recibe una figura y calcula su posicion y velocidad respecto al tiempo
 */
void movimiento(figura *fig){
	//Se calcula nTime, el tiempo que ha pasado desde la ultima llamada a esta funcion con este parametro fig
	float nTime = 0-(*fig).time;
	(*fig).time = glutGet(GLUT_ELAPSED_TIME)*0.001;
        nTime += (*fig).time;
	nTime *= juego.rate;

	//VELOCIDAD v = t*a + v0
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


	//POSICION x = t*v + x0
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
	
	//se llama a la funcion movimiento para que haga el resto de los calculos de movimiento
	movimiento(fig);

	//se hace un loop del espacio en pantalla
	if((*fig).pos.x>7.5){
		(*fig).pos.x -= 15;
	}
	if((*fig).pos.x<-7.5){
		(*fig).pos.x += 15;
	}
	if((*fig).pos.y>4.5){
		(*fig).pos.y -= 9;
	}
	if((*fig).pos.y<-4.5){
		(*fig).pos.y += 9;
	}
}

/*
 * Funcino que dibuja un blanco en la posicion dada
 */
void dibujar_blanco(float x, float y, float z){
  glPushMatrix();
    glLoadIdentity();
    glTranslatef(x,y,z);
    glutSolidTorus(0.05,0.5,20,20);
  glPopMatrix();

  //glFlush();
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

void dibujar_nave(){
	glPushMatrix();
  		glTranslatef(objetos[0].pos.x,objetos[0].pos.y,objetos[0].pos.z);
  		glRotatef(-180.0,0,1,0);
  		glmUnitize(modelo_TURPIAL);
 		glmFacetNormals(modelo_TURPIAL);
  		glmVertexNormals(modelo_TURPIAL, 90.0);
  		glEnable(GL_COLOR_MATERIAL);
  		glmDraw(modelo_TURPIAL, GLM_SMOOTH | GLM_MATERIAL);
 	glPopMatrix();

}

void dibujar_lasers(){
	int i;
	for (i =1 ; i < 2 ; i++){
    	if(objetos[i].state != 0){
      		glPushMatrix();
      			GLfloat red[] = {1.0f,0.0f,0.0f,1.0f};
      			GLfloat black[] = {0.0f,0.0f,0.0f,1.0f};
      			GLfloat diffuse[] = {0.8f,0.8f,0.8f,1.0f};
      			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION , red);
      			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , red);
      			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , red);
      			glTranslatef(objetos[i].pos.x,objetos[i].pos.y,objetos[i].pos.z);
      			dibujar_cubo(0.1,0.5,0.1);
      			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION , black);
      			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , black);
      			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , diffuse);
      		glPopMatrix();
    	}
  	}
}




/*
 * Funcion que dibuja a todos los objetos que se veran en la escena
 */
void dibujar_objetos(){
  int i = 0;
  //Se dibuja la nave
  dibujar_nave();
  //Se dibujan los lasers
  dibujar_lasers();

  //para cada anillo y blanco se evalua si va a estar en pantalla
  for (i = 0 ; i < 5 ; i++){
      screen_time(&anillos[i]);
      screen_time(&blancos[i]);
  }
 
  //se dibujan los toros
  for (i = 0 ; i<5 ; i++){
    if(anillos[i].state != 0){
      dibujar_blanco(anillos[i].pos.x,anillos[i].pos.y,anillos[i].pos.z);  
    }
  }

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
        if (!fread(data,1,imageSize,file)){
		fclose(file);
		return -1;
	}
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
 * Funcion que hace todos los calculos de movimiento en el mundo
 */
void movimiento_global(){
	//se calcula el movimiento de la nave
	movimiento_nave(&objetos[0]);
  
	//se calcula el movimiento de los lasers
	screen_time(&objetos[1]);
	if(objetos[1].state != 0){
		movimiento(&objetos[1]);
	}
}


/*
 * Funcion que imprime todos los datos necesarios para el juego en pantalla
 */
void print_pantalla_global(){
	print_pantalla(objetos[0].pos.x,objetos[0].pos.y,".posicion");
	sprintf(juego.rate_str,"Rate : %f",juego.rate);
	print_pantalla(0.0,3.5,juego.rate_str); 
	print_pantalla(-6.0,-4.0,juego.fps_str);//Se imprime el FPS
	sprintf(juego.puntaje,"Puntos: %d",juego.puntos);
	print_pantalla(4.0,-4.0,juego.puntaje);//Se imprime el puntaje
}



/*
 * Funcion display de glut que dibuja de nuevo la escena
 */
void display(){
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
 
  //calculos de movimiento antes de dibujar la escena
  movimiento_global(); 
  


  //se pone la camara siempre detras de la nave
  gluLookAt(0.0, //Coordenada X
	    0.0, //Coordenada Y
	    objetos[0].pos.z+15.0, //Coordenada Z
	    0.0, 0.0, objetos[0].pos.z, //Posicion Inicial Camara
	    0.0, 1, 0.0 //Vector UP En este caso, Z
  );


  //dibujando el display 
  glClearColor(0.0,0.0,0.0,0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  //cambiamos la posicion de la luz GL_LIGHT0
  glLightfv(GL_LIGHT0, GL_POSITION, lData4);


  //se dibuja el background
  glDisable(GL_LIGHTING);
  drawBackground();
  glEnable(GL_LIGHTING);

  //se dibujan todos los objetos
  dibujar_objetos();

  //se imprimen todos los datos en pantalla
  print_pantalla_global();

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
	//arriba
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
	//pausa
        if (juego.rate == 0){
        	juego.rate = juego.rate_store;
        }else{
        	juego.rate_store = juego.rate;
        	juego.rate = 0;
        }
        break;
     case '+':
	//aumento de rate
        juego.rate *= 2;
        break;
     case '-':
	//disminucion de rate
        juego.rate /= 2;
  }
}


/*
 * Recibe el input del usuario y mueve la nave
 */
void teclado_up (unsigned char tecla, int x, int y){
  switch(tecla){
	case 'a':
		//izquierda
		movement_pressed_x = 0;
		break;
	case 's':
		//abajo
		movement_pressed_y = 0;
		break;
	case 'w':
		//arriba
		movement_pressed_y = 0;
		break;
	case 'd':
		//derecha
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
        objetos[1].vel.z = -35.0;
        //estado
        objetos[1].state = 0;
        //limite de velocidad
        objetos[1].maxVel = 30.0;
	//parametro inertia
        objetos[1].inertia = 1.0;
	//parametro engineAcc
        objetos[1].engineAcc = 2.0;


	//objetos[2] = toro;
        //posicion
        anillos[0].pos.x = 0;
        anillos[0].pos.y = 0;
        anillos[0].pos.z = -35.0;
        //aceleracion
        anillos[0].acc.x = 0;
        anillos[0].acc.y = 0;
        anillos[0].acc.z = 0;
        //velocidad
        anillos[0].vel.x = 0;
        anillos[0].vel.y = 0;
        anillos[0].vel.z = 0.0;
        //estado
        anillos[0].state = 0;
        //limite de velocidad
        anillos[0].maxVel = 0.0;
	//parametro inertia
        anillos[0].inertia = 0.0;
	//parametro engineAcc
        anillos[0].engineAcc = 0.0;


	juego.rate = 1.0;


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
      				objetos[1].pos = objetos[0].pos;
				objetos[1].pos.z -= 1;
				objetos[1].state = 1;
			}
			 break;
        }
}

/*
 * Funcion que calcula los cuadros por segundo de la aplicacion
 */
void calcular_fps(){
	juego.frames++;
	juego.time_d = glutGet(GLUT_ELAPSED_TIME);
	int nTime = juego.time_d - juego.time_a;
	if(nTime > 1000){
		juego.fps = juego.frames / (nTime / 1000.0f);
		sprintf(juego.fps_str,"%f FPS",juego.fps);
		juego.time_a = juego.time_d;
		juego.frames = 0;
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

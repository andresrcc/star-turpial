#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <string.h>
#include "glm.h"
#include <math.h>
#include <pthread.h>
#include "AL/al.h"
#include "AL/alut.h"
#include "SDL/SDL.h"

#define INERTIA_DELTA 0.005 //Delta de velocidad en el cual se considera que un objeto se detuvo completamente
#define TORUS_RADIO 0.25
#define BLANCO_RADIO 0.2
#define BUFFSIZE 20
#define COLLISION_NAVE_ANILLO 0
#define COLLISION_NAVE_BLANCO 1
#define COLLISION_LASER_BLANCO 2
#define TORUS_NUM sizeof anillos / sizeof(anillos[1]) //num elementos en arreglo anillo 
#define BLANCO_NUM sizeof blancos / sizeof(blancos[1]) //num elementos en arreglo blancos
#define EDIF_NUM sizeof edificios / sizeof(edificios[1]) //num elementos en arreglo edificios
#define GAME_OVER 1
#define GAME_ON 0
#define LASER_SHOT 1
#define LASER_NOT_SHOT 0
#define TEXTURE_MOVEMENT 0.5


//Buffer de sonido y fuente de sonido
ALuint	sound_buffer;
ALuint	source;


//Variables de Carga de Sonido
Uint8 *data;
SDL_AudioSpec wav_spec;
Uint32 size;

//Posicion y velocidad de la fuente de sonido
ALfloat src_pos[]={ 0.0, 0.0, 1.0};
ALfloat src_vel[]={ 0.0, 0.0, 1.0};

//Posicion, velocidad y origen del listener(oyente)
ALfloat ls_pos[]={0.0,0.0,0.0};
ALfloat ls_vel[]={0.0,0.0,0.0};
ALfloat	ls_origin[]={0.0,0.0,1.0,0.0,1.0,0.0};




//registro que representa un vector de 3 dimensiones
struct vector{
        float x;
        float y;
        float z;
};typedef struct vector vector3D;

//registro que representa a un objeto
struct objeto{
	//int dead;
	int through; //si ya se encuentra despues de la nave
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

struct listaFigura{
	figura esta;
	struct listaFigura * next;
};typedef struct listaFigura listaFiguras; 

//-----------------------------------------------------------VARIABLES GLOBALES---------------------------

int movement_pressed_x = 0; //1. hay una tecla presionada que afecta el movimiento en el eje x, 2.'1' es falso
int movement_pressed_y = 0; //1. hay una tecla presionada que afecta el movimiento en el eje y, 2.'1' es falso
int movement_pressed_z = 0; //1. hay una tecla presionada que afecta el movimiento en el eje z, 2.'1' es falso
int shot = LASER_NOT_SHOT; //si se disparo una laser
int game_over = GAME_ON; //si se acabo el juego
GLuint texture; //textura de fondo
GLuint texture_suelo;
GLuint texture_nave;
GLuint texture_edificio;
datosJuego juego; //datos relevantes para el juego
figura *target; //objetivo del laser
GLuint selecBuffer[BUFFSIZE];//Buffer de seleccion para el picking
pthread_t banda;
figura objetos[2]; //objetos[0]: nave, objetos[1]: laser
figura blancos[40]; //arreglo con todos los blanco del escenario
figura anillos[30]; //arreglo con todos los anillos del escenario
figura edificios[15]; //arreglo con todos los edificios del escenario
//Colores de luces
GLfloat lData4[] = {-1,1,0,0.0};
GLfloat red[] = {1.0f,0.0f,0.0f,0.0f};
GLfloat black[] = {0.0f,0.0f,0.0f,1.0f};
GLfloat white[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat diffuse[] = {0.8f,0.8f,0.8f,1.0f};
GLfloat blue[] = {0.0f,0.0f,1.0f,1.0f};
GLfloat yellow[] = {1.0f,1.0f,0.0f,0.0f};
GLfloat lData0[] = {1,1,100,0.0f};
GLfloat lData1[] = {0.05, 0.05, 0.05,1.0f};
GLfloat lData2[] = {1,1,1,1};
GLfloat lData3[] = {1,1,1,1};
GLfloat anilloAmbient[] = {0.25f,0.25f,0.25f,1.0f};
GLfloat anilloDiffuse[] = {0.4f,0.4f,0.4f,1.0f};
GLfloat anilloSpecular[] = {0.774597f,0.774597f,0.774597f,1.0f};
GLfloat anilloShininess[] = {0.6f};
GLfloat blancoAmbient[] = {0.19125f,0.0735f,0.0225f,1.0f};
GLfloat blancoDiffuse[] = {0.7038f,0.27048f,0.0828f,1.0f};
GLfloat blancoSpecular[] = {0.256777f,0.13722f,0.086014f,1.0f};
GLfloat blancoShininess[] = {0.1f};



/**
 *Objetos
 * Anillos
 * Blancos
 * Turpial Cosmico (Nave)
 */

figura nave;
figura laser;
GLMmodel *modelo_TURPIAL = NULL;
GLMmodel *modelo_BLANCO = NULL;
GLMmodel *modelo_ANILLO = NULL;

//Para guardar objetos quadratic. Estos sirven para
//crear los anillos y blancos con funciones de la libreria GLU
GLUquadricObj *quadratic;







//----------------------------------------------------FUNCIONES QUE SE UTILIZAN PARA PONER TEXTO EN PANTALLA----------------------------


/*
 * Funcion que se encarga de imprimir en la posicion data en panatalla el string pasado como argumento
 */
void print_pantalla(float x, float y, char *string)
{
	glPushMatrix();
		glRasterPos3f(x, y, nave.pos.z+0.3);
		int len, i;
		len = (int) strlen(string);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION , white);
      		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , white);
      		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , white);
      		for (i = 0; i < len; i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
		}
	glPopMatrix();
}


/*
 * Funcion que imprime todos los datos necesarios para el juego en pantalla
 */
void print_pantalla_global(){
	print_pantalla(-0.12,0.45," "); 
	if(game_over == GAME_OVER){
   		 print_pantalla(-0.2,0,"GAME OVER");
   		 print_pantalla(-0.5,-0.1,"presione 'y' para reiniciar o 'q' para salir");
  	} 
	sprintf(juego.rate_str,"Rate : %f",juego.rate);
	print_pantalla(-0.12,0.45,juego.rate_str); 
	print_pantalla(-.7,-0.45,juego.fps_str);//Se imprime el FPS
	sprintf(juego.puntaje,"Puntos: %d",juego.puntos);
	print_pantalla(0.5,-0.45,juego.puntaje);//Se imprime el puntaje
}






//------------------------------------------------------FUNCIONES DE FISICA DEL JUEGO-----------------------------------------


/*
 * Hace que la nave dispare hacia adelante
 */
void shoot_ahead(){
	shot = LASER_NOT_SHOT;
	if(laser.state != 1){
      		laser.pos = nave.pos;
		laser.vel.x = 0;
		laser.vel.y = 0;
		laser.vel.z = -laser.maxVel;
		//laser.pos.z -= 1;
		laser.state = 1;
	}
}

/*
 * Esta funcion se encarga de disparar hacia un objetivo
 */
void shoot(figura *target){
	shot = LASER_NOT_SHOT;
	//Si el laser no esta en pantalla
	if(laser.state != 1){
		//primero se calcula el modulo del vector posicion del objetivo respecto a la nave
 	     	float modulo;
		laser.vel.x = 100*((*target).pos.x - nave.pos.x);
		laser.vel.y = 100*((*target).pos.y - nave.pos.y);
		laser.vel.z = 100*((*target).pos.z - nave.pos.z);
		modulo = pow(laser.vel.x,2) +  pow(laser.vel.y,2) + pow(laser.vel.z,2);
		modulo = sqrt(modulo);
		//utilizando el vector direccion le damos un vector velocidad a nuestro laser para que impacte con el objetivo
		laser.vel.x *= laser.maxVel / modulo;
		laser.vel.y *= laser.maxVel / modulo;
		laser.vel.z *= laser.maxVel / modulo;
		laser.pos = nave.pos;
	//	laser.pos.z += 1;
		//marcamos el laser como que esta en pantalla
		laser.state = 1;
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

	//Game Over?
	if(nave.pos.z < -60){
		game_over = GAME_OVER;
	}

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
	if((*fig).pos.x>1.0){
		(*fig).pos.x -= 2.0;
	}
	if((*fig).pos.x<-1.0){
		(*fig).pos.x += 2.0;
	}
	if((*fig).pos.y>0.65){
		(*fig).pos.y -= 1.3;
	}
	if((*fig).pos.y<-0.65){
		(*fig).pos.y += 1.3;
	}
}


void screen_time(figura *fig);


/*
 * Funcion que hace todos los calculos de movimiento en el mundo
 */
void movimiento_global(){
	//se calcula el movimiento de la nave
	movimiento_nave(&nave);
  
	//se calcula el movimiento de los lasers
	screen_time(&laser);
	if(laser.state != 0){
		movimiento(&laser);
	}else{
		glDisable(GL_LIGHT1);
		laser.time = glutGet(GLUT_ELAPSED_TIME)*0.001;
	}
}












//--------------------------------------------LOGICA DEL JUEGO--------------------------------------------

/*
 * Funicion que cambia el estado de una figura dependiendo de si esta o no en al area de interes para el juego
 * (area de interes: volumen contenido ente el plano z=nave.z y el plano z=nave.z-30)
 */
void screen_time(figura *fig){
	float guard = (*fig).pos.z-nave.pos.z;
	if(guard < -30 || guard > 1 /*|| (*fig).through != 0*/){
		(*fig).state = 0;
	}else{
		(*fig).state = 1;
	}
}


/*
 * esta funcion se encarga de manejar la colisiones
 * base y figura son los objetos que colisionan
 * points son los puntos que se agregan al marcados si ocurre la colision
 * type es el tipo de colision : COLLISION_NAVE_ANILLO, COLLISION_NAVE_BLANCO, COLLISION_LASER_BLANCO
 */
void collision_points(figura *base,figura *fig, int points, int type){
	//Si el objeto se encuentra detras de la nave no ahce falta calcular ninguna colision porque no nos interesa
	if((*fig).through!=0){
		return;
	}
	//El delta que se utiliza para saber si los objetos colisionan, depende de la variable type
	float z_delta;
	switch(type){
		case COLLISION_NAVE_ANILLO:
			z_delta = 0.1;
			break;
		case COLLISION_NAVE_BLANCO:
			z_delta = 0.2;
			break;
		case COLLISION_LASER_BLANCO:
			z_delta = 0.4;
			break;
	}
	//Distancia en z entre los objetos
	float guard = (*fig).pos.z-(*base).pos.z;
	if(guard > -z_delta && guard < z_delta){
		float radio;
		if(type == COLLISION_NAVE_ANILLO){
			radio = TORUS_RADIO;
		}else/*COLLISION_NAVE_BLANCO o COLLISION_LASER_BLANCO*/{
			radio = BLANCO_RADIO;
		}
		//Se calcula ahora la distancia en el plano x,y
		guard = (*fig).pos.x - (*base).pos.x;
		float guard2 = (*fig).pos.y - (*base).pos.y;
		guard = guard*guard + guard2*guard2;
		if(guard < radio*radio){
			//la colision ocurrio, por lo tanto se modifica el puntaje
			juego.puntos += points;
			//se marca el segundo objeto como que ya dejo de interesarnos
			(*fig).through = 1;
			//el objeto es destruido
			//se evita que el puntaje sea negativo
			if (juego.puntos < 0){
				juego.puntos = 0;
			}
		}
	}
	
}




//-----------------------PICKING-------------------------

void picking_ON(int mousex,int mousey){
	GLint viewport[4];
	glSelectBuffer(BUFFSIZE,selecBuffer);
	glRenderMode(GL_SELECT);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glGetIntegerv(GL_VIEWPORT,viewport);
	gluPickMatrix(mousex,viewport[3]-mousey,5.0,5.0,viewport);
	gluPerspective(45.0f,1.0,0.1,30.0);
	glMatrixMode(GL_MODELVIEW);
	glInitNames();
}



void processHits (GLint hits, GLuint buffer[])
{
	unsigned int i, j;
	GLuint names, *ptr, minZ,*ptrNames, numberOfNames;
	ptr = (GLuint *) buffer;
	minZ = 0xffffffff;
	for (i = 0; i < hits; i++) {	
		names = *ptr;
		ptr++;
		if (*ptr < minZ) {
			numberOfNames = names;
			minZ = *ptr;
			ptrNames = ptr+2;
		}  
		ptr += names+2;
	}
	ptr = ptrNames;
	shot = LASER_SHOT;
	target = &blancos[*ptr];
}


void picking_OFF(){
	int hits;
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	//glFlush();
	hits = glRenderMode(GL_RENDER);
	if(hits>0){
		processHits(hits,selecBuffer);
	}
}



//---------------------------------------------------FUNCIONES PARA DIBUJAR EN PANTALLA----------------------------------



/*
 * Funcino que dibuja un anillo en la posicion dada
 */
void dibujar_anillo(float x, float y, float z){
  glPushMatrix();
    glTranslatef(x,y,z);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT , black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , anilloSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , anilloDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, anilloShininess);
    glutSolidTorus(0.025,TORUS_RADIO,20,20);
  glPopMatrix();
}

/*
 * Funcino que dibuja un blanco en la posicion dada
 */
void dibujar_blanco(float x, float y, float z){
  glPushMatrix();
    glTranslatef(x,y,z);
    glScalef(0.2,0.2,0.2);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT , black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , blancoSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , blue);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, blancoShininess);
    glutSolidOctahedron();
    glRotatef(45.0,0.0,0.0,1.0);
    glutSolidOctahedron(); 
  glPopMatrix();
}


/*
 * Funcino que dibuja una explosion
 */
void dibujar_explosion(float x, float y, float z){
  glPushMatrix();
    //glEnable(GL_LIGHT2);
    glTranslatef(x,y,z);
    glScalef(0.2,0.2,0.2);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION , black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , red);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , yellow);
    glutSolidSphere(0.7,10,10);
    //glDisable(GL_LIGHT2);
  glPopMatrix();
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
  		glTranslatef(nave.pos.x,nave.pos.y,nave.pos.z);
  		glRotatef(-180.0,0,1,0);
		
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION , black);
    		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , black);
    		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , diffuse);
		glEnable(GL_TEXTURE_2D);
  		glBindTexture(GL_TEXTURE_2D, texture_nave);
		glmDraw(modelo_TURPIAL, GLM_SMOOTH | GLM_MATERIAL);
 	glPopMatrix();

}

void dibujar_lasers(){
	int i;
    	if(laser.state != 0){
	GLfloat pos[] = {laser.pos.x, laser.pos.y, laser.pos.z, 1.0f};
	glLightfv(GL_LIGHT1 , GL_POSITION, pos);
		
      		glPushMatrix();
      			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION , red);
      			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , red);
      			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , red);
      			glTranslatef(laser.pos.x,laser.pos.y,laser.pos.z);
      			dibujar_cubo(0.05,0.25,0.05);
       		glPopMatrix();
    	}
}



void dibujar_suelo(){
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glBindTexture( GL_TEXTURE_2D, texture_suelo );
		float z = nave.pos.z;
		float avanzado_textura = -z * TEXTURE_MOVEMENT;
		glBegin(GL_QUADS);
			glNormal3f(0,1,0);
			glTexCoord2d(10.0,avanzado_textura+15.0);glVertex3f(9.0,-0.65,z-30.0);
			glTexCoord2d(10.0,avanzado_textura);glVertex3f(9.0,-0.65,z);
			glTexCoord2d(-10.0,avanzado_textura);glVertex3f(-9.0,-0.65,z);
			glTexCoord2d(-10.0,avanzado_textura+15.0);glVertex3f(-9.0,-0.65,z-30.0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

//Dibuja los edificios en la escena 

void dibujar_edificio(float x, float y, float z){
  glPushMatrix();
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D,texture_edificio);
      glTranslatef(x,y,z);
      dibujar_cubo(0.5,1,1);
      glDisable(GL_TEXTURE_2D);
  glPopMatrix();

}


/*
 * Funcion que dibuja a todos los objetos que se veran en la escena
 */
void dibujar_objetos(){
  int i = 0;
  dibujar_suelo();
  //Se dibuja la nave
  dibujar_nave();
  //Se dibujan los lasers
  dibujar_lasers();

  //para cada anillo, blanco y edificio se evalua si va a estar en pantalla
  for (i = 0 ; i < TORUS_NUM ; i++){
      screen_time(&anillos[i]);
      collision_points(&nave,&anillos[i],1,COLLISION_NAVE_ANILLO);
  }

  for (i = 0 ; i < BLANCO_NUM ; i++){
      screen_time(&blancos[i]);
      if(laser.state ==1){
         collision_points(&laser,&blancos[i],1,COLLISION_LASER_BLANCO);	
      }
      collision_points(&nave,&blancos[i],-3,COLLISION_NAVE_BLANCO);
  }
 
  //Con edificios, no depende del laser. 
  //Pero si chocas te quitan 2 puntos
  for (i = 0 ; i < EDIF_NUM ; i++){
      screen_time(&edificios[i]);
      //    if(laser.state ==1){
      //  collision_points(&laser,&blancos[i],1,COLLISION_LASER_BLANCO);	
      // }
      //collision_points(&nave,&blancos[i],-3,COLLISION_NAVE_BLANCO);
  }

  //se dibujan los toros
  for (i = 0 ; i < TORUS_NUM ; i++){
    if(anillos[i].state != 0){
         dibujar_anillo(anillos[i].pos.x,anillos[i].pos.y,anillos[i].pos.z);  
    }
  }

//se dibujan los edificios
  for (i = 0 ; i < EDIF_NUM ; i++){
    if(edificios[i].state != 0){
         dibujar_edificio(edificios[i].pos.x,0.0,edificios[i].pos.z);  
    }
  }
  
  
 //se dibujan los blancos
  for (i = 0 ; i < BLANCO_NUM ; i++){
    if(blancos[i].state != 0){
      if(blancos[i].through == 0){
         dibujar_blanco(blancos[i].pos.x,blancos[i].pos.y,blancos[i].pos.z);  
      }else{
         dibujar_explosion(blancos[i].pos.x,blancos[i].pos.y,blancos[i].pos.z);
      }
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
		glBindTexture( GL_TEXTURE_2D, texture );
		float z = nave.pos.z - 30.0; 
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




//--------------------------------------------------------GLUT/OPENGL------------------------------------------------




/*
 * Funcion display de glut que dibuja de nuevo la escena
 */
void display(){

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
 
  //calculos de movimiento antes de dibujar la escena
  movimiento_global(); 
  if(shot == LASER_SHOT){
    if(target != NULL){
      shoot(target);
    }else{
      shoot_ahead();
    }
  } 


  //se pone la camara siempre detras de la nave
  gluLookAt(0.0, //Coordenada X
	    0.0, //Coordenada Y
	    nave.pos.z+2, //Coordenada Z
	    0.0, 0.0, nave.pos.z, //Posicion Inicial Camara
	    0.0, 1, 0.0 //Vector UP En este caso, Z
  );


  //dibujando el display 
  glClearColor(0.0,0.0,0.0,0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  //cambiamos la posicion de la luz GL_LIGHT0
  glLightfv(GL_LIGHT0, GL_POSITION, lData4);


  //se dibuja el background
  glDisable(GL_LIGHTING);
  drawBackground();//DESCOMENTAR ESTO
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
  float acc = nave.engineAcc;
  switch(tecla){
     case 27:
	exit(0);
     case 'w':
	//arriba
	movement_pressed_y = 1;
        nave.acc.y = acc;
        break;
     case 'a':
        //izquierda: 
	movement_pressed_x = 1;
        nave.acc.x = -acc;
        break;
     case 's':
	//abajo:
	movement_pressed_y = 1;
        nave.acc.y = -acc;
        break;
     case 'd':
	//derecha:
	movement_pressed_x = 1;
        nave.acc.x = acc;
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
     case 'y':
       //restart:
       if(game_over == GAME_OVER){
         init();
	 game_over = GAME_ON;
       }
       break;
     case 'q':
	if(game_over == GAME_OVER){
          al_salir();
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

float coord_aleatoria(int inf, int sup, int flag){
  static int Init = 0;
  int coordenada;
  float coord;

  if (Init = 0){
    srand(time(NULL));
    Init = 1;
  }

  coordenada = (rand() % (sup - inf + 1) + inf);
  if (flag == 1){
  coord = coordenada *0.4;
  return coord;
  }
  return coordenada;

}


/**
 * Posiciona los anillos y toros en el juego
 */
void crear_nivel(){
  int i;
    for (i = 0 ; i < TORUS_NUM ; i++){

      anillos[i].pos.z = coord_aleatoria(-60,0,0);
      anillos[i].pos.x = coord_aleatoria(-2,2,1);
      anillos[i].pos.y = coord_aleatoria(-1,1,1);
      //0.65 //1
    }

    for (i = 0 ; i < BLANCO_NUM ; i++){
      blancos[i].pos.z = coord_aleatoria(-60,0,0);
      blancos[i].pos.x = coord_aleatoria(-2,2,1);
      blancos[i].pos.y = coord_aleatoria(-1,1,1);
    }

    for (i = 0 ; i < EDIF_NUM ; i++){
      edificios[i].pos.z = coord_aleatoria(-60,0,0);
      edificios[i].pos.x = coord_aleatoria(-2,2,1);
      edificios[i].pos.y = coord_aleatoria(-1,1,1);
    }

    /*	blancos[0].pos.z = -7;
	blancos[0].pos.x = -0.2;
	blancos[0].pos.y = 0.25;

	blancos[1].pos.z = -11;
	blancos[1].pos.x = 0.6;
	blancos[1].pos.y = 0.25;

	blancos[2].pos.z = -15;
	blancos[2].pos.x = -0.15;
	blancos[2].pos.y = -0.22;

	blancos[3].pos.z = -20;
	blancos[3].pos.x = 0.5;
	blancos[3].pos.y = -0.1;

	blancos[4].pos.z = -25;
	blancos[4].pos.x = 0.0;
	blancos[4].pos.y = 0.25;
    */

}

/*
 * Se realizan las inicializaciones pertinente para poder ejecutar el juego
 */
void init(){
	//nave g= nave;
        //posicion
        nave.pos.x = 0;
        nave.pos.y = 0;
        nave.pos.z = 0;
        //aceleracion
        nave.acc.x = 0;
        nave.acc.y = 0;
        nave.acc.z = 0;
        //velocidad
        nave.vel.x = 0;
        nave.vel.y = 0;
        nave.vel.z = -1.5;
        //estado
        nave.state = 1;
        //limite de velocidad
        nave.maxVel = 1.5;
	//parametro inertia
        nave.inertia = 0.75;
	//parametro engineAcc
        nave.engineAcc = 1.0;



	//objetos[10] = proyectil;
        //posicion
        laser.pos.x = 0;
        laser.pos.y = 0;
        laser.pos.z = 2.0;
        //aceleracion
        laser.acc.x = 0;
        laser.acc.y = 0;
        laser.acc.z = 0;
        //velocidad
        laser.vel.x = 0;
        laser.vel.y = 0;
        laser.vel.z = -17.0;
        //estado
        laser.state = 0;
        //limite de velocidad
        laser.maxVel = 17.0;
	//parametro inertia
        laser.inertia = 1.0;
	//parametro engineAcc
        laser.engineAcc = 2.0;

	//objetos[2] = toro;
        //posicion

	crear_nivel();
 
	juego.rate = 1.0;


	texture = loadBMP("stars.bmp");
	texture_suelo = loadBMP("bridge.bmp");
        texture_edificio = loadBMP("edificio.bmp");
	
}

/*
 * Funcion que maneja los clicks en el mouse
 */
GLvoid mouse_action(GLint button, GLint state, GLint x, GLint y){
        int i;
	switch(button){
                case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN){
				for (i = 0 ; i<5 ; i++){
					picking_ON(x,y);
					for (i = 0 ; i < BLANCO_NUM ; i++){
						glPushName(i);
						dibujar_blanco(blancos[i].pos.x,blancos[i].pos.y,blancos[i].pos.z);  
						glPopName();
					}
					picking_OFF();
				}
				if(shot==LASER_NOT_SHOT){
					shot = LASER_SHOT;
					target = NULL;
				}
				//shoot_ahead();	
			}
			break;
        }
}

ALboolean MusicaActiva(ALuint source) {
    ALuint state;
    if(alIsSource(source) == AL_FALSE) { //Comprueba si es una fuente
        return AL_FALSE;
    }
    state =AL_INITIAL;

#ifdef _WIN32
    alGetSourcei(source,AL_SOURCE_STATE, &state);  //Obtiene el estado de la fuente para windows
#endif
#ifdef _LINUX
    alGetSourceiv(source,AL_SOURCE_STATE, &state); // Obtiene el estado de la fuente para Linux
#endif
    switch (state) {
     case AL_PLAYING:
     return AL_TRUE;   // Si la fuente se esta reproduciendo devuelve AL_TRUE en caso contrario AL_FALSE

    default:
    break;
}
return AL_FALSE;
}


void init_musica(){
  //Verifica la carga del archivo de sonido
  char loaded;

  //Iniciar OpenAL
  alutInit(NULL,0);
  //Generando buffer...
  alGenBuffers(1,&sound_buffer);

  //Si falla la creacion del buffer
  if((alGetError()) != AL_NO_ERROR){
    printf("Error al crear buffer");
    exit(1);
  }

  //Cargando archivo de sonido
  loaded = SDL_LoadWAV("musica.wav", &wav_spec,&data,&size);

  //Si falla la carga del archivo
  if(loaded==NULL){
    printf("No se pudo cargar la musica de fondo");
    exit(1);
  }

  //Metemos el sonido en el buffer
  alBufferData(sound_buffer,AL_FORMAT_STEREO16,data,size,wav_spec.freq);

  //Liberamos el sonido cargado que ya esta en el buffer
  SDL_FreeWAV(data);

  //Generamos la fuente del sonido en el juego
  alGenSources(1,&source);

  //Si ocurre un error al crear la fuente de sonido
  if(alGetError() != AL_NO_ERROR){
    printf("No se pudo crear la fuente de sonido");
    exit(1);
  }


  //Posicion, Velocidad, Fuente de sonido
  alSourcefv(source,AL_POSITION,src_pos);
  alSourcefv(source,AL_VELOCITY,src_pos);

  //Posicion, Velocidad, Origen del oyente 
  alListenerfv(AL_POSITION,ls_pos);
  alListenerfv(AL_VELOCITY,ls_vel);
  alListenerfv(AL_ORIENTATION,ls_origin);

  //Pasamos el sonido del buffer a la fuente de sonido
  alSourcei(source,AL_BUFFER,sound_buffer);

  //Si la musica no esta activa, press play!
  if(!MusicaActiva(source)){
    alSourcePlay(source);
  }

  //Como reiniciar la musica? O_O
  // alSourceStop(source);

  //alutExit();
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



void al_salir(){
	exit(0);	
}


//---------------------------------------------------------------MAIN---------------------------------------------





int main (int argc, char** argv){
  //Inicializamos la ventana
  glutInit(&argc,argv);
  glutInitWindowSize(600,400);
  glutInitWindowPosition(10,50);
  glutCreateWindow("Star Turpial");
 
  //Musica de fondo //la ponemos en idle?
  init_musica();

  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  //LUZ AMBIENTAL
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, lData0);
  glLightfv(GL_LIGHT0,GL_AMBIENT,lData1);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,lData2);
  glLightfv(GL_LIGHT0,GL_SPECULAR,lData3);
  //LUZ DEL LASER
  glLightfv(GL_LIGHT1, GL_AMBIENT, black);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, red);
  glLightfv(GL_LIGHT1, GL_SPECULAR, red);
  //LUZ DE EXPLOSIONES
  glLightfv(GL_LIGHT2, GL_AMBIENT, black);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, yellow);
  glLightfv(GL_LIGHT2, GL_SPECULAR, yellow);

  //Podemos convertir esto en una funcion 
  //posicion objeto una vez que tengamos los objetos

  modelo_TURPIAL = glmReadOBJ("objetos/Feisar_Ship.obj");
  if (!modelo_TURPIAL) exit(0);
  //Textura de la nave
  //texture_nave = loadBMP("diffuse.bmp"); 
  glmUnitize(modelo_TURPIAL);
  glmFacetNormals(modelo_TURPIAL);
  glmVertexNormals(modelo_TURPIAL, 90.0);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION , black);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , black);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , diffuse);
  glmScale(modelo_TURPIAL,0.2);


  //funciones de GLUT
  glutDisplayFunc(display);
  glutReshapeFunc(cambios_ventana);
  glutKeyboardFunc(teclado);
  glutKeyboardUpFunc(teclado_up);
  glutMouseFunc(mouse_action);
  glutIdleFunc(idle);
  glutCloseFunc(al_salir);
  glEnable(GL_DEPTH_TEST);
  init();

  //main loop
  glutMainLoop();
  return 0;
}

//PRACTICA FINAL G2 TSUNAMI DEMOCRATICO
//MIEMBROS: Angel Lopez Arias, Alejandro Perez Fernandez, Pablo Bayon Gutierrez y Diego Simon Gonzalez

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>

#define QR 0 //Usuarios que intentan acceder por QR
#define INVITACION 1 //Usuarios que intentan acceder por invitacion
#define TAMCOLADEFECTO 15

struct solicitud{
	int id;
	bool atendido;
	int tipo; //INVITACION = 1; QR = 0
	bool descartado; //Lo hemos anyadido nosotros
	pthread_t tid;
	bool encolado;
};
int tamCola =15;
int id = 1;
//int numSolicitudes = 0; //contador que usamos para movernos entre las solicitudes, hilos, y las struct
bool peticionSolicitudes = true;

//Contador de solicitudes
struct solicitud *cola; //El tamanyo en principio es 15, pero puede variar

//pthread_t *hilos; //TODOS LOS HILOS EN UN SOLO PUNTERO???
//struct solicitud *solicitudes; //ES NECESARIO??? YO USARIA SOLO LA COLA DE SOLICITUDES
int posicionSiguiente();
void nuevaSolicitud(int);
int calculaAleatorio(int min, int max);
int generadorID();
void *sol(void *arg);

void *sol(void *arg){ //Funcion que ejecutan los hilos al crearse
	struct solicitud *s;
	s = (struct solicitud *) arg;
	//printf("Soy una solicitud, y mi id es %d\n", s->id);
	printf("Soy una solicitud, y mi id es %d\n", (*s).id);
	if(s->tipo == INVITACION)
		printf("Soy de tipo Invitación\n");
	else
		printf("Soy de tipo QR\n");
	printf("Atendido = %d y descartado = %d\n", s->atendido, s->descartado);
	//printf("Mi id es %d, atendido = %d, mi tipo es ' %d ' y descartado es = %d\n", s->id, s->atendido, s->tipo, s->descartado);
	//TO DO
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){

	cola = (struct solicitud *)malloc(sizeof(struct solicitud)*(TAMCOLADEFECTO));
	//hilos = (pthread_t *)malloc(sizeof(pthread_t));
	//solicitudes = (struct solicitud *)malloc(sizeof(struct solicitud));

	struct sigaction sLlegaSolicitud;
	sLlegaSolicitud.sa_handler = nuevaSolicitud;//Se asigna la manejadora nuevaSolicitud a la estrutura sigaction 

	if(sigaction(SIGINT, &sLlegaSolicitud, NULL) == -1){ //Enmascaramos la senal SIGUSR1 (a partir de ahora se llamara a nuevaSolicitud cuando reciba SIGUSR1)
		perror("Error al comunicar la llegada de una nueva solicitud");
		exit(-1);
	}

	if(sigaction(SIGQUIT, &sLlegaSolicitud, NULL) == -1){ //Enmascaramos la senal SIGUSR1 (a partir de ahora se llamara a nuevaSolicitud cuando reciba SIGUSR2)
		perror("Error al comunicar la llegada de una nueva solicitud");
		exit(-1);
	}

	if(sigaction(SIGTSTP, &sLlegaSolicitud, NULL) == -1){ //Enmascaramos la senal SIGUSR1 (a partir de ahora se llamara a nuevaSolicitud cuando reciba SIGUSR2)
		perror("Error al comunicar la llegada de una nueva solicitud");
		exit(-1);
	}
	
	while(peticionSolicitudes == true)
		pause();

	printf("Una vez creados todos las solicitudes, esto es lo que tenemos en el sistema:\n\n");
	for(int i=0; i<posicionSiguiente()-1; i++){
		printf("-----------------------------------------\n");
		printf("ID --> %d\n", (*(cola+i)).id);
		printf("ATENDIDO --> %d\n", (*(cola+i)).atendido);
		if((*(cola+i)).tipo == QR)
			printf("TIPO --> QR\n");
		else
			printf("TIPO --> Invitación\n");
		printf("descartado --> %d\n", (*(cola+i)).descartado);
	}
	printf("-----------------------------------------\n\n");

	/*for(int i=0; i<numSolicitudes; i++){
		pthread_join(*(hilos+i), NULL);
	}*/
	free(cola);
	printf("Todo esta cumplido, podemos cerrar\n");

	//pthread_join(*(hilos+0), NULL);
	//printf("Hilo 1 muerto\n");
	//pthread_exit(NULL);
	return 0;
}

void nuevaSolicitud(int sig){
	int siguiente= posicionSiguiente();
	//LO PRIMERO QUE HAY QUE COMPROBAR ES SI HAY HUECO PARA ENTRAR EN LA COLA
	if(posicionSiguiente()==tamCola){
		//LOG
	}
	else{

		(*(cola+siguiente)).id = generadorID();
		(*(cola+siguiente)).atendido = false;
		(*(cola+siguiente)).descartado = false;
		(*(cola+siguiente)).encolado = false;
		if(sig == SIGINT){ //SIGUSR1 -- invitación
			(*(cola+siguiente)).tipo = INVITACION;
			pthread_create(&(*(cola+siguiente)).tid, NULL, sol, &*(cola+siguiente));
			/*(*(solicitudes+numSolicitudes)).id = generadorID();
			(*(solicitudes+numSolicitudes)).atendido = false;
			(*(solicitudes+numSolicitudes)).tipo = 0;
			(*(solicitudes+numSolicitudes)).descartado = false;
			pthread_create(&*(hilos+numSolicitudes), NULL, sol, &*(solicitudes+numSolicitudes));

			numSolicitudes++;
			hilos = (pthread_t *) realloc(hilos, numSolicitudes); //Se reserva memoria para un elemento mas
			*/


		}else if(sig == SIGQUIT){ //SIGUSR2 -- QR
			(*(cola+siguiente)).tipo = QR;
			pthread_create(&(*(cola+siguiente)).tid, NULL, sol, &*(cola+siguiente));
			/*(*(solicitudes+numSolicitudes)).id = generadorID();
			(*(solicitudes+numSolicitudes)).atendido = false;
			(*(solicitudes+numSolicitudes)).tipo = 1;
			(*(solicitudes+numSolicitudes)).descartado = false;
			pthread_create(&*(hilos+numSolicitudes), NULL, sol, &*(solicitudes+numSolicitudes));

			numSolicitudes++;
			hilos = (pthread_t *) realloc(hilos, numSolicitudes); //Se reserva memoria para un elemento mas
			*/
		}else if(sig == SIGTSTP){
			peticionSolicitudes = false;
		}
	}
	
}


int posicionSiguiente(){
	int i=0;

	while(cola[i].id!=NULL){
		i++;
	}

	if(i>=tamCola){
		i=-1;
	}

	return i;
}
int calculaAleatorio(int min, int max){
	int aleatorio = (int) rand() % (max-min+1) + min;
	return aleatorio;
}

int generadorID(){
	return id++; 
}
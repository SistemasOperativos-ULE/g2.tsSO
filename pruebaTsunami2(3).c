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
	int tipo; //INVITACION = 0; QR = 1
	bool aceptado; //Lo hemos anyadido nosotros
};

int id = 1;
int numSolicitudes = 0; //contador que usamos para movernos entre las solicitudes, hilos, y las struct
bool peticionSolicitudes = true;

//Contador de solicitudes
struct solicitud *cola; //El tamanyo en principio es 15, pero puede variar

pthread_t *hilos; //TODOS LOS HILOS EN UN SOLO PUNTERO???
struct solicitud *solicitudes; //ES NECESARIO??? YO USARIA SOLO LA COLA DE SOLICITUDES

void nuevaSolicitud(int);
int calculaAleatorio(int min, int max);
int generadorID();
void *sol(void *arg);

void *sol(void *arg){ //Funcion que ejecutan los hilos al crearse
	struct solicitud *s;
	s = (struct solicitud *) arg;
	printf("Soy una solicitud, y mi id es %d\n", s->id);
	if(s->tipo == 0)
		printf("Soy de tipo Invitación\n");
	else
		printf("Soy de tipo QR\n");
	printf("Atendido = %d y Aceptado = %d\n", s->atendido, s->aceptado);
	//printf("Mi id es %d, atendido = %d, mi tipo es ' %d ' y aceptado es = %d\n", s->id, s->atendido, s->tipo, s->aceptado);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){

	cola = (struct solicitud *)malloc(sizeof(struct solicitud)*(TAMCOLADEFECTO));
	hilos = (pthread_t *)malloc(sizeof(pthread_t));
	solicitudes = (struct solicitud *)malloc(sizeof(struct solicitud));

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
	for(int i=0; i<numSolicitudes; i++){
		printf("-----------------------------------------\n");
		printf("ID --> %d\n", (*(solicitudes+i)).id);
		printf("ATENDIDO --> %d\n", (*(solicitudes+i)).atendido);
		if((*(solicitudes+i)).tipo == 1)
			printf("TIPO --> QR\n");
		else
			printf("TIPO --> Invitación\n");
		printf("ACEPTADO --> %d\n", (*(solicitudes+i)).aceptado);
	}
	printf("-----------------------------------------\n\n");

	for(int i=0; i<numSolicitudes; i++){
		pthread_join(*(hilos+i), NULL);
	}

	//free(solicitudes);
	printf("Todo esta cumplido, podemos cerrar\n");

	//pthread_join(*(hilos+0), NULL);
	//printf("Hilo 1 muerto\n");
	//pthread_exit(NULL);
	return 0;
}

void nuevaSolicitud(int sig){

	//LO PRIMERO QUE HAY QUE COMPROBAR ES SI HAY HUECO PARA ENTRAR EN LA COLA
	
	if(sig == SIGINT){ //SIGUSR1 -- invitación

		(*(solicitudes+numSolicitudes)).id = generadorID();
		(*(solicitudes+numSolicitudes)).atendido = false;
		(*(solicitudes+numSolicitudes)).tipo = 0;
		(*(solicitudes+numSolicitudes)).aceptado = false;
		pthread_create(&*(hilos+numSolicitudes), NULL, sol, &*(solicitudes+numSolicitudes));

		numSolicitudes++;
		hilos = (pthread_t *) realloc(hilos, numSolicitudes); //Se reserva memoria para un elemento mas
		

	}else if(sig == SIGQUIT){ //SIGUSR2 -- QR
		(*(solicitudes+numSolicitudes)).id = generadorID();
		(*(solicitudes+numSolicitudes)).atendido = false;
		(*(solicitudes+numSolicitudes)).tipo = 1;
		(*(solicitudes+numSolicitudes)).aceptado = false;
		pthread_create(&*(hilos+numSolicitudes), NULL, sol, &*(solicitudes+numSolicitudes));

		numSolicitudes++;
		hilos = (pthread_t *) realloc(hilos, numSolicitudes); //Se reserva memoria para un elemento mas

	}else if(sig == SIGTSTP){
		peticionSolicitudes = false;
	}
}


int calculaAleatorio(int min, int max){
	int aleatorio = (int) rand() % (max-min+1) + min;
	return aleatorio;
}

int generadorID(){
	return id++; 
}
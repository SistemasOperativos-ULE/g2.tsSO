#include <stdio.h>

//exit 
#include <stdlib.h> 

//hilos
#include <pthread.h> 

//fork
#include <sys/types.h> 

#include <unistd.h>
#include <time.h>

//valores boolean
#include <stdbool.h>

//senyales
#include <signal.h>

#define QR 0 //Usuarios que intentan acceder por QR
#define INVITACION 1 //Usuarios que intentan acceder por invitacion
#define PRO 3
#define TAMCOLADEFECTO 15
#define ATENCIONCORRECTA 1 
#define ERRORESDATOS 2 
#define ANTECEDENTES 3
#define SOLICITUD 0
#define ATENDEDOR 1
#define ACTIVIDADSOCIAL 2
#define PORATENDER 0
#define ATENDIENDO 1
#define ATENDIDO 2
#define TAMACTIVIDAD 4

struct atendedor{
	int id;
	int tipo; //INVITACION = 1; QR = 0, PRO=3;
	int numSolicitudes;
	pthread_t tid;
};

int idAtendedor;
struct atendedor *atendedores; //Punteros para que se pueda modificar el numero de atendedores dinamicamente
int numeroAtendedores;
bool fin;

int posicionSiguiente(int tipoCola);
void nuevoAtendedor(int tipo);
int generadorID(int tipo);
void aumentarNumAtendedores(int sig);
void finPrograma(int sig);
void *accionesAtendedor(void *arg);

/** 
* FUNCION PRINCIPAL
*
**/
int main(int argc, char *argv[]){

	struct sigaction sPrepararFin, sAumentarAtendedores;
	sPrepararFin.sa_handler = finPrograma;
	
	if(sigaction(SIGINT, &sPrepararFin, NULL) == -1){
		perror("Error al comunicar la finalizacion del programa");
		exit(-1);
	}

	sAumentarAtendedores.sa_handler = aumentarNumAtendedores;

	if(sigaction(SIGABRT, &sAumentarAtendedores, NULL) == -1){
		perror("Error en la peticion de aumento del numero de atendedores");
		exit(-1);
	}

	fin = false;
	idAtendedor= 0;
	numeroAtendedores = 3;

	atendedores = (struct atendedor *)calloc(sizeof(struct atendedor)*(numeroAtendedores), numeroAtendedores);

	for(int i = 0; i < numeroAtendedores; i++){
		atendedores[i].id = -1;
	}

	nuevoAtendedor(INVITACION);

	nuevoAtendedor(QR);

	for(int i=0;i<(numeroAtendedores-2);i++){
		nuevoAtendedor(PRO);
	}

	while(!fin){
		sleep(1);
	}

	pthread_exit(NULL);
}

/** 
* FUNCION MANEJADORA PARA LA SENIAL SIGINT, QUE TERMINARA EL PROGRAMA DE MANERA ORDENADA
*
**/
void finPrograma(int sig){

	if(fin){
		printf("Ya se esta trabajando en finalizar el programa.");
	}else{
		fin=true;
		printf("Ha llegado la senial de fin de programa.");		
	}	
}

/*
* CREA LOS HILOS DE LOS ATENDEDORES DEPENDIENDO DE SU TIPO
*/
void nuevoAtendedor(int tipo){

	printf("Metodo nuevo atendedor\n");
	int siguiente = posicionSiguiente(ATENDEDOR);
	printf("Siguiente = %d\n", siguiente);

	if(siguiente == -1){
		printf("No se puede anyadir otro atendedor\n");
	}else{
		(*(atendedores+siguiente)).id = idAtendedor++;
		(*(atendedores+siguiente)).tipo = tipo;
		(*(atendedores+siguiente)).numSolicitudes = 0;
		pthread_create(&(*(atendedores+siguiente)).tid, NULL, accionesAtendedor, &*(atendedores+siguiente));
		printf("Creado hilo de atendedor %d en la posicion %d\n", tipo, siguiente);
	}
}

int posicionSiguiente(int tipoCola){

	int i;
	bool encontrado;

	//pthread_mutex_lock();
	i = 0;
	encontrado = false;
	switch(tipoCola){
		case ATENDEDOR://Cola atendedores
			while((i < numeroAtendedores) && (!encontrado)){
				if(atendedores[i].id == -1){
					encontrado = true;
				}else{
					i++;
				}
			}
			if(i >= numeroAtendedores){
				i = -1;
			}
			break;
	}
	return i;
}

/*
*  INTRODUCE EN EL PROGRAMA EL NUMERO DE ATENDEDORES PRO DESEADOS POR EL USUARIO
*/
void aumentarNumAtendedores(int sig){
    int numNuevosAtendedores;

    if(fin){
    	printf("Ya se está acabando el programa, no se puede modificar\n");
    }else{

	    do{
		    printf("Introduzca el numero de atendedores que desea que existan en el sistema: \n");
		    printf("(Debe ser mayor que %d)\n", numeroAtendedores);
		    scanf("%d",&numNuevosAtendedores);
		}while(numNuevosAtendedores<=numeroAtendedores);

		printf("Saliendo del while de pregunta. Resultado = %d\n", numNuevosAtendedores);
	 
		//atendedores = (struct atendedor *)realloc(atendedores, (numNuevosAtendedores)*sizeof(struct atendedor *)); //TODO al ponerle mas espacio al puntero pone bien el id de los atendedores a -1 pero sigue dando errores
		struct atendedor *atendedoresNuevos;
		atendedoresNuevos = (struct atendedor *)calloc((numNuevosAtendedores)*sizeof(struct atendedor *), numNuevosAtendedores);
		
		printf("Cola atendedores: \n");
		for(int i=0; i<numeroAtendedores; i++){
			printf("Posicion %d --> Atendedor con id: %d\n", i, atendedores[i].id);
		}

		for(int i=0; i<numeroAtendedores; i++){	
			atendedoresNuevos[i] = atendedores[i];
		}	

		for(int i=numeroAtendedores; i<numNuevosAtendedores; i++){	
			atendedoresNuevos[i].id = -1;
		}	

		printf("Cola nuevos atendedores: \n");
		for(int i=0; i<numNuevosAtendedores; i++){
			printf("Posicion %d --> Atendedor con id: %d\n", i, atendedoresNuevos[i].id);
		}

		free(atendedores);
		atendedores = atendedoresNuevos;


		printf("Cola atendedores ACTUALIZADA: \n");
		for(int i=0; i<numNuevosAtendedores; i++){
			printf("Posicion %d --> Atendedor con id: %d\n", i, atendedores[i].id);
		}

 		int antiguosAtendedores = numeroAtendedores;
		numeroAtendedores = numNuevosAtendedores;

		printf("Seteado el numero de Atendedores nuevo	\n");

    	for(int i = antiguosAtendedores; i<numeroAtendedores; i++){
    		printf("Creado otro atendedor tipo PRO (%d)\n", i);
    		nuevoAtendedor(PRO);
    	}

    	for(int i=0; i<numeroAtendedores; i++){
			printf("El id del atendedor %d ahora es %d\n", i, (atendedores+i)->id);
		}

    	printf("Fin del for	\n");
    	//free(atendedoresNuevos);
	}
}

/** 
* ATIENDE A LAS SOLICITUDES, LES ASIGNA SUS TIPOS DE DATOS Y DUERME 10 SEGUNDOS CADA 5 SOLICITUDES ATENDIDAS
*
**/
void *accionesAtendedor(void *arg){

	struct atendedor *atendedorActual = (struct atendedor *) arg;
	int aleatorio;
	int idActual;

	printf("Atendedor %d: me he creado\n", atendedorActual->id);

	while(!fin){
		sleep(1);
	}

	printf("Atendedor %d: me elimino\n", atendedorActual->id);

	pthread_exit(NULL);
}
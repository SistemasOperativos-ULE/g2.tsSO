//PRACTICA FINAL G2 TSUNAMI DEMOCRATICO
//MIEMBROS: Angel Lopez Arias, Alejandro Perez Fernandez, Pablo Bayon Gutierrez y Diego Simon Gonzalez

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

#define QR 0 //Usuarios que intentan acceder por QR

#define INVITACION 1 //Usuarios que intentan acceder por invitacion

#define TAMCOLADEFECTO 15


/**
* DECLARACIONES GLOBALES
*
*
**/


struct solicitud{
	int id;
	bool atendido;
	int tipo; //invitacion o QR
	bool aceptado; //Lo hemos anyadido nosotros
};

//Semaforos y variables condicion


//Contador de solicitudes
int contadorSolicitudes = 1;

struct solicitud *cola; //El tamanyo en principio es 15, pero puede variar

//Lista de 4 usuarios en una actividad social
struct solicitud colaActividadSocial[4];

//Atendedores (lista o no)
pthread_t *atendedoresQR, atendedoresINV, atendedoresPRO; //Punteros para que se pueda modificar el numero de atendedores dinamicamente

FILE *registroTiempos;

int tamCola;

/**
* DECLARACIONES DE FUNCIONES
*
**/
//TODO hay que estudiar la signatura de las funciones

void nuevaSolicitud();
void accionesSolicitud();
void accionesAtendedor();
void accionesCoordinadorSocial();
void writeLogMessage(char *id, char *msg);
int calculaAleatorio(int min, int max);
void acabarPrograma(int sig);
void solicitudRechazada();
void compactar();


//	Se usa la notacion EM para zonas de exclusion mutua y VC para zonas en las que se deben usar variables condicion


/** 
* FUNCION PRINCIPAL
*
**/
int main(int argc, char *argv[]){

	int numPro;

	struct sigaction sLlegaSolicitud, sFinalizar;
	
	if(argc==1){
		tamCola = atoi(argv[1]);
		cola=(struct solicitud *)malloc(sizeof(struct solicitud)*(tamCola));
		atendedoresPRO = (pthread_t *)malloc(sizeof(pthread_t)*1);
	}else if(argc==2){
		tamCola = atoi(argv[1]);
		cola=(struct solicitud *)malloc(sizeof(struct solicitud)*(tamCola));

		numPro = atoi(argv[2]);
		atendedoresPRO = (pthread_t *)malloc(sizeof(pthread_t)*numPro);
	}else{
		tamCola = TAMCOLADEFECTO;
		cola=(struct solicitud *)malloc(sizeof(struct solicitud)*(TAMCOLADEFECTO));
		atendedoresPRO = (pthread_t *)malloc(sizeof(pthread_t)*1);
	}


	sLlegaSolicitud.sa_handler = nuevaSolicitud;//Se asigna la manejadora nuevaSolicitud a la estrutura sigaction 

	if(sigaction(SIGUSR1, &sLlegaSolicitud, NULL) == -1){ //Enmascaramos la senal SIGUSR1 (a partir de ahora se llamara a nuevaSolicitud cuando reciba SIGUSR1)
		perror("Error al comunicar la llegada de una nueva solicitud");
		exit(-1);
	}

	if(sigaction(SIGUSR2, &sLlegaSolicitud, NULL) == -1){ //Enmascaramos la senal SIGUSR1 (a partir de ahora se llamara a nuevaSolicitud cuando reciba SIGUSR2)
		perror("Error al comunicar la llegada de una nueva solicitud");
		exit(-1);
	}


	sFinalizar.sa_handler = acabarPrograma;//Se asigna la manejadora acabarPrograma a la estrutura sigaction 
	
	if(sigaction(SIGINT, &sFinalizar, NULL) == -1){ //Enmascaramos la senal SIGINT (a partir de ahora se llamara a acabarPrograma cuando reciba SIGINT)
		perror("Error al comunicar la finalizacion del programa");
		exit(-1);
	}



	//Manejar las seniales SIGUSR1, SIGUSR2 y SIGINT

	//EM  Inicializar recursos (semaforos, contador solicitudes, lista solcitues, atendedores, actividades sociales, fichero log, variables condicion)

	//3 hilos atendedores

	//Hilo coordinateur

	//Esperar por una de esas 3 seniales

	//Esperar seniales de forma infinita
}



/** 
* FUNCION QUE SIMULA LA LLEGADA DE UNA NUEVA SOLICITUD
*
**/
void nuevaSolicitud(){
	//Comprobar si hay espacio

	/**
	* EM if(hayEspacio){
	*	EM aniadir solicitud
	*	EM contador++
	*	EM nuevaSolicitud.id = contadorSolicitudes
	*	EM nuevaSolicitud.atendido = 0
	*	EM tipo = switch depende de la senial recibida
	*	EM Crear hilo para solicitud
	*}else{
	*	ignorar la llamada
	*}
	*
	**/
}



/** 
* FUNCION QUE MANEJA LAS ACCIONES QUE PUEDE REALIZAR UNA SOLICITUD
*
**/
void accionesSolicitud(){
	/** EM GUARDAR EN EL LOG:
	* 1- EM la hora de entrada
	* 2- EM el tipo de solicitud
	**/
	//3-dormir 4 segundos

	/** 
	*4- estoy siendo atendido??
	* if(noEstaSiendoAtendido){
	*	5-calcular el comportamiento de la solicitud 
	*		a-Si se va y  EM se escribe en el log, se acaba el hilo y EM se libera espacio en la cola
	*		b-Sino duerme 4 segundos y vuelve al punto 4
	*}else{
	*	6- Esperamos a que termine de ser atendido
	*	7- Calculamos si decide o no participar en una actividad social
	*	8 if(decideParticipar){
	*		a. EM if(puedeEntrar en la lista){
	*			EM entra, si es el ultimo avisa al cordinateur, libera espacio en cola, guardamos log de que esta preparado para actividad
	*			VC se queda esperando a poder comenzar, NADA duerme 3 segundos, VC sale de la cola y el ultimo avisa al coordinateur
	*			EM guardamos el log en el que deja la actividad
	*		b. else, espera 3 segundos y vuelve a a
	*	9- }else{
	*			EM Libera su posicion en cola de solicitudes y se va, escribe en el log
	*	   }
	*	10- Fin del hilo de usuario
	*
	**/


}



/** 
* FUNCION QUE MANEJA LAS TAREAS DEL ATENDEDOR
*
**/
void accionesAtendedor(){

	//SI SE TRATA DE UN ATENDEDOR PRO ATENDERA A QUIEN MAS TIEMPO LLEVE INDEPENDIENTEMENTE DEL TIPO

	/**
	* 1- EM Busca la primera solicitud de su tipo (la que mas tiempo lleve esperando)
	*	a. EM Si no hay de mi tipo busco de otro
	*	b. Si no hay usuarios para atender, duermo 1 segundo y vuelvo a 1
	* 2- Calculamos el tipo de atencion y en funcion de esto el tiempo de atencion (70, 20, 10)
	* 3- EM Cambiamos el flag atendido
	* 4- EM Guardamos en el log la hora de atencion
	* 5- Dormimos el tiempo de atencion
	* 6- EM Guardamos en el log la hora de fin de la atencion
	* 7- EM Guardamos en el log el motivo del fin de la atencion
	* 8- EM Cambiamos el flag de atendido
	* 9- Mira si le toca tomar cafe
	* 10- Volvemos al paso 1 y EM buscamos el siguiente (siempre con prioridad a su tipo)
	*
	**/
}



/** 
* FUNCION QUE MANEJA LAS ACCIONES DEL COORDINADOR SOCIAL
*
**/
void accionesCoordinadorSocial(){
	/**
	* 1- EM Espera a que le avisen de que hay 4 y cierra la lista para que ninguno mas entre
	* 2- VC Avisa a los procesos de que comiencen la actividad 
	* 3- EM Escribe en el log que comienza la actividad
	* 4- VC Espera a que le digan que han finalizado
	* 5- EM Escribe en el log que finaliza la actividad
	* 6- EM Abre la lista de nuevo y vuelve al paso 1
	*
	**/

}



/** 
* FUNCION QUE ESCRIBE LOS LOGS EN SU FICHERO
*
**/
void writeLogMessage(char *id, char *msg){
	//Tiempo actual
	time_t now = time(0);
	struct tm *tlocal = localtime(&now);
	char stnow[19];
	strftime(stnow, 19, "%d/%m/%y %H:%M:%S", tlocal);
	//Escribimos en el log
	registroTiempos = fopen("registroTiempos.log", "a");
	fprintf(registroTiempos, "[%s] %s: %s\n", stnow, id, msg);
	fclose(registroTiempos);
}



/** 
* FUNCION QUE CALCULA UN NUMERO ALEATORIO EN UN RANGO DADO 
*
**/
int calculaAleatorio(int min, int max){

	int aleatorio = (int) rand() % (max-min+1) + min;

	return aleatorio;
}



/** 
* FUNCION MANEJADORA PARA LA SENIAL SIGINT, QUE TERMINARA EL PROGRAMA DE MANERA ORDENADA
*
**/
void acabarPrograma(int sig){

}

/**
*
*
**/
void solicitudRechazada(){

}


/**
*
*
**/
bool encolar(struct solicitud solicitudEncolada){

	bool encolado=false;
	int posicion = posicionSiguiente();

	if(posicion ==-1){
		encolado = false;
	}else{
		cola[posicionSiguiente()] = solicitudEncolada;
		encolado=true;
	}

	return encolado;
	
}


void compactar(){
	int i=0; j=0, siguiente = posicionSiguiente();

	while(i<siguiente){
		if(cola[i]==NULL){
			for(j=i; j<siguiente; j++) {
				cola[i] = cola[i+1];
			}
			siguiente--;
			cola[siguiente] = NULL;
			i=siguiente;
		}
		i++;
	}
}


int posicionSiguiente(){
	int i=0;

	while(cola[i]!=NULL){
		i++;
	}

	if(i==tamCola){
		i=-1;
	}

	return i;
}

struct solicitud descartar(){
	//TODO REVISAR ESTE METODO 
	struct solicitud desencolada;

	desencolada = cola[];

	cola[posicionSiguiente()] = NULL;

	compactar();

	return desencolada;

}
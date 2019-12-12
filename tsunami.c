#include <stdio.h>

//exit 
#include <stdlib.h> 

//hilos
#include <pthread.h> 

//fork
#include <sys/types.h> 

#include <unistd.h>
#include <time.h>


/**
* DECLARACIONES GLOBALES
*
*
**/

//Semaforos y variables condicion

//Contador de solicitudes

//Lista de 15 usuarios (podemos hacer un array de int de 3 dimensiones o hacer un array de estructuras) Hay que almacenar ID, Atendido y Tipo

//Lista de 4 usuarios en una actividad social

//Atendedores (lista o no)

//Fichero de logs (FILE *logFile)



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


//	Se usa la notacion EM para zonas de exclusion mutua y VC para zonas en las que se deben usar variables condicion


/** 
* FUNCION PRINCIPAL
*
**/
int main(int argc, char *argv[]){
	//Manejar las seniales SIGUSR1, SIGUSR2 y SIGINT

	//EM  Inicializar recursos (semaforos, contador solicitudes, lista solcitues, atendedores, actividades sociales, fichero log, variables condicion)

	//3 hilos atendedores

	//Hilo coordinateur

	//Esperar por una de esas 3 seniales

	//Esperar seniales de forma infinita
}



/** 
* FUNCION QUE ESCRIBE LOS LOGS EN SU FICHERO
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
* FUNCION QUE ESCRIBE LOS LOGS EN SU FICHERO
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
* FUNCION QUE ESCRIBE LOS LOGS EN SU FICHERO
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
* FUNCION QUE ESCRIBE LOS LOGS EN SU FICHERO
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
	logFile = fopen(logFileName, "a");
	fprintf(logFile, "[%s] %s: %s\n", stnow, id, msg);
	fclose(logFile);
}
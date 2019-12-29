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


/**
* DECLARACIONES GLOBALES
*
*
**/

struct solicitud{
	int id;
	int tipo; //INVITACION = 1; QR = 0
	int atendido; //posicion 0 sin atender, 1 siendo atendido, 2 ya ha sido atendido
	pthread_t tid;
	int tipoDatos; //Atencion correcta, errores en los datos o antecedentes policiales
};

int idSolicitud;

//Inicialmente se pone al tamanio por defecto, puede que haya que cambiarlo en la parte extra de la practica (modificacion dinamica)
int tamCola;

pthread_mutex_t datosSolicitud, actividadSocial, escribirLog, comprobarFin;

pthread_cond_t empezadActividad, avisarCoordinador, candado;

struct atendedor{
	int id;
	int tipo; //INVITACION = 1; QR = 0, PRO=3;
	int numSolicitudes;
	pthread_t tid;
};

int idAtendedor;

struct atendedor *atendedores; //Punteros para que se pueda modificar el numero de atendedores dinamicamente
int numeroAtendedores;
//Contador de solicitudes
struct solicitud *cola; //El tamanyo en principio es 15, pero puede variar

struct solicitud colaActividadSocial[TAMACTIVIDAD]; //TODO

int contadorActividad;

bool candadoEntrarActividad;


FILE *registroTiempos;

bool fin; //protegida??

/**
* DECLARACIONES DE FUNCIONES
*
**/
//TODO eliminar declaraciones sobrantes

void nuevaSolicitud(int sig);
void *accionesSolicitud(void *arg);
void *accionesAtendedor(void *arg);
void *accionesCoordinadorSocial(void *arg);
void writeLogMessage(char *id, char *msg);
int calculaAleatorio(int min, int max);
void ignoraSenyales(int sig);
void compactar();
void imprimeEstado();
int posicionSiguiente(int tipoCola);
void nuevoAtendedor(int tipo);
int generadorID(int tipo);
bool descartar(int val);
int buscarSiguiente(int tipo);
void borrarDeLaCola(int id);
void borrarColaActividad(int id);
//void acabarPrograma(int sig);
bool estanTodosAtendidos();





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

	struct sigaction sLlegaSolicitud, sPrepararFin, sFinalizar;
	pthread_t coordinador;
	char buffer[100], quienHabla[50];
	sLlegaSolicitud.sa_handler = nuevaSolicitud;

	if(sigaction(SIGUSR1, &sLlegaSolicitud, NULL) == -1){
		perror("Error al comunicar la llegada de una nueva solicitud");
		exit(-1);
	}

	if(sigaction(SIGUSR2, &sLlegaSolicitud, NULL) == -1){ 
		perror("Error al comunicar la llegada de una nueva solicitud");
		exit(-1);
	}


	sPrepararFin.sa_handler = ignoraSenyales;
	
	if(sigaction(SIGINT, &sPrepararFin, NULL) == -1){
		perror("Error al comunicar la finalizacion del programa");
		exit(-1);
	}


	//TODO esto perez y yo creemos que no hace falta, en vez de usar otra senial y otra manejadora podemos usar join a los hilos atendedor y coordinador social 


	/*
	sFinalizar.sa_handler = acabarPrograma;

	if(sigaction(SIGTRAP, &sFinalizar, NULL) == -1){
		perror("Error al comunicar la finalizacion del programa");
		exit(-1);
	}
	*/

	if(argc==2){
		tamCola = atoi(argv[1]);
		numeroAtendedores = 3;	
	}else if(argc==3){
		tamCola = atoi(argv[1]);
		numeroAtendedores = atoi(argv[2])+2;
	}else{
		tamCola = TAMCOLADEFECTO;
		numeroAtendedores = 3;
	}

	
	if(pthread_mutex_init(&datosSolicitud, NULL) != 0) {
		//Error en la inicializacion del mutex
		exit(-1);
	}

	if(pthread_mutex_init(&actividadSocial, NULL) != 0) {
		//Error en la inicializacion del mutex
		exit(-1);
	}

	if(pthread_mutex_init(&comprobarFin, NULL) != 0) {
	//Error en la inicializacion del mutex
	exit(-1);
	}

	if(pthread_cond_init(&avisarCoordinador,NULL) != 0){
		//Error en la inicializacion del mutex
		exit(-1);
	}

	if(pthread_cond_init(&empezadActividad,NULL) != 0){
		//Error en la inicializacion del mutex
		exit(-1);
	}

	if(pthread_cond_init(&candado,NULL) != 0){
		//Error en la inicializacion del mutex
		exit(-1);
	}

	pthread_mutex_lock(&escribirLog);
	sprintf(buffer, "Empieza");	
	sprintf(quienHabla,"\n\nSistema"); 
	writeLogMessage(quienHabla, buffer);
	pthread_mutex_unlock(&escribirLog);

	srand(time(NULL));

	idSolicitud = 1;
	fin = false;
	idAtendedor= 1;
	contadorActividad = 0;
	candadoEntrarActividad = true;


	cola=(struct solicitud *)malloc(sizeof(struct solicitud)*(tamCola));
	atendedores = (struct atendedor *)malloc(sizeof(struct atendedor)*(numeroAtendedores));

	//Inicializacion de la identificacion de las estructuras para poder buscar la posicionSiguiente de la cola
	for(int i = 0; i < TAMCOLADEFECTO; i++){
		cola[i].id = -1;
	}

	for(int i = 0; i < numeroAtendedores; i++){
		atendedores[i].id = -1;
	}

	//CREAMOS LOS ATENDEDORES
	nuevoAtendedor(INVITACION);

	nuevoAtendedor(QR);

	for(int i=0;i<(numeroAtendedores-2);i++){
		nuevoAtendedor(PRO);
	}

	pthread_create(&coordinador, NULL, accionesCoordinadorSocial, NULL);

	while(fin==false){ //TODO ES ESTA LA MEJOR OPCION? puede ser que intente comprobar si se ha llegado al final antes de que fin=true en la manejadora
		pause(); //TODO hay que comprobar el fin de programa dentro de un mutex
	}

	//Aqui ya ha llegado la senial de que se acaba el programa, asique espero a que se mueran los atendedores y el coordinador social
	pthread_join(coordinador,NULL);

	for(int i = 0; i < numeroAtendedores; i++){
		pthread_join((*(atendedores+i)).tid,NULL);
	}


	//TODO LIBERAR TODOS LOS PUNTEROS, DESTRUIR MUTEX, ETC
	free(cola);
	free(atendedores);

	pthread_mutex_lock(&escribirLog);
	sprintf(buffer, "Se ha acabado el programa"); //TODO comprobar que esto se escribe lo ultimo
	sprintf(quienHabla,"Sistema"); 
	writeLogMessage(quienHabla, buffer);
	pthread_mutex_unlock(&escribirLog);

	return 0;
}


/*
* FUNCION QUE CREA LOS ATENDEDORES SEGUN EL TIPO QUE SEAN
*/
void nuevoAtendedor(int tipo){

	int siguiente;
	siguiente = posicionSiguiente(ATENDEDOR);
	if(siguiente == -1){
		printf("No se puede anyadir otro atendedor\n");
	}else{
		(*(atendedores+siguiente)).id = generadorID(ATENDEDOR);
		(*(atendedores+siguiente)).tipo = tipo;
		(*(atendedores+siguiente)).numSolicitudes = 0;
		pthread_create(&(*(atendedores+siguiente)).tid, NULL, accionesAtendedor, &*(atendedores+siguiente));
	}
}

int generadorID(int tipo){
	if(tipo == ATENDEDOR){
		return idAtendedor++;
	}else{
		return idSolicitud++;
	}
}



/** 
* FUNCION QUE SIMULA LA LLEGADA DE UNA NUEVA SOLICITUD
*
**/
void nuevaSolicitud(int sig){
	//Comprobar si hay espacio

	/**
	* EM if(hayEspacio){
	*	EM aniadir solicitud
	*	EM contador++
	*	EM nuevaSolicitud.id = id
	*	EM nuevaSolicitud.atendido = 0
	*	EM tipo = switch depende de la senial recibida
	*	EM Crear hilo para solicitud
	*}else{
	*	ignorar la llamada
	*}
	*
	**/	

	//TODO if(fin==true) no cogemos mas senyales
	int siguiente;
	bool ignorarSenyales = false;
	char buffer[100], quienHabla[50];

	pthread_mutex_lock(&comprobarFin);
	ignorarSenyales = fin;
	pthread_mutex_unlock(&comprobarFin);

	if(!ignorarSenyales){
		pthread_mutex_lock(&datosSolicitud);
		siguiente = posicionSiguiente(SOLICITUD);
		if(siguiente == -1){
			printf("No se puede anyadir otra solicitud\n");
			pthread_mutex_lock(&escribirLog);
			sprintf(buffer, "La solicitud no ha podido entrar porque la cola estaba llena");	
			sprintf(quienHabla,"Sistema"); 
			writeLogMessage(quienHabla, buffer);
			pthread_mutex_unlock(&escribirLog);
			pthread_mutex_unlock(&datosSolicitud);
		}else{
			(*(cola+siguiente)).id = generadorID(SOLICITUD);
			(*(cola+siguiente)).atendido = PORATENDER;
			(*(cola+siguiente)).tipoDatos = 69;

			printf("La solicitud %d tiene el flag de atendido a %d\n", (*(cola+siguiente)).id, (*(cola+siguiente)).atendido);

			if(sig == SIGUSR1){ //SIGUSR1 -- invitación
				(*(cola+siguiente)).tipo = INVITACION;
				pthread_mutex_unlock(&datosSolicitud);
				pthread_create(&(*(cola+siguiente)).tid, NULL, accionesSolicitud, &*(cola+siguiente));

			}else if(sig == SIGUSR2){ //SIGUSR2 -- QR
				(*(cola+siguiente)).tipo = QR;
				pthread_mutex_unlock(&datosSolicitud);
				pthread_create(&(*(cola+siguiente)).tid, NULL, accionesSolicitud, &*(cola+siguiente));

			}
		}
	}else{
		pthread_mutex_lock(&escribirLog);
		sprintf(buffer, "No pueden entrar mas solicitudes porque el programa esta en proceso de finalizacion");	
		sprintf(quienHabla,"Sistema"); 
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);
	}
}



/** 
* FUNCION QUE MANEJA LAS ACCIONES QUE PUEDE REALIZAR UNA SOLICITUD
*
**/
void *accionesSolicitud(void *arg){ //Funcion que ejecutan los hilos al crearse

	/** EM GUARDAR EN EL LOG:
	* 1- EM la hora de entrada
	* 2- EM el tipo de solicitud
	**/
	//3-dormir 3 segundos

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
	*			EM guardamos el log el que deja la actividad
	*		b. else, espera 3 segundos y vuelve a a
	*	9- }else{
	*			EM Libera su posicion en cola de solicitudes y se va, escribe en el log
	*	   }
	*	10- Fin del hilo de usuario
	*
	**/

	//TODO ojo con los mutex datosSolicitud en esta funcion

	struct solicitud *solicitudActual; //la estructura a la que apunta el puntero que le pasamos al hilo
	solicitudActual = (struct solicitud *) arg;
	int siguienteActSoc;
	char buffer[100], quienHabla[50];
	int idActual;
	int tipoActual;

	//TODO comprobar si hay que tener un mutex aqui
	pthread_mutex_lock(&datosSolicitud);
	idActual = solicitudActual->id;
	tipoActual = solicitudActual->tipo;
	pthread_mutex_unlock(&datosSolicitud);	

	pthread_mutex_lock(&escribirLog);

	if(tipoActual == QR){
		sprintf(buffer, "Acabo de entrar en el sistema, soy de tipo QR");	
	}else{
		sprintf(buffer, "Acabo de entrar en el sistema, soy de tipo Invitación");
	}
	sprintf(quienHabla, "Solicitud %d", idActual); 
	writeLogMessage(quienHabla, buffer);
	pthread_mutex_unlock(&escribirLog);

	//imprimimos los datos que tiene la estructura asociada al hilo
	//Esta parte del codigo es solo para comprobar el funcionamiento

	//TODO comprobar si se puede borrar este mutex
	//pthread_mutex_lock(&datosSolicitud);
	printf("Soy una solicitud, y mi id es %d\n", idActual);
	if(tipoActual == INVITACION){
		printf("Soy de tipo Invitación\n");
	}else{
		printf("Soy de tipo QR\n");
	}
	//printf("Atendido = %d", solicitudActual->atendido);
	//pthread_mutex_unlock(&datosSolicitud);

	sleep(4);

	//Ahora viene la parte en la que la solicitud comprueba cada 3 segundos si ha sido aceptada o rechazada
	//TODO revisar el mutex de datosSolicitud
	pthread_mutex_lock(&datosSolicitud);
	while(solicitudActual->atendido == PORATENDER){
		if (descartar(tipoActual)== false){
			pthread_mutex_unlock(&datosSolicitud);
			sleep(3);
			pthread_mutex_lock(&datosSolicitud);
		} else{
			pthread_mutex_lock(&escribirLog);
			sprintf(buffer, "He sido descartada");	
			sprintf(quienHabla, "Solicitud %d", idActual); 
			writeLogMessage(quienHabla, buffer);
			pthread_mutex_unlock(&escribirLog);
			borrarDeLaCola(idActual);
			pthread_mutex_unlock(&datosSolicitud);
			pthread_exit(NULL);
		}
	}
	pthread_mutex_unlock(&datosSolicitud);
	pthread_mutex_lock(&escribirLog);
	sprintf(buffer, "Estoy siendo atendida");	
	sprintf(quienHabla, "Solicitud %d", idActual); 
	writeLogMessage(quienHabla, buffer);
	pthread_mutex_unlock(&escribirLog);

	
	//TODO espera activa? variable condicion? que pasa con la variable condicion cuando ha varios que estan haciendo esta espera a la vez?
	//TODO revisar el mutex de datosSolicitud
	pthread_mutex_lock(&datosSolicitud);
	while(solicitudActual->atendido == ATENDIENDO){
		pthread_mutex_unlock(&datosSolicitud);
		sleep(1);
		pthread_mutex_lock(&datosSolicitud);	
	}
	pthread_mutex_unlock(&datosSolicitud);

	//Ya ha sido atendido
	pthread_mutex_lock(&datosSolicitud);
	pthread_mutex_lock(&escribirLog);
	//aqui el atendedor ya ha seteado la clase de solicitud que somos

	/* TODO esto seria lo suyo
	if(solicitudActual->tipoDatos==ATENCIONCORRECTA){
		printf("Mis datos son correctos\n");
		sprintf(buffer, "Mis datos son correctos");
	}else if(solicitudActual->tipoDatos==ERRORESDATOS){
		printf("Mis datos tienen errores\n");
		sprintf(buffer, "Mis datos tienen errores");
	}else if(solicitudActual->tipoDatos==ANTECEDENTES){
		printf("¡¡TENGO ANTECEDENTES!!\n");
		sprintf(buffer, "¡¡TENGO ANTECEDENTES!!");
	}else{
		sprintf(buffer, "NO tengo ni puta idea de que tipo son mis datos");
		printf("ESTO ESTA MAL %d\n", solicitudActual->tipoDatos);
	}
	*/

	if(solicitudActual->tipoDatos == ANTECEDENTES){
		printf("¡¡TENGO ANTECEDENTES!!\n");
		sprintf(buffer, "¡¡TENGO ANTECEDENTES!!");
	}else if(solicitudActual->tipoDatos == ERRORESDATOS){
		printf("Mis datos tienen errores\n");
		sprintf(buffer, "Mis datos tienen errores");
	}else{
		printf("Mis datos son correctos\n");
		sprintf(buffer, "Mis datos son correctos");
	}

	sprintf(quienHabla, "Solicitud %d", idActual); 
	writeLogMessage(quienHabla, buffer);

	pthread_mutex_unlock(&datosSolicitud);

	pthread_mutex_unlock(&escribirLog);

	pthread_mutex_lock(&datosSolicitud);
	if(solicitudActual->tipoDatos == ANTECEDENTES){ //Este tipo de solicitud no puede participar en actividades sociales, asi que se va pa su casa
		pthread_mutex_lock(&escribirLog);
		sprintf(buffer, "Como no puedo participar en ninguna actividad social, me voy.");	
		sprintf(quienHabla, "Solicitud %d", idActual); 
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);

		//Inicializamos todos los parametros de la estructura del hilo a cero
		borrarDeLaCola(idActual);
		pthread_mutex_unlock(&datosSolicitud);
		//Eliminamos el hilo con el exit
		pthread_exit(NULL);

	}else if(calculaAleatorio(0, 1) == 0){ //NO QUIERE PARTICIPAR
		pthread_mutex_lock(&escribirLog);
		sprintf(buffer, "No quiero participar en ninguna actividad social, así que me voy.");	
		sprintf(quienHabla, "Solicitud %d",idActual);
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);

		//Inicializamos todos los parametros de la estructura del hilo a cero
		borrarDeLaCola(idActual);
		pthread_mutex_unlock(&datosSolicitud);
		//Eliminamos el hilo con el exit
		pthread_exit(NULL);

	}else{

		pthread_mutex_unlock(&datosSolicitud);
		pthread_mutex_lock(&escribirLog);
		sprintf(buffer, "Estoy lista para participar en la actividad social.");	
		sprintf(quienHabla, "Solicitud %d", idActual);
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);

		//Aqui la solicitud ya ha sido atendida y sabe de que tipo son sus datos, si se ha acabado el programa sse autodestruye
		if(fin){
			sprintf(buffer, "Me autodestruyo debido a que se ha dado la orden de acabar el programa.");
			writeLogMessage(quienHabla, buffer);

			//TODO Comprobar si esto esta bien
			pthread_mutex_lock(&datosSolicitud);
			borrarDeLaCola(idActual);
			pthread_mutex_unlock(&datosSolicitud);

			pthread_exit(NULL);
		}

		pthread_mutex_lock(&actividadSocial);

		//TODO Hay que ver como manejamos las solicitudes que ya se han metido en ese wait antes de que llegue la senial de fin de programa
		//Creemos que la unica manera de hacerlo es hcer un destroy a estas solicitudes. 

		while(!candadoEntrarActividad){
			pthread_cond_wait(&candado, &actividadSocial);
		}


		//Aqui ya puede entrar a la actividad social
		pthread_mutex_lock(&datosSolicitud);
		colaActividadSocial[contadorActividad] = *solicitudActual; //TODO comprobar que se copia el contenido

		contadorActividad++;

		borrarDeLaCola(idActual);
		pthread_mutex_unlock(&datosSolicitud);

		if(contadorActividad == TAMACTIVIDAD){
			pthread_cond_signal(&avisarCoordinador);
		}
		
		//pthread_mutex_unlock(&actividadSocial);

		//TODO escribir en el log que estoy preparado para la actividad

		//pthread_mutex_lock(&actividadSocial);

		pthread_cond_wait(&empezadActividad, &actividadSocial);

		//Esto es la actividad basicamente, dormir
		

		if(contadorActividad == 1){ //TODO buscar una opcion mejor de que en total se espere 3 segundos
			sleep(3); 
		}

		//TODO borrar de la cola
		borrarColaActividad(idActual);
		contadorActividad--;

		if(contadorActividad == 0){
			printf("SEEEEEEEE ACABOOOOO\n");
			pthread_cond_signal(&avisarCoordinador); //Avisa de que ya han salido todos de la actividad
		}

		pthread_mutex_lock(&escribirLog);
		//Escribir en el log que se ha acabado la actividad
		pthread_mutex_unlock(&escribirLog);

		pthread_mutex_unlock(&actividadSocial);


	}

	//-->NO , SE EXPULSA
	//-->SI , ESPERA A QUE LA COLA DE ACTV SE VACIE Y ENTRA, PARA ESPERAR A SER MATADO (VARIABLE CONDICION
	//TODO//wait(coordinador.me.mata.o.coordinador.me.mata)	
	pthread_exit(NULL);
}



/** 
* FUNCION QUE MANEJA LAS TAREAS DEL ATENDEDOR
*
**/
void *accionesAtendedor(void *arg){

	//SI SE TRATA DE UN ATENDEDOR PRO ATENDERA A QUIEN MAS TIEMPO LLEVE INDEPENDIENTEMENTE DEL TIPO
	struct atendedor *atendedorActual = (struct atendedor *) arg;
	int solicitudAatender;
	int aleatorio;
	int tiempoAtencion;
	char buffer[100], quienHabla[50];

	//TODO Comprobar

	pthread_mutex_lock(&comprobarFin);

	sprintf(quienHabla, "Atendedor %d", atendedorActual->id);

	//TODO comprobar por que solo se autodestruye bien el atendedor 3, los demas se quedan por ahi en algun sitio

	while(!estanTodosAtendidos() || !fin){

		pthread_mutex_unlock(&comprobarFin);

		pthread_mutex_lock(&datosSolicitud);
		solicitudAatender = buscarSiguiente(atendedorActual->tipo);
		if(solicitudAatender == -1){
				pthread_mutex_unlock(&datosSolicitud);
				sleep(1);
		}else{

			aleatorio = calculaAleatorio(1,10);
			printf("El numero aleatorio calculado es: %d\n", aleatorio);

			if(aleatorio <= 7){
				(cola[solicitudAatender]).tipoDatos = ATENCIONCORRECTA;
				tiempoAtencion = calculaAleatorio(1,4);
				printf("A la solicitud %d le estoy metiendo el tipo %d y soy el atendedor %d\n", cola[solicitudAatender].id, cola[solicitudAatender].tipoDatos, atendedorActual->id);
			}else if(aleatorio <= 9){
				(cola[solicitudAatender]).tipoDatos = ERRORESDATOS;
				tiempoAtencion = calculaAleatorio(2,6);
				printf("A la solicitud %d le estoy metiendo el tipo %d y soy el atendedor %d\n", cola[solicitudAatender].id, cola[solicitudAatender].tipoDatos, atendedorActual->id);
			}else{
				(cola[solicitudAatender]).tipoDatos = ANTECEDENTES;
				tiempoAtencion = calculaAleatorio(6,10);
				printf("A la solicitud %d le estoy metiendo el tipo %d y soy el atendedor %d\n", cola[solicitudAatender].id, cola[solicitudAatender].tipoDatos, atendedorActual->id);
			}

			cola[solicitudAatender].atendido = ATENDIDO;
			pthread_mutex_unlock(&datosSolicitud);

			sleep(tiempoAtencion);

			//TODO hace falta mutex aqui? Los atendedores no se eliminan de la cola
			atendedorActual->numSolicitudes++;

			//TODO TENER EN CUENTA LOS DOS CAMBIOS DE FLAG DE ATENDIDO QUE PONE EL DISENIO

			if((atendedorActual->numSolicitudes % 5 == 0) && (atendedorActual->numSolicitudes!=0)){
				//le toca tomar el cafe
				pthread_mutex_lock(&escribirLog);
				sprintf(buffer, "Voy a tomar un cafe llevo %d",atendedorActual->numSolicitudes);	
				//sprintf(quienHabla, "Atendedor %d",atendedorActual->id); 
				writeLogMessage(quienHabla, buffer);
				pthread_mutex_unlock(&escribirLog);
				sleep(10);
				pthread_mutex_lock(&escribirLog);
				sprintf(buffer, "Acabe de tomar un cafe");	
				//sprintf(quienHabla, "Atendedor %d",atendedorActual->id); 
				writeLogMessage(quienHabla, buffer);
				pthread_mutex_unlock(&escribirLog);
			}
		}
		pthread_mutex_lock(&comprobarFin);
	}

	pthread_mutex_unlock(&comprobarFin);

	//Aqui ya se ha atendido a todo el mundo y ya ha llegado la senial de fin de programa, asique el atendedor se autodestruye
	sprintf(buffer, "Me autodestruyo");

	pthread_mutex_lock(&escribirLog);
	writeLogMessage(quienHabla, buffer);
	pthread_mutex_unlock(&escribirLog);

	pthread_exit(NULL);


	//TODO esperar hasta que la cola de la actividad social este vacia
}



/** 
* FUNCION QUE MANEJA LAS ACCIONES DEL COORDINADOR SOCIAL
*
**/
void *accionesCoordinadorSocial(void *arg){
	/**
	* 1- EM Espera a que le avisen de que hay 4 y cierra la lista para que ninguno mas entre
	* 2- VC Avisa a los procesos de que comiencen la actividad 
	* 3- EM Escribe en el log que comienza la actividad
	* 4- VC Espera a que le digan que han finalizado
	* 5- EM Escribe en el log que finaliza la actividad
	* 6- EM Abre la lista de nuevo y vuelve al paso 1
	*
	**/

	char buffer[50], quienHabla[50];


	pthread_mutex_lock(&comprobarFin);
	while(fin == false){
		pthread_mutex_unlock(&comprobarFin);


		pthread_mutex_lock(&actividadSocial);

		pthread_cond_wait(&avisarCoordinador, &actividadSocial);
		candadoEntrarActividad = false;

		pthread_cond_broadcast(&empezadActividad);

		pthread_mutex_lock(&escribirLog);
		printf("La actividad puede comenzar\n");
		sprintf(buffer, "La actividad puede comenzar");	
		sprintf(quienHabla, "Coordinador"); 
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);


		pthread_cond_wait(&avisarCoordinador, &actividadSocial);

		pthread_mutex_lock(&escribirLog);
		sprintf(buffer, "La actividad ha finalizado");	
		sprintf(quienHabla, "Coordinador"); 
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);


		candadoEntrarActividad = true;

		pthread_mutex_unlock(&actividadSocial);

		pthread_mutex_lock(&comprobarFin);

	}

	pthread_mutex_unlock(&comprobarFin);

	//Como he salido del bucle, se ha acabado el programa y se destruye el hilo
	sprintf(buffer, "Me autodestruyo");

	pthread_mutex_lock(&escribirLog);
	writeLogMessage(quienHabla, buffer);
	pthread_mutex_unlock(&escribirLog);

	pthread_exit(NULL);

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


void imprimeEstado(){

	printf("Una vez creados todos las solicitudes, esto es lo que tenemos en el sistema:\n\n");

	//TODO Cuidado si la funcion posicionSiguiente devuelve -1
	int siguiente = posicionSiguiente(SOLICITUD);
	for(int i=0; i< siguiente; i++){
		printf("-----------------------------------------\n");
		printf("ID --> %d\n", (*(cola+i)).id);
		printf("ATENDIDO --> %d\n", (*(cola+i)).atendido);
		if((*(cola+i)).tipo == QR){
			printf("TIPO --> QR\n");
		}else{
			printf("TIPO --> Invitación\n");
		}
	}
	printf("-----------------------------------------\n\n");
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
void ignoraSenyales(int sig){
	char buffer[50], quienHabla[50];

	//TODO 

	if(fin){
		printf("Ya se esta trabajando en finalizar el programa");
		pthread_mutex_lock(&escribirLog);
		sprintf(buffer, "Ya se esta trabajando en finalizar el programa");
		sprintf(quienHabla, "Sistema"); 
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);
	}else{
		fin=true;
		sprintf(buffer, "Ha llegado la senial de fin de programa");
		sprintf(quienHabla, "Sistema"); 
		pthread_mutex_lock(&escribirLog);
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);

		imprimeEstado();
	}
	
}

/**
*
*
**/
/*bool encolar(struct solicitud solicitudEncolada){

	bool encolado=false;
	int posicion = posicionSiguiente(SOLICITUD);

	if(posicion ==-1){
		encolado = false;
	}else{
		cola[posicionSiguiente()] = solicitudEncolada;
		encolado=true;
	}

	return encolado;
	
}*/

void borrarColaActividad(int id){
	bool encontrado = false;
	int i = 0;
	while(i < TAMACTIVIDAD && !encontrado){
		if(colaActividadSocial[i].id == id){
			colaActividadSocial[i].id = -1;
			encontrado = true;
		}else{
			i++;
		}
	}
}


void borrarDeLaCola(int id){
	//TODO revisar el mutex de datosSolicitud
	bool encontrado = false;
	int i = 0;
	while(i < tamCola && !encontrado){
		if(cola[i].id == id){
			cola[i].id = -1;
			cola[i].atendido = PORATENDER;
			encontrado = true;
		}else{
			i++;
		}
	}
	compactar(i);
}

void compactar(int posicionVacia){

		bool acabado = false;
		int i = posicionVacia;

		while((i<tamCola-1) && (!acabado)){
			cola[i] = cola[i+1];

			if(cola[++i].id==-1){
				acabado=true;
			}else{
				cola[i].id = -1;
			}
		}
}


int posicionSiguiente(int tipoCola){

	int i;
	bool encontrado;

	//pthread_mutex_lock();
	i = 0;
	encontrado = false;
	switch(tipoCola){
		case SOLICITUD://Cola solicitudes
			while(i < tamCola && !encontrado){
				if(cola[i].id == -1){
					encontrado = true;
				}else{
					i++;
				}
			}

			if(i >= tamCola){
				i = -1;
			}
			break;
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

		case ACTIVIDADSOCIAL:
			while(i < TAMACTIVIDAD && !encontrado){
				if(colaActividadSocial[i].id == -1){
					encontrado = true;
				}else{
					i++;
				}
			}

			if(i >= TAMACTIVIDAD){
				i = -1;
			}
			break;
	}
	//pthread_mutex_unlock();
	//printf("El tipo de cola %d da posicion %d\n", tipoCola, i);
	return i;
}

bool descartar(int val){
	//Dependiendo del tipo, se descarta un porcentaje u otro
	bool expulsar = false;

	if(val == QR){
		if(calculaAleatorio(1, 10)<=3){ //Un 30% se descartan
			expulsar=true;
		}else{
			if(calculaAleatorio(1, 100)<=15){ //Un 15% del resto se descartan
				expulsar=true;
			}
		}
	}

	if(val == INVITACION){
		if(calculaAleatorio(1, 10)<=1){ //Un 10% se descartan
			expulsar=true;
		}else{
			if(calculaAleatorio(1, 100)<=15){ //Un 15% del resto se descartan
				expulsar=true;
			}
		}
	}

	return expulsar;
}


int buscarSiguiente(int tipo){
	int encontrado = -1;
	bool busquedaTerminada = false;
	int i = 0;
	int siguiente = posicionSiguiente(SOLICITUD);
	if(siguiente != -1){
		switch(tipo){
			case PRO:
				while(!busquedaTerminada && i<tamCola){
					if(cola[i].atendido == PORATENDER && cola[i].id != -1){
						encontrado = i;
						busquedaTerminada = true;
					}else{
						i++;
					}
				}
				break;
			default:
				while(!busquedaTerminada && i<tamCola){
					if(cola[i].atendido == PORATENDER && cola[i].tipo == tipo && cola[i].id != -1){
						encontrado = i;
						busquedaTerminada = true;
					}else{
						i++;
					}
				}
				if(encontrado == -1){
					return buscarSiguiente(PRO);
				}
		}
	}

	return encontrado;
}

bool estanTodosAtendidos(){
	int i = 0;
	bool algunoSinAtender = false;

	pthread_mutex_lock(&datosSolicitud);

	//Cada vez que se elimina una soliicitud de la cola se compacta, asi que solo cabe la posibilidad de que este vacia si la primera posicion esta vacia
	while(cola[i].id!=-1 && i<tamCola && !algunoSinAtender ){
		if(cola[i].atendido != ATENDIDO){
			algunoSinAtender = true;
		}else{
			i++;
		}
	}

	pthread_mutex_unlock(&datosSolicitud);

	return !algunoSinAtender;
}

/*
void acabarPrograma(int sig){
	//TODO Comprobar si esto hace falta
	//TODO destruir todo, para ANGEL
	//Realmente solo hay que destruir todo lo que esta siempre, es decir, mutex, atendedores, etc.
}
*/
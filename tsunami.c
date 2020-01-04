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
	int tipoDatos; //Atencion correcta(1), errores en los datos(2) o antecedentes(3)
};

int idSolicitud;

//Inicialmente se pone al tamanio por defecto, puede que haya que cambiarlo en la parte extra de la practica (modificacion dinamica)
int tamCola;

pthread_mutex_t datosSolicitud, actividadSocial, escribirLog, comprobarFin;

pthread_cond_t empezadActividad, avisarCoordinador, candadoActividadAbierto;

struct atendedor{
	int id;
	int tipo; //INVITACION = 1; QR = 0, PRO=3;
	int numSolicitudes;
	pthread_t tid;
};

int idAtendedor;

struct atendedor *atendedores; //Punteros para que se pueda modificar el numero de atendedores dinamicamente

int numeroAtendedores;

struct solicitud *cola;

struct solicitud colaActividadSocial[TAMACTIVIDAD];

int contadorActividad;

bool candadoEntrarActividad;


FILE *registroTiempos;

bool fin;

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
void finPrograma(int sig);
void compactar();
int posicionSiguiente(int tipoCola);
void nuevoAtendedor(int tipo);
int generadorID(int tipo);
bool descartar(int val);
int buscarSiguiente(int tipo);
void borrarDeLaCola(int id);
void borrarColaActividad(int id);
bool estanTodosAtendidos();
void aumentarNumAtendedores(int sig);
void aumentarNumSolicitudes(int sig);



/** 
* FUNCION PRINCIPAL
*
**/
int main(int argc, char *argv[]){

	struct sigaction sLlegaSolicitud, sPrepararFin, sFinalizar, sAumentarSolicitudes, sAumentarAtendedores;
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


	sPrepararFin.sa_handler = finPrograma;
	
	if(sigaction(SIGINT, &sPrepararFin, NULL) == -1){
		perror("Error al comunicar la finalizacion del programa");
		exit(-1);
	}

	sAumentarSolicitudes.sa_handler = aumentarNumSolicitudes;

	if(sigaction(SIGTRAP, &sAumentarSolicitudes, NULL) == -1){
		perror("Error en la peticion de aumento del numero de solicitudes");
		exit(-1);
	}

	sAumentarAtendedores.sa_handler = aumentarNumAtendedores;

	if(sigaction(SIGABRT, &sAumentarAtendedores, NULL) == -1){
		perror("Error en la peticion de aumento del numero de atendedores");
		exit(-1);
	}

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
		perror("Error en la inicializacion de un mutex");
		exit(-1);
	}

	if(pthread_mutex_init(&actividadSocial, NULL) != 0) {
		perror("Error en la inicializacion de un mutex");
		exit(-1);
	}

	if(pthread_mutex_init(&comprobarFin, NULL) != 0) {
		perror("Error en la inicializacion de un mutex");		
		exit(-1);
	}

	if(pthread_cond_init(&avisarCoordinador,NULL) != 0){
		perror("Error en la inicializacion de una variable condicion");	
		exit(-1);
	}

	if(pthread_cond_init(&empezadActividad,NULL) != 0){
		perror("Error en la inicializacion de una variable condicion");	
		exit(-1);
	}

	if(pthread_cond_init(&candadoActividadAbierto,NULL) != 0){
		perror("Error en la inicializacion de una variable condicion");	
		exit(-1);
	}

	pthread_mutex_lock(&escribirLog);
	sprintf(buffer, "EMPIEZA");	
	sprintf(quienHabla,"Sistema"); 
	writeLogMessage(quienHabla, buffer);
	pthread_mutex_unlock(&escribirLog);

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

	nuevoAtendedor(INVITACION);

	nuevoAtendedor(QR);

	for(int i=0;i<(numeroAtendedores-2);i++){
		nuevoAtendedor(PRO);
	}

	pthread_create(&coordinador, NULL, accionesCoordinadorSocial, NULL);

	for(int i = 0; i < numeroAtendedores; i++){
		pthread_join((*(atendedores+i)).tid,NULL);
	}

	pthread_cancel(coordinador);

	for(int i = 0; i < tamCola; i++){
		if((*(cola+i)).id !=-1){
			pthread_cancel((*(cola+i)).tid);
		}
	}

	for(int i = 0; i < TAMACTIVIDAD; i++){

		if((colaActividadSocial+i)->id !=-1){
			pthread_cancel((colaActividadSocial+i)->tid);
		}
	}

	pthread_cancel(coordinador);

	pthread_mutex_lock(&escribirLog);
	sprintf(buffer, "El coordinador ha fallecido");
	sprintf(quienHabla,"Sistema"); 
	writeLogMessage(quienHabla, buffer);
	pthread_mutex_unlock(&escribirLog);

	free(cola);
	free(atendedores);

	pthread_mutex_lock(&escribirLog);
	sprintf(buffer, "Se ha acabado el programa\n----------------------------------------------------\n");
	sprintf(quienHabla,"Sistema"); 
	writeLogMessage(quienHabla, buffer);
	pthread_mutex_unlock(&escribirLog);
	

	if(pthread_mutex_destroy(&datosSolicitud) != 0){
		perror("Error al destruir datosSolicitud");	
		exit(-1);
	}

	//No hemos destruido el mutex actividadSocial ni la variable condicion avisarCoordinador, porque debido a nuestro disenyo, el coordinador 
	//esta esperando en avisarCoordinador asociada a ese mutex a la hora de destruirlo

	if(pthread_mutex_destroy(&escribirLog) != 0){
		perror("Error al destruir escribirLog");	
		exit(-1);
	}

	if(pthread_mutex_destroy(&comprobarFin) != 0){
		perror("Error al destruir comprobarFin");	
		exit(-1);
	}

	if(pthread_cond_destroy(&empezadActividad) != 0){
		perror("Error al destruir empezadActividad");	
		exit(-1);
	}

	if(pthread_cond_destroy(&candadoActividadAbierto) != 0){
		perror("Error al destruir candadoActividadAbierto");	
		exit(-1);
	}

	return 0;
}


/*
* CREA LOS HILOS DE LOS ATENDEDORES DEPENDIENDO DE SU TIPO
*/
void nuevoAtendedor(int tipo){

	int siguiente;
	siguiente = posicionSiguiente(ATENDEDOR);

	(*(atendedores+siguiente)).id = generadorID(ATENDEDOR);
	(*(atendedores+siguiente)).tipo = tipo;
	(*(atendedores+siguiente)).numSolicitudes = 0;
	pthread_create(&(*(atendedores+siguiente)).tid, NULL, accionesAtendedor, &*(atendedores+siguiente));
}


/*
* CALCULA EL SIGUIENTE ID PARA ATENDEDORES Y SOLICITUDES
*/
int generadorID(int tipo){
	if(tipo == ATENDEDOR){
		return idAtendedor++;
	}else{
		return idSolicitud++;
	}
}



/** 
* SI HAY ESPACIO EN LA COLA CREA UN HILO PARA LA SOLICITUD, SI NO, LA IGNORA
*
**/
void nuevaSolicitud(int sig){


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
			printf("No se puede anyadir otra solicitud, la cola esta llena\n");
			pthread_mutex_lock(&escribirLog);
			sprintf(buffer, "La solicitud no ha podido entrar porque la cola estaba llena");	
			sprintf(quienHabla,"Sistema"); 
			writeLogMessage(quienHabla, buffer);
			pthread_mutex_unlock(&escribirLog);
			pthread_mutex_unlock(&datosSolicitud);
		}else{
			(*(cola+siguiente)).id = generadorID(SOLICITUD);
			(*(cola+siguiente)).atendido = PORATENDER;

			printf("La solicitud %d tiene el flag de atendido a %d\n", (*(cola+siguiente)).id, (*(cola+siguiente)).atendido);

			if(sig == SIGUSR1){ //SIGUSR1 -- Invitación
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
* LA SOLICITUD ESPERA A SER ATENDIDA Y EN FUNCION DE SI LE DEJAN ACCEDER A UNA ACTIVIDAD, LUEGO DECIDIRA SI
* PARTICIPA EN ELLA O NO. SI DECIDE PARTICIPAR, ENTRA EN LA COLA DE LA ACTIVIDAD SOCIAL
*/
void *accionesSolicitud(void *arg){ 

	struct solicitud *solicitudActual; //la estructura a la que apunta el puntero que le pasamos al hilo
	int siguienteActSoc;
	char buffer[100], quienHabla[50];
	int idActual;
	int tipoActual;

	solicitudActual = (struct solicitud *) arg;

	//Para poder cancelar el hilo en cualquier momento
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	srand(pthread_self());

	pthread_mutex_lock(&datosSolicitud);
	idActual = solicitudActual->id;
	tipoActual = solicitudActual->tipo;
	pthread_mutex_unlock(&datosSolicitud);	

	pthread_mutex_lock(&escribirLog);
	if(tipoActual == QR){
		sprintf(buffer, "Acabo de entrar en el sistema, soy de tipo QR.");	
	}else{
		sprintf(buffer, "Acabo de entrar en el sistema, soy de tipo Invitación.");
	}
	sprintf(quienHabla, "Solicitud %d", idActual); 
	writeLogMessage(quienHabla, buffer);
	pthread_mutex_unlock(&escribirLog);

	sleep(4);

	pthread_mutex_lock(&datosSolicitud);

	while(solicitudActual->atendido == PORATENDER){
		if (descartar(tipoActual) == false){
			pthread_mutex_unlock(&datosSolicitud);
			sleep(3);
			pthread_mutex_lock(&datosSolicitud);
		} else {

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

	
	pthread_mutex_lock(&datosSolicitud);
	while(solicitudActual->atendido == ATENDIENDO){
		pthread_mutex_unlock(&datosSolicitud);
		sleep(1);
		pthread_mutex_lock(&datosSolicitud);	
	}

	pthread_mutex_lock(&escribirLog);

	
	if(solicitudActual->tipoDatos == ANTECEDENTES){
		sprintf(buffer, "¡¡TENGO ANTECEDENTES!!");
	}else if(solicitudActual->tipoDatos == ERRORESDATOS){
		sprintf(buffer, "Mis datos tienen errores");
	}else{
		sprintf(buffer, "Mis datos son correctos");
	}
	
	pthread_mutex_unlock(&escribirLog);

	pthread_mutex_unlock(&datosSolicitud); //TODO unlock y lock seguidos?
	pthread_mutex_lock(&datosSolicitud);

	if(solicitudActual->tipoDatos == ANTECEDENTES){ 

		pthread_mutex_lock(&escribirLog);
		sprintf(buffer, "Como no puedo participar en ninguna actividad social, me voy.");	
		sprintf(quienHabla, "Solicitud %d", idActual); 
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);

		borrarDeLaCola(idActual);
		pthread_mutex_unlock(&datosSolicitud);
		pthread_exit(NULL);

	}else if(calculaAleatorio(0, 1) == 0){ //NO QUIERE PARTICIPAR
		pthread_mutex_lock(&escribirLog);
		sprintf(buffer, "No quiero participar en ninguna actividad social, así que me voy.");	
		sprintf(quienHabla, "Solicitud %d",idActual);
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);

		borrarDeLaCola(idActual);
		pthread_mutex_unlock(&datosSolicitud);

		pthread_exit(NULL);

	}else{

		pthread_mutex_unlock(&datosSolicitud);
		pthread_mutex_lock(&escribirLog);
		sprintf(buffer, "Estoy lista para participar en la actividad social.");	
		sprintf(quienHabla, "Solicitud %d", idActual);
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);

		//Aqui la solicitud ya ha sido atendida y sabe de que tipo son sus datos, si se ha acabado el programa se autodestruye
		if(fin){
			pthread_mutex_lock(&escribirLog);
			sprintf(buffer, "Me autodestruyo debido a que se ha dado la orden de acabar el programa.");
			writeLogMessage(quienHabla, buffer);

			pthread_mutex_lock(&datosSolicitud);
			borrarDeLaCola(idActual);
			pthread_mutex_unlock(&datosSolicitud);

			pthread_exit(NULL);
		}

		pthread_mutex_lock(&actividadSocial);

		while(!candadoEntrarActividad){
			pthread_cond_wait(&candadoActividadAbierto, &actividadSocial);
		}

		//Aqui ya puede entrar a la actividad social
		pthread_mutex_lock(&datosSolicitud);

		colaActividadSocial[contadorActividad] = *solicitudActual;

		contadorActividad++;

		borrarDeLaCola(idActual);
		pthread_mutex_unlock(&datosSolicitud);

		if(contadorActividad == TAMACTIVIDAD){
			pthread_cond_signal(&avisarCoordinador);
		}

		pthread_cond_wait(&empezadActividad, &actividadSocial);
		
		if(contadorActividad == 1){
			sleep(3); 
		}

		borrarColaActividad(idActual);
		contadorActividad--;

		if(contadorActividad == 0){
			pthread_cond_signal(&avisarCoordinador); //Avisa de que ya han salido todos de la actividad
		}

		pthread_mutex_unlock(&actividadSocial);


	}

	pthread_exit(NULL);
}



/** 
* ATIENDE A LAS SOLICITUDES, LES ASIGNA SUS TIPOS DE DATOS Y DUERME 10 SEGUNDOS CADA 5 SOLICITUDES ATENDIDAS
**/
void *accionesAtendedor(void *arg){

	//TODO Ojo a acceder a campos del atendedor sin mutex cuando hagamos lo de asignacion dinamica de recursos.

	srand(pthread_self());

	struct atendedor *atendedorActual;
	int solicitudAatender;
	int aleatorio;
	int tiempoAtencion;
	char buffer[100], quienHabla[50];

	atendedorActual = (struct atendedor *) arg; 

	pthread_mutex_lock(&comprobarFin);

	sprintf(quienHabla, "Atendedor %d", atendedorActual->id);

	while(!estanTodosAtendidos() || !fin){

		pthread_mutex_unlock(&comprobarFin);

		pthread_mutex_lock(&datosSolicitud);
		solicitudAatender = buscarSiguiente(atendedorActual->tipo);

		if(solicitudAatender == -1){
				pthread_mutex_unlock(&datosSolicitud);
				sleep(1);
		}else{

			pthread_mutex_lock(&escribirLog);
			sprintf(buffer, "Estoy atendiendo a la solicitud %d.", cola[solicitudAatender].id);	
			writeLogMessage(quienHabla, buffer);
			pthread_mutex_unlock(&escribirLog);

			aleatorio = calculaAleatorio(1,10);
			printf("El numero aleatorio calculado es: %d\n", aleatorio);

			cola[solicitudAatender].atendido = ATENDIENDO;

			if(aleatorio <= 7){
				(cola[solicitudAatender]).tipoDatos = ATENCIONCORRECTA;
				tiempoAtencion = calculaAleatorio(1,4);
				
				pthread_mutex_lock(&escribirLog);
				sprintf(buffer, "Ya he acabado de atender a la solicitud %d, que tiene todo en regla.", cola[solicitudAatender].id);	
				writeLogMessage(quienHabla, buffer);
				pthread_mutex_unlock(&escribirLog);

			}else if(aleatorio <= 9){
				(cola[solicitudAatender]).tipoDatos = ERRORESDATOS;
				tiempoAtencion = calculaAleatorio(2,6);
			
				pthread_mutex_lock(&escribirLog);
				sprintf(buffer, "Ya he acabado de atender a la solicitud %d, que tiene errores en sus datos.", cola[solicitudAatender].id);	
				writeLogMessage(quienHabla, buffer);
				pthread_mutex_unlock(&escribirLog);

			}else{
				(cola[solicitudAatender]).tipoDatos = ANTECEDENTES;
				tiempoAtencion = calculaAleatorio(6,10);
			
				pthread_mutex_lock(&escribirLog);
				sprintf(buffer, "Ya he acabado de atender a la solicitud %d, que tiene antecedentes.", cola[solicitudAatender].id);	
				writeLogMessage(quienHabla, buffer);
				pthread_mutex_unlock(&escribirLog);

			}

			pthread_mutex_unlock(&datosSolicitud);

			sleep(tiempoAtencion);

			cola[solicitudAatender].atendido = ATENDIDO;

			//TODO ojo cuando hagamos asignacion dinamica
			atendedorActual->numSolicitudes++;

			if((atendedorActual->numSolicitudes % 5 == 0) && (atendedorActual->numSolicitudes!=0)){
				pthread_mutex_lock(&escribirLog);
				sprintf(buffer, "Voy a tomar un cafe, llevo %d",atendedorActual->numSolicitudes);	
				writeLogMessage(quienHabla, buffer);
				pthread_mutex_unlock(&escribirLog);
				sleep(10);
				pthread_mutex_lock(&escribirLog);
				sprintf(buffer, "Ya he tomado el cafe");	
				writeLogMessage(quienHabla, buffer);
				pthread_mutex_unlock(&escribirLog);
			}
		}
		pthread_mutex_lock(&comprobarFin);
	}

	pthread_mutex_unlock(&comprobarFin);

	sprintf(buffer, "Me autodestruyo");

	pthread_mutex_lock(&escribirLog);
	writeLogMessage(quienHabla, buffer);
	pthread_mutex_unlock(&escribirLog);

	pthread_exit(NULL);

}



/** 
* ESPERA A QUE ENTREN 4 SOLICITUDES A LA ACTIVIDAD, AVISA DEL COMIENZO Y ESPERA A QUE FINALICEN
*
**/
void *accionesCoordinadorSocial(void *arg){

	//Para que se pueda cancelar el hilo en cualquier momento
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	char buffer[50], quienHabla[50];


	pthread_mutex_lock(&comprobarFin);
	while(fin == false){
		pthread_mutex_unlock(&comprobarFin);


		pthread_mutex_lock(&actividadSocial);

		pthread_cond_wait(&avisarCoordinador, &actividadSocial);
		candadoEntrarActividad = false;


		pthread_cond_broadcast(&empezadActividad);

		pthread_mutex_lock(&escribirLog);
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

		pthread_cond_broadcast(&candadoActividadAbierto);

		pthread_mutex_unlock(&actividadSocial);

		pthread_mutex_lock(&comprobarFin);

	}

	pthread_mutex_unlock(&comprobarFin);

}



/** 
* ESCRIBE LOS LOGS EN SU FICHERO
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
void finPrograma(int sig){
	char buffer[50], quienHabla[50];

	pthread_mutex_lock(&comprobarFin);
	if(fin){
		pthread_mutex_unlock(&comprobarFin);
		printf("Ya se esta trabajando en finalizar el programa");
		pthread_mutex_lock(&escribirLog);
		sprintf(buffer, "Ya se esta trabajando en finalizar el programa.");
		sprintf(quienHabla, "Sistema"); 
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);
	}else{
		fin=true;
		pthread_mutex_unlock(&comprobarFin);
		sprintf(buffer, "Ha llegado la senial de fin de programa.");
		sprintf(quienHabla, "Sistema"); 
		pthread_mutex_lock(&escribirLog);
		writeLogMessage(quienHabla, buffer);
		pthread_mutex_unlock(&escribirLog);
	}
	
	
}

/**
* PONE EL ID PASADO COMO ARGUMENTO DE LA COLA DE LA ACTIVIDAD A -1, PARA INDICAR QUE SE HA ELIMINADO
* ESE ELEMENTO DE LA COLA
**/
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

/**
* PONE EL ID PASADO COMO ARGUMENTO A -1 EN LA COLA DE SOLICITUDES Y LLAMA A COMPACTAR
**/
void borrarDeLaCola(int id){
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

/**
* ELIMINA LAS POSICIONES VACIAS QUE PUEDA HABER POR EL MEDIO DE LA COLA Y LAS SITUA AL FINAL
**/
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

/**
* 
**/
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
* AUMENTA EL NUMERO MAXIMO DE SOLICITUDES QUE PUEDEN ENTRAR EN EL SISTEMA
*/
void aumentarNumSolicitudes(int sig){
    int numSolicitudes;

    pthread_mutex_lock(&comprobarFin);

    if(fin){
    	printf("Ya se está acabando el programa, no se puede modificar\n");
    }else{

    	do{
    		printf("Introduzca el numero de solicitudes que desea que existan en el sistema: \n");
		    scanf("%d",&numSolicitudes);
    	}while(numSolicitudes<=tamCola);
		    
	 
	    //TODO hacemos muchos locks o que hacemos?

	    pthread_mutex_lock(&datosSolicitud);
	    pthread_mutex_lock(&actividadSocial);
	    pthread_mutex_lock(&escribirLog);

	 
	    cola = (struct solicitud *)realloc(cola, numSolicitudes*sizeof(struct solicitud *));

	    tamCola = numSolicitudes;

	    pthread_mutex_unlock(&datosSolicitud);
	    pthread_mutex_unlock(&actividadSocial);
	    pthread_mutex_unlock(&escribirLog);


	}

    pthread_mutex_unlock(&comprobarFin);
    
}
 
 
/*
*  INTRODUCE EN EL PROGRAMA EL NUMERO DE ATENDEDORES PRO DESEADOS POR EL USUARIO
*/
void aumentarNumAtendedores(int sig){
    int numNuevosAtendedores;
 
        pthread_mutex_lock(&comprobarFin);

    if(fin){
    	printf("Ya se está acabando el programa, no se puede modificar\n");
    }else{

    	do{
		    printf("Introduzca el numero de solicitudes que desea que existan en el sistema: \n");
		    scanf("%d",&numNuevosAtendedores);
		}while(numNuevosAtendedores<=numeroAtendedores);

	    pthread_mutex_lock(&datosSolicitud);
	    pthread_mutex_lock(&actividadSocial);
	    pthread_mutex_lock(&escribirLog);

	 
		atendedores = (struct atendedor *)realloc(atendedores, numNuevosAtendedores*sizeof(struct atendedor *));

		numeroAtendedores = numNuevosAtendedores;

		for(int i = numeroAtendedores; i < numNuevosAtendedores; i++){
        	nuevoAtendedor(PRO);
    	}

	    pthread_mutex_unlock(&datosSolicitud);
	    pthread_mutex_unlock(&actividadSocial);
	    pthread_mutex_unlock(&escribirLog);


	}

    pthread_mutex_unlock(&comprobarFin);

}

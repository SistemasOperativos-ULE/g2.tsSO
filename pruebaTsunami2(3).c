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
#define PRO 3
#define TAMCOLADEFECTO 15
#define ATENCIONCORRECTA 1 
#define ERRORESDATOS 2 
#define ANTECEDENTES 3
#define SOLICITUD 0
#define ATENDEDOR 1

struct solicitud{
	int id;
	int tipo; //INVITACION = 1; QR = 0
	bool descartado; //Lo hemos anyadido nosotros
	bool atendido;
	pthread_t tid;
	int clase; //Atencion correcta, errores en los datos o antecedentes policiales
};

int idSolicitud = 1;

//Inicialmente se pone al tamanio por defecto, puede que haya que cambiarlo en la parte extra de la practica (modificacion dinamica)
int tamCola = TAMCOLADEFECTO;
//int numSolicitudes = 0; //contador que usamos para movernos entre las solicitudes, hilos, y las struct
bool peticionSolicitudes = true;


struct atendedor{
	int id;
	int tipo; //INVITACION = 1; QR = 0, PRO=3;
	int numSolicitudes;
	pthread_t tid;
};

int idAtendedor= 1;

struct atendedor *atendedores; //Punteros para que se pueda modificar el numero de atendedores dinamicamente
int numeroAtendedores = 3;
//Contador de solicitudes
struct solicitud *cola; //El tamanyo en principio es 15, pero puede variar

//pthread_t *hilos; //TODOS LOS HILOS EN UN SOLO PUNTERO???
//struct solicitud *solicitudes; //ES NECESARIO??? YO USARIA SOLO LA COLA DE SOLICITUDES
int posicionSiguiente(int tipoCola);
void nuevaSolicitud(int tipo);
int calculaAleatorio(int min, int max);
int generadorID(int tipo);
void *sol(void *arg);
bool descartar(int val);
void borrarEstructura(void *args);
void nuevoAtendedor(int tipo);
int buscarSiguiente(int tipo);


void *sol(void *arg){ //Funcion que ejecutan los hilos al crearse
	struct solicitud *s; //la estructura a la que apunta el puntero que le pasamos al hilo
	s = (struct solicitud *) arg;

	//imprimimos los datos que tiene la estructura asociada al hilo
	printf("Soy una solicitud, y mi id es %d\n", (*s).id);
	if(s->tipo == INVITACION)
		printf("Soy de tipo Invitación\n");
	else
		printf("Soy de tipo QR\n");
	printf("Atendido = %d y descartado = %d\n", s->atendido, s->descartado);
	
	//Ahora viene la parte en la que la solicitud comprueba cada 3 segundos si ha sido aceptada o rechazada
	while(s->descartado == false && s->atendido == false){
		if (descartar(s->tipo)== false)
			sleep(3);
		else{
			borrarEstructura(&s);
			//Eliminamos el hilo con el exit
			pthread_exit(NULL);
		}
	}


	//Cuando sale del while es que tiene uno de los 2 a true
	//aqui el atendedor ya ha seteado la clase de solicitud que somos
	if(s->clase==ATENCIONCORRECTA)
		printf("Mis datos estan correctos\n");
	else if(s->clase==ERRORESDATOS)
		printf("Mis datos estan con errores\n");
	else if(s->clase==ANTECEDENTES)
		printf("TENGO ANTECEDENTES!!\n");

	//      TTTTTTTTTTTTTTOOOOOOOOOOOOOODDDDDDDDDDDDDDDDDDDDDDOOOOOOOOOOOOOOO
	// AQUI HAY QUE ESPERAR A QUE ACABE DE SER ATENDIDO

	if(s->clase == ANTECEDENTES){ //Este tipo de solicitud no puede participar en actividades sociales, asique se va pa su casa
		//Inicializamos todos los parametros de la estructura del hilo a cero
		borrarEstructura(&s);
		//Eliminamos el hilo con el exit
		pthread_exit(NULL);

	}else if(calculaAleatorio(0, 1)==0){ //NO QUIERE PARTICIPAR
		//Inicializamos todos los parametros de la estructura del hilo a cero
		borrarEstructura(&s);
		//Eliminamos el hilo con el exit
		pthread_exit(NULL);

	}else{
		//TOOOOOOODOOOOOOOOOOOOOOOOOOOOO
		//SI QUIERE PARTICIPAR
		//TODO: ESPERAR A LA COLA VACIA
	}

	//-->NO , SE EXPULSA
	//-->SI , ESPERA A QUE LA COLA DE ACTV SE VACIE Y ENTRA, PARA ESPERAR A SER MATADO (VARIABLE CONDICION
	//TO DO//wait(coordinador.me.mata.o.coordinador.me.mata)
	
	pthread_exit(NULL);
}

void *accionesAtendedor(void *arg){

	//SI SE TRATA DE UN ATENDEDOR PRO ATENDERA A QUIEN MAS TIEMPO LLEVE INDEPENDIENTEMENTE DEL TIPO
	struct atendedor *at = (struct atendedor *) arg;
	int solicitudAatender;
	int aleatorio;
	int tiempoAtencion;

	while(true){

		do{
			solicitudAatender = buscarSiguiente(at->tipo);
			if(solicitudAatender == -1){
				sleep(1);
			}
		}while(solicitudAatender == -1);

		aleatorio = calculaAleatorio(1,10);

		if(aleatorio <= 7){
			cola[solicitudAatender].clase = ATENCIONCORRECTA;
			tiempoAtencion = calculaAleatorio(1,4);
		}else if(aleatorio <= 9){
			cola[solicitudAatender].clase = ERRORESDATOS;
			tiempoAtencion = calculaAleatorio(2,6);
		}else{
			cola[solicitudAatender].clase = ANTECEDENTES;
			tiempoAtencion = calculaAleatorio(6,10);
		}

		cola[solicitudAatender].atendido = true;

		sleep(tiempoAtencion);

		//TODO TENER EN CUENTA LOS DOS CAMBIOS DE FLAG DE ATENDIDO QUE PONE EL DISENIO

		if(at->numSolicitudes % 5 == 0){
			//le toca tomar el cafe
			sleep(10);
		}

	}

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


int buscarSiguiente(int tipo){
	int encontrado = -1;
	bool busquedaTerminada = false;
	int i = 0;
	int siguiente = posicionSiguiente(SOLICITUD);
	if(siguiente != 0){
		switch(tipo){
			case PRO:
				while(!busquedaTerminada){
					if(cola[i].descartado == false && cola[i].atendido == false){
						encontrado = i;
						busquedaTerminada = true;
					}else{
						i++;
					}
				}
				break;

			default:
				while(!busquedaTerminada){
					if(cola[i].descartado == false && cola[i].atendido == false && cola[i].tipo == tipo){
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

void borrarEstructura(void *args){

	/*struct solicitud *s; //la estructura a la que apunta el puntero que le pasamos al hilo
	s = (struct solicitud *) arg;

	s->id = -1;
	s->tipo = -1;
	s->aceptado = false;
	s->descartado = false;
	s->atendido = false;
	s->tid = NULL;*/

	//FUNCION DESENCOLAR

}

int main(int argc, char *argv[]){
	cola = (struct solicitud *)malloc(sizeof(struct solicitud)*(TAMCOLADEFECTO));
	atendedores = (struct atendedor *)malloc(sizeof(struct atendedor)*(numeroAtendedores));
	int siguiente;
	struct sigaction sLlegaSolicitud;
	sLlegaSolicitud.sa_handler = nuevaSolicitud;//Se asigna la manejadora nuevaSolicitud a la estrutura sigaction


	//Inicializamos los id de las solicitudes a -1 para comprobar la posicion siguiente mas adelante

	for(int i = 0; i < TAMCOLADEFECTO; i++){
		cola[i].id = -1;
	}


	//CREAMOS LOS ATENDEDORES
	nuevoAtendedor(INVITACION);

	nuevoAtendedor(QR);

	for(int i=3;i<=numeroAtendedores;i++){
		nuevoAtendedor(PRO);
	}


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
	for(int i=0; i<posicionSiguiente(SOLICITUD)-1; i++){
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
	pthread_exit(NULL);
	//rerurn 0;
}


/*
* FUNCION QUE CREA LOS ATENDEDORES SEGUN EL TIPO QUE SEAN
*/
void nuevoAtendedor(int tipo){

	int siguiente;

	siguiente= posicionSiguiente(ATENDEDOR);
	(*(atendedores+siguiente)).id = generadorID(ATENDEDOR);
	(*(atendedores+siguiente)).tipo = tipo;
	(*(atendedores+siguiente)).numSolicitudes = 0;
	pthread_create(&(*(atendedores+siguiente)).tid, NULL, accionesAtendedor, &*(atendedores+siguiente));

}

	
void nuevaSolicitud(int sig){
	int siguiente= posicionSiguiente(SOLICITUD);
	//LO PRIMERO QUE HAY QUE COMPROBAR ES SI HAY HUECO PARA ENTRAR EN LA COLA
	if(posicionSiguiente(SOLICITUD)==tamCola){
		//LOG
	}
	else{

		(*(cola+siguiente)).id = generadorID(SOLICITUD);
		(*(cola+siguiente)).atendido = false;
		(*(cola+siguiente)).descartado = false;

		if(sig == SIGINT){ //SIGUSR1 -- invitación
			(*(cola+siguiente)).tipo = INVITACION;
			pthread_create(&(*(cola+siguiente)).tid, NULL, sol, &*(cola+siguiente));

		}else if(sig == SIGQUIT){ //SIGUSR2 -- QR
			(*(cola+siguiente)).tipo = QR;
			pthread_create(&(*(cola+siguiente)).tid, NULL, sol, &*(cola+siguiente));

		}else if(sig == SIGTSTP){
			peticionSolicitudes = false;
		}
	}
}

int posicionSiguiente(int tipoCola){


	int i=0;
	switch(tipoCola){
		case 0://Cola solicitudes
			while(cola[i].id != -1){
				i++;
			}
			if(i>=tamCola){
				i=-1;
			}
			return i;
			break;
		case 1://Cola atendedores
			i=0;
			while(atendedores[i].id != -1){
				i++;
			}
			if(i>=numeroAtendedores){
				i=-1;
			}
			return i;
			break;
	}
	
}

int calculaAleatorio(int min, int max){
	int aleatorio = (int) rand() % (max-min+1) + min;
	return aleatorio;
}

int generadorID(int tipo){
	if(tipo=SOLICITUD)
		return idAtendedor++;
	else
		return idSolicitud++;
}

bool descartar(int val){
	//Dependiendo del tipo, se descarta un porcentaje u otro
	bool expulsar = false;

	if(val == QR){
		if(calculaAleatorio(1, 10)<=3) //Un 30% se descartan
			expulsar=true;
		else
			if(calculaAleatorio(1, 100)<=15) //Un 15% del resto se descartan
				expulsar=true;
	}

	if(val == INVITACION){
		if(calculaAleatorio(1, 10)<=1) //Un 10% se descartan
			expulsar=true;
		else
			if(calculaAleatorio(1, 100)<=15) //Un 15% del resto se descartan
				expulsar=true;
	}

	return expulsar;
}
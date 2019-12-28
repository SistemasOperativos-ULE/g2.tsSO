#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void menu();
int opcion();

int main(){
	int num;
	do{
		menu();
		num = opcion();
		switch(num){
			case 0:
				printf("Has elegido salir del menu\n");
				break;
			case 1:
				printf("Enviando un SIGUSR1 - Solicitud tipo Invitación\n");
				kill(getpid(), SIGUSR1);
				break;
			case 2:
				printf("Enviando un SIGUSR2 - Solicitud tipo QR\n");
				kill(getpid(), SIGUSR2);
				break;
			case 3:
				printf("Enviando un SIGINT - Finalizando programa\n");
				kill(getpid(), SIGINT);
				break;
			default:
				printf("Se ha introducido un numero raro\n");
		}
	}while(num!=0);
}

void menu(){
	printf("------------------------------------------------------\n");
	printf("--                MENU PRACTICA FINAL               --\n");
	printf("------------------------------------------------------\n");
	printf("1) SOLICITUD TIPO INVITACIÓN\n");
	printf("2) SOLICITUD TIPO QR\n");
	printf("3) FINALIZAR PROGRAMA\n");
	printf("0) Salir del menu\n");
}

int opcion(){
	int resultado;
	printf("\nIntroduce una opcion:\n");
	scanf("%d", &resultado);
	return resultado;
}
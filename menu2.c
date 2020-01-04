#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
void fin(int sig);
void ejecutar(int senial1,int senial2,int alternar,int pid, int espera);
void ejecutarAvanzado(int senial1,int senial2,int alternar,int espera,int pid,int repetir,int patron1,int patron2);
void menu(int pid);
int pid = 0;
int main(int argc, char *argv[]){

//Función principal.
	if(argv[1] == NULL){
		//Error
		printf("El número de argumentos no coincide con los solicitados\n");
		printf("Introduzca:\n");
		printf("./mandador pid\n");
	}else{
		//Defino las variables que voy a utilizar
		//Asigno el pid
		pid = atoi(argv[1]);
		menu(pid);
	}
	return 0;
}

void menu(int pid){
int senial1 = 0,senial2 = 0,alternar = 0, defecto = 0,espera = 0, avanzado = 0,repetir = 0, patron1 = 0,patron2 = 0;
	//Enmascaro la señal de salida
	signal(SIGINT, &fin);
	//Configuracion basica
	printf("Prefiere probar con los valores predefinidos?\n");
	printf("1- Si\n2- No\n");
	scanf("%d",&defecto);
	//Configurar
	if(defecto == 2){
		printf("Indique el numero de señales SIGUSR1\n");
		scanf("%d",&senial1);
		printf("Indique el numero de señales SIGUSR2\n");
		scanf("%d",&senial2);
		printf("Desea altenarlas?\n");
		printf("1- Si\n2- No\n");
		scanf("%d",&alternar);
		printf("Introduzca el numero de segundos de espera entre cada señal\n");
		scanf("%d",&espera);
		printf("Configuracion avanzada\n");
		printf("1- Si\n2- No\n");
		scanf("%d",&avanzado);
		if(avanzado == 2){
			printf("Ejecutando...\n");
			ejecutar(senial1,senial2,alternar,pid,espera);
		}else{
			printf("Configuracion avanzada...\n");
			printf("Desea repetir señales?\n");
			printf("1- Si\n2- No\n");
			scanf("%d",&repetir);
			if(repetir == 1){
				printf("Introduzca el patron de repeticion o introduzca 0 0  para patron aleatorio\n");
				printf("Ejemplo:\n4\n1\n4 SIGUSR1,1 SIGUSR2\n");
				scanf("%d",&patron1);
				scanf("%d",&patron2);
				if(patron1 == 0 && patron2 == 0){
					srand(time(NULL));
					patron1 = rand() % (senial1-1+1) + 1;
					patron2 = rand() % (senial2-1+1) + 1;
					printf("Patron aleatorio %d %d\n",patron1,patron2);
				}
			}
			ejecutarAvanzado(senial1,senial2,alternar,espera,pid,repetir,patron1,patron2);
		}
	//Por defecto
	}else{
		printf("Valores por defecto,10,7,1,1\n");
		ejecutar(10,7,1,pid,1);
	}
}

void ejecutar(int senial1,int senial2,int alternar,int pid, int espera){
	int i = 0;
	int max = senial1+senial2, logs = 0;
	char cmd1[100];
	char cmd2[100];
	sprintf(cmd1,"kill -10 %d",pid);
	sprintf(cmd2,"kill -12 %d",pid);
	int valor1 = 0,valor2 = 0,contador1 = 0,contador2 = 0;
	//No Alterna señales
	if(alternar == 1){
		//Repito ambos hasta que llegue al numero de señales indicado, mando las 2 señales a la vez en cada bucle
		for(i = 0;i < max;i++){
			if(i < senial1){
				valor1 = system(cmd1);
			}
			if(i < senial2){
				valor2 = system(cmd2);
			}
			sleep(espera);
		}
	//Alterna señales
	}else{
		for(i = 0;i < max;i++){
			if(i % 2 == 0 && contador1 < senial1){
				valor1 = system(cmd1);
				contador1++;
			}else{
				if(contador2 < senial2){
					valor2 = system(cmd2);
					contador2++;
				}
			}
			sleep(espera);
		}
	}
	//Fin del programa
	sprintf(cmd1,"kill -2 %d",pid);
	valor1 = system(cmd1);
	//Logs
	printf("Desea abrir los logs?\n");
	printf("1- Si\n2- No\n");
	scanf("%d",&logs);
	if(logs == 1){
		sprintf(cmd1,"gedit registroTiempos.log &");
		valor1 = system(cmd1);
	}
}

void ejecutarAvanzado(int senial1,int senial2,int alternar,int espera,int pid,int repetido,int patron1,int patron2){
int i = 0;
	int max = senial1+senial2, logs = 0;
	char cmd1[100];
	char cmd2[100];
	sprintf(cmd1,"kill -10 %d",pid);
	sprintf(cmd2,"kill -12 %d",pid);
	int valor1 = 0,valor2 = 0, contador1 = 0,contador2 = 0;
	int repetido1 = 0, repetido2 = 0;
	//Patron
	if(alternar == 1){
		if(repetido == 1){
			while(contador1+contador2 < max){
			//Repito el bucle de SIGUSR1
			while(contador1 < senial1 && repetido1 < patron1){
				valor1 = system(cmd1);
				repetido1++;
				contador1++;
			}
			//Ya he terminado el bucle o no me quedan señales de SIGUSR1
			if((repetido1 == patron1) || (contador1 == senial1)){
				repetido2 = 0;
			}
			////Repito el bucle de SIGUSR2
			while(contador2 < senial2 && repetido2 < patron2){
				valor2 = system(cmd2);
				repetido2++;
				contador2++;
			}
			//Ya he terminado el bucle o no me quedan señales de SIGUSR2
			if((repetido2 == patron2) || (contador2 == senial2)){
				repetido1 = 0;
			}
			sleep(espera);
		}
		}else{
			for(i = 0;i < max;i++){
				if(i < senial1){
					valor1 = system(cmd1);
				}
				if(i < senial2){
					valor2 = system(cmd2);
				}
				sleep(espera);
			}
		}
		
	//Alterna señales
	}else{
		for(i = 0;i < max;i++){
			if(i % 2 == 0 && contador1 < senial1){
				valor1 = system(cmd1);
			}else{
				if(contador2 < senial2){
					valor2 = system(cmd2);
				}
			}
			sleep(espera);
		}
	}
	//Fin del programa
	sprintf(cmd1,"kill -2 %d",pid);
	valor1 = system(cmd1);
	//Logs
	printf("Desea abrir los logs?\n");
	printf("1- Si\n2- No\n");
	scanf("%d",&logs);
	if(logs == 1){
		sprintf(cmd1,"gedit registroTiempos.log &");
		valor1 = system(cmd1);
	}
}

void fin(int sig){
		printf("\nMatando a los procesos\n");
		int valor1 = 0, logs = 0;
		char cmd1[100];
		sprintf(cmd1,"kill -2 %d",pid);
		valor1 = system(cmd1);
		printf("Desea abrir los logs?\n");
		printf("1- Si\n2- No\n");
		scanf("%d",&logs);
		if(logs == 1){
			sprintf(cmd1,"gedit registroTiempos.log &");
			valor1 = system(cmd1);
		}
		exit(0);
}
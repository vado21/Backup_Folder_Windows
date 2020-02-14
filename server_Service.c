#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <malloc.h>
#include <sys/types.h>
#include <winsock2.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <sys/types.h>
///Server
#include <stdio.h>
#include <windows.h>
#include<winsock2.h>
#include <conio.h>
#include <dir.h>
#include <process.h>
#include <errno.h>

long estado=0;	//Indicador para rastrear la actividad del servicio
				// (no muy importante en este ejemplo)
int fh_dia,fh_mes,fh_anio,fh_hora,fh_minuto,fh_segundo;
FILE *arch;

SERVICE_STATUS_HANDLE interfaz_al_SCM;
SERVICE_STATUS serviceStatus;
SERVICE_TABLE_ENTRY tabla;
char nombreServicio[]="ServiceP15";

HANDLE elThread;
DWORD elThreadID;

VOID WINAPI ServiceMain(DWORD dwArgc,LPTSTR *lpszArgv);
VOID WINAPI mainHandle(DWORD fdwControl);

///Variables del Servicio Servidor
char dirname[] = "C:\\Users\\eduar_000\\Documents\\Respaldo_server\\";
char SendBuff[512],RecvBuff[1000];
char username[100];
WSADATA wsaData;
SOCKET conn_socket,comm_socket;
SOCKET comunicacion;
struct sockaddr_in server;
struct sockaddr_in client;
struct hostent *hp;
int resp,stsize;

///Funciones del Servicio Servidor
void gotoxy(int x,int y){
 printf("%c[%d;%df",0x1B,y,x);
 }

void creardirectorio(){

    int check;
    ///se le agrega al directorio el nombre del folder
    strcat(dirname,username);
    printf("Este es el directorio %s\n",dirname);
    check = mkdir(dirname);
    strcat(dirname,"\\");
    // check if directory is created or not
    if (!check)
        printf("Directory created\n");
    else {
        printf("Unable to create directory\n");
    }
}

void servidor(){

  //Inicializamos la DLL de sockets
  resp=WSAStartup(MAKEWORD(1,0),&wsaData);
  if(resp){
    printf("Error al inicializar socket\n");
    getchar();return resp;
  }

  //Obtenemos la IP que usará nuestro servidor...
  // en este caso localhost indica nuestra propia máquina...
  hp=(struct hostent *)gethostbyname("10.22.180.21");

  if(!hp){
    printf("No se ha encontrado servidor...\n");
    getchar();WSACleanup();return WSAGetLastError();
  }

  // Creamos el socket...
  conn_socket=socket(AF_INET,SOCK_STREAM, 0);
  if(conn_socket==INVALID_SOCKET) {
    printf("Error al crear socket\n");
    getchar();WSACleanup();return WSAGetLastError();
  }

  memset(&server, 0, sizeof(server)) ;
  memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
  server.sin_family = hp->h_addrtype;
  server.sin_port = htons(6000);

  // Asociamos ip y puerto al socket
  resp=bind(conn_socket, (struct sockaddr *)&server, sizeof(server));
  ////
  if(resp==SOCKET_ERROR){
    printf("Error al asociar puerto e ip al socket\n");
    closesocket(conn_socket);WSACleanup();
    getchar();return WSAGetLastError();
  }

  if(listen(conn_socket, 1)==SOCKET_ERROR){
    printf("Error al habilitar conexiones entrantes\n");
    closesocket(conn_socket);WSACleanup();
    getchar();return WSAGetLastError();
  }

  // Aceptamos conexiones entrantes
  printf("Esperando conexiones entrantes... \n");
  stsize=sizeof(struct sockaddr);
  comm_socket=accept(conn_socket,(struct sockaddr *)&client,&stsize);
    if(comm_socket==INVALID_SOCKET){
        printf("Error al aceptar conexión entrante\n");
        closesocket(conn_socket);WSACleanup();
        getchar();return WSAGetLastError();
    }
    printf("Conexión entrante desde: %s\n", inet_ntoa(client.sin_addr));
    ///closesocket(conn_socket);
    ///Recibir el nombre de usuario
    ///Aqui se para el servidor

    recv(comm_socket, RecvBuff, sizeof(RecvBuff), 0);
    ///username
    strcpy(username, RecvBuff);
    ///Crea el directorio

    creardirectorio();

    printf("recibi %s \n",username);
        while(1){
        ///Esperando a recibir el servidor
        recv(comm_socket, RecvBuff, sizeof(RecvBuff), 0);
        ///Crear un nuevo archivo o Editar un archivo
        ///printf("%s\n",RecvBuff);
        if(strcmp(RecvBuff,"crear") == 0){
            printf("Tipo 0\n");
            ///Nombre del archivo
            recv (comm_socket, RecvBuff, sizeof(RecvBuff), 0);
            ///printf("Nombre del archivo que recibi es: %s\n",RecvBuff);
            char filepath[1000];
            strcpy(filepath,dirname);
            strcat(filepath,RecvBuff);
            int a = remove(filepath);
            int bytesReceived = 0;
            char recvBuff[1024];
            FILE *fp;
            fp = fopen(filepath, "wb");
            if(NULL == fp)
            {
            printf("Error opening file");
            return 1;
            }
            long double sz=1;
            /* Receive data in chunks of 256 bytes */
            while(bytesReceived = recv(comm_socket, recvBuff, 1024,0),bytesReceived > 0){
                if(strcmp(recvBuff,"parar") == 0 ){
                    break;
                }
                sz++;
                ///gotoxy(0,4);
                ///printf("Received: %llf Mb",(sz/1024));
                //fflush(stdout);
                // recvBuff[n] = 0;
                ///printf("Recibi %s ",recvBuff);
                fwrite(recvBuff, 1,bytesReceived,fp);
                // printf("%s \n", recvBuff);
            }
            printf("termino\n");

            /*
            FILE *fp;
            fp = fopen(filepath, "wb" );
            ///Texto del archivo
            char contenido[100000];
            contenido[0] = '\0';
            recv(comm_socket, contenido, sizeof(contenido), 0);
            contenido[strlen(contenido)] = '\0';
            fwrite(contenido ,sizeof(char), strlen(contenido), fp );
            printf("contenido %s \n",contenido);
            fclose(fp);
            ///sendfile(comm_socket, fp);
            ///fwrite(RecvBuff, 1 ,sizeof(RecvBuff), fichero);
            */
            fclose(fp);
        }
        else if(strcmp(RecvBuff,"borrar") == 0){
            printf("Tipo 1\n");
            recv (comm_socket, RecvBuff, sizeof(RecvBuff), 0);
            printf("Nombre del archivo que recibi es: %s\n",RecvBuff);
            char filepath[1000];
            strcpy(filepath,dirname);
            strcat(filepath,RecvBuff);
            ///funcion remueve el archivo
            int a = remove(filepath);
        }

    }



    /*
  strcpy(SendBuff,"Hola Cliente... .P");

  printf("Enviando Mensaje... \n");
  send (comm_socket, SendBuff, sizeof(SendBuff), 0);
  printf("Datos enviados: %s \n", SendBuff);

  printf("Recibiendo Mensaje... \n");
  recv (comm_socket, RecvBuff, sizeof(RecvBuff), 0);
  printf("Datos recibidos: %s \n", RecvBuff);

  getchar();*/

  // Cerramos el socket de la comunicacion

  // Cerramos liberia winsock
  //WSACleanup();

}

void main(void) {
    tabla.lpServiceName=nombreServicio;
	tabla.lpServiceProc=&ServiceMain;
	StartServiceCtrlDispatcher(&tabla);
}

void setServiceStatusState(unsigned int state) {
	serviceStatus.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState=state;
	serviceStatus.dwControlsAccepted=SERVICE_CONTROL_INTERROGATE
										|SERVICE_ACCEPT_STOP
										|SERVICE_ACCEPT_PAUSE_CONTINUE
										|SERVICE_ACCEPT_SHUTDOWN;
	serviceStatus.dwWin32ExitCode=NO_ERROR;
	serviceStatus.dwServiceSpecificExitCode=0;
	serviceStatus.dwCheckPoint=estado++;
	serviceStatus.dwWaitHint=100;
}

DWORD WINAPI corre(LPVOID lpParameter) {
	setServiceStatusState(SERVICE_RUNNING);	//Avisar que YA ESTAMOS CORRIENDO!!!
	SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios

	while(TRUE) {
        servidor();
        ///Crear directorio


        return (EXIT_SUCCESS);
	}
}

VOID WINAPI mainHandle(DWORD fdwControl) {
	switch(fdwControl) {
		case SERVICE_CONTROL_STOP:
					setServiceStatusState(SERVICE_STOP_PENDING);
					SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios

					if (TerminateThread(elThread,0)) {
						setServiceStatusState(SERVICE_STOPPED);
						SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios
					}
					break;
		case SERVICE_CONTROL_PAUSE:
					setServiceStatusState(SERVICE_PAUSE_PENDING);	//Avisar que YA ESTAMOS CORRIENDO!!!
					SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios

					SuspendThread(elThread);

					setServiceStatusState(SERVICE_PAUSED);
					SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios
					break;
		case SERVICE_CONTROL_CONTINUE:
					setServiceStatusState(SERVICE_CONTINUE_PENDING);
					SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios

					ResumeThread(elThread);

					setServiceStatusState(SERVICE_RUNNING);	//Avisar que YA ESTAMOS CORRIENDO!!!
					SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios
					break;
		default:
					setServiceStatusState(SERVICE_RUNNING);
					SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios
					break;
	}
}

VOID WINAPI ServiceMain(DWORD dwArgc,LPTSTR *lpszArgv) {
	interfaz_al_SCM=RegisterServiceCtrlHandler(nombreServicio,mainHandle);

	setServiceStatusState(SERVICE_START_PENDING);
	SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Avisar que estamos "alive and kicking!"

	elThread=CreateThread(NULL,0,corre,NULL,0,&elThreadID);
}

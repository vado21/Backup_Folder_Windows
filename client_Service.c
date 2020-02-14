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
///Cliente
///1er paso poner ip del servidor
///Tu nombre de usuario
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <pthread.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

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

///Variables del Servicio Cliente
///Buffers de envio
char SendBuff[1000],RecvBuff[1000];
///Paths de Archivos
char filesPath[50][500];
char filesName[50][500];
int size;
char Fuente[50];
SOCKET conn_socket;
///Variables de TIEMPO
FILETIME lastWriteTime[50];
FILETIME t1[50];
FILETIME t2[50];
FILETIME t3[50];
///Variables de servidor
WSADATA wsaData;
struct sockaddr_in server;
struct hostent *hp;
int resp;

///Funciones del Servicio Cliente
void enviarArchivo(int i){

    SendBuff[0] = '\0';
    strcpy(SendBuff,"crear");
    printf("envio %s",SendBuff);
    send(conn_socket,SendBuff,sizeof(SendBuff),0);

    ///Enviar Nombre del Archivo
    strcpy(SendBuff,filesName[i]);
    printf("El nombre del archivo que envio es: %s\n",SendBuff);
    send(conn_socket,SendBuff,sizeof(SendBuff),0);

    char sendbuffer[100000];
    sendbuffer[0] = '\0';
    FILE *fp = fopen(filesPath[i], "rb");

    ///Enviar contenido del archivo
    ///int connfd=(int)*arg;
    ///write(conn_socket, filesName[i],256);
        while(1){
            /* First read file in chunks of 256 bytes */
            unsigned char buff[1024]={0};
            int nread = fread(buff,1,1024,fp);
            //printf("Bytes read %d \n", nread);

            /* If read was success, send data. */
            if(nread > 0)
            {
                //printf("Sending \n");
                ///printf("Le envio %s \n",buff);
                send(conn_socket, buff, sizeof(buff),0);
            }
            if (nread < 1024)
            {
                if (feof(fp)){
                    printf("End of file\n");
                    send(conn_socket,"parar",1024,0);
                }
                if (ferror(fp)){
                    printf("Error reading\n");
                }
                break;
            }
        }


        printf("Termine\n");
    ///int b = fread(sendbuffer, 1, sizeof(sendbuffer), fp);
    /*
    printf("Enviar contenido del archivo\n");
    char linea[1024];

    while(fgets(linea, 1024, (FILE*) fp)){
        strcat(sendbuffer,linea);
        strcat(sendbuffer,"\n");
    }

    send(conn_socket, sendbuffer, sizeof(sendbuffer), 0);*/

    fclose(fp);
   /// printf("se envio %s ",sendbuffer);
    printf("Ya termino\n");
}

int encuentraArchivo(char nuevoArchivo[500]){
    int N = size;
    int i;
    for(i = 0; i<N; i++){
        if(strcmp (filesName[i], nuevoArchivo) == 0){
            return 0;
        }
    }
    return 1;
}

void borrarArchivo(int pos){
        int i;
        printf("Se borro el archivo %s \n",filesPath[pos]);
        printf("Posicion: %d \n",pos);
        ///Enviar archivo en el servidor
        strcpy(SendBuff,"borrar");
        send(conn_socket,SendBuff,sizeof(SendBuff),0);
        strcpy(SendBuff,filesName[pos]);
        send(conn_socket,SendBuff,sizeof(SendBuff),0);
        for(i=pos; i<size-1; i++)
        {
            ///arr[i] = arr[i + 1];
            strcpy(filesPath[i], filesPath[i+1]);
            strcpy(filesName[i], filesName[i+1]);
            lastWriteTime[i] = lastWriteTime[i+1];
            t1[i] = t1[i+1];
            t2[i] = t2[i+1];
            t3[i] = t3[i+1];
        }
        size--;
        printf("Los archivos que quedan son:\n");
        int j;
        for(j = 0; j<size;j++){
            printf("%s\n",filesPath[j]);
        }
        printf("Listo\n");
}

void *monitorearfolder(){
    ///Declaracion de variables
	HANDLE changeNotifHandle;
	HANDLE fileHandle;
	char chr = 0;
	DWORD waitResult;
	int flag=0;
	DWORD lastError;
	int borro;
	int N = size;
	int i;
	int iPos;
	///Empieza el monitoreo
	while (chr!=27) {
		if (!flag){
			changeNotifHandle=FindFirstChangeNotification(Fuente,FALSE,FILE_NOTIFY_CHANGE_FILE_NAME);
        }
		else
			FindNextChangeNotification(changeNotifHandle);
		waitResult = WaitForSingleObject(changeNotifHandle,500);
		flag=1;
		switch(waitResult) {
			case WAIT_OBJECT_0:
                    borro = 0;
					Sleep(50);
					N = size;
					///Detectar si un archivo se borro
                    for(i = 0; i<N;i++){
                        fileHandle=CreateFile(filesPath[i],GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
                        if(fileHandle != INVALID_HANDLE_VALUE) {
                        }
                        else{
                            lastError=GetLastError();
                            if (lastError == ERROR_SHARING_VIOLATION)
                                printf("El archivo cambio!\n");
                            else if (lastError == ERROR_FILE_NOT_FOUND) {
                                printf("Borraron el achivo ... que hacemos?\n");
                                borrarArchivo(i);
                                ///Enviar borrar
                                CloseHandle(fileHandle);
                                borro = 1;
                                break;
                            }
                            else{
                                printf("INVALID_HANDLE_VALUE! ... %ld\n",lastError);
                            }
                        }
                        CloseHandle(fileHandle);
                    }
                    if(borro == 1){
                        break;
					}

					///Se creo un nuevo archivo

					printf("Se creo nuevo archivo\n");
                    struct dirent *de;
                    DIR *dr = opendir(Fuente);
                    while ((de = readdir(dr)) != NULL){
                        if(strcmp (de->d_name, ".") && strcmp (de->d_name, "..")){
                            if(encuentraArchivo(de->d_name) == 1){
                                iPos = size;
                                strcpy(filesPath[iPos], Fuente);
                                strcpy(filesName[iPos], de->d_name);
                                strcat(filesPath[iPos],"\\");
                                strcat(filesPath[iPos], de->d_name);
                                size = size + 1;
                                enviarArchivo(iPos);
                                ///Inicializar tiempos
                                lastWriteTime[iPos].dwHighDateTime = 0;
                                lastWriteTime[iPos].dwLowDateTime = 0;
                                fileHandle=CreateFile(filesPath[iPos],GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
                                if(GetFileTime(fileHandle,&t1[iPos],&t2[iPos],&t3[iPos]))
                                    memcpy(&lastWriteTime[iPos],&t3[iPos],sizeof(FILETIME));

                                CloseHandle(fileHandle);
                                break;
                            }
                        }
                    }
                    closedir(dr);
                    CloseHandle(fileHandle);
                    N = size;
                    for(i = 0; i<N;i++){
                        printf("%s\n",filesPath[i]);
                    }
					break;
			case WAIT_TIMEOUT:
					printf(".");
					break;
			default:
					printf("ERROR!!!\n");
					break;
		}
		if (kbhit())
			chr=getch();
	}
	FindCloseChangeNotification(changeNotifHandle);
	printf("Listo\n");
    pthread_exit(NULL);
}

void *monitoreararchivos(){

	HANDLE changeNotifHandle;
	HANDLE fileHandle;
	char chr = 0;
	DWORD waitResult;
	int flag=0;
	/*FILETIME lastWriteTime,t1,t2,t3;*/
	DWORD lastError;
    int i;
    for(i = 0; i<size;i++){
        lastWriteTime[i].dwHighDateTime = 0;
        lastWriteTime[i].dwLowDateTime = 0;
    }

    ///Inicializar tiempos de archivos
    for(i = 0; i<size;i++){
        fileHandle=CreateFile(filesPath[i],GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
        //Si el
        if (fileHandle!=INVALID_HANDLE_VALUE) {
            if(GetFileTime(fileHandle,&t1[i],&t2[i],&t3[i]))
                memcpy(&lastWriteTime[i],&t3[i],sizeof(FILETIME));
            CloseHandle(fileHandle);
        }
        else {
            lastError=GetLastError();
            if (lastError==ERROR_FILE_NOT_FOUND){
            }
        }
    }
	while (chr!=27) {

		if (!flag)
			changeNotifHandle=FindFirstChangeNotification(Fuente,FALSE,FILE_NOTIFY_CHANGE_LAST_WRITE);
		else
			FindNextChangeNotification(changeNotifHandle);
		waitResult = WaitForSingleObject(changeNotifHandle,500);
		flag=1;
		switch(waitResult) {
			case WAIT_OBJECT_0:
					printf("Cambio algun archivo\n");
					Sleep(50);

                    for(i = 0; i<size;i++){
					fileHandle=CreateFile(filesPath[i],GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
					if (fileHandle!=INVALID_HANDLE_VALUE) {
						if(GetFileTime(fileHandle,&t1[i],&t2[i],&t3[i])) {
							if ((t3[i].dwHighDateTime!=lastWriteTime[i].dwHighDateTime)||(t3[i].dwLowDateTime!=lastWriteTime[i].dwLowDateTime)) {
								printf("El archivo cambio\n");
								printf("El archivo que cambio fue %s\n",filesPath[i]);
								memcpy(&lastWriteTime[i],&t3[i],sizeof(FILETIME));
								///Este archivo
                                enviarArchivo(i);
								CloseHandle(fileHandle);
								break;
							} else {
								printf("El archivo NO cambio\n");
							}
						}
						CloseHandle(fileHandle);
					}
                    }
					break;
			case WAIT_TIMEOUT:
					printf(".");
					break;
			default:
					printf("ERROR!!!\n");
					break;
		}
		if (kbhit())
			chr=getch();
	}
	FindCloseChangeNotification(changeNotifHandle);
	printf("Listo\n");

    pthread_exit(NULL);
}

void inicializarServidor(){
    resp=WSAStartup(MAKEWORD(1,0),&wsaData);
    if(resp){
        printf("Error al inicializar socket\n");
        getchar();return -1;
    }
    hp=(struct hostent *)gethostbyname("10.22.180.21");

    if(!hp){
        printf("No se ha encontrado servidor...\n");
        getchar();WSACleanup();return WSAGetLastError();
    }
    conn_socket=socket(AF_INET,SOCK_STREAM, 0);
    if(conn_socket==INVALID_SOCKET) {
        printf("Error al crear socket\n");
        getchar();WSACleanup();return WSAGetLastError();
    }
    memset(&server, 0, sizeof(server)) ;
    memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
    server.sin_family = hp->h_addrtype;
    server.sin_port = htons(6000);

    if(connect(conn_socket,(struct sockaddr *)&server,sizeof(server))==SOCKET_ERROR){
        printf("Fallo al conectarse con el servidor\n");
        closesocket(conn_socket);
        WSACleanup();getchar();return WSAGetLastError();
    }

    printf("Conexión establecida con: %s\n", inet_ntoa(server.sin_addr));


    ///Nombre de usuario
    strcpy(SendBuff,"David");
    send(conn_socket,SendBuff,sizeof(SendBuff),0);

    int i;
    char iType = '0';

    for(i = 0; i<size;i++){
        enviarArchivo(i);
    }
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
        ///Inicializar los archivos
        ///struct files f;
        size = 0;
        ///Direccion del Folder
        strcpy(Fuente, "C:\\Users\\eduar_000\\Desktop\\Respaldo");
        printf("%s \n",Fuente);

        struct dirent *de;
        DIR *dr = opendir(Fuente);
        if (dr == NULL)
        {
            printf("Could not open current directory");
            return;
        }
        while ((de = readdir(dr)) != NULL){
            if(strcmp (de->d_name, ".") && strcmp (de->d_name, "..")){
                int iPos = size;
                strcpy(filesPath[iPos], Fuente);
                strcpy(filesName[iPos], de->d_name);
                strcat(filesPath[iPos],"\\");
                strcat(filesPath[iPos], de->d_name);
                size = size + 1;
            }
        }
        closedir(dr);
        int N = size;
        int i;
        printf("Empieza\n");
        for(i = 0; i<N;i++){
            printf("%s\n",filesPath[i]);
        }

        ///Llamada al servidor por primera vez
        inicializarServidor();

        ///Copiar archivos en otro folder

        ///Llamada de threads

        pthread_t threads[2];
        int rc = 0;
        rc = pthread_create(threads + 0, NULL, monitorearfolder,NULL);
        rc = pthread_create(threads + 1, NULL, monitoreararchivos,NULL);
        pthread_exit(NULL);
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

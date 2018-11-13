#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

char texto[100];
int contador=0,k;
FILE *archivo;

//Threads
pthread_t w[200],r[200];

//Semáforos
sem_t mutex, queue;
sem_t lector, escritor;

/******************************************************************************/

void read(){
  sem_wait(&queue);
    sem_wait(&lector);
      sem_wait(&mutex);
    sem_post(&lector);
        //ZONA CRITICA
      archivo = fopen("entrada.txt", "a");
      printf("\n******************************\nLECTOR");
      printf("\nLeyendo...");
      for(k=0;k<100;k++){
        fputs("LECTOR.\n",archivo);
      }
      fclose(archivo);
      //FINAL ZONA CRITICA
    sem_post(&mutex);
  sem_post(&queue);
}

/******************************************************************************/

void write(){
  sem_wait(&escritor);
    contador++;
    if(contador==1){
      sem_wait(&lector);
    }
    sem_wait(&mutex);
      //ZONA CRITICA
      printf("\n******************************\nESCRITOR");
      archivo=fopen("entrada.txt","a");
      printf("\nEscribiendo...");
      for(k=0;k<10;k++){
        fputs("ESCRITOR.\n",archivo);
      }
      fclose(archivo);
      //FINAL ZONA CRITICA
    sem_post(&mutex);
  sem_post(&escritor);

  sem_wait(&escritor);
    contador--;
    if(contador==0){
      sem_post(&lector);
    }
  sem_post(&escritor);
}

/******************************************************************************/

int proceso(int ESC,int LEC){
  //Se inicializan los semáforos en 1
  sem_init(&mutex,0,1);
  sem_init(&queue,0,1);
  sem_init(&lector,0,1);
  sem_init(&escritor,0,1);
  int a,b;

  //Se crean los lectores
  for(a=0;a<=LEC;a++){
    pthread_create(&r[a],NULL,(void *)read,NULL);
  }

  //Luego los escritores
  for(b=0;b<=ESC;b++){
    pthread_create(&w[b],NULL,(void *)write,NULL);
  }

  //Espera lectores
  for(a=0;a<=LEC;a++){
    pthread_join(r[a],NULL);
  }

  //Espera escritores
  for(b=0;b<=ESC;b++){
    pthread_join(w[b],NULL);
  }

  printf("\n");
  return 0;
}

/******************************************************************************/

int main(){
  char caja[100];
  int writer,reader;

  //LECTOR
  printf("Ingrese la cantidad de lectores:\n");
  fgets(caja,sizeof(texto),stdin);
  sscanf(caja,"%d",&reader);

  while(reader==0 || reader>100){
    printf("Ingrese un numero valido menor a 100\n");
    fgets(caja,sizeof(texto),stdin);
    sscanf(caja,"%d",&reader);
  }
  //END LECTOR

  //ESCRITOR
  printf("Ingrese la cantidad de escritores:\n");
  fgets(caja,sizeof(texto),stdin);
  sscanf(caja,"%d",&writer);

  while(writer==0 || writer>100){
    printf("Ingrese un numero valido menor a 100\n");
    fgets(caja,sizeof(texto),stdin);
    sscanf(caja,"%d",&writer);
  }
  //END ESCRITOR

  proceso(writer-1,reader-1);

  return(0);
}
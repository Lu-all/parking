#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>

int num_plazas;
int num_coches;
pthread_mutex_t mutex;
pthread_cond_t plazas_libres_cond;
int num_plazas_libres;
int * plazas;
pthread_t* coches;


void *coche (void *args)
{
    while(1)
    {
        int id_coche = *(int *) args;
        sleep(rand() % 5 + 1);
        pthread_mutex_lock(&mutex); //Entra
        while(num_plazas_libres == 0){
            pthread_cond_wait(&plazas_libres_cond, &mutex);  //Si no hay plazas libres
        }
        // Buscamos la plaza libre
        int i = 0;
        int plaza_ocupada = -1;
        while (i <num_plazas && plaza_ocupada==-1){ //Mientras que no hayamos encontrado la plaza libre
            if(plazas[i] == 0){  // Hemos encontrado la plaza
                plaza_ocupada = i;  // Marcamos la plaza que ocuparemos
                plazas[i] = id_coche;  // Ocupamos la plaza
                --num_plazas_libres;  //Tenemos una plaza libre menos
                printf("ENTRADA: Coche %d aparca en %d. Plazas libre: %d\n", id_coche, plaza_ocupada, num_plazas_libres);
                for (int j=0; j<num_plazas; j++) {  // Estado actual del parking
                    if(j==0){
                        printf("Parking: ");
                    }
                    printf("[%d] ", plazas[j]);
                    if(j==num_plazas-1){
                        printf("\n");
                    }
                }
            }
            else i++;  //Si no hemos encontrado la plaza libre, seguimos buscando
        }
        pthread_mutex_unlock(&mutex);  // Ya hemos aparcado
        sleep(rand() % 5 + 1);
        pthread_mutex_lock(&mutex);  // Sale
        num_plazas_libres++;  //Tenemos una plaza libre mas
        plazas[plaza_ocupada]=0;  // La plaza que ocupabamos la marcamos como libre
        printf("SALIDA: Coche %d saliendo. Plazas libre: %d\n", id_coche, num_plazas_libres);
        for (int j=0; j<num_plazas; j++) {  // Estado actual del parking
            if(j==0){
                printf("Parking: ");
            }
            printf("[%d] ", plazas[j]);
            if(j==num_plazas-1){
                printf("\n");
            }
        }
        pthread_cond_signal(&plazas_libres_cond);  // Ya no esta lleno el parking
        pthread_mutex_unlock(&mutex);  // Ya hemos salido
    }
}

int main(int argc, char const *argv[]) {
    num_coches = 10;
    num_plazas = 5;
    plazas = (int*) malloc(sizeof(int) * num_plazas);
    coches = (pthread_t *) malloc(sizeof(pthread_t) * num_coches);
    int id_coches[num_coches];
    for(int k = 0; k<num_coches; k++){
        id_coches[k] = k+1;
    }
    num_plazas_libres = num_plazas;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&plazas_libres_cond, NULL);
    for(int i = 0; i<num_plazas; i++){
        plazas[i] = 0;
    }
    for (int i = 0; i<num_coches; i++){
        pthread_create(&coches[i], NULL, coche, (void *)&id_coches[i]);
    }
    for (int i = 0; i<num_coches; i++){
        pthread_join(coches[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&plazas_libres_cond);
    return 0;
}

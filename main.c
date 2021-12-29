#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>

int num_plazas;
pthread_mutex_t mutex;
pthread_cond_t plazas_libres_cond;
int num_plazas_libres;
int *plazas;
pthread_t *coches;
pthread_t *camiones;

void *coche(void *args) {
    while (1) {
        int id_coche = *(int *) args;
        sleep(rand() % 5 + 1);
        pthread_mutex_lock(&mutex); //Entra
        while (num_plazas_libres == 0) {
            pthread_cond_wait(&plazas_libres_cond, &mutex);  //Si no hay plazas libres
        }
        // Buscamos la plaza libre
        int i = 0;
        int plaza_ocupada = -1;
        while (i < num_plazas && plaza_ocupada == -1) { //Mientras que no hayamos encontrado la plaza libre
            if (plazas[i] == 0) {  // Hemos encontrado la plaza
                plaza_ocupada = i;  // Marcamos la plaza que ocuparemos
                plazas[i] = id_coche;  // Ocupamos la plaza
                --num_plazas_libres;  //Tenemos una plaza libre menos
                printf("ENTRADA: Coche %d aparca en %d. Plazas libre: %d\n", id_coche, plaza_ocupada,
                       num_plazas_libres);
                for (int j = 0; j < num_plazas; j++) {  // Estado actual del parking
                    if (j == 0) {
                        printf("Parking: ");
                    }
                    printf("[%d] ", plazas[j]);
                    if (j == num_plazas - 1) {
                        printf("\n");
                    }
                }
            } else i++;  //Si no hemos encontrado la plaza libre, seguimos buscando
        }
        pthread_mutex_unlock(&mutex);  // Ya hemos aparcado
        sleep(rand() % 5 + 1);
        pthread_mutex_lock(&mutex);  // Sale
        num_plazas_libres++;  //Tenemos una plaza libre mas
        plazas[plaza_ocupada] = 0;  // La plaza que ocupabamos la marcamos como libre
        printf("SALIDA: Coche %d saliendo. Plazas libre: %d\n", id_coche, num_plazas_libres);
        for (int j = 0; j < num_plazas; j++) {  // Estado actual del parking
            if (j == 0) {
                printf("Parking: ");
            }
            printf("[%d] ", plazas[j]);
            if (j == num_plazas - 1) {
                printf("\n");
            }
        }
        pthread_cond_signal(&plazas_libres_cond);  // Ya no esta lleno el parking
        pthread_mutex_unlock(&mutex);  // Ya hemos salido
    }
}

void *camion(void *args) {
    while (1) {
        int id_camion = *(int *) args;
        sleep(rand() % 5 + 1);
        pthread_mutex_lock(&mutex); //Entra
        while (num_plazas_libres < 2) {
            pthread_cond_wait(&plazas_libres_cond, &mutex);  //Si no entra un camion
        }
        // Buscamos las plazas libre
        int i = 0;
        int plaza_ocupada = -1;
        while (i < num_plazas && plaza_ocupada == -1) { //Mientras que no hayamos encontrado la plaza libre
            if (i != num_plazas -1 && plazas[i] == 0 && plazas[i+1] == 0) {  // Hemos encontrado la plaza
                plaza_ocupada = i;  // Marcamos la plaza que ocuparemos
                plazas[i] = id_camion;  // Ocupamos las plazas
                plazas[i+1] = id_camion;
                num_plazas_libres = num_plazas_libres - 2;  //Tenemos dos plazas libres menos
                printf("ENTRADA: Camion %d aparca en %d. Plazas libre: %d\n", id_camion, plaza_ocupada,
                       num_plazas_libres);
                for (int j = 0; j < num_plazas; j++) {  // Estado actual del parking
                    if (j == 0) {
                        printf("Parking: ");
                    }
                    printf("[%d] ", plazas[j]);
                    if (j == num_plazas - 1) {
                        printf("\n");
                    }
                }
            } else i++;  //Si no hemos encontrado la plaza libre, seguimos buscando
        }
        pthread_mutex_unlock(&mutex);  // Ya hemos aparcado
        sleep(rand() % 5 + 1);
        pthread_mutex_lock(&mutex);  // Sale
        num_plazas_libres = num_plazas_libres + 2;  //Tenemos una plaza libre mas
        plazas[plaza_ocupada] = 0;  // Las plazas que ocupabamos las marcamos como libres
        plazas[plaza_ocupada + 1] = 0;
        printf("SALIDA: Camion %d saliendo. Plazas libre: %d\n", id_camion, num_plazas_libres);
        for (int j = 0; j < num_plazas; j++) {  // Estado actual del parking
            if (j == 0) {
                printf("Parking: ");
            }
            printf("[%d] ", plazas[j]);
            if (j == num_plazas - 1) {
                printf("\n");
            }
        }
        pthread_cond_signal(&plazas_libres_cond);  // Ya no esta lleno el parking
        pthread_mutex_unlock(&mutex);  // Ya hemos salido
    }
}

int main(int argc, char const *argv[]) {
    int num_coches = 10;
    int num_camiones = 2;
    num_plazas = 7;
    // Creamos las plazas
    plazas = (int *) malloc(sizeof(int) * num_plazas);
    // Creamos los coches
    coches = (pthread_t *) malloc(sizeof(pthread_t) * num_coches);
    int id_coches[num_coches];
    for (int k = 0; k < num_coches; k++) {
        id_coches[k] = k + 1;
    }

    // Creamos los camiones
    camiones = (pthread_t *) malloc(sizeof(pthread_t) * num_camiones);
    int id_camiones[num_camiones];
    for (int k = 0; k < num_camiones; k++) {
        id_camiones[k] = 100 + k + 1;
    }

    // Iniciamos mutex, condiciones y parking
    num_plazas_libres = num_plazas;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&plazas_libres_cond, NULL);
    for (int i = 0; i < num_plazas; i++) {
        plazas[i] = 0;
    }

    // Ponemos en funcionamiento coches y camiones
    for (int i = 0; i < num_coches; i++) {
        pthread_create(&coches[i], NULL, coche, (void *) &id_coches[i]);
    }
    for (int i = 0; i < num_camiones; i++) {
        pthread_create(&camiones[i], NULL, camion, (void *) &id_camiones[i]);
    }

    // Paramos coches y camiones
    for (int i = 0; i < num_coches; i++) {
        pthread_join(coches[i], NULL);
    }
    for (int i = 0; i < num_camiones; i++) {
        pthread_join(camiones[i], NULL);
    }

    // Liberamos memoria
    free(coches);
    free(camiones);
    free(plazas);

    // Destruimos mutex y condiciones
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&plazas_libres_cond);
    return 0;
}

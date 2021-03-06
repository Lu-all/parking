#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <string.h>

int num_total_plazas;
int num_plazas;
int num_plantas;
int prioridad_coches;
pthread_mutex_t mutex;
pthread_cond_t plazas_libres_cond;
pthread_cond_t camion_esperando;
int camiones_esperando;
int num_plazas_libres;
int *plazas;
pthread_t *coches;
pthread_t *camiones;

void *coche(void *args) {
    while (1) {
        int id_coche = *(int *) args;  // Recuperamos la matricula del coche
        sleep(rand() % 5 + 1);  //Estamos fuera del parking un tiempo aleatorio

        pthread_mutex_lock(&mutex); //Entra

        // Calculamos las plazas que reservamos en el caso de tener prioridad los camiones
        // De menor a mayor carga de trabajo: 2c < c < c/2 < caso critico (2/3)
        int plazas_reservadas;
        if (prioridad_coches == 1 ||
            num_total_plazas <= 2) { // Si no hay prioridad a los camiones o no se puede reservar
            plazas_reservadas = 0;
        } else if (num_total_plazas / 2 <= camiones_esperando * 2) {
            if (num_total_plazas / 2 <= camiones_esperando) {
                if (num_total_plazas / 2 <= camiones_esperando / 2) {
                    plazas_reservadas = 2 * num_total_plazas / 3; // Caso critico
                } else {
                    plazas_reservadas = camiones_esperando / 2; // Caso c/2
                }
            } else {
                plazas_reservadas = camiones_esperando;  // Caso c
            }
        } else {
            plazas_reservadas = camiones_esperando * 2;  // Caso 2c
        }

        // Vemos si tenemos plazas libres y no hay un camion con prioridad
        while (num_plazas_libres == 0 ||
               (prioridad_coches == 0 && camiones_esperando > 0 && num_plazas_libres < plazas_reservadas)) {
            if (camiones_esperando > 0) {
                pthread_cond_wait(&camion_esperando, &mutex);  //Si hay un camion esperando
            }
            pthread_cond_wait(&plazas_libres_cond, &mutex);  //Si no hay plazas libres
        }

        // Buscamos la plaza libre
        int i = 0;
        int plaza_ocupada = -1;
        while (i < num_total_plazas && plaza_ocupada == -1) { //Mientras que no hayamos encontrado la plaza libre
            if (plazas[i] == 0) {  // Hemos encontrado la plaza
                plaza_ocupada = i;  // Marcamos la plaza que ocuparemos
                plazas[i] = id_coche;  // Ocupamos la plaza
                num_plazas_libres--;  //Tenemos una plaza libre menos

                //Imprimimos
                printf("ENTRADA: Coche %d aparca en %d. Plazas libre: %d\n", id_coche, plaza_ocupada,
                       num_plazas_libres);

                for (int j = 0; j < num_total_plazas; j++) {  // Estado actual del parking
                    if (j == 0) {
                        printf("Parking: ");
                        if(num_plantas>1){
                            printf("\n");
                        }
                    }
                    printf("[%d] ", plazas[j]);
                    if (j == num_total_plazas - 1) {
                        printf("\n");
                    }
                    if (num_plantas > 1 && (j % num_plazas == num_plazas - 1)) {  // Si hay mas de una planta
                        printf("\n");
                    }
                }

            } else i++;  //Si no hemos encontrado la plaza libre, seguimos buscando
        }

        pthread_mutex_unlock(&mutex);  // Ya hemos aparcado

        sleep(rand() % 5 + 1); //Estamos aparcados un tiempo aleatorio

        pthread_mutex_lock(&mutex);  // Sale

        num_plazas_libres++;  //Tenemos una plaza libre mas
        if (plaza_ocupada != -1) {
            plazas[plaza_ocupada] = 0;  // La plaza que ocupabamos la marcamos como libre

            // Imprimimos
            printf("SALIDA: Coche %d saliendo. Plazas libre: %d\n", id_coche, num_plazas_libres);
            for (int j = 0; j < num_total_plazas; j++) {  // Estado actual del parking
                if (j == 0) {
                    printf("Parking: ");
                    if(num_plantas>1){
                        printf("\n");
                    }
                }
                printf("[%d] ", plazas[j]);
                if (j == num_total_plazas - 1) {
                    printf("\n");
                }
                if (num_plantas > 1 && (j % num_plazas == num_plazas - 1)) {  // Si hay mas de una planta
                    printf("\n");
                }
            }
        }

        pthread_cond_signal(&plazas_libres_cond);  // Ya no esta lleno el parking
        pthread_mutex_unlock(&mutex);  // Ya hemos salido
    }
}

void *camion(void *args) {
    while (1) {
        int id_camion = *(int *) args; // Recuperamos la matricula del camion

        sleep(rand() % 5 + 1);  // Esta fuera un tiempo aleatorio

        pthread_mutex_lock(&mutex); //Entra

        camiones_esperando++; // Anadimos un camion esperando

        while (num_plazas_libres < 2) {
            pthread_cond_wait(&plazas_libres_cond, &mutex);  //Si no tiene sitio
        }

        // Buscamos las plazas libres
        int c = 0;
        int plaza_ocupada = -1;
        while (c < num_total_plazas && plaza_ocupada == -1) { //Mientras que no hayamos encontrado la plaza libre

            // Hemos encontrado la plaza
            if ((c != num_total_plazas - 1 && plazas[c] == 0 && plazas[c + 1] == 0)&&(num_plantas==1 || (num_plantas>1 && c % num_plazas != num_plazas - 1))) {
                camiones_esperando--; // Ya no estamos esperando
                plaza_ocupada = c;  // Marcamos la plaza que ocuparemos
                plazas[c] = id_camion;  // Ocupamos las plazas
                plazas[c + 1] = id_camion;
                num_plazas_libres = num_plazas_libres - 2;  //Tenemos dos plazas libres menos

                // Imprimimos
                printf("ENTRADA: Camion %d aparca en %d-%d. Plazas libre: %d\n", id_camion, plaza_ocupada,
                       plaza_ocupada + 1,
                       num_plazas_libres);
                for (int j = 0; j < num_total_plazas; j++) {  // Estado actual del parking
                    if (j == 0) {
                        printf("Parking: ");
                        if(num_plantas>1){
                            printf("\n");
                        }
                    }
                    printf("[%d] ", plazas[j]);
                    if (j == num_total_plazas - 1) {
                        printf("\n");
                    }
                    if (num_plantas > 1 && (j % num_plazas == num_plazas - 1)) {  // Si hay mas de una planta
                        printf("\n");
                    }
                }
            } else c++;  //Si no hemos encontrado las plazas libres, seguimos buscando
        }

        pthread_cond_signal(&camion_esperando);  // Ya no estamos esperando

        pthread_mutex_unlock(&mutex);  // Ya hemos aparcado

        sleep(rand() % 5 + 1); // Estamos aparcados un tiempo aleatorio

        pthread_mutex_lock(&mutex);  // Sale

        if (plaza_ocupada != -1) {
            num_plazas_libres = num_plazas_libres + 2;  //Tenemos dos plazas libres mas
            plazas[plaza_ocupada] = 0;  // Las plazas que ocupabamos las marcamos como libres
            plazas[plaza_ocupada + 1] = 0;

            // Imprimimos
            printf("SALIDA: Camion %d saliendo de plazas %d-%d. Plazas libre: %d\n", id_camion, plaza_ocupada,
                   plaza_ocupada + 1, num_plazas_libres);
            for (int j = 0; j < num_total_plazas; j++) {  // Estado actual del parking
                if (j == 0) {
                    printf("Parking: ");
                    if(num_plantas>1){
                        printf("\n");
                    }
                }
                printf("[%d] ", plazas[j]);
                if (j == num_total_plazas - 1) {
                    printf("\n");
                }
                if (num_plantas > 1 && (j % num_plazas == num_plazas - 1)) {  // Si hay mas de una planta
                    printf("\n");
                }
            }

        } else camiones_esperando--;

        pthread_cond_signal(&plazas_libres_cond);  // Ya no esta lleno el parking
        pthread_mutex_unlock(&mutex);  // Ya hemos salido
    }
}

int main(int argc, char const *argv[]) {
    int num_coches;
    int num_camiones;
    int id_camion_base = 0;
    if (argc < 3) {
        printf("Pocos argumentos. Procediendo con valores por defecto.\n");
        num_coches = 40;
        num_camiones = 2;
        num_plazas = 10;
        num_plantas = 1;
        printf("Plazas: %d\n", num_plazas);
        printf("Plantas: %d\n", num_plantas);
        printf("Coches: %d\n", num_coches);
        printf("Camiones: %d\n", num_camiones);
    } else if (argc == 3) {
        if (strcmp(argv[1], "0") == 0) {
            num_plazas = 0;
        } else {
            num_plazas = (int) strtol(argv[1], NULL, 10);
            if (num_plazas < 1) {
                fprintf(stderr, "Numero de plazas incorrecto: %s\n", argv[1]);
                return 1;
            }
        }
        if (strcmp(argv[2], "0") == 0) {
            num_plantas = 0;
        } else {
            num_plantas = (int) strtol(argv[2], NULL, 10);
            if (num_plantas < 1) {
                fprintf(stderr, "Numero de plantas incorrecto: %s\n", argv[2]);
                return 1;
            }
        }
        num_coches = 2 * num_plazas * num_plantas;
        num_camiones = 0;
        printf("Plazas: %d\n", num_plazas);
        printf("Plantas: %d\n", num_plantas);
        printf("Coches: %d\n", num_coches);
        printf("Camiones: %d\n", num_camiones);
    } else if (argc == 4) {
        if (strcmp(argv[1], "0") == 0) {
            num_plazas = 0;
        } else {
            num_plazas = (int) strtol(argv[1], NULL, 10);
            if (num_plazas < 1) {
                fprintf(stderr, "Numero de plazas incorrecto: %s\n", argv[1]);
                return 1;
            }
        }
        if (strcmp(argv[2], "0") == 0) {
            num_plantas = 0;
        } else {
            num_plantas = (int) strtol(argv[2], NULL, 10);
            if (num_plantas < 1) {
                fprintf(stderr, "Numero de plantas incorrecto: %s\n", argv[2]);
                return 1;
            }
        }
        if (strcmp(argv[3], "0") == 0) {
            num_coches = 0;
        } else {
            num_coches = (int) strtol(argv[3], NULL, 10);
            if (num_coches < 1) {
                fprintf(stderr, "Numero de coches incorrecto: %s\n", argv[3]);
                return 1;
            }
        }
        num_camiones = 0;
        printf("Plazas: %d\n", num_plazas);
        printf("Plantas: %d\n", num_plantas);
        printf("Coches: %d\n", num_coches);
        printf("Camiones: %d\n", num_camiones);
    } else if (argc == 5) {
        if (strcmp(argv[1], "0") == 0) {
            num_plazas = 0;
        } else {
            num_plazas = (int) strtol(argv[1], NULL, 10);
            if (num_plazas < 1) {
                fprintf(stderr, "Numero de plazas incorrecto: %s\n", argv[1]);
                return 1;
            }
        }
        if (strcmp(argv[2], "0") == 0) {
            num_plantas = 0;
        } else {
            num_plantas = (int) strtol(argv[2], NULL, 10);
            if (num_plantas < 1) {
                fprintf(stderr, "Numero de plantas incorrecto: %s\n", argv[2]);
                return 1;
            }
        }
        if (strcmp(argv[3], "0") == 0) {
            num_coches = 0;
        } else {
            num_coches = (int) strtol(argv[3], NULL, 10);
            if (num_coches < 1) {
                fprintf(stderr, "Numero de coches incorrecto: %s\n", argv[3]);
                return 1;
            }
        }
        if (strcmp(argv[4], "0") == 0) {
            num_camiones = 0;
        } else {
            num_camiones = (int) strtol(argv[4], NULL, 10);
            if (num_camiones < 1) {
                fprintf(stderr, "Numero de camiones incorrecto: %s\n", argv[4]);
                return 1;
            }
        }
        printf("Plazas: %d\n", num_plazas);
        printf("Plantas: %d\n", num_plantas);
        printf("Coches: %d\n", num_coches);
        printf("Camiones: %d\n", num_camiones);
    } else {
        printf("Demasiados argumentos. Ignorando argumentos extra\n");
        if (strcmp(argv[1], "0") == 0) {
            num_plazas = 0;
        } else {
            num_plazas = (int) strtol(argv[1], NULL, 10);
            if (num_plazas < 1) {
                fprintf(stderr, "Numero de plazas incorrecto: %s\n", argv[1]);
                return 1;
            }
        }
        if (strcmp(argv[2], "0") == 0) {
            num_plantas = 0;
        } else {
            num_plantas = (int) strtol(argv[2], NULL, 10);
            if (num_plantas < 1) {
                fprintf(stderr, "Numero de plantas incorrecto: %s\n", argv[2]);
                return 1;
            }
        }
        if (strcmp(argv[3], "0") == 0) {
            num_coches = 0;
        } else {
            num_coches = (int) strtol(argv[3], NULL, 10);
            if (num_coches < 1) {
                fprintf(stderr, "Numero de coches incorrecto: %s\n", argv[3]);
                return 1;
            }
        }
        if (strcmp(argv[4], "0") == 0) {
            num_camiones = 0;
        } else {
            num_camiones = (int) strtol(argv[4], NULL, 10);
            if (num_camiones < 1) {
                fprintf(stderr, "Numero de camiones incorrecto: %s\n", argv[4]);
                return 1;
            }
        }
        printf("Plazas: %d\n", num_plazas);
        printf("Plantas: %d\n", num_plantas);
        printf("Coches: %d\n", num_coches);
        printf("Camiones: %d\n", num_camiones);
    }

    // Comprobacion de validez de valores
    if (num_plantas < 1) {
        fprintf(stderr, "El numero de plantas especificadas es 0. No hay parking.\n");
        return 1;
    }
    if (num_plazas < 1) {
        fprintf(stderr, "El numero de plazas especificadas es 0. No hay parking.\n");
        return 1;
    }
    if (num_coches == 0 && num_camiones == 0) {
        fprintf(stderr, "No hay ni coches ni camiones.\n");
        return 1;
    }
    printf("\n");

    //Calculamos en que numero empiezan los id de los camiones
    int num_coches_copia = num_coches;
    while (num_coches_copia>0){
        num_coches_copia = num_coches_copia/10;
        if(id_camion_base==0){
            id_camion_base=10;
        }else{
            id_camion_base= id_camion_base*10;
        }
    }

    // Si los camiones son 1/4 de los coches, tienen prioridad los camiones en el parking (0 = !prioridad coches)
    prioridad_coches = num_camiones < num_coches / 4 ? 0 : 1;

    // Creamos las plazas
    num_total_plazas = num_plazas * num_plantas;
    plazas = (int *) malloc(sizeof(int) * num_total_plazas);

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
        id_camiones[k] = id_camion_base + k + 1;
    }

    // Iniciamos mutex, condiciones y parking
    num_plazas_libres = num_total_plazas;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&plazas_libres_cond, NULL);
    pthread_cond_init(&camion_esperando, NULL);
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
    pthread_cond_destroy(&camion_esperando);
    return 0;
}

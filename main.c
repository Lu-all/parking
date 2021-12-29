#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>
pthread_mutex_t mutex;

int main() {
    printf("Hello, World!\n");
    return 0;
}

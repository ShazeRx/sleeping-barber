#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define MAX_CUSTOMERS 20;
int i, numCustomers, numChairs;
pthread_mutex_t srvCust;
void *barber_func(void *junk);
void *client_func(void *junk);
void *create_customers(void *junk);
sem_t barber_ready;
sem_t customer_ready;
sem_t changeSeats;
int available_seats;
int clients_left = 0;
int service_finished = 0;
int current_client;
int main(int argc, char *argv[]) {
    pthread_t barber_thread;

    pthread_t customer_thread;

    printf("Enter the number of Customers : ");
    scanf("%d", &numCustomers);

    printf("Enter the number of Chairs : ");
    scanf("%d", &numChairs);

    sem_init(&barber_ready, 0, 0);

    sem_init(&customer_ready, 0, 0);

    sem_init(&changeSeats, 0, 1);

    available_seats = numChairs;

    int status = pthread_create(&barber_thread, NULL, (void *) barber_func, NULL);

    if (status != 0) {
        printf("Failed to create a barber thread\n");
        exit(status);
    }

    pthread_create(&customer_thread, NULL, (void *) create_customers, NULL);

    pthread_join(customer_thread, NULL);

    pthread_join(barber_thread, NULL);
}
void *barber_func(void *junk) {
    int clients_serviced = 0;
    while (!service_finished) {

        sem_wait(&customer_ready);

        sem_wait(&changeSeats);

        available_seats++;

        sem_post(&changeSeats);

        sem_post(&barber_ready);

        pthread_mutex_lock(&srvCust);

        int service_time = 1 + (rand() % 5);

        sleep(service_time);

        pthread_mutex_unlock(&srvCust);
        printf("Finished work with client: %d\n", current_client);
        clients_serviced++;
        if (clients_serviced == (numCustomers - clients_left)) {
            service_finished = 1;
        }
    }
    printf("Barber finished work, going home\n");
    pthread_exit(NULL);
}
void *client_func(void *number) {
    int id = *(int *) number;
    sem_wait(&changeSeats);
    if (available_seats > 0) {
        available_seats--;

        sem_post(&customer_ready);

        sem_post(&changeSeats);

        printf("2: Res:%d WRomm: %d/%d [in: %d]\n", clients_left, available_seats, numChairs, current_client);

        sem_wait(&barber_ready);

        current_client = id;

        printf("3: Res:%d WRomm: %d/%d [in: %d]\n", clients_left, available_seats, numChairs, current_client);
    } else {
        sem_post(&changeSeats);
        clients_left++;
        printf("4: Res:%d WRomm: %d/%d [in: %d]\n", clients_left, available_seats, numChairs, current_client);
    }
    pthread_exit(NULL);
}
void *create_customers(void *junk) {
    int thread_init_numbers[numCustomers];
    for (int i = 1; i <= numCustomers; i++) {
        thread_init_numbers[i] = i;
        pthread_t customer_thread;
        int status = pthread_create(&customer_thread, NULL, client_func, &thread_init_numbers[i]);
        if (status != 0) {
            printf("Failed to create a customer thread\n");
            exit(status);
        }
        usleep(2000);
    }
}

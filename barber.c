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
int clients_left_count = 0;
int service_finished = 0;
int current_client;
int debug = 0;

typedef struct Queue {
    int id;
    struct Queue *next;
} Position;


Position *barber_queue = NULL;
Position *clients_left = NULL;

int main(int argc, char *argv[]) {

    if (argc != 1) {
        if ((strncmp(argv[1], "-debug", 6) == 0)) {
            debug = 1;
        }
    }

    srand(time(NULL));

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
        if (clients_serviced == (numCustomers - clients_left_count)) {
            service_finished = 1;
        }
    }
    printf("Barber finished work, going home\n");
    pthread_exit(NULL);
}

void add_client_to_queue(Position **start, int id) {
    Position *new = malloc(sizeof(Position));
    new->id = id;
    new->next = NULL;
    if (*start == NULL) {
        *start = new;
    } else {
        Position *position_iterator = *start;
        while (position_iterator->next != NULL) {
            position_iterator = position_iterator->next;
        }
        position_iterator->next = new;
    }
}

void print_queue(Position *first) {
    Position *iterator = NULL;
    if (first == NULL) {
        return;
    } else {
        iterator = first;
        while (iterator != NULL) {
            printf("%d ", iterator->id);
            iterator = iterator->next;
        }
    }
}

void print_info() {
    printf("2: Res:%d WRomm: %d/%d [in: %d]\n", clients_left_count, available_seats, numChairs, current_client);
    if (debug) {
        printf("In Queue:");
        print_queue(barber_queue);
        printf("\n");
        printf("Left: ");
        print_queue(clients_left);
        printf("\n");
    }
}

void *client_func(void *number) {
    int id = *(int *) number;
    sem_wait(&changeSeats);
    if (available_seats > 0) {
        available_seats--;

        sem_post(&customer_ready);

        add_client_to_queue(&barber_queue, id);

        sem_post(&changeSeats);

        print_info();

        sem_wait(&barber_ready);

        current_client = id;

        print_info();
    } else {
        add_client_to_queue(&clients_left, id);

        printf("Client with id: %d left because of no available seats!\n", id);

        sem_post(&changeSeats);

        clients_left_count++;

        print_info();
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
        int sleep_random_time = rand() % 5;
        sleep(sleep_random_time);
    }
}

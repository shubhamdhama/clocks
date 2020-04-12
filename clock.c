#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 2
typedef void *(*process_func_pointer_t)(void *);
typedef struct {
    int passed_timestamp;
} Channel;

typedef struct {
    int id;
    int timestamp;
} Process;

Channel channels[NUM_THREADS][NUM_THREADS];

int max(int x, int y) {
    if (x > y) {
        return x;
    }
    return y;
}

void send(Process(*sender), int receiver_id) {
    printf("Sent from %d to %d\n", (*sender).id, receiver_id);
    channels[(*sender).id][receiver_id].passed_timestamp = (*sender).timestamp;
    ++(*sender).timestamp;
};

void receive(Process *receiver, int sender_id) {
    while (channels[sender_id][(*receiver).id].passed_timestamp == -1) {
    }
    printf("Received at %d from %d\n", (*receiver).id, sender_id);
    (*receiver).timestamp =
        max(channels[sender_id][(*receiver).id].passed_timestamp + 1,
            (*receiver).timestamp);
    ++(*receiver).timestamp;
    channels[sender_id][(*receiver).id].passed_timestamp = -1;
};

void *process_0(void *thd_id) {
    long virtual_process_id = (long)thd_id;
    printf("Process: %ld\n", virtual_process_id);

    Process p_0 = {0, 0};

    send(&p_0, 1);
    receive(&p_0, 1);
    printf("%d\n", p_0.timestamp);
}

void *process_1(void *thd_id) {
    long virtual_process_id = (long)thd_id;
    printf("Process: %ld\n", virtual_process_id);

    Process p_1 = {1, 0};
    receive(&p_1, 0);
    send(&p_1, 0);
    printf("%d\n", p_1.timestamp);
}

void execute_processes(process_func_pointer_t processes[]) {
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        for (int j = 0; j < NUM_THREADS; ++j) {
            channels[i][j].passed_timestamp = -1;
            channels[j][i].passed_timestamp = -1;
        }
    }

    for (long t = 0; t < NUM_THREADS; t++) {
        printf("In main: creating thread %ld\n", t);
        int rc = pthread_create(threads + t, NULL, processes[t], (void *)t);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(1);
        }
    }
    for (long t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }
}

int main() {
    process_func_pointer_t processes[NUM_THREADS];
    processes[0] = process_0;
    processes[1] = process_1;
    execute_processes(processes);
    return 0;
}

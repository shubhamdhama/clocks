#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 2
#define MAX_EVENTS_COUNT 30

typedef void *(*process_func_pointer_t)(void *);

typedef struct {
    int sender;
    int receiver;
    int sent_timestamp;
    int received_timestamp;
} Event;

typedef struct {
    Event *passed_event;
} Channel;

typedef struct {
    int id;
    int timestamp;
    Event **events;
    int event_counter;
} Process;

Channel channels[NUM_THREADS][NUM_THREADS];

int max(int x, int y) {
    if (x > y) {
        return x;
    }
    return y;
}

int print_event(Event *event) {
    printf(
        "Event: sender: %d, receiver: %d, sent_timestamp: %d, "
        "received_timestamp: %d\n",
        event->sender, event->receiver, event->sent_timestamp,
        event->received_timestamp);
}

void send(Process *sender, int receiver_id) {
    int sender_id = sender->id;

    printf("Sent from %d to %d\n", sender_id, receiver_id);

    Event *event = (Event *)malloc(sizeof(Event));
    *event = (Event){.sender = sender_id,
                     .receiver = receiver_id,
                     .sent_timestamp = sender->timestamp,
                     .received_timestamp = -1};

    channels[sender_id][receiver_id].passed_event = event;

    Event *sent_event = channels[sender_id][receiver_id].passed_event;

    sender->events[sender->event_counter] = event;
    ++sender->event_counter;

    ++(sender->timestamp);

    printf("Sent ");
    print_event(sent_event);
};

void receive(Process *receiver, int sender_id) {
    int receiver_id = receiver->id;
    while (channels[sender_id][receiver_id].passed_event == NULL)
        ;

    printf("Received at %d from %d\n", receiver_id, sender_id);

    Event *received_event = channels[sender_id][receiver_id].passed_event;

    receiver->timestamp =
        max(receiver->timestamp, received_event->sent_timestamp + 1);
    received_event->received_timestamp = receiver->timestamp;

    channels[sender_id][receiver_id].passed_event = NULL;

    receiver->events[receiver->event_counter] = received_event;
    ++receiver->event_counter;

    ++(receiver->timestamp);

    printf("Received ");
    print_event(received_event);
};

Process create_process(int process_id) {
    Process process = (Process){
        .id = process_id,
        .timestamp = 0,
        .events = NULL,
        .event_counter = 0,
    };
    process.events = (Event **)malloc(MAX_EVENTS_COUNT * sizeof(Event *));
    return process;
}

void print_process_events(Process *process) {
    printf("\n\n");
    printf("Clock time at end of process %d: %d\n", process->id,
           process->timestamp);
    printf("Total events occured %d\n", process->event_counter);
    for (int i = 0; i < process->event_counter; ++i) {
        if (process->id == process->events[i]->sender) {
            printf("Sent ");
        } else {
            printf("Received ");
        }
        printf("%d ", i);
        print_event(process->events[i]);
    }
    printf("\n\n");
}

void *process_0(void *thd_id) {
    long virtual_process_id = (long)thd_id;
    printf("Process: %ld\n", virtual_process_id);

    Process p_0 = create_process(virtual_process_id);

    send(&p_0, 1);
    receive(&p_0, 1);

    print_process_events(&p_0);
}

void *process_1(void *thd_id) {
    long virtual_process_id = (long)thd_id;
    printf("Process: %ld\n", virtual_process_id);

    Process p_1 = create_process(virtual_process_id);

    receive(&p_1, 0);
    send(&p_1, 0);

    print_process_events(&p_1);
}

void execute_processes(process_func_pointer_t processes[]) {
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        for (int j = 0; j < NUM_THREADS; ++j) {
            channels[i][j].passed_event = NULL;
            channels[j][i].passed_event = NULL;
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

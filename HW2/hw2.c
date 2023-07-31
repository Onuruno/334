#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#include "hw2_output.h"

struct args {
    int index;
    int isFirstAddition;
};
 
int N,M,K;
int *counts;
int **A,**B,**C,**D,**J,**L,**R;
sem_t* sem1;
sem_t* sem2;
sem_t mutex;
 
void* addRow(void *arguments){

    if(((struct args*)arguments)->isFirstAddition){        
        for (int i=0; i<M; i++){
            
            int result = A[((struct args*)arguments)->index][i] 
                    + B[((struct args*)arguments)->index][i];
            J[((struct args*)arguments)->index][i] = result;
            hw2_write_output(0, ((struct args*)arguments)->index, i, result);
        }
        sem_post(&sem1[((struct args*)arguments)->index]);
    } else{
        for (int i=0; i<K; i++){
            int result = C[((struct args*)arguments)->index][i] 
                    + D[((struct args*)arguments)->index][i];
            L[((struct args*)arguments)->index][i] = result;
            hw2_write_output(1, ((struct args*)arguments)->index, i, result);
            
            sem_wait(&mutex);
            if(++counts[i] == M){
                sem_post(&sem2[i]);
            }
            sem_post(&mutex);
        }
    }

    return NULL;
}

void* multiplyRow(void *arguments){

    sem_wait(&sem1[((struct args*)arguments)->index]);

    for (int i=0; i<K; i++){
        
        sem_wait(&sem2[i]);
        sem_post(&sem2[i]);

        int result = 0;

        for (int j=0; j<N; j++){
            result += J[((struct args*)arguments)->index][j]  * L[j][i];
        }
        hw2_write_output(2, ((struct args*)arguments)->index, i, result);
        R[((struct args*)arguments)->index][i] = result;
    }

    return NULL;
}

int main(){

    hw2_init_output();

    pthread_t *threads1, *threads2, *threads3;

    scanf("%d %d", &N, &M);
    threads1 = (pthread_t *) malloc(N * sizeof(pthread_t));
    threads2 = (pthread_t *) malloc(M * sizeof(pthread_t));
    threads3 = (pthread_t *) malloc(N * sizeof(pthread_t));
    A = (int**) malloc(N * sizeof(int*));
    J = (int**) malloc(N * sizeof(int*));
    for(int i=0; i<N; i++){
        
        A[i] = (int*) malloc(M * sizeof(int));
        J[i] = (int*) malloc(M * sizeof(int));

        for(int j=0; j<M; j++){
            scanf("%d", &A[i][j]);
        }
    }

    scanf("%d %d", &N, &M);    
    B = (int**) malloc(N * sizeof(int*));
    for(int i=0; i<N; i++){
        
        B[i] = (int*) malloc(M * sizeof(int));

        for(int j=0; j<M; j++){
            scanf("%d", &B[i][j]);
        }
    }

    scanf("%d %d", &M, &K);
    C = (int**) malloc(M * sizeof(int*));
    L = (int**) malloc(M * sizeof(int*));
    for(int i=0; i<M; i++){
        
        C[i] = (int*) malloc(K * sizeof(int));
        L[i] = (int*) malloc(K * sizeof(int));

        for(int j=0; j<K; j++){
            scanf("%d", &C[i][j]);
        }
    }

    scanf("%d %d", &M, &K);
    D = (int**) malloc(M * sizeof(int*));
    for(int i=0; i<M; i++){
        
        D[i] = (int*) malloc(K * sizeof(int));

        for(int j=0; j<K; j++){
            scanf("%d", &D[i][j]);
        }
    }

    R = (int**) malloc(N * sizeof(int*));
    for(int i=0; i<N; i++){
        R[i] = (int*) malloc(K * sizeof(int));
    }

    counts = (int*) malloc(K * sizeof(int));

    sem1 = (sem_t*) malloc(N * sizeof(sem_t));
    sem2 = (sem_t*) malloc(K * sizeof(sem_t));
    sem_init(&mutex, 0, 1);
    for(int i=0; i<N; i++){
        sem_init(&sem1[i], 0, 0);
    }
    for(int i=0; i<K; i++){
        sem_init(&sem2[i], 0, 0);
    }

    for(int i=0; i<N; i++){
        
        struct args *arguments = (struct args *)malloc(sizeof(struct args));
        arguments->index = i;
        arguments->isFirstAddition = 1;

        pthread_create(&threads1[i], NULL, &addRow, (void *)arguments);
    }

    for(int i=0; i<M; i++){
        struct args *arguments = (struct args *)malloc(sizeof(struct args));
        arguments->index = i;
        arguments->isFirstAddition = 0;

        pthread_create(&threads2[i], NULL, &addRow, (void *)arguments);
    }

    for(int i=0; i<N; i++){
        
        struct args *arguments = (struct args *)malloc(sizeof(struct args));
        arguments->index = i;

        pthread_create(&threads3[i], NULL, &multiplyRow, (void *)arguments);
    }

    for(int i=0; i<N; i++){
        pthread_join(threads1[i], NULL);
    }
    for(int i=0; i<M; i++){
        pthread_join(threads2[i], NULL);
    }
    for(int i=0; i<N; i++){
        pthread_join(threads3[i], NULL);
    }

    for(int i=0; i<N; i++){
        for(int j=0; j<K; j++){
            printf("%d ", R[i][j]);
        }
        printf("\n");
    }
 
    return 0;
}
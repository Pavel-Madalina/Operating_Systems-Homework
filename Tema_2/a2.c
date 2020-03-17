#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "a2_helper.h"

sem_t sem_th1, sem_th3, sem_4, sem_th10_begin, sem_th10_end, sem_th10;

void *thread_func(void *arg){
    int th_id = *(int *)arg;
    if(th_id == 3){
        sem_wait(&sem_th3);
        info(BEGIN, 2, th_id);
        info(END, 2, th_id);
        sem_post(&sem_th1);
    } else {
        info(BEGIN, 2, th_id);
    }
    if(th_id == 1){
        sem_post(&sem_th3);
        sem_wait(&sem_th1);
        info(END, 2, th_id); 
    } else if (th_id != 3){
        info(END, 2, th_id);
    }
    return NULL;
}

void *thread_func_36(void *arg){
    int th_id = *(int *)arg;
    if(th_id != 10){
        sem_wait(&sem_th10_begin); 
    }
    sem_wait(&sem_4); 
    info(BEGIN, 9, th_id);
    if(th_id == 10){
        for(int i=0;i<35;i++){ 
            sem_post(&sem_th10_begin); 
        }
        for(int i=0;i<3;i++){
            sem_wait(&sem_th10); 
        }
    }
    if(th_id != 10) {
        sem_post(&sem_th10); 
        sem_wait(&sem_th10_end); 
    }
    info(END, 9, th_id);
    if(th_id == 10){
        for(int i=0;i<35;i++){
            sem_post(&sem_th10_end); 
        }
    } 
    sem_post(&sem_4);
    return NULL;
}

int main(){
    init();
    
    info(BEGIN, 1, 0);

    int pid2 = -1,pid3 = -1,pid4 = -1, pid5 = -1,pid6 = -1,pid7 = -1,pid8 = -1,pid9 = -1;

    pid2 = fork();
    if(pid2 == -1){
        perror("Could not create child process no 2");
        return -1;
    } else if (pid2 == 0){
        info(BEGIN, 2, 0);

        pid3 = fork();
        if(pid3 == -1){
            perror("Could not create child process no 3");
            return -1;
        } else if(pid3 == 0){
            info(BEGIN, 3, 0);

            pid4 = fork();
            if(pid4 == -1){
                perror("Could not create child process no 4");
                return -1;
            } else if(pid4 == 0){
                info(BEGIN,4,0);

                pid8 = fork();
                if(pid8 == -1){
                    perror("Could not create child process no 8");
                    return -1;
                } else if(pid8 == 0){
                    info(BEGIN,8,0);

                    info(END,8,0);
                    exit(0);
                }
                waitpid(pid8,NULL,0);
                info(END,4,0);
                exit(0);
            }
            waitpid(pid4,NULL,0);

            info(END,3,0);
            exit(0);
        }

        pid5 = fork();
        if(pid5 == -1){
            perror("Could not create child process no 5");
            return -1;
        } else if(pid5 == 0){
            info(BEGIN,5,0);

            pid7 = fork();
            if(pid7 == -1){
                perror("Could not create child process no 7");
                return -1;
            } else if(pid7 == 0){
                info(BEGIN,7,0);

                info(END,7,0);
                exit(0);
            }
            waitpid(pid7,NULL,0);
            info(END,5,0);
            exit(0);
        }

        pid6 = fork();
        if(pid6 == -1){
            perror("Could not create child process no 6");
            return -1;
        } else if(pid6 == 0){
            info(BEGIN,6,0);

            pid9 = fork();
            if(pid9 == -1){
                perror("Could not create child process no 9");
                return -1;
            } else if(pid9 == 0){
                info(BEGIN,9,0);

                pthread_t threads[36];
                int ids[36];

                if(sem_init(&sem_4, 0, 4) != 0) {
                    perror("error initializing the semaphore");
                    return 1;
                }
                if(sem_init(&sem_th10_begin, 0, 0) != 0) {
                    perror("error initializing the semaphore");
                    return 1;
                }
                if(sem_init(&sem_th10_end, 0, 0) != 0) { 
                    perror("error initializing the semaphore");
                    return 1;
                }
                if(sem_init(&sem_th10, 0, 0) != 0) {
                    perror("error initializing the semaphore");
                    return 1;
                }
                for(int i=0;i<36;i++){
                    ids[i]=i+1;
                    pthread_create(&threads[i],NULL,thread_func_36,&ids[i]);
                }
                for(int i=0;i<36;i++){
                    pthread_join(threads[i],NULL);
                }
                sem_destroy(&sem_4);
                sem_destroy(&sem_th10_begin);
                sem_destroy(&sem_th10_end);
                sem_destroy(&sem_th10);

                info(END,9,0);
                exit(0);
            }
            waitpid(pid9,NULL,0);
            info(END,6,0);
            exit(0);
        }
        waitpid(pid3,NULL,0);
        waitpid(pid5,NULL,0);
        waitpid(pid6,NULL,0);

        pthread_t th[4];
        int id[4];
        
        if(sem_init(&sem_th1, 0, 0) != 0) {
                perror("error initializing the semaphore");
                return 1;
        }
        if(sem_init(&sem_th3, 0, 0) != 0) {
                perror("error initializing the semaphore");
                return 1;
        }
        for(int i=0;i<4;i++){
            id[i]=i+1;
            pthread_create(&th[i],NULL,thread_func,&id[i]);
        }
        for(int i=0;i<4;i++){
            pthread_join(th[i],NULL);
        }
        sem_destroy(&sem_th1);
        sem_destroy(&sem_th3);

        info(END,2,0);
        exit(0);
    }
    wait(NULL);
    info(END, 1, 0);
    return 0;
}

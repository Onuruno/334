#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

#include "message.h"
#include "logging.h"

#define ARGLENGTH 150
#define MAXBOMB 100

int main(int argc, char const *argv[]) {

    int width, height;
    int obstacteCount, bomberCount, activeBomberCount, bombCount, activeBombCount;
    int x,y,durability,argCount;
    int i, j;
    int **map;
    int *bomberLocations;
    int *bombLocations;
    int *bombRadius;
    int *bomberArgCounts;
    char **bomberArgs;
    int **fd1;
    int **fd2;
    int **fd3;
    int **fd4;
    pid_t *bomberpids;
    pid_t *bombpids;
    int status;

    scanf("%d %d %d %d", &width, &height, &obstacteCount, &bomberCount);

    activeBomberCount = bomberCount;
    bombCount = 0;
    activeBombCount = 0;
    map = (int **)malloc(height * sizeof(int*));
    bomberLocations = (int *)malloc(2* bomberCount * sizeof(int));
    bomberArgCounts = (int *)malloc(bomberCount * sizeof(int));
    bomberArgs = (char**) malloc(bomberCount * sizeof(char*));
    bomberpids = (int *)malloc(bomberCount * sizeof(pid_t));

    for(i=0; i<height; i++){
        map[i] = (int *)malloc(width * sizeof(int));
        for(j=0; j<width; j++){
            map[i][j] = 0;
        }
    }
    
    for(i=0; i<obstacteCount; i++){
        scanf("%d %d %d", &x, &y, &durability);
        map[y][x] = durability;
    }

    for(i=0; i<bomberCount; i++){
        scanf("%d %d %d", &x, &y, &argCount);
        bomberLocations[2*i] = x;
        bomberLocations[2*i+1] = y;

        bomberArgCounts[i] = argCount;
        bomberArgs[i] = (char*) malloc(ARGLENGTH);

        j=0;
        while(fgets(bomberArgs[i], ARGLENGTH, stdin) != NULL && j<1){
            j++;
        };
    }

    fd1 = malloc(sizeof(int*)*(bomberCount));
    for(i=0;i<bomberCount;i++)
    {
      fd1[i] = malloc(sizeof(int)*2);
    }

    fd2 = malloc(sizeof(int*)*(bomberCount));
    for(i=0;i<bomberCount;i++)
    {
      fd2[i] = malloc(sizeof(int)*2);
    }

    for(i=0; i<bomberCount;i++){
        pipe(fd1[i]);
        pipe(fd2[i]);
    }

    for(i=0; i<bomberCount;i++){
        bomberpids[i] = fork();
        if(bomberpids[i] == 0){    //Child            
            
            dup2(fd1[i][0], 0);     //redirects input end to stdin
            dup2(fd2[i][1], 1);     //redirects output end to stdout
            
            for(j=0; j<bomberCount; j++){
                close(fd1[j][0]);       //close other pipes
                close(fd1[j][1]);
                close(fd2[j][0]);
                close(fd2[j][1]);
            }

            char **arguments = (char**) malloc(bomberArgCounts[i] * sizeof(char*));
            char *token;
            int t = 0;
            token = strtok(bomberArgs[i], " ");
            arguments[t] = token;

            while(token != NULL) {
                token = strtok(NULL, " ");
                arguments[++t] = token;
            }
            execv(arguments[0], arguments);
        }
        else {              //Parent
            close(fd1[i][0]);       //parent writes to fd1
            close(fd2[i][1]);       //parent reades from fd2
        }
    }

    int k=0;
    int b=0;

    fd3 = malloc(sizeof(int*)*MAXBOMB);
    fd4 = malloc(sizeof(int*)*MAXBOMB);
    for(i=0; i<MAXBOMB; i++){
        fd3[i] = malloc(sizeof(int)*2);
        fd4[i] = malloc(sizeof(int)*2);

        pipe(fd3[i]);
        pipe(fd4[i]);
    }

    im* in = malloc(sizeof(*in));
    omp* ompr = malloc(sizeof(*ompr));
    while(activeBomberCount || activeBombCount){

        struct pollfd ufds1[bombCount];
        for(b=0; b<bombCount; b++){
            ufds1[b].fd = fd4[b][0];
            ufds1[b].events = POLLIN;
        }

        if(poll(ufds1, bombCount, 0) > 0){

            for(b=0; b<bombCount; b++){
                if(ufds1[b].revents & POLLIN){
                    if(read_data(fd4[b][0], in) > 0){
                        if(in->type == BOMB_EXPLODE){

                            imp* impr = malloc(sizeof(*impr));
                            impr->pid = bombpids[b];
                            impr->m = in;
                            print_output(impr, NULL, NULL, NULL);

                            int posx = bombLocations[2*b];
                            int posy = bombLocations[2*b+1];
                            int radius = bombRadius[b];
                            for(i=radius; i>=0; i--){
                                if(posx+i<width){   //right
                                    int bomberx;
                                    int bombery;
                                    for(int t=0; t<bomberCount; t++){       //check for bomber
                                        if(bomberLocations[2*t] == (posx+i) && bomberLocations[2*t+1] == posy){
                                            for(j=0; j<i; j++){
                                                if(map[posy][posx+j] != 0){
                                                    break;
                                                }
                                            }
                                            if(j == i){
                                                bomberLocations[2*t] = -1;      //(-1, -1) => heaven
                                                bomberLocations[2*t+1] = -1;
                                            }
                                        }
                                    }
                                    if(map[posy][posx+i] != 0){              //check for obstacle
                                        for(j=0; j<i; j++){
                                            if(map[posy][posx+j] != 0){
                                                break;
                                            }
                                        }
                                        if(j == i){
                                            if(map[posy][posx+i] > 0){
                                                map[posy][posx+i]-=1;
                                            }
                                            
                                            obsd* obstacle = malloc(sizeof(*obstacle));
                                            obstacle->position.x = posx+i;
                                            obstacle->position.y = posy;
                                            obstacle->remaining_durability = map[posy][posx+i];

                                            print_output(NULL, NULL, obstacle, NULL);
                                        }
                                    }
                                }
                                if(posx-i>=0){      //left
                                    int bomberx;
                                    int bombery;
                                    for(int t=0; t<bomberCount; t++){       //check for bomber
                                        if(bomberLocations[2*t] == (posx-i) && bomberLocations[2*t+1] == posy){
                                            for(j=0; j<i; j++){
                                                if(map[posy][posx-j] != 0){
                                                    break;
                                                }
                                            }
                                            if(j == i){
                                                bomberLocations[2*t] = -1;      //(-1, -1) => heaven
                                                bomberLocations[2*t+1] = -1;
                                            }
                                        }
                                    }
                                    if(map[posy][posx-i] != 0){
                                        for(j=0; j<i; j++){
                                            if(map[posy][posx-j] != 0){
                                                break;
                                            }
                                        }
                                        if(j == i){
                                            if(map[posy][posx-i] > 0){
                                                map[posy][posx-i]-=1;
                                            }

                                            obsd* obstacle = malloc(sizeof(*obstacle));
                                            obstacle->position.x = posx-i;
                                            obstacle->position.y = posy;
                                            obstacle->remaining_durability = map[posy][posx-i];

                                            print_output(NULL, NULL, obstacle, NULL);
                                        }
                                    }
                                }
                                if(posy-i>=0){      //up
                                    int bomberx;
                                    int bombery;
                                    for(int t=0; t<bomberCount; t++){       //check for bomber
                                        if(bomberLocations[2*t] == posx && bomberLocations[2*t+1] == (posy-i)){
                                            for(j=0; j<i; j++){
                                                if(map[posy-j][posx] != 0){
                                                    break;
                                                }
                                            }
                                            if(j == i){
                                                bomberLocations[2*t] = -1;      //(-1, -1) => heaven
                                                bomberLocations[2*t+1] = -1;
                                            }
                                        }
                                    }
                                    if(map[posy-i][posx] != 0){
                                        for(j=0; j<i; j++){
                                            if(map[posy-j][posx] != 0){
                                                break;
                                            }
                                        }
                                        if(j == i){
                                            if(map[posy-i][posx] > 0){
                                                map[posy-i][posx]-=1;
                                            }

                                            obsd* obstacle = malloc(sizeof(*obstacle));
                                            obstacle->position.x = posx;
                                            obstacle->position.y = posy-i;
                                            obstacle->remaining_durability = map[posy-i][posx];

                                            print_output(NULL, NULL, obstacle, NULL);
                                        }
                                    }
                                }
                                if(posy+i<height){  //down
                                    int bomberx;
                                    int bombery;
                                    for(int t=0; t<bomberCount; t++){       //check for bomber
                                        if(bomberLocations[2*t] == posx && bomberLocations[2*t+1] == (posy+i)){
                                            for(j=0; j<i; j++){
                                                if(map[posy+j][posx] != 0){
                                                    break;
                                                }
                                            }
                                            if(j == i){
                                                bomberLocations[2*t] = -1;      //(-1, -1) => heaven
                                                bomberLocations[2*t+1] = -1;
                                            }
                                        }
                                    }
                                    if(map[posy+i][posx] != 0){
                                        for(j=0; j<i; j++){
                                            if(map[posy+j][posx] != 0){
                                                break;
                                            }
                                        }
                                        if(j == i){
                                            if(map[posy+i][posx] > 0){
                                                map[posy+i][posx]-=1;
                                            }

                                            obsd* obstacle = malloc(sizeof(*obstacle));
                                            obstacle->position.x = posx;
                                            obstacle->position.y = posy+i;
                                            obstacle->remaining_durability = map[posy+i][posx];

                                            print_output(NULL, NULL, obstacle, NULL);
                                        }
                                    }
                                }
                            }
                            bombLocations[2*b] = -1; //bombs can also go to heaven(R.I.P.)
                            bombLocations[2*b+1] = -1;

                            activeBombCount--;
                        }
                    }
                }
            }
        }

        struct pollfd ufds2[bomberCount];
        for(k=0; k<bomberCount; k++){
            ufds2[k].fd = fd2[k][0];
            ufds2[k].events = POLLIN;
        }

        if(poll(ufds2, bomberCount, 0) > 0){

            for(k=0; k<bomberCount; k++){
                if(ufds2[k].revents & POLLIN){
                    if(read_data(fd2[k][0], in) > 0){
                        om* out = malloc(sizeof(*out));

                        imp* impr = malloc(sizeof(*impr));
                        impr->pid = bomberpids[k];
                        impr->m = in;
                        print_output(impr, NULL, NULL, NULL);

                        if(activeBomberCount == 1){
                            out->type = BOMBER_WIN;
                            activeBomberCount--;

                            ompr->pid = bomberpids[k];
                            ompr->m = out;
                            print_output(NULL, ompr, NULL, NULL);

                            send_message(fd1[k][1], out);
                        }
                        else if(bomberLocations[2*k] == -1 && bomberLocations[2*k+1] == -1){
                            out->type = BOMBER_DIE;
                            activeBomberCount--;

                            ompr->pid = bomberpids[k];
                            ompr->m = out;
                            print_output(NULL, ompr, NULL, NULL);

                            send_message(fd1[k][1], out);
                        }
                        else if(in->type == BOMBER_START){

                            out->type = BOMBER_LOCATION;
                            out->data.new_position.x = bomberLocations[2*k];
                            out->data.new_position.y = bomberLocations[2*k+1];

                            ompr->pid = bomberpids[k];
                            ompr->m = out;
                            print_output(NULL, ompr, NULL, NULL);

                            send_message(fd1[k][1], out);
                        }
                        else if(in->type == BOMBER_MOVE){

                            if(in->data.target_position.x >= 0 && in->data.target_position.x < width 
                                && abs(in->data.target_position.x - bomberLocations[2*k]) + abs(in->data.target_position.y - bomberLocations[2*k+1]) < 2
                                && in->data.target_position.y >= 0 && in->data.target_position.y < height
                                && map[in->data.target_position.y][in->data.target_position.x] == 0){
                                int l;
                                for(l=0; l<bomberCount; l++){

                                    if(l!=k && bomberLocations[2*l] == in->data.target_position.x 
                                            && bomberLocations[2*l+1] == in->data.target_position.y){
                                        break;
                                    }
                                }
                                if(l == bomberCount){
                                    bomberLocations[2*k] = in->data.target_position.x;
                                    bomberLocations[2*k+1] = in->data.target_position.y;
                                }
                            }

                            out->type = BOMBER_LOCATION;
                            out->data.new_position.x = bomberLocations[2*k];
                            out->data.new_position.y = bomberLocations[2*k+1];

                            ompr->pid = bomberpids[k];
                            ompr->m = out;
                            print_output(NULL, ompr, NULL, NULL);

                            send_message(fd1[k][1], out);
                        }
                        else if(in->type == BOMBER_SEE){

                            int objectCount=0;
                            od objects[25];
                            
                            int posx = bomberLocations[2*k];
                            int posy = bomberLocations[2*k+1];

                            int l, m;
                            for(l=0; l<bomberCount; l++){           //check for bombers in vision range
                               if(l != k && bomberLocations[2*l] != -1 && bomberLocations[2*l+1] != -1 
                                    && abs(bomberLocations[2*l] - posx) + abs(bomberLocations[2*l+1] - posy) <= 3){
                                    
                                    objects[objectCount].position.x=bomberLocations[2*l];
                                    objects[objectCount].position.y=bomberLocations[2*l+1];
                                    objects[objectCount++].type = BOMBER;
                               } 
                            }

                            for(l=0; l<bombCount; l++){             //check for bombs in vision range
                               if(bombLocations[2*l] != -1 && bombLocations[2*l+1] != -1 
                                    && abs(bombLocations[2*l] - posx) + abs(bombLocations[2*l+1] - posy) <= 3){
                                    
                                    objects[objectCount].position.x=bomberLocations[2*l];
                                    objects[objectCount].position.y=bomberLocations[2*l+1];
                                    objects[objectCount++].type = BOMB;
                               } 
                            }

                            for(l=0; l<height; l++){                //check for obstacles in vision range
                                for(m=0; m<width; m++){
                                    if(map[l][m] != 0 && abs(m - posx) + abs(l - posy) <= 3){

                                        objects[objectCount].position.x=m;
                                        objects[objectCount].position.y=l;
                                        objects[objectCount++].type = OBSTACLE;
                                    }
                                }
                            }

                            out->type = BOMBER_VISION;
                            out->data.object_count = objectCount;

                            ompr->pid = bomberpids[k];
                            ompr->m = out;
                            print_output(NULL, ompr, NULL, objects);

                            send_message(fd1[k][1], out);

                            if(objectCount){
                                send_object_data(fd1[k][1], objectCount, objects);
                            }
                        }
                        else if(in->type == BOMBER_PLANT){
                            
                            if(bombCount>0){
                                bombpids = (int *)realloc(bombpids, (bombCount+1) * sizeof(pid_t));
                                bombLocations = (int *)realloc(bombLocations, 2*(bombCount+1) * sizeof(int));
                                bombRadius = (int *)realloc(bombRadius, (bombCount+1) * sizeof(int));
                            }
                            else {
                                bombpids = (int *)malloc(sizeof(pid_t));
                                bombLocations = (int *)malloc(2*(bombCount+1) * sizeof(int));
                                bombRadius = (int *)malloc((bombCount+1) * sizeof(int));
                            }

                            bombpids[bombCount] = fork();
                            if (bombpids[bombCount] == 0){      //bomb

                                dup2(fd3[bombCount][0], 0);
                                dup2(fd4[bombCount][1], 1);

                                for(j=0; j<MAXBOMB; j++){
                                    close(fd3[j][0]);       //close other pipes
                                    close(fd3[j][1]);
                                    close(fd4[j][0]);
                                    close(fd4[j][1]);
                                }

                                char str[256];
                                sprintf(str, "%ld", in->data.bomb_info.interval);

                                execl("./bomb", "./bomb", str, (char*) NULL);
                            }
                            else {              //Parent
                                close(fd3[bombCount][0]);       //parent writes to fd3
                                close(fd4[bombCount][1]);       //parent reades from fd4
                            }

                            bombLocations[2*bombCount] = bomberLocations[2*k];
                            bombLocations[2*bombCount+1] = bomberLocations[2*k+1];

                            bombRadius[bombCount] = in->data.bomb_info.radius;

                            bombCount++;
                            activeBombCount++;

                            out->type = BOMBER_PLANT_RESULT;
                            out->data.planted = 1;

                            ompr->pid = bomberpids[k];
                            ompr->m = out;
                            print_output(NULL, ompr, NULL, NULL);

                            send_message(fd1[k][1], out);
                        }
                    }
                }
            }
        }
        usleep(1000);
    }
    
    for(int a=0; a<bomberCount; a++){
        close(fd1[a][1]);
        close(fd2[a][0]);
        waitpid(bomberpids[a], &status, 0);
    }
    for(int b=0; b<bombCount; b++){
        close(fd3[b][1]);
        close(fd4[b][0]);
        waitpid(bombpids[b], &status, 0);
    }

    return 0;
}

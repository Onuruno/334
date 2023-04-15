#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "message.h"
#include "logging.h"

#define ARGLENGTH 150

int main(int argc, char const *argv[]) {

    int width, height;
    int obstacteCount, bomberCount, activeBomberCount;
    int x,y,durability,argCount;
    int i, j, k;
    int *map;
    int *bomberLocations;
    int *bomberArgCounts;
    char **bomberArgs;
    int **fd1;
    int **fd2;
    pid_t *childpids;

    scanf("%d %d %d %d", &width, &height, &obstacteCount, &bomberCount);

    activeBomberCount = bomberCount;
    map = (int *)malloc(width * height * sizeof(int));
    bomberLocations = (int *)malloc(2* bomberCount * sizeof(int));
    bomberArgCounts = (int *)malloc(bomberCount * sizeof(int));
    bomberArgs = (char**) malloc(bomberCount * sizeof(char*));
    childpids = (int *)malloc(bomberCount * sizeof(pid_t));

    for(i=0; i<height*width; i++){
        map[i] = 0;
    }
    
    for(i=0; i<obstacteCount; i++){
        scanf("%d %d %d", &x, &y, &durability);
        map[y*width + x] = durability;
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

    fd1 = malloc(sizeof(int*)*(bomberCount+10));
    for(i=0;i<bomberCount+10;i++)
    {
      fd1[i] = malloc(sizeof(int)*2);
    }

    fd2 = malloc(sizeof(int*)*(bomberCount+10));
    for(i=0;i<bomberCount+10;i++)
    {
      fd2[i] = malloc(sizeof(int)*2);
    }

    for(i=0; i<bomberCount;i++){
        pipe(fd1[i]);
        pipe(fd2[i]);
    }

    for(i=0; i<bomberCount;i++){
        childpids[i] = fork();
        if(childpids[i] == 0){    //Child            
            for(j=0; j<bomberCount; j++){
                if(i == j){
                    dup2(fd1[j][0], 0);     //redirects input end to stdin
                    dup2(fd2[j][1], 1);     //redirects output end to stdout
                }
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

    k=0;
    im* in = malloc(sizeof(*in)); 
    while(read(fd2[k][0], in, sizeof(im)) > 0){
        om* out = malloc(sizeof(*out));

        imp* impr = malloc(sizeof(*impr));
        impr->pid = childpids[k];
        impr->m = in;
        print_output(impr, NULL, NULL, NULL);

        if(in->type == BOMBER_START){

            out->type = BOMBER_LOCATION;
            out->data.new_position.x = bomberLocations[2*k];
            out->data.new_position.y = bomberLocations[2*k+1];
        }
        else if(in->type == BOMBER_MOVE){

            int valid=1;
            if(in->data.target_position.x >= 0 && in->data.target_position.x < width 
                && abs(in->data.target_position.x - bomberLocations[2*k]) + abs(in->data.target_position.y - bomberLocations[2*k+1]) < 2
                && in->data.target_position.y >= 0 && in->data.target_position.y < height
                && map[in->data.target_position.y * width + in->data.target_position.x] == 0){

                for(int l=0; l<bomberCount; l++){

                    if(l!=k && bomberLocations[2*l] == in->data.target_position.x 
                            && bomberLocations[2*l+1] == in->data.target_position.y){

                        valid=0;
                        break;
                    }
                }
            }
            else{
                valid=0;
            }

            if(valid){
                bomberLocations[2*k] = in->data.target_position.x;
                bomberLocations[2*k+1] = in->data.target_position.y;
            }

            out->type = BOMBER_LOCATION;
            out->data.new_position.x = bomberLocations[2*k];
            out->data.new_position.y = bomberLocations[2*k+1];
        }
        else if(in->type == BOMBER_SEE){

            

            out->type = BOMBER_VISION;
            out->data.object_count = 0;
        }

        omp* ompr = malloc(sizeof(*ompr));
        ompr->pid = childpids[k];
        ompr->m = out;
        print_output(NULL, ompr, NULL, NULL);

        write(fd1[k][1], out, sizeof(om));

        k = (k+1)%bomberCount;
        sleep(1);
    }
        

    return 0;
}

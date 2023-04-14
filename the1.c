#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include "message.h"
#include "logging.h"

#define ARGLENGTH 150

int main(int argc, char const *argv[]) {

    int width, height;
    int obstacteCount, bomberCount;
    int x,y,durability,argCount;
    int i, j, k;
    int *map;
    int *bomberLocations;
    int *bomberArgCounts;
    char **bomberArgs;
    int **fd1;
    int **fd2;

    scanf("%d %d %d %d", &width, &height, &obstacteCount, &bomberCount);

    map = (int *)malloc(width * height * sizeof(int));
    bomberLocations = (int *)malloc(2* bomberCount * sizeof(int));
    bomberArgCounts = (int *)malloc(bomberCount * sizeof(int));
    bomberArgs = (char**) malloc(bomberCount * sizeof(char*));
    

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

        j=1;
        while(fgets(bomberArgs[i], ARGLENGTH, stdin) != NULL && j<2){
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
        if(fork() == 0){    //Child            
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
            printf("%s", token);
            arguments[t++] = token;

            while(token != NULL ) {
                token = strtok(NULL, " ");
                arguments[t++] = token;
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
    om* out = malloc(sizeof(*out));
    while(read(fd2[k][0], in, sizeof(im)) > 0){
        printf("%d: %d geldi.\n", k, in->type);

        if(in->type == BOMBER_START){
            
            out->type = BOMBER_LOCATION;
            out->data.new_position.x = bomberLocations[2*k];
            out->data.new_position.y = bomberLocations[2*k+1];
            
            write(fd1[k][1], out, sizeof(out));
        }
        else if(in->type == BOMBER_MOVE){
            printf("Istek -> x:%d y:%d\n", in->data.target_position.x, in->data.target_position.y);
            int valid=1;
            if(in->data.target_position.x >= 0 && in->data.target_position.x < width 
                && in->data.target_position.y >= 0 && in->data.target_position.y < height
                && map[in->data.target_position.y * width + in->data.target_position.x] == 0){

                for(int l=0; l<bomberCount; l++){

                    if(l!=k && bomberLocations[2*l] != in->data.target_position.x 
                            && bomberLocations[2*l+1] != in->data.target_position.y){
                        
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
                //printf("x:%d y:%d\n", bomberLocations[2*k], bomberLocations[2*k+1]);
            }
            
            out->type = BOMBER_LOCATION;
            out->data.new_position.x = bomberLocations[2*k];
            out->data.new_position.y = bomberLocations[2*k+1];

            write(fd1[k][1], out, sizeof(out));
        }
        else if(in->type == BOMBER_SEE){

            out->type = BOMBER_VISION;
            out->data.object_count = 0;

            write(fd1[k][1], out, sizeof(out));
        }
        
        k = (k+1)%bomberCount;
        sleep(1);
    }
        

    return 0;
}
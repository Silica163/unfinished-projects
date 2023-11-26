#include <stdio.h>
#include <stdlib.h>

// readFile
// give a file path and file size
// return a pointer to content of file ( char * )
char *readFile(char path[],int readSize,int lastZero){
    char* content = calloc((readSize + lastZero),sizeof(char));
    FILE *f = fopen(path,"r");
    if(f == NULL){
        printf("can't read %s \n",path);
    }
    fread(content,readSize,1,f);
    fclose(f);
    return content;
}

/* writeFile
 * give a file path, pointer to buffer and file size
 * return state of file 0 success -1 fail
 * */
int writeFile(char path[],char *buffer[],int writeSize){
    FILE *wf = fopen(path,"wb");
    if( wf == NULL ){
        printf("could not write to %s",path);
        return -1;
    }
    fwrite(*buffer,writeSize,1,wf);
    fclose(wf);
    return 0;
}



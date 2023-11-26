#include <stdio.h>
#include <stdlib.h>
// file information 
#include <sys/stat.h>
#include <stdarg.h>

#include "fileio.h"

/*
 * quote '>' parse like normal paragraph
 * start with '> ' like header
 * */

// html tag type
#define TAG_NONE 0
#define TAG_H 1
#define TAG_P 2
#define TAG_HR 3

#define MD_H '#'
#define MD_QUOTE '>'

#define DEBUG 0 
int stdLog(const char *format, ...){
    int status = 0;
    va_list args;
    va_start(args, format);
    if(DEBUG)
        return vprintf(format, args);
    va_end(args);
    return status;
}

int main(int argc,char *argv[]){

    if(argc <= 1){
        printf("no argument provided\n");
        return 1;
    }

    char *mdPath = argv[1];

//    printf("Reading %s \n",mdPath);
    struct stat mdStat;
    if(lstat(mdPath,&mdStat) == -1){
        printf("can't get stat of %s\n",mdPath);
        return 1;
    }

    char *mdContent = readFile(mdPath,mdStat.st_size,1);

    char htmlStart[] = "<!DOCTYPE HTML><html><head><title>md2html test</title></head><body>\n";
    char htmlEnd[] = "</body></html>\n";

    FILE * mdOut = stdout;
    fprintf(mdOut,htmlStart);

    // process status
    int hLevel = 0;
    int tag = 0;
    int paraReady = 0;
    int newLine = 0;

    // hr rule
    char hr = 0;
    int hrLevel = 0;
    int hrAval = 0;

    // process buffer
    char * mdLineBuff = malloc(128);
    int lineBuffPtr = 0;
    char * mdPBuff = malloc(1024);
    int paraBuffPtr = 0;

/*
 * plan 
 * 1. process line by line
 * 2. go to all character in the line
 * 3. get all command (#, -,...)
 * 4. generate output by command
*/
    for(int mdPointer = 0;mdPointer < mdStat.st_size; mdPointer++){

        char mdNow = mdContent[mdPointer];
        stdLog("char at %.2d has value 0x%.2x in hex and %c as a single character.\n",mdPointer,mdNow,mdNow);
        mdLineBuff[lineBuffPtr] = mdNow;
        if(mdNow != '\n'){
            lineBuffPtr ++;
            newLine = 0;
            continue;
        }

        mdLineBuff[lineBuffPtr] = 0;
        newLine ++;

        const int lineBuffLen = lineBuffPtr+1;

        hrAval = 1;
        int newLineBak;

        // scan for command
        for(int i = 0; i < lineBuffLen; i++ ){
            char lNow = mdLineBuff[i];
            // break when reach end of line
            if(lNow == 0)break;

            if(hrAval){
                if( !hr && !hrLevel && (lNow == '*' || lNow == '-' || lNow == '_'))hr = lNow;
                if(lNow == hr){
                    hrLevel ++;
                    newLineBak = newLine;
                    newLine = 0;
                }
                if(hrLevel >= 3){
                    paraReady = 1;
                    tag = TAG_HR;
                }
                if(lNow != hr && lNow != ' '){
                    tag = TAG_NONE;
                    hrAval = 0;
                    newLine = newLineBak;
                }
            }

            if(lNow == MD_H){
                tag = TAG_H;
                hrAval = 0;
                hLevel++;
                continue;
            }

            if(hLevel && lNow == ' '){
                tag = TAG_H;
                hrAval = 0;
                paraReady = 1;
                continue;
            }

            if(!hrAval && hLevel == i && lNow != ' '){
                tag = TAG_P;
                hLevel = 0;
                paraReady = 0;
                break;
            }

            if(!tag){
                tag = TAG_P;
                paraReady = 0;
            }
        }

        if(!tag && newLine > 1){
            paraReady = 1;
        }

        //print a paragraph
        if(paraReady && paraBuffPtr){
            mdPBuff[paraBuffPtr] = 0;
            fprintf(mdOut,"<p>%s</p>\n",mdPBuff);

            paraReady = 0;
            paraBuffPtr = 0;
            newLine = 0;
        }

        // convert to html tag
        switch(tag){
            case TAG_NONE:
                break;
            case TAG_H:
                int shift = (hLevel + 1);
                char *HOut = malloc(128 - shift);
                for(int i = 0;i<128;i++){
                    HOut[i] = mdLineBuff[i+shift];
                    if(HOut[i] == 0)break;
                }
                fprintf(mdOut,"<h%d>%s</h%d>\n",hLevel,HOut,hLevel);
                free(HOut);
                tag = TAG_NONE;
                hLevel = 0;
                newLine = 0;
                break;
            case TAG_P:
                for(int i = 0;i <= lineBuffPtr;i++){
                    mdPBuff[i+paraBuffPtr] = mdLineBuff[i];
                    if(mdLineBuff[i] == 0)
                        mdPBuff[i+paraBuffPtr] = '\n';
                }
                paraBuffPtr += lineBuffPtr + 1;
                break;
            case TAG_HR:
                fprintf(mdOut,"<hr>\n");
                tag = TAG_NONE;
                break;
        }
        lineBuffPtr = 0;
        hr = 0;
        hrLevel = 0;
        tag = TAG_NONE;
    }
    fprintf(mdOut,htmlEnd);

    return 0;
}

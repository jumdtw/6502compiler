#include<stdio.h>
#include <string.h>
#define MAX 10000

char buf[MAX]={0};
char tmp[1000]={0};

char *read_file(char *filename){
    FILE *fp ;
    if((fp=fopen(filename,"r"))!=NULL){
        /*ファイルの終わりまで繰り返し読み込む*/

        while( fgets(tmp,MAX,fp) != NULL ){
            strcat(buf,tmp);
            //printf("%s",tmp);
        }
    }
    //printf("%s\n",buf);
    return buf;
}


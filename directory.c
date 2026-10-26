#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int scanfolder(int, char*);
int diag(char*);
int insersort(int);

/*
typedef int (*FCMP)(const void*, const void*);
int compare(const void*, const void*);
int compare(const void* s1, const void* s2) {
    return(strcmp((const char*) s1, (const char*) s2));
}
*/

static char* list[512];

int insertsort(int num) {
int i=0;
int j;
char* p;
while(++i < num) {
  j = i - 1;
  p = list[i];
  while(strcmp(p, list[j]) < 0) {
    list[j+1] = list[j];
    j--;
  }
  list[j+1] = p;
}
return(0);
}

int diag(char* mes) {
    write(1, mes, strlen(mes));
    write(1, ": ", 2);
    write(1, sys_errlist[errno], strlen(sys_errlist[errno]));
    write(1, "\n", 1);
    exit(errno);
}

int scanfolder(int type, char* suf) {
DIR* fdir;
struct dirent* folder;
struct stat sbuf[1];
char* start;
char* s;
int count=0;
int len;

if((fdir = opendir(".")) == NULL)
    return(errno);
list[count] = "\n";
count++;
start = sbrk(0);
while((folder = readdir(fdir)) != NULL) { 
    /* sbuf->st_mode = 0; */
    stat(folder->d_name, sbuf);
    if(((sbuf->st_mode) & S_IFMT) != type) 
      continue;
    len = strlen(folder->d_name);
    s = sbrk(len+1);
    memcpy(s, folder->d_name, len);
    list[count] = s;
    count++;
    list[count] = NULL;
}    
closedir(fdir);
    /*qsort(start, i, 256, (FCMP) compare); */
insertsort(count);    
count=1;
while(list[count] != NULL) {
    len = strlen(list[count]);
    write(1, list[count], len);
    write(1, suf, 2);
    count++;
}    
brk(start);
return(0);
}

int main(int argc, char* argv[]) {
if(chdir(argv[1]) < 0)
	return(diag(argv[1]));  
if(scanfolder(S_IFDIR, "/\n") > 0)
	return(diag(argv[1]));  
scanfolder(S_IFREG, " \n");
return(0);
}


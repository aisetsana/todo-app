#ifndef FILE_MNG_H
#define FILE_MNG_H
#define MAX_SIZE 256

extern char buffer[64];
extern char fileString[MAX_SIZE];

char* getLocation();
void substring(char* s1, char* s2, int a, int b);
int getFileLines(char* loc);
char* loadTaskName(char* loc, int line, int returnType);
bool fileExists(char* loc);
void createFile();
void appendToCfg(char* name, int prio);
void deleteLineFmFile(char* loc, int line);
#endif
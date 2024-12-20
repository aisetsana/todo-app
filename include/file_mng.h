#ifndef FILE_MNG_H
#define FILE_MNG_H
#define MAX_SIZE 256

#include <stdbool.h>

extern char buffer[MAX_SIZE];
extern char fileString[MAX_SIZE];

char* getLocation();
void substring(char* s1, char* s2, int a, int b);
int getFileLines(char* loc);
char* loadTaskName(char* loc, int line, int returnType);
bool fileExists(char* loc);
void createFile();
void appendToCfg(char* name, int prio);
void swap(int line, int direction);
void edit(int line, int index, char* input);
void editName(int line, char* input);
void editPriority(int line, char* input);
void deleteLineFmFile(char* loc, int line);
#endif
/* Made by aisetsana on 9/15/2024 */

#include "file_mng.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#define MAX_SIZE 256

char buffer[MAX_SIZE];
char* getLocation() {
    sprintf_s(buffer, MAX_SIZE, "%s\\%s", getenv("USERPROFILE"), "\\tasks.cfg");
    return buffer;
}

void substring(char* s1, char* s2, int a, int b) {
    int i;
    for (i = 0; i < b; i++) {
        s2[i] = s1[a + i];
    }
    s2[i] = 0;
}
char fileString[MAX_SIZE];

int getFileLines(char* loc) {
    FILE* file;
    if ((file = fopen(loc, "r")) == NULL) {
        perror("Could not open file\n");
        return 0;
    }

    char string[MAX_SIZE];
    int lines = 0;
    while (fgets(string, MAX_SIZE, file) != NULL) {
        //        string[strcspn(string, "\n")] = 0;
        lines++;
    }
    fclose(file);
    return lines;
}

// Return type:  1: name, 2: priority, 3: ID, 4: timestamp; int line starts from 1 not 0
char* loadTaskName(char* loc, int line, int returnType) {
    FILE* file;
    if ((file = fopen(loc, "r")) == NULL) {
        perror("Could not open file, are you sure it exists?\n");
    }
    int count = 0;
    while (fgets(fileString, MAX_SIZE, file) != NULL) {
        if (line == count) break;

        fileString[strcspn(fileString, "\n")] = 0;
        count++;
    }
    char* input;
    char *name, *prio, *id, *timestamp;

    input = fileString;
    char *clone, *token;
    clone = input;
    int j = 0;
    while ((token = strtok_r(clone, ",", &clone))) {
        substring(token, input, strcspn(token, "=") + 1, strcspn(token, "\n"));
        switch (j) {
            case 0:
                name = malloc((int)strlen(input) * sizeof(char));
                strcpy(name, input);
                break;
            case 1:
                prio = malloc((int)strlen(input) * sizeof(char));
                strcpy(prio, input);
                break;
            case 2:
                id = malloc((int)strlen(input) * sizeof(char));
                strcpy(id, input);
                break;
            case 3:
                timestamp = malloc((int)strlen(input) * sizeof(char));
                strcpy(timestamp, input);
                break;
        }
        j++;
    }
    fclose(file);

    if (returnType == 1)
        return name;
    else if (returnType == 2) {
        if (strncmp(prio, "LOW", 3) == 0)
            return "0";
        else if (strncmp(prio, "MED", 3) == 0)
            return "1";
        else if (strncmp(prio, "HIGH", 3) == 0)
            return "2";
    } else if (returnType == 3) {
        return id;
    } else if (returnType == 4) {
        return timestamp;
    } else {
        perror("Invalid return type");
    }
}

bool fileExists(char* loc) {
    struct stat buffer;
    return (stat(loc, &buffer) == 0);
}
void createFile() {
    if (!fileExists(getLocation())) {
        FILE* file;

        file = fopen(getLocation(), "w");  // if file doesnt exist, create it
    }
}
void appendToCfg(char* name, int prio) {
    time_t now = time(NULL);

    FILE* file = fopen(getLocation(), "a");
    int id = getFileLines(getLocation());

    char _prio[16];
    if (prio == 0) strcpy(_prio, "LOW");
    if (prio == 1) strcpy(_prio, "MEDIUM");
    if (prio == 2) strcpy(_prio, "HIGH");

    fprintf(file, "NAME=%s,PRIORITY=%s,ID=%d,TIMESTAMP=%d\n", name, _prio, id, (int)now);

    fclose(file);
}

// A super shitty way to swap which its the same as deleting a line except it... doesn't delete a line
// Direction is which way to go: 0 to swap down, -1 to swap up (swapping up is handled by basically swapping down the task before the selected task)
void swap(int line, int direction) {
    line = line + direction;

    FILE *file, *tempFile;
    char temp[256];
    char* loc = getLocation();
    strcpy(temp, loc);
    strcat(temp, ".temp");

    char buffer[2048];
    file = fopen(loc, "r");
    tempFile = fopen(temp, "w");
    if (file == NULL) {
        perror("Could not open main file!\n");
    } else if (tempFile == NULL) {
        perror("Could not open temp file!\n");
    } else if (direction != 0 && direction != -1) {
        perror("Direction for swap can only be 0 or -1\n");
    }

    char _l1[256], _l2[256];
    bool keepReading = true;
    int currentLine = 0;
    char prios[3][8] = {"LOW", "MEDIUM", "HIGH"};
    bool linehit = false;

    char nLine[256], pLine[256];  // next line and prev line respectively

    do {
        fgets(buffer, 2048, file);
        if (feof(file))
            keepReading = false;
        else {
            if (currentLine != line) {
                if (linehit) {  // swap w prev line
                    snprintf(pLine, 256, "NAME=%s,PRIORITY=%s,ID=%d,TIMESTAMP=%ld",
                             loadTaskName(loc, currentLine - 1, 1),
                             prios[strtol(loadTaskName(loc, currentLine - 1, 2), NULL, 10)],
                             currentLine,
                             strtol(loadTaskName(loc, currentLine - 1, 4), NULL, 10));
                    fprintf(tempFile, "%s\n", pLine);
                    linehit = false;
                } else {
                    fprintf(tempFile, "NAME=%s,PRIORITY=%s,ID=%d,TIMESTAMP=%ld\n",
                            loadTaskName(loc, currentLine, 1),
                            prios[strtol(loadTaskName(loc, currentLine, 2), NULL, 10)],
                            currentLine,
                            strtol(loadTaskName(loc, currentLine, 4), NULL, 10));
                }
            } else {  // swap w next line
                linehit = true;
                snprintf(nLine, 256, "NAME=%s,PRIORITY=%s,ID=%d,TIMESTAMP=%ld",
                         loadTaskName(loc, currentLine + (1), 1),
                         prios[strtol(loadTaskName(loc, currentLine + 1, 2), NULL, 10)],
                         currentLine,
                         strtol(loadTaskName(loc, currentLine + (1), 4), NULL, 10));
                fprintf(tempFile, "%s\n", nLine);
            }
        }
        currentLine++;
    } while (keepReading);
    fclose(file);
    fclose(tempFile);
    remove(loc);
    rename(temp, loc);
}

void edit(int line, int index, char* input) {
    FILE *file, *tempFile;
    char temp[256];
    char* loc = getLocation();
    strcpy(temp, loc);
    strcat(temp, ".temp");

    char buffer[2048];
    file = fopen(loc, "r");
    tempFile = fopen(temp, "w");
    if (file == NULL) {
        perror("Could not open main file!\n");
    } else if (tempFile == NULL) {
        perror("Could not open temp file!\n");
    } else if (index != 0 && index != 1) {
        perror("Index can only be either 0 or 1!\n");
    }

    bool keepReading = true;
    int currentLine = 0;
    char prios[3][8] = {"LOW", "MEDIUM", "HIGH"};
    bool linehit = false;

    do {
        fgets(buffer, 2048, file);
        int id = strtol(loadTaskName(loc, currentLine, 3), NULL, 10);
        long prio = strtol(loadTaskName(loc, currentLine, 2), NULL, 10);
        if (feof(file))
            keepReading = false;
        else {
            if (currentLine == line) {
                if (index == 0) {  // if index is 0, we're editing the name value
                    fprintf(tempFile, "NAME=%s,PRIORITY=%s,ID=%d,TIMESTAMP=%ld\n",
                            input,
                            prios[prio],
                            id,
                            strtol(loadTaskName(loc, currentLine, 4), NULL, 10));
                } else if (index == 1) {  // if index is 1, we're editing the prio value
                    if (strncmp(input, "LOW", 3) == 0)
                        prio = 0;
                    else if (strncmp(input, "MED", 3) == 0)
                        prio = 1;
                    else if (strncmp(input, "HIGH", 3) == 0)
                        prio = 2;
                    else
                        prio = 0;

                    fprintf(tempFile, "NAME=%s,PRIORITY=%s,ID=%d,TIMESTAMP=%ld\n",
                            loadTaskName(loc, currentLine, 1),
                            prios[prio],
                            id,
                            strtol(loadTaskName(loc, currentLine, 4), NULL, 10));
                }
            } else {
                fprintf(tempFile, "NAME=%s,PRIORITY=%s,ID=%d,TIMESTAMP=%ld\n",
                        loadTaskName(loc, currentLine, 1),
                        prios[prio],
                        id,
                        strtol(loadTaskName(loc, currentLine, 4), NULL, 10));
            }
        }
        currentLine++;
    } while (keepReading);
    fclose(file);
    fclose(tempFile);
    remove(loc);
    rename(temp, loc);
}

// edit a specific line in the config file; index is two values 0 for editing the name, 1 for editing the priority
void editName(int line, char* input) {
    edit(line, 0, input);
}

void editPriority(int line, char* input) {
    edit(line, 1, input);
}

void deleteLineFmFile(char* loc, int line) {
    FILE *file, *tempFile;
    char temp[256];
    strcpy(temp, loc);
    strcat(temp, ".temp");

    char buffer[2048];
    file = fopen(loc, "r");
    tempFile = fopen(temp, "w");
    if (file == NULL) {
        perror("Could not open main file!\n");
    } else if (tempFile == NULL) {
        perror("Could not open temp file!\n");
    }

    bool keepReading = true;
    int currentLine = 0;
    char prios[3][8] = {"LOW", "MEDIUM", "HIGH"};

    do {
        fgets(buffer, 2048, file);
        int id = strtol(loadTaskName(loc, currentLine, 3), NULL, 10);
        int prio = strtol(loadTaskName(loc, currentLine, 2), NULL, 10);
        if (feof(file))
            keepReading = false;
        else {
            if (currentLine != line) {
                if (currentLine > line) id--;
                fprintf(tempFile, "NAME=%s,PRIORITY=%s,ID=%d,TIMESTAMP=%ld\n",
                        loadTaskName(loc, currentLine, 1),
                        prios[prio],
                        id,
                        strtol(loadTaskName(loc, currentLine, 4), NULL, 10));
            }
        }
        currentLine++;
    } while (keepReading);
    fclose(file);
    fclose(tempFile);
    remove(loc);
    rename(temp, loc);
}

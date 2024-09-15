/* Made by karroch37 on 9/15/2024 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#define MAX_SIZE 256

char buffer[64];
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

char* loadTaskName(char* loc, int line, int returnType) {
    FILE* file;
    if ((file = fopen(loc, "r")) == NULL) {
        perror("Could not open file, are you sure it exists?\n");
    }
    int count = 0;
    while (fgets(fileString, MAX_SIZE, file) != NULL) {
        fileString[strcspn(fileString, "\n")] = 0;
        count++;

        if (line == count) break;
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
    } else if (returnType == 3)
        return id;
    else if (returnType == 4)
        return timestamp;
    else
        perror("Invalid return type");
}

bool fileExists(char* loc) {
    struct stat buffer;
    return (stat(loc, &buffer) == 0);
}
void createFile() {
    if (!fileExists(getLocation())) {
        FILE* file = fopen(getLocation(), "r");

        file = fopen(getLocation(), "w");
    } else {
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

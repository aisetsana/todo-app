#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "../include/file_mng.h"
#include "../include/raylib.h"

// TODO: add a scrollbar, add up and down buttons to sort stuff

#define WIDTH 1045
#define HEIGHT 596
#define BACKGROUND_COLOR \
    CLITERAL(Color) { 30, 30, 30, 255 }
#define BOX_COLOR \
    CLITERAL(Color) { 20, 20, 20, 255 }
#define FONT_SIZE 36

typedef enum Priority {
    LOW,
    MEDIUM,
    HIGH,
} Priority;

typedef enum Screen {
    MAIN_SCREEN,
    CREATE_SCREEN,
    EDIT_SCREEN,
} Screen;

typedef struct {
    const char *title;
    Priority priority;
    int id;

} Task;

Screen currentScreen = MAIN_SCREEN;
static int tasks = 0;
Font font;
Font thinFont;

Task array[128];

Task getTask(int n) {
    return array[n];
}

void addTask(const char *title, int priority, int id) {
    tasks++;

    // initialize the array

    array[id].title = title;
    array[id].priority = priority;
    array[id].id = id;
}

void deleteTask(int id) {
    array[id].id = -1;
}

void reloadTasks() {
    // delete all tasks
    for (int i = 0; i < tasks; i++) {
        deleteTask(i);  // id
    }

    // re-add the tasks
    for (int i = 0; i < getFileLines(getLocation()); i++) {
        array[i].title = loadTaskName(getLocation(), i, 1);            // name
        array[i].priority = strtol(loadTaskName(getLocation(), i, 2),  // prio
                                   NULL, 10);
        array[i].id = strtol(loadTaskName(getLocation(), i, 3), NULL, 10);
    }
}

void loadTasks() {
    for (int i = 0; i < getFileLines(getLocation()); i++) {
        addTask(loadTaskName(getLocation(), i, 1),         // name
                strtol(loadTaskName(getLocation(), i, 2),  // prio
                       NULL, 10),
                strtol(loadTaskName(getLocation(), i, 3), NULL, 10));  // id
    }
}

Task getHoveredTask() {
    Task task = {0};
    task.id = -1;  // to setup for if the task exists checks, just check if the task id is -1
    if (currentScreen == MAIN_SCREEN) {
        for (int i = 0; i < tasks; i++) {
            if (GetMousePosition().y > 96 + (48 * i) && GetMousePosition().y < 96 + (48 * (i + 1))) {
                task = getTask(i);
            }
        }
    }
    return task;
}

int getPriority(char *prio) {
    if (strcmp(prio, "LOW") == 0)
        return 0;
    else if (strcmp(prio, "MEDIUM") == 0)
        return 1;
    else if (strcmp(prio, "HIGH") == 0)
        return 2;
    else
        return -1;
}
void drawTextWithShadow(Font font, const char *title, Vector2 position, int fontSize, int spacing, int offset, Color color) {
    DrawTextEx(font, title, (Vector2){position.x - offset, position.y + offset}, fontSize, spacing, CLITERAL(Color){0, 0, 0, 100});
    DrawTextEx(font, title, position, fontSize, spacing, color);
}

void DrawAlert(int posX, int posY, int width, int height, char *text) {
    Rectangle rec = {.x = posX, .y = posY, .height = height, .width = width};
    Rectangle shadowRec = {.x = posX - 5, .y = posY - 2, .height = height, .width = width};
    int fontSize = 26;
    Vector2 textSize = MeasureTextEx(font, text, fontSize, 0.2);
    DrawRectangleRounded(shadowRec, 0.5, 0, CLITERAL(Color){0, 0, 0, 35});
    DrawRectangleRounded(rec, 0.5, 0, BOX_COLOR);
    DrawTextEx(font, text, (Vector2){posX + width / 2 - textSize.x / 2, posY + (height / 2) - textSize.y / 2}, fontSize, 0.2, WHITE);
}

Texture deleteTexture, tup, tdown, tedit;
void drawTasks() {
    Color circleColor[3] = {GREEN, YELLOW, RED};

    int x = 15;
    int radius = 7;
    int offset = 15;
    int fontSize = 20;
    int dropoff = 0;
    int dX = WIDTH - 32 - 5;  // delete texture X
    int uX = dX - 32 - 5;     // up and down texture X
    int eX = uX - 32 - 5;     // edit texture X
    int _i = 0;               // placeholder int for index i for id
    for (int i = 0; i < tasks; i++) {
        int id = strtol(loadTaskName(getLocation(), _i, 3), NULL, 10);

        Task task = getTask(i);
        bool deleted = false;
        int seconds = strtol(loadTaskName(getLocation(), _i, 4), NULL, 10);
        time_t t = seconds;
        struct tm *time = localtime(&t);
        int y = 96 + (48 * i) + dropoff;
        if (task.id < 0) {
            dropoff -= 48;
            deleted = true;
        }
        if (!deleted) {
            _i++;

            DrawCircle(x, y + MeasureTextEx(font, task.title, fontSize, 2).y / 2, radius, circleColor[task.priority]);
            DrawTextEx(font, task.title, (Vector2){x + radius + offset, y - MeasureTextEx(font, task.title, fontSize, 2).y / 2}, fontSize, 2, RAYWHITE);
            DrawTextEx(thinFont,
                       TextFormat("%d-%02d-%02d %02d:%02d:%02d", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday,
                                  time->tm_hour, time->tm_min, time->tm_sec),
                       (Vector2){x + radius + offset, y + MeasureTextEx(font, task.title, fontSize, 2).y / 2}, fontSize - 2, 2, RAYWHITE);
            DrawTextureEx(deleteTexture, (Vector2){dX, y}, 0, 1, WHITE);
            DrawRectangleLines(dX, y, 32, 32, WHITE);

            DrawTextureEx(tup, (Vector2){uX + (16 * 0.1), y}, 0, 0.9, WHITE);
            DrawTextureEx(tdown, (Vector2){uX + (16 * 0.1), y + 16}, 0, 0.9, WHITE);
            DrawRectangleLines(uX, y, 32, 15, WHITE);
            DrawRectangleLines(uX, y + 16 + 1, 32, 15, WHITE);

            if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){dX, y, 32, 32})) {  // check if delete button was clicked
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    printf("\033[1;31mDeleted task: \033[0m%s\n", task.title);
                    deleteLineFmFile(getLocation(), id);
                    deleteTask(i);
                    _i--;
                }
            }

            // Only draw the edit texture when hovering
            if (GetMousePosition().y > 96 + (48 * i) && GetMousePosition().y < 96 + (48 * (i + 1))) {
                DrawTextureEx(tedit, (Vector2){eX, y}, 0, 0.2, WHITE);
                if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){eX, y, 32, 32}))
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        currentScreen = EDIT_SCREEN;
                    }

                DrawRectangle(0,
                              y - MeasureTextEx(font, task.title, fontSize, 2).y / 2 - 3,
                              WIDTH, 48,
                              CLITERAL(Color){255, 255, 255, 10});  // Draw hover rec, y value is minused to start from text Y
            }

            // collission for up button
            if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){uX, y, 32, 16})) {  // can't use getHoveredTask() cause of a bug and i cant bother to fix it
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    printf("\033[1;31mMoved task: \033[0m%s \033[1;31mup\033[0m\n", task.title);

                    swap(id, -1);
                    reloadTasks();
                }
            }

            // collission for down button
            if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){uX, y + 16, 32, 16})) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    printf("\033[1;31mMoved task: \033[0m%s \033[1;31mdown\033[0m\n", task.title);

                    swap(id, 0);
                    reloadTasks();
                }
            }
        }
    }
}

int main(void) {
    const char *title = "TodoMate";
    InitWindow(WIDTH, HEIGHT, title);  // inits window and setups GPU
    createFile();
    float x, y;
    thinFont = LoadFontEx("resources\\fonts\\AzeretMono-ExtraLight.ttf", FONT_SIZE, 0, 250);
    font = LoadFontEx("resources\\fonts\\AzeretMono-Medium.ttf", FONT_SIZE, 0, 250);
    SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);  // Upscale the font

    Image addButton = LoadImage("resources\\icons\\add.png");  // load into cpu
    Image backImage = LoadImage("resources\\icons\\back.png");
    Image deleteButton = LoadImage("resources\\icons\\del.png");
    Image icon = LoadImage("resources\\icons\\icon.png");  // shitty image I made but it'll do for now
    Image up = LoadImage("resources\\icons\\up.png");
    Image down = LoadImage("resources\\icons\\down.png");
    Image edit = LoadImage("resources\\icons\\edit.png");
    SetWindowIcon(icon);
    UnloadImage(icon);
    Texture2D addTexture = LoadTextureFromImage(addButton);  // load into gpu
    Texture2D backTexture = LoadTextureFromImage(backImage);
    deleteTexture = LoadTextureFromImage(deleteButton);
    tup = LoadTextureFromImage(up);
    tdown = LoadTextureFromImage(down);
    tedit = LoadTextureFromImage(edit);
    UnloadImage(addButton);  // unload from cpu
    UnloadImage(backImage);
    UnloadImage(deleteButton);
    UnloadImage(up);
    UnloadImage(down);
    UnloadImage(edit);

    Color color = RAYWHITE;
    loadTasks();

    x = 15;
    y = 15;
    bool mouseOnText = false;
    int textBoxLetterCount = 0;
    int prioBoxLetterCount = 0;
    int hoveredTextC = 0;
    int hoveredPrioC = 0;
    int hoveredTaskID;
    int padding = 64;

    char textBoxInput[32] = "\0";
    char prioBoxInput[32] = "\0";

    char hoveredText[32] = "\0";
    char hoveredPrio[32] = "\0";

    float addScale = 1.5f;
    float backScale = 0.1f;
    float addX = WIDTH - (32 * addScale);
    float addY = 1;
    int width = MeasureTextEx(font, title, FONT_SIZE, 2).x;
    int height = MeasureTextEx(font, title, FONT_SIZE, 2).y;
    int focus = 0;  // which input box to put focus and typing cursor on, 0 is name box by default

    char prios[3][8] = {"LOW", "MEDIUM", "HIGH"};

    bool inputError = false;
    Rectangle rec = {
        .x = x,
        .y = y,
        .width = width,
        .height = height,
    };

    Rectangle add = {
        .x = addX,
        .y = addY,
        .width = width,
        .height = height,
    };
    Rectangle backButton = {
        .x = 10, .y = 10, .width = 512 * backScale, .height = 512 * backScale};
    Rectangle textBox = {
        .x = 25,
        .y = 10 + 32 + padding,
        .width = 480,
        .height = 40,
    };
    Rectangle prioBox = {
        .x = 25,
        .y = textBox.y * 2,
        .width = 250,
        .height = 40,
    };
    Rectangle saveBox = {
        .x = 25,
        .y = HEIGHT - 100,
        .width = 120,
        .height = 50,
    };
    int frames = 0;
    bool pressed = false;
    clock_t begin = clock();
    double timeSpent = 0.0;
    SetTargetFPS(60);
    SetExitKey(0);  // set to NULL so it doesn't exit by pressing ESCAPE
    while (!WindowShouldClose()) {
        switch (currentScreen) {
            case MAIN_SCREEN: {
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);

                hoveredTaskID = getHoveredTask().id;
                snprintf(hoveredText, 32, getHoveredTask().title);
                snprintf(hoveredPrio, 32, prios[getHoveredTask().priority]);
                hoveredTextC = strlen(hoveredText);
                hoveredPrioC = strlen(hoveredPrio);

                if (CheckCollisionPointRec(GetMousePosition(), add)) {
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        currentScreen = CREATE_SCREEN;
                    }
                }
                if (IsKeyPressed(KEY_R)) {
                    printf("Reloaded tasks\n");

                    reloadTasks();
                    pressed = true;
                }
                if (pressed) {
                    timeSpent = (double)(clock() - begin);
                    DrawAlert(WIDTH / 2 - 512 / 2, HEIGHT - 140, 512, 50, "Tasks have been reloaded!");  // 140 is just a feelsgood number
                    if (timeSpent / CLOCKS_PER_SEC >= 3.0) {                                             // how much time to display the alert for
                        begin = clock();                                                                 // reset clock
                        pressed = false;
                    }
                }
            } break;
            case CREATE_SCREEN: {
                if ((CheckCollisionPointRec(GetMousePosition(), backButton) &&
                     IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ||
                    IsKeyPressed(KEY_ESCAPE)) {
                    focus = 0;

                    currentScreen = MAIN_SCREEN;
                }

                if (CheckCollisionPointRec(GetMousePosition(), textBox) || CheckCollisionPointRec(GetMousePosition(), prioBox))
                    mouseOnText = true;
                else
                    mouseOnText = false;
                if (mouseOnText) {
                    SetMouseCursor(MOUSE_CURSOR_IBEAM);
                } else
                    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                if (IsKeyPressed(KEY_TAB)) {
                    if (focus == 0)
                        focus = 1;
                    else if (focus == 1)
                        focus = 0;
                }
                if (CheckCollisionPointRec(GetMousePosition(), textBox)) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        focus = 0;
                    }
                }

                if (CheckCollisionPointRec(GetMousePosition(), prioBox)) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        focus = 1;
                    }
                }

                int key = GetCharPressed();
                while (key > 0) {
                    if (focus == 0) {
                        if (key >= 32 && key <= 125 && textBoxLetterCount <= 32) {
                            textBoxInput[textBoxLetterCount] = (char)key;
                            textBoxInput[textBoxLetterCount + 1] = 0;  // null terminate
                            textBoxLetterCount++;
                        }
                    } else if (focus == 1) {
                        if (key >= 32 && key <= 125 && prioBoxLetterCount <= 32) {
                            prioBoxInput[prioBoxLetterCount] = (char)key;
                            prioBoxInput[prioBoxLetterCount + 1] = 0;  // null terminate
                            prioBoxLetterCount++;
                        }
                    }
                    key = GetCharPressed();
                }

                if (IsKeyPressed(KEY_BACKSPACE)) {
                    if (focus == 0) {
                        textBoxLetterCount--;
                        if (textBoxLetterCount < 0) textBoxLetterCount = 0;
                        textBoxInput[textBoxLetterCount] = 0;
                    } else if (focus == 1) {
                        prioBoxLetterCount--;
                        if (prioBoxLetterCount < 0) prioBoxLetterCount = 0;
                        prioBoxInput[prioBoxLetterCount] = 0;
                    }
                }

                if (IsKeyPressed(KEY_ENTER) ||
                    (CheckCollisionPointRec(GetMousePosition(), saveBox) &&
                     IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
                    strcpy(prioBoxInput, TextToUpper(prioBoxInput));
                    if (strcmp(prioBoxInput, "LOW") == 0 || strcmp(prioBoxInput, "MEDIUM") == 0 || strcmp(prioBoxInput, "HIGH") == 0) {
                        currentScreen = MAIN_SCREEN;
                        appendToCfg(textBoxInput, getPriority(prioBoxInput));
                        addTask(loadTaskName(getLocation(), getFileLines(getLocation()), 1),
                                strtol(loadTaskName(getLocation(), getFileLines(getLocation()), 2), NULL, 10),
                                getFileLines(getLocation()) - 1);
                        reloadTasks();
                        printf("\033[1;32mCreated task: \033[0m%s\n",
                               loadTaskName(getLocation(), getFileLines(getLocation()), 1));
                        focus = 0;
                        if (inputError) {
                            inputError = false;
                        }

                    } else {
                        inputError = true;
                    }
                }

            } break;
            case EDIT_SCREEN: {
                if ((CheckCollisionPointRec(GetMousePosition(), backButton) &&
                     IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ||
                    IsKeyPressed(KEY_ESCAPE)) {
                    focus = 0;

                    currentScreen = MAIN_SCREEN;
                }

                if (CheckCollisionPointRec(GetMousePosition(), textBox) || CheckCollisionPointRec(GetMousePosition(), prioBox))
                    mouseOnText = true;
                else
                    mouseOnText = false;
                if (mouseOnText) {
                    SetMouseCursor(MOUSE_CURSOR_IBEAM);
                } else
                    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                if (IsKeyPressed(KEY_TAB)) {
                    if (focus == 0)
                        focus = 1;
                    else if (focus == 1)
                        focus = 0;
                }
                if (CheckCollisionPointRec(GetMousePosition(), textBox)) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        focus = 0;
                    }
                }

                if (CheckCollisionPointRec(GetMousePosition(), prioBox)) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        focus = 1;
                    }
                }

                int key = GetCharPressed();
                while (key > 0) {
                    if (focus == 0) {
                        if (key >= 32 && key <= 125 && hoveredTextC <= 32) {
                            hoveredText[hoveredTextC] = (char)key;
                            hoveredText[hoveredTextC + 1] = 0;  // null terminate
                            hoveredTextC++;
                        }
                    } else if (focus == 1) {
                        if (key >= 32 && key <= 125 && hoveredPrioC <= 32) {
                            hoveredPrio[hoveredPrioC] = (char)key;
                            hoveredPrio[hoveredPrioC + 1] = 0;  // null terminate
                            hoveredPrioC++;
                        }
                    }
                    key = GetCharPressed();
                }

                if (IsKeyPressed(KEY_BACKSPACE)) {
                    if (focus == 0) {
                        hoveredTextC--;
                        if (hoveredTextC < 0) hoveredTextC = 0;
                        hoveredText[hoveredTextC] = 0;
                    } else if (focus == 1) {
                        hoveredPrioC--;
                        if (hoveredPrioC < 0) hoveredPrioC = 0;
                        hoveredPrio[hoveredPrioC] = 0;
                    }
                }

                if (IsKeyPressed(KEY_ENTER) ||
                    (CheckCollisionPointRec(GetMousePosition(), saveBox) &&
                     IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
                    strcpy(hoveredPrio, TextToUpper(hoveredPrio));
                    if (strcmp(hoveredPrio, "LOW") == 0 || strcmp(hoveredPrio, "MEDIUM") == 0 || strcmp(hoveredPrio, "HIGH") == 0) {
                        currentScreen = MAIN_SCREEN;
                        editName(hoveredTaskID, hoveredText);
                        editPriority(hoveredTaskID, hoveredPrio);
                        reloadTasks();
                        printf("\033[1;33mEdited task: \033[0m%s\n",
                               loadTaskName(getLocation(), getFileLines(getLocation()), 1));
                        focus = 0;
                        if (inputError) {
                            inputError = false;
                        }

                    } else {
                        inputError = true;
                    }
                }

            } break;
        }
        frames++;
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        switch (currentScreen) {
            case MAIN_SCREEN: {
                if (CheckCollisionPointRec(GetMousePosition(), rec)) {
                    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                        color = (Color){175, 175, 175, 255};
                    } else
                        color = RAYWHITE;
                }
                drawTextWithShadow(font, title, (Vector2){x, y}, FONT_SIZE, 1, 3, color);  // draw title
                DrawLineEx((Vector2){x, MeasureTextEx(font, title, FONT_SIZE, 1).y + y},   // draw title line
                           (Vector2){MeasureTextEx(font, title, FONT_SIZE, 1).x + x,
                                     MeasureTextEx(font, title, FONT_SIZE, 1).y + y},
                           3, color);

                // draw the add button
                DrawTextureEx(addTexture, (Vector2){addX, addY}, 0, addScale, WHITE);
                drawTasks();

            } break;
            case CREATE_SCREEN: {
                DrawTextureEx(backTexture, (Vector2){10, 10}, 0, backScale, WHITE);

                DrawTextEx(font, "Name: ", (Vector2){25, 10 + padding}, 24, 2, WHITE);
                DrawRectangleRounded(textBox, 0.2f, 0, WHITE);
                DrawTextEx(font, textBoxInput, (Vector2){textBox.x + 3, textBox.y + 8}, 24, 0, BLACK);

                DrawTextEx(font, "Priority: ", (Vector2){25, prioBox.y - padding + 25}, 24, 2, WHITE);
                DrawRectangleRounded(prioBox, 0.2f, 0, WHITE);
                DrawTextEx(font, prioBoxInput, (Vector2){prioBox.x + 3 + 5, prioBox.y + 8}, 24, 0, BLACK);

                DrawRectangleRounded(saveBox, 0.5f, 0, CLITERAL(Color){47, 158, 0, 255});
                DrawTextEx(font, "SAVE", (Vector2){(saveBox.x + saveBox.width / 2) - saveBox.x, saveBox.height / 4 + saveBox.y}, 24, 0, WHITE);
                if (inputError) {
                    DrawTextEx(font, "Input a proper priority(LOW, MEDIUM, HIGH)", (Vector2){prioBox.x, prioBox.y + prioBox.height + 10}, 24, 0, RED);
                }
                if ((frames / 30) % 2 == 0) {
                    if (focus == 0) {
                        DrawText("|", textBox.x + MeasureTextEx(font, textBoxInput, 24, 0).x + 8, textBox.y + 8, 24, BLACK);

                    } else if (focus == 1) {
                        DrawText("|", prioBox.x + MeasureTextEx(font, prioBoxInput, 24, 0).x + 8, prioBox.y + 8, 24, BLACK);
                    }
                }

            } break;
            case EDIT_SCREEN: {
                DrawTextureEx(backTexture, (Vector2){10, 10}, 0, backScale, WHITE);

                DrawTextEx(font, "Name: ", (Vector2){25, 10 + padding}, 24, 2, WHITE);
                DrawRectangleRounded(textBox, 0.2f, 0, WHITE);
                DrawTextEx(font, hoveredText, (Vector2){textBox.x + 3, textBox.y + 8}, 24, 0, BLACK);

                DrawTextEx(font, "Priority: ", (Vector2){25, prioBox.y - padding + 25}, 24, 2, WHITE);
                DrawRectangleRounded(prioBox, 0.2f, 0, WHITE);
                DrawTextEx(font, hoveredPrio, (Vector2){prioBox.x + 3 + 5, prioBox.y + 8}, 24, 0, BLACK);

                DrawRectangleRounded(saveBox, 0.5f, 0, CLITERAL(Color){47, 158, 0, 255});
                DrawTextEx(font, "SAVE", (Vector2){(saveBox.x + saveBox.width / 2) - saveBox.x, saveBox.height / 4 + saveBox.y}, 24, 0, WHITE);
                if (inputError) {
                    DrawTextEx(font, "Input a proper priority(LOW, MEDIUM, HIGH)", (Vector2){prioBox.x, prioBox.y + prioBox.height + 10}, 24, 0, RED);
                }
                if ((frames / 30) % 2 == 0) {
                    if (focus == 0) {
                        DrawText("|", textBox.x + MeasureTextEx(font, hoveredText, 24, 0).x + 8, textBox.y + 8, 24, BLACK);

                    } else if (focus == 1) {
                        DrawText("|", prioBox.x + MeasureTextEx(font, hoveredPrio, 24, 0).x + 8, prioBox.y + 8, 24, BLACK);
                    }
                }

            } break;
        }

        EndDrawing();
    }
    UnloadTexture(addTexture);
    UnloadTexture(backTexture);
    UnloadTexture(deleteTexture);
    UnloadTexture(tup);
    UnloadTexture(tdown);
    UnloadTexture(tedit);
    UnloadFont(font);
    CloseWindow();
    return 0;
}

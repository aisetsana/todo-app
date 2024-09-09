#include "../include/raylib.h"

#define WIDTH 800
#define HEIGHT 450
#define BACKGROUND_COLOR \
    CLITERAL(Color) { 38, 38, 38, 255 }

int main(void) {
    InitWindow(WIDTH, HEIGHT, "Todo App");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BACKGROUND_COLOR);

        DrawText("Congrats! You created your first window!", 190, 200, 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
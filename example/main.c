//
// Created by blomq on 2026-06-19.
//
#include <raylib.h>

#include "MS/MS_Minefield.h"

const int TILE_SIZE = 24;
const float TILE_SIZE_TEX = 16.0f;

const int FIELD_SIZE = 10;
const int FIELD_MINES = 10;

int main(int argc, char **argv) {
    InitWindow(FIELD_SIZE * TILE_SIZE, FIELD_SIZE * TILE_SIZE, "Minesweeper");
    SetTargetFPS(60);

    MS_Minefield minefield;
    MS_MinefieldCreate(&minefield, FIELD_SIZE, FIELD_SIZE, FIELD_MINES);

    const Texture2D tilesTex = LoadTexture("tiles.png");

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_R)) {
            if (MS_MinefieldIsGameOver(&minefield) || MS_MinefieldIsWin(&minefield)) {
                MS_MinefieldReset(&minefield, FIELD_SIZE, FIELD_SIZE, FIELD_MINES);
                SetWindowTitle("Minesweeper");
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            const int x = GetMouseX() / TILE_SIZE;
            const int y = GetMouseY() / TILE_SIZE;
            MS_GameState gameState = MS_MinefieldOpenTile(&minefield, x, y);

            if (gameState == MINESWEEPER_STATE_LOST) {
                SetWindowTitle("Minesweeper - Game Over!");
            }
            if (gameState == MINESWEEPER_STATE_WON) {
                SetWindowTitle("Minesweeper - You Won!");
            }
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            const int x = GetMouseX() / TILE_SIZE;
            const int y = GetMouseY() / TILE_SIZE;
            MS_MinefieldToggleFlag(&minefield, x, y);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int y = 0; y < FIELD_SIZE; ++y) {
            for (int x = 0; x < FIELD_SIZE; ++x) {
                const int tileTexIndex = MS_MinefieldGetTileSprite(&minefield, x, y);
                const Rectangle sourceRect = (Rectangle){
                    (float) (tileTexIndex % 4) * TILE_SIZE_TEX, (float) (tileTexIndex / 4) * TILE_SIZE_TEX,
                    TILE_SIZE_TEX, TILE_SIZE_TEX
                };
                const Rectangle destRect = (Rectangle){x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                DrawTexturePro(tilesTex, sourceRect, destRect, (Vector2){0, 0}, 0, WHITE);
            }
        }

        EndDrawing();
    }

    UnloadTexture(tilesTex);
    MS_MinefieldDestroy(&minefield);

    CloseWindow();
}

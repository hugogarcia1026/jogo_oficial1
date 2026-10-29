#include "raylib.h"
#include <algorithm>
#include <cstdio>

constexpr int TOTAL = 9;

int main() {
    const char* arquivos[TOTAL] = {
        "Rock001.glb", "Rock002.glb", "Rock003.glb",
        "Cliff001.glb", "Cliff002.glb", "Cliff003.glb",
        "TreeLow001.glb", "Bush001.glb", "Grass001.glb"
    };

    Model modelos[TOTAL] = {};
    float escalas[TOTAL] = {};
    float centrosX[TOTAL] = {};
    float basesY[TOTAL] = {};
    float centrosZ[TOTAL] = {};
    bool carregados[TOTAL] = {};

    InitWindow(1100, 750, "ECLIPSE - Cena de teste de assets");
    SetTargetFPS(60);

    int totalCarregados = 0;

    for (int i = 0; i < TOTAL; i++) {
        char caminho[256];
        std::snprintf(caminho, sizeof(caminho),
                      "assets/models/environment/%s", arquivos[i]);

        modelos[i] = LoadModel(caminho);

        if (modelos[i].meshCount <= 0) {
            TraceLog(LOG_WARNING, "Falha ao carregar: %s", arquivos[i]);
            continue;
        }

        BoundingBox b = GetModelBoundingBox(modelos[i]);

        float dx = b.max.x - b.min.x;
        float dy = b.max.y - b.min.y;
        float dz = b.max.z - b.min.z;
        float maior = std::max({dx, dy, dz});

        if (maior <= 0.0001f) {
            UnloadModel(modelos[i]);
            modelos[i] = {};
            continue;
        }

        escalas[i] = 2.0f / maior;
        centrosX[i] = (b.min.x + b.max.x) * 0.5f;
        basesY[i] = b.min.y;
        centrosZ[i] = (b.min.z + b.max.z) * 0.5f;
        carregados[i] = true;
        totalCarregados++;
    }

    Camera3D camera = {0};
    camera.position = {12.0f, 10.0f, 14.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();
        ClearBackground(Color{18, 17, 29, 255});

        BeginMode3D(camera);
        DrawGrid(30, 1.0f);

        for (int i = 0; i < TOTAL; i++) {
            if (!carregados[i]) continue;

            float x = (i % 3 - 1) * 5.0f;
            float z = (i / 3 - 1) * 5.0f;

            Vector3 posicao = {
                x - centrosX[i] * escalas[i],
                -basesY[i] * escalas[i],
                z - centrosZ[i] * escalas[i]
            };

            DrawModel(modelos[i], posicao, escalas[i], WHITE);
        }

        EndMode3D();

        DrawText("ECLIPSE - TESTE DE ASSETS", 20, 18, 24, RAYWHITE);
        DrawText(TextFormat("Modelos carregados: %d/%d",
                            totalCarregados, TOTAL),
                 20, 50, 18, LIGHTGRAY);
        DrawText("Mouse: orbitar camera | ESC: sair",
                 20, 78, 16, LIGHTGRAY);

        for (int i = 0; i < TOTAL; i++) {
            int coluna = i % 3;
            int linha = i / 3;
            Color cor = carregados[i] ? GREEN : RED;

            DrawText(arquivos[i],
                     20 + coluna * 220,
                     650 + linha * 25,
                     15, cor);
        }

        EndDrawing();
    }

    for (int i = 0; i < TOTAL; i++) {
        if (carregados[i]) UnloadModel(modelos[i]);
    }

    CloseWindow();
    return 0;
}

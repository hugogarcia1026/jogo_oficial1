#pragma once

#include "raylib.h"
#include <cmath>

// Horizonte procedural em 360 graus.
// As formações acompanham o jogador para manter a continuidade visual.
inline void DesenharAtmosfera(Vector3 centroMundo, Vector3 jogador, float tempo)
{
    const float PI_LOCAL = 3.14159265358979323846f;

    const Color fundo = {20, 23, 34, 255};
    const Color meio = {28, 30, 40, 255};
    const Color frente = {36, 35, 43, 255};

    // Camada mais distante: uma silhueta quase contínua no horizonte.
    // Variações determinísticas evitam que os picos sejam todos iguais.
    for (int i = 0; i < 48; ++i)
    {
        const float angulo = (2.0f * PI_LOCAL * i) / 48.0f;
        const float variacao = std::sin(i * 2.17f) * 3.0f
                             + std::cos(i * 0.73f) * 2.0f;

        const float distancia = 230.0f + std::sin(i * 1.31f) * 7.0f;
        const float altura = 12.0f + variacao;
        const float x = centroMundo.x + std::cos(angulo) * distancia;
        const float z = centroMundo.z + std::sin(angulo) * distancia;

        DrawCylinder(
            {x, -1.0f + altura * 0.5f, z},
            0.0f,
            8.0f + std::fmod(static_cast<float>(i * 7), 5.0f),
            altura,
            16,
            fundo);
    }

    // Picos adicionais e deslocados para quebrar a regularidade do anel.
    for (int i = 0; i < 32; ++i)
    {
        const float angulo = (2.0f * PI_LOCAL * (i + 0.45f)) / 32.0f;
        const float distancia = 205.0f + std::sin(i * 1.9f) * 9.0f;
        const float altura = 7.0f + std::fmod(
            static_cast<float>(i * 13), 8.0f);

        const float x = centroMundo.x + std::cos(angulo) * distancia;
        const float z = centroMundo.z + std::sin(angulo) * distancia;

        DrawCylinder(
            {x, -0.5f + altura * 0.5f, z},
            0.0f,
            6.0f + std::fmod(static_cast<float>(i * 3), 4.0f),
            altura,
            16,
            meio);
    }

    // Formações intermediárias nas laterais do vale.
    // Elas permanecem fora da área central para não encobrir o jogador.
    for (int lado = -1; lado <= 1; lado += 2)
    {
        for (int i = 0; i < 12; ++i)
        {
            const float z = centroMundo.z - 34.0f + i * 6.0f;
            const float x = centroMundo.x + lado * (
                25.0f + 2.5f * std::sin(i * 1.7f));

            const float altura = 4.0f + std::fmod(
                static_cast<float>(i * 7 + lado * 3), 5.0f);

            DrawCubeV(
                {x, altura * 0.5f - 0.2f, z},
                {3.5f + std::fmod(static_cast<float>(i), 2.0f),
                 altura,
                 5.0f + std::fmod(static_cast<float>(i * 3), 3.0f)},
                frente);
        }
    }

    // Energia violeta discreta espalhada pelo vale.
    // As posições são relativas ao jogador; as partículas flutuam suavemente.
    for (int i = 0; i < 28; ++i)
    {
        const float fase = tempo * (0.35f + (i % 4) * 0.08f);
        const float angulo = i * 2.39996f + fase * 0.12f;
        const float distancia = 7.0f + (i % 6) * 2.4f;

        const float x = jogador.x + std::cos(angulo) * distancia;
        const float z = jogador.z + std::sin(angulo) * distancia - 8.0f;
        const float y = 0.8f + (i % 6) * 0.72f
                      + std::sin(tempo * 0.9f + i) * 0.22f;

        DrawSphere(
            {x, y, z},
            0.075f,
            Fade(Color{135, 55, 220, 255}, 0.18f));

        DrawSphere(
            {x, y, z},
            0.028f,
            Fade(Color{205, 155, 255, 255}, 0.82f));
    }
}
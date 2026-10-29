
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "atmosfera.h"
#include <cmath>

constexpr int LARGURA = 1280;
constexpr int ALTURA = 720;

constexpr float VELOCIDADE_JOGADOR = 5.0f;
constexpr float VELOCIDADE_CORRIDA = 8.0f;
constexpr float CUSTO_ESQUIVA = 28.0f;
constexpr float DURACAO_ESQUIVA = 0.24f;
constexpr float VELOCIDADE_ESQUIVA = 22.0f;
constexpr float RECARGA_ESQUIVA = 0.60f;

struct Jogador {
    Vector3 posicao;
    float rotacao = 0.0f;
    float resistencia = 100.0f;
    float tempoEsquiva = 0.0f;
    float recargaEsquiva = 0.0f;
    Vector3 direcaoEsquiva = {0.0f, 0.0f, -1.0f};
    float fasePassos = 0.0f;
    float intensidadeMovimento = 0.0f;
    float inclinacao = 0.0f;
};

Vector3 Suavizar(Vector3 atual, Vector3 destino, float velocidade, float dt) {
    return Vector3Lerp(atual, destino, 1.0f - expf(-velocidade * dt));
}

float SuavizarAngulo(float atual, float destino, float velocidade, float dt) {
    float diferenca = fmodf(destino - atual + 540.0f, 360.0f) - 180.0f;
    return atual + diferenca * (1.0f - expf(-velocidade * dt));
}

float Interpolar(float atual, float destino, float velocidade, float dt) {
    return atual + (destino - atual) * (1.0f - expf(-velocidade * dt));
}

Vector3 Frente(float rotacao) {
    return {
        -sinf(rotacao * DEG2RAD),
        0.0f,
        -cosf(rotacao * DEG2RAD)
    };
}

// ============================================================
// PERSONAGEM
// ============================================================

void DesenharMembro(
    Vector3 junta,
    float rotacaoY,
    float rotacaoX,
    Vector3 deslocamentoLocal,
    Vector3 tamanho,
    Color cor
) {
    rlPushMatrix();

    rlTranslatef(junta.x, junta.y, junta.z);
    rlRotatef(rotacaoY, 0.0f, 1.0f, 0.0f);
    rlRotatef(rotacaoX, 1.0f, 0.0f, 0.0f);

    rlTranslatef(
        deslocamentoLocal.x,
        deslocamentoLocal.y,
        deslocamentoLocal.z
    );

    DrawCubeV({0.0f, 0.0f, 0.0f}, tamanho, cor);

    rlPopMatrix();
}

// ============================================================
// CENARIO
// ============================================================

void DesenharRuina(Vector3 posicao, Vector3 tamanho, Color cor) {
    DrawCubeV(posicao, tamanho, cor);
    DrawCubeWiresV(posicao, tamanho, Fade(BLACK, 0.25f));
}

void DesenharChao() {
    const Color terraEscura = {35, 34, 42, 255};
    const Color terraMedia = {43, 41, 49, 255};
    const Color pedraClara = {72, 68, 72, 255};
    const Color pedraMedia = {57, 54, 61, 255};
    const Color pedraEscura = {38, 37, 44, 255};
    const Color violetaFraco = {83, 53, 112, 255};

    // Base do terreno.
    DrawPlane(
        {0.0f, -0.12f, 0.0f},
        {100.0f, 100.0f},
        terraEscura
    );

    // Caminho antigo de pedra.
    for (int i = 0; i < 34; ++i) {
        float indice = static_cast<float>(i);
        float z = 17.0f - indice * 1.45f;
        float desvio = sinf(indice * 0.34f) * 0.65f;
        float largura = 3.0f + sinf(indice * 0.71f) * 0.55f;
        float altura = 0.10f + static_cast<float>(i % 3) * 0.025f;

        Color cor = pedraMedia;

        if (i % 5 == 0) {
            cor = pedraClara;
        } else if (i % 4 == 0) {
            cor = pedraEscura;
        }

        DrawCubeV(
            {desvio, -0.045f + altura * 0.5f, z},
            {largura, altura, 1.35f},
            cor
        );

        if (i % 3 == 0) {
            DrawCubeV(
                {desvio + largura * 0.28f, 0.015f, z + 0.25f},
                {0.035f, 0.012f, 0.65f},
                terraEscura
            );
        }
    }

    // Placas de pedra espalhadas pelo terreno.
    for (int i = 0; i < 64; ++i) {
        float x = static_cast<float>((i * 37) % 53) - 26.0f;
        float z = static_cast<float>((i * 29) % 61) - 31.0f;

        if (fabsf(x) < 2.2f && z > -24.0f && z < 16.0f) {
            continue;
        }

        float largura = 0.45f + static_cast<float>(i % 4) * 0.24f;
        float profundidade = 0.45f + static_cast<float>((i * 3) % 4) * 0.20f;
        float altura = 0.08f + static_cast<float>(i % 3) * 0.055f;
        float rotacao = static_cast<float>((i * 31) % 35) - 17.0f;

        Color cor = pedraMedia;

        if (i % 7 == 0) {
            cor = pedraClara;
        } else if (i % 5 == 0) {
            cor = pedraEscura;
        }

        rlPushMatrix();
        rlTranslatef(x, -0.02f + altura * 0.5f, z);
        rlRotatef(rotacao, 0.0f, 1.0f, 0.0f);

        DrawCubeV(
            {0.0f, 0.0f, 0.0f},
            {largura, altura, profundidade},
            cor
        );

        rlPopMatrix();
    }

    // Rochas e destroços maiores.
    for (int i = 0; i < 28; ++i) {
        float x = static_cast<float>((i * 19) % 47) - 23.0f;
        float z = static_cast<float>((i * 23) % 49) - 25.0f;

        if (fabsf(x) < 2.8f && z > -23.0f && z < 15.0f) {
            continue;
        }

        float tamanho = 0.35f + static_cast<float>(i % 4) * 0.18f;
        float altura = 0.25f + static_cast<float>(i % 3) * 0.16f;
        Color cor = (i % 4 == 0) ? pedraMedia : pedraEscura;

        DrawCubeV(
            {x, altura * 0.5f - 0.015f, z},
            {tamanho, altura, tamanho * 1.15f},
            cor
        );

        if (i % 3 == 0) {
            DrawCubeV(
                {x + tamanho * 0.65f, 0.035f, z + tamanho * 0.45f},
                {tamanho * 1.25f, 0.07f, tamanho * 0.75f},
                pedraMedia
            );
        }
    }

    // Fendas violetas antigas.
    for (int i = 0; i < 10; ++i) {
        float lado = (i % 2 == 0) ? -1.0f : 1.0f;
        float x = lado * (3.5f + static_cast<float>((i * 7) % 12));
        float z = 10.0f - static_cast<float>(i) * 4.1f;
        float comprimento = 0.55f + static_cast<float>(i % 3) * 0.35f;

        DrawCubeV(
            {x, -0.005f, z},
            {0.045f, 0.018f, comprimento},
            Fade(violetaFraco, 0.72f)
        );

        if (i % 2 == 0) {
            DrawCubeV(
                {x + 0.12f, -0.004f, z + 0.18f},
                {0.22f, 0.016f, 0.035f},
                Fade(violetaFraco, 0.55f)
            );
        }
    }

    // Fragmentos pequenos no solo.
    for (int i = 0; i < 36; ++i) {
        float x = static_cast<float>((i * 13) % 49) - 24.0f;
        float z = static_cast<float>((i * 17) % 55) - 27.0f;

        if (fabsf(x) < 2.0f && z > -22.0f && z < 14.0f) {
            continue;
        }

        float escala = 0.12f + static_cast<float>(i % 4) * 0.07f;

        DrawCubeV(
            {x, -0.015f + escala * 0.25f, z},
            {escala * 1.8f, escala * 0.5f, escala},
            (i % 2 == 0) ? pedraMedia : terraMedia
        );
    }
}

void DesenharRuinas() {
    const Color pedra = {69, 65, 64, 255};
    const Color pedraEscura = {43, 43, 48, 255};
    const Color pedraClara = {91, 84, 80, 255};
    const Color violeta = {103, 65, 145, 255};

    // Colunatas principais à esquerda.
    DesenharRuina(
        {-8.0f, 2.0f, -7.0f},
        {1.8f, 4.0f, 1.8f},
        pedra
    );

    DesenharRuina(
        {-10.0f, 2.6f, -7.0f},
        {1.8f, 5.2f, 1.8f},
        pedraEscura
    );

    DesenharRuina(
        {-8.0f, 4.2f, -7.0f},
        {2.4f, 0.45f, 2.4f},
        pedra
    );

    // Colunatas principais à direita.
    DesenharRuina(
        {8.0f, 2.5f, -10.0f},
        {1.8f, 5.0f, 1.8f},
        pedraEscura
    );

    DesenharRuina(
        {10.0f, 1.7f, -10.0f},
        {1.8f, 3.4f, 1.8f},
        pedra
    );

    DesenharRuina(
        {8.0f, 5.1f, -10.0f},
        {2.2f, 0.4f, 2.2f},
        pedra
    );

    // Muros desabados.
    DesenharRuina(
        {-5.0f, 1.0f, -15.0f},
        {4.0f, 2.0f, 1.0f},
        pedra
    );

    DesenharRuina(
        {6.0f, 0.8f, 1.0f},
        {3.0f, 1.6f, 1.4f},
        pedraEscura
    );

    DesenharRuina(
        {-13.0f, 1.0f, -18.0f},
        {1.0f, 2.0f, 4.0f},
        pedra
    );

    // Fragmentos de um arco antigo.
    DesenharRuina(
        {-3.9f, 3.0f, -12.0f},
        {0.65f, 3.6f, 0.75f},
        pedraEscura
    );

    DesenharRuina(
        {-1.8f, 4.2f, -12.0f},
        {0.65f, 1.2f, 0.75f},
        pedra
    );

    DesenharRuina(
        {-2.8f, 4.8f, -12.0f},
        {2.5f, 0.55f, 0.85f},
        pedraClara
    );

    // Coluna quebrada caída.
    rlPushMatrix();
    rlTranslatef(11.5f, 0.55f, -3.0f);
    rlRotatef(18.0f, 0.0f, 0.0f, 1.0f);

    DrawCubeV(
        {0.0f, 0.0f, 0.0f},
        {3.2f, 0.75f, 0.85f},
        pedraEscura
    );

    DrawCubeWiresV(
        {0.0f, 0.0f, 0.0f},
        {3.2f, 0.75f, 0.85f},
        Fade(BLACK, 0.3f)
    );

    rlPopMatrix();

    // Blocos menores junto às estruturas.
    for (int i = 0; i < 14; ++i) {
        float x = static_cast<float>((i * 11) % 25) - 12.0f;
        float z = -4.0f - static_cast<float>((i * 7) % 17);
        float tamanho = 0.35f + static_cast<float>(i % 3) * 0.18f;
        float altura = 0.25f + static_cast<float>(i % 4) * 0.12f;

        if (fabsf(x) < 2.8f) {
            x += (x < 0.0f) ? -3.0f : 3.0f;
        }

        DrawCubeV(
            {x, altura * 0.5f, z},
            {tamanho * 1.4f, altura, tamanho},
            (i % 2 == 0) ? pedra : pedraEscura
        );
    }

    // Marcas violetas nas bases das estruturas.
    DrawCubeV(
        {-8.0f, 0.22f, -5.98f},
        {0.08f, 0.035f, 0.48f},
        Fade(violeta, 0.85f)
    );

    DrawCubeV(
        {8.0f, 0.22f, -8.98f},
        {0.08f, 0.035f, 0.48f},
        Fade(violeta, 0.85f)
    );

    DrawCubeV(
        {-5.0f, 0.18f, -14.45f},
        {0.55f, 0.025f, 0.06f},
        Fade(violeta, 0.65f)
    );
}

// ============================================================
// COLOSSO AZH'KHAR
// ============================================================

void DesenharColosso(Vector3 base, float tempo) {
    Color pedra = {82, 77, 74, 255};
    Color pedraEscura = {43, 42, 46, 255};
    Color rachadura = {170, 91, 49, 255};
    Color olho = {255, 119, 54, 255};

    float respiracao = sinf(tempo * 0.8f) * 0.10f;
    float balanco = sinf(tempo * 0.35f) * 0.12f;

    DrawCylinder(
        {base.x, base.y + 0.02f, base.z},
        4.1f, 4.8f, 0.06f, 24,
        Fade(BLACK, 0.38f)
    );

    DrawCubeV(
        {base.x - 1.0f, base.y + 2.0f, base.z},
        {1.4f, 4.0f, 1.6f},
        pedraEscura
    );

    DrawCubeV(
        {base.x + 1.0f, base.y + 2.0f, base.z},
        {1.4f, 4.0f, 1.6f},
        pedraEscura
    );

    DrawCubeV(
        {base.x - 1.0f, base.y + 0.42f, base.z + 0.3f},
        {2.0f, 0.84f, 2.5f},
        pedra
    );

    DrawCubeV(
        {base.x + 1.0f, base.y + 0.42f, base.z + 0.3f},
        {2.0f, 0.84f, 2.5f},
        pedra
    );

    DrawCubeV(
        {base.x + balanco, base.y + 7.0f + respiracao, base.z},
        {4.5f, 6.0f, 2.5f},
        pedra
    );

    DrawCubeWiresV(
        {base.x + balanco, base.y + 7.0f + respiracao, base.z},
        {4.5f, 6.0f, 2.5f},
        Fade(pedraEscura, 0.8f)
    );

    DrawCubeV(
        {base.x - 3.0f, base.y + 9.0f, base.z},
        {2.0f, 2.2f, 2.2f},
        pedraEscura
    );

    DrawCubeV(
        {base.x + 3.0f, base.y + 9.0f, base.z},
        {2.0f, 2.2f, 2.2f},
        pedraEscura
    );

    DrawCubeV(
        {base.x - 3.1f, base.y + 5.9f, base.z + 0.1f},
        {1.3f, 4.2f, 1.5f},
        pedra
    );

    DrawCubeV(
        {base.x + 3.1f, base.y + 5.9f, base.z + 0.1f},
        {1.3f, 4.2f, 1.5f},
        pedra
    );

    DrawCubeV(
        {base.x - 3.1f, base.y + 3.7f, base.z + 0.2f},
        {1.6f, 1.5f, 1.8f},
        pedraEscura
    );

    DrawCubeV(
        {base.x + 3.1f, base.y + 3.7f, base.z + 0.2f},
        {1.6f, 1.5f, 1.8f},
        pedraEscura
    );

    DrawCubeV(
        {base.x, base.y + 10.2f, base.z},
        {1.6f, 1.5f, 1.6f},
        pedraEscura
    );

    DrawCubeV(
        {base.x, base.y + 11.5f, base.z - 0.15f},
        {2.2f, 2.2f, 2.0f},
        pedra
    );

    DrawCubeV(
        {base.x - 0.9f, base.y + 12.7f, base.z},
        {0.45f, 1.2f, 0.6f},
        pedraEscura
    );

    DrawCubeV(
        {base.x + 0.9f, base.y + 12.7f, base.z},
        {0.45f, 1.2f, 0.6f},
        pedraEscura
    );

    DrawSphere(
        {base.x - 0.48f, base.y + 11.65f, base.z - 1.02f},
        0.16f, olho
    );

    DrawSphere(
        {base.x + 0.48f, base.y + 11.65f, base.z - 1.02f},
        0.16f, olho
    );

    DrawCubeV(
        {base.x, base.y + 7.6f, base.z - 1.28f},
        {0.22f, 2.0f, 0.08f},
        rachadura
    );

    DrawCubeV(
        {base.x + 0.45f, base.y + 7.1f, base.z - 1.29f},
        {0.8f, 0.15f, 0.08f},
        rachadura
    );

    DrawCubeV(
        {base.x - 0.4f, base.y + 8.0f, base.z - 1.29f},
        {0.7f, 0.15f, 0.08f},
        rachadura
    );
}

// ============================================================
// JOGADOR
// ============================================================

void DesenharJogador(const Jogador &j, float tempo) {
    float passo = sinf(j.fasePassos) * j.intensidadeMovimento;
    float passoOposto = sinf(j.fasePassos + PI) * j.intensidadeMovimento;

    float saltoVisual = 0.0f;

    if (j.intensidadeMovimento > 0.05f) {
        saltoVisual = fabsf(sinf(j.fasePassos * 2.0f))
                      * 0.07f * j.intensidadeMovimento;
    }

    float respiracao = sinf(tempo * 2.0f) * 0.025f;
    bool esquivando = j.tempoEsquiva > 0.0f;

    Color roupa = esquivando
        ? Color{83, 43, 129, 255}
        : Color{34, 48, 77, 255};

    Color roupaClara = esquivando
        ? Color{139, 87, 193, 255}
        : Color{54, 73, 112, 255};

    Color pele = esquivando
        ? Color{201, 177, 225, 255}
        : Color{190, 166, 143, 255};

    Vector3 frente = Frente(j.rotacao);

    float x = j.posicao.x;
    float y = j.posicao.y + saltoVisual;
    float z = j.posicao.z;
    float alturaTronco = y + 1.12f + respiracao;

    if (esquivando) {
        for (int i = 1; i <= 4; ++i) {
            float distancia = static_cast<float>(i) * 0.32f;

            Vector3 p = Vector3Subtract(
                j.posicao,
                Vector3Scale(j.direcaoEsquiva, distancia)
            );

            unsigned char alpha =
                static_cast<unsigned char>(100 - i * 18);

            DrawCubeV(
                {p.x, p.y + 0.95f, p.z},
                {0.55f, 1.6f, 0.45f},
                Color{110, 46, 190, alpha}
            );
        }

        DrawCylinder(
            {x, j.posicao.y + 0.025f, z},
            0.65f, 1.05f, 0.035f, 24,
            Fade(VIOLET, 0.45f)
        );
    }

    rlPushMatrix();

    rlTranslatef(x, alturaTronco, z);
    rlRotatef(j.rotacao, 0.0f, 1.0f, 0.0f);

    rlRotatef(
        j.inclinacao + (esquivando ? -10.0f : 0.0f),
        1.0f, 0.0f, 0.0f
    );

    DrawCubeV(
        {0.0f, 0.0f, 0.0f},
        {0.72f, 1.02f, 0.43f},
        roupa
    );

    DrawCubeV(
        {0.0f, 0.02f, -0.225f},
        {0.43f, 0.45f, 0.055f},
        roupaClara
    );

    rlPopMatrix();

    Vector3 ombroE = {x - 0.43f, alturaTronco + 0.31f, z};
    Vector3 ombroD = {x + 0.43f, alturaTronco + 0.31f, z};

    float bracoE = esquivando ? -32.0f : passoOposto * 28.0f;
    float bracoD = esquivando ? -32.0f : passo * 28.0f;

    DesenharMembro(
        ombroE, j.rotacao, bracoE,
        {0.0f, -0.36f, 0.0f},
        {0.23f, 0.76f, 0.26f}, roupaClara
    );

    DesenharMembro(
        ombroD, j.rotacao, bracoD,
        {0.0f, -0.36f, 0.0f},
        {0.23f, 0.76f, 0.26f}, roupaClara
    );

    DrawSphere({ombroE.x, ombroE.y - 0.77f, ombroE.z}, 0.12f, pele);
    DrawSphere({ombroD.x, ombroD.y - 0.77f, ombroD.z}, 0.12f, pele);

    DrawCubeV(
        {x, y + 0.59f, z},
        {0.59f, 0.32f, 0.39f},
        DARKGRAY
    );

    Vector3 quadrilE = {x - 0.19f, y + 0.48f, z};
    Vector3 quadrilD = {x + 0.19f, y + 0.48f, z};

    float pernaE = esquivando ? -18.0f : passo * 30.0f;
    float pernaD = esquivando ? 18.0f : passoOposto * 30.0f;

    DesenharMembro(
        quadrilE, j.rotacao, pernaE,
        {0.0f, -0.30f, 0.0f},
        {0.25f, 0.60f, 0.27f}, roupa
    );

    DesenharMembro(
        quadrilD, j.rotacao, pernaD,
        {0.0f, -0.30f, 0.0f},
        {0.25f, 0.60f, 0.27f}, roupa
    );

    Vector3 peE = Vector3Add(
        {x - 0.19f, y + 0.08f, z},
        Vector3Scale(frente, 0.12f)
    );

    Vector3 peD = Vector3Add(
        {x + 0.19f, y + 0.08f, z},
        Vector3Scale(frente, 0.12f)
    );

    DrawCubeV(peE, {0.28f, 0.16f, 0.43f}, BLACK);
    DrawCubeV(peD, {0.28f, 0.16f, 0.43f}, BLACK);

    DrawCubeV(
        {x, alturaTronco + 0.57f, z},
        {0.20f, 0.20f, 0.20f},
        DARKGRAY
    );

    Vector3 cabeca = {x, alturaTronco + 0.87f, z};

    DrawSphere(cabeca, 0.27f, pele);

    DrawSphere(
        {cabeca.x, cabeca.y + 0.10f, cabeca.z + 0.035f},
        0.265f, DARKGRAY
    );

    DrawCubeV(
        Vector3Add(
            cabeca,
            Vector3Add(
                Vector3Scale(frente, 0.22f),
                {0.0f, -0.025f, 0.0f}
            )
        ),
        {0.23f, 0.07f, 0.075f},
        esquivando ? Color{220, 157, 255, 255} : GOLD
    );

    if (esquivando) {
        DrawSphere(
            Vector3Add(
                {x, alturaTronco, z},
                Vector3Scale(frente, 0.24f)
            ),
            0.075f,
            Color{220, 155, 255, 255}
        );
    }
}

// ============================================================
// INTERFACE — ECLIPSE
// ============================================================

Color CorPainel() {
    return {10, 12, 22, 220};
}

Color CorVioleta() {
    return {166, 105, 238, 255};
}

void DesenharPainel(int x, int y, int w, int h) {
    Rectangle r = {
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(w),
        static_cast<float>(h)
    };

    DrawRectangleRounded(r, 0.12f, 8, CorPainel());
    DrawRectangleRoundedLines(r, 0.12f, 8, {142, 111, 190, 190});

    DrawRectangle(x + 10, y + 12, 3, h - 24, CorVioleta());
}

void DesenharBarraHUD(
    int x, int y, int largura, int altura,
    float valor, Color cor
) {
    Rectangle fundo = {
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(largura),
        static_cast<float>(altura)
    };

    DrawRectangleRounded(fundo, 0.5f, 8, {29, 29, 42, 255});

    float fracao = Clamp(valor / 100.0f, 0.0f, 1.0f);
    int preenchimento = static_cast<int>(largura * fracao);

    if (preenchimento > 0) {
        Rectangle barra = {
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(preenchimento),
            static_cast<float>(altura)
        };

        DrawRectangleRounded(barra, 0.5f, 8, cor);

        if (preenchimento > 8) {
            DrawRectangle(
                x + 4, y + 2,
                preenchimento - 8, 2,
                Fade(WHITE, 0.30f)
            );
        }
    }
}

// Mira central discreta, em violeta.
void DesenharMiraHUD() {
    const int cx = LARGURA / 2;
    const int cy = ALTURA / 2;
    const Color violeta = {198, 157, 255, 190};
    const Color sombra = {8, 7, 15, 170};

    DrawCircleLines(cx, cy, 7.0f, sombra);
    DrawCircleLines(cx, cy, 6.0f, violeta);

    DrawLine(cx - 11, cy, cx - 4, cy, sombra);
    DrawLine(cx + 4, cy, cx + 11, cy, sombra);
    DrawLine(cx, cy - 11, cx, cy - 4, sombra);
    DrawLine(cx, cy + 4, cx, cy + 11, sombra);

    DrawLine(cx - 10, cy, cx - 5, cy, violeta);
    DrawLine(cx + 5, cy, cx + 10, cy, violeta);
    DrawLine(cx, cy - 10, cx, cy - 5, violeta);
    DrawLine(cx, cy + 5, cx, cy + 10, violeta);
}

void DesenharHUD(
    const Jogador &j,
    bool movendo,
    bool correndo,
    bool esquivando,
    float tempo,
    float distanciaColosso
) {
    Color branco = {235, 232, 244, 255};
    Color secundario = {165, 165, 184, 255};
    Color violeta = CorVioleta();

    // Sombras para melhorar a leitura da interface.
    DrawRectangleGradientV(
        0, 0, LARGURA, 132,
        Fade(BLACK, 0.82f),
        Fade(BLACK, 0.0f)
    );

    DrawRectangleGradientV(
        0, ALTURA - 145, LARGURA, 145,
        Fade(BLACK, 0.0f),
        Fade(BLACK, 0.88f)
    );

    // Identidade visual.
    DrawCircle(58, 55, 31, Fade(violeta, 0.15f));
    DrawCircleLines(58, 55, 31, violeta);
    DrawPoly({58, 55}, 4, 18, 45.0f, violeta);
    DrawPoly({58, 55}, 4, 9, 45.0f, {20, 17, 34, 255});

    DrawText("ECLIPSE", 101, 24, 28, branco);
    DrawText("ECOS DOS COLOSSOS", 103, 55, 12, secundario);
    DrawText("CAPITULO I  /  VALE DAS RUINAS", 103, 79, 12, GOLD);

    // Painel de resistência e esquiva.
    DesenharPainel(24, 112, 292, 112);

    DrawText("RESISTENCIA", 43, 127, 11, secundario);

    DrawText(
        TextFormat("%03i", static_cast<int>(j.resistencia)),
        264, 125, 16, branco
    );

    Color corResistencia = j.resistencia < 25.0f
        ? Color{225, 74, 83, 255}
        : violeta;

    DesenharBarraHUD(
        43, 148, 246, 11,
        j.resistencia, corResistencia
    );

    DrawText("ESQUIVA SOMBRIA", 43, 173, 11, secundario);

    if (esquivando) {
        DrawText("ATIVA", 242, 173, 11, Color{219, 168, 255, 255});
    } else if (j.recargaEsquiva > 0.0f) {
        DrawText(
            TextFormat("%.1fs", j.recargaEsquiva),
            250, 173, 11, GOLD
        );
    } else if (j.resistencia >= CUSTO_ESQUIVA) {
        DrawText("PRONTA", 241, 173, 11, {160, 225, 190, 255});
    } else {
        DrawText("SEM VIGOR", 224, 173, 10, GOLD);
    }

    // Bússola.
    DrawRectangleRounded(
        {435, 23, 410, 48},
        0.25f, 8, Fade(CorPainel(), 0.88f)
    );

    DrawLine(455, 48, 825, 48, Fade(RAYWHITE, 0.45f));

    DrawText("O", 470, 30, 11, secundario);
    DrawText("N", 633, 28, 13, branco);
    DrawText("L", 784, 30, 11, secundario);

    DrawPoly({640, 48}, 3, 8, 0.0f, GOLD);
    DrawText("VALE DAS RUINAS", 568, 79, 11, branco);

    // Identificação do colosso.
    DesenharPainel(930, 20, 326, 104);

    DrawText("PRESENCA DETECTADA", 951, 33, 10, secundario);
    DrawText("AZH'KHAR", 951, 49, 23, branco);
    DrawText("O COLOSSO ADORMECIDO", 952, 77, 11, {216, 171, 132, 255});

    DrawRectangle(951, 101, 281, 3, Fade(GOLD, 0.75f));

    if (distanciaColosso < 32.0f) {
        float pulso = 4.0f + sinf(tempo * 4.0f) * 1.5f;
        DrawCircle(1232, 41, pulso, {255, 110, 83, 255});
    } else {
        DrawCircle(1232, 41, 4, {120, 190, 150, 255});
    }

    // Objetivo da exploração.
    DesenharPainel(LARGURA - 284, 145, 260, 86);

    DrawText("◇  OBJETIVO ATUAL", LARGURA - 263, 158, 12, GOLD);
    DrawText("Explore as ruinas antigas", LARGURA - 263, 181, 12, branco);
    DrawText("e descubra o eco.", LARGURA - 263, 200, 12, secundario);

    // Estado e comandos de movimento.
    DesenharPainel(24, ALTURA - 112, 292, 84);

    const char *estado = "EM ALERTA";

    if (esquivando) {
        estado = "ESQUIVA SOMBRIA";
    } else if (movendo && correndo) {
        estado = "CORRENDO";
    } else if (movendo) {
        estado = "CAMINHANDO";
    }

    DrawText("ESTADO", 43, ALTURA - 96, 10, secundario);

    DrawText(
        estado, 43, ALTURA - 78, 16,
        esquivando ? Color{219, 168, 255, 255} : branco
    );

    DrawText("WASD", 43, ALTURA - 51, 11, GOLD);
    DrawText("Mover", 88, ALTURA - 51, 11, secundario);
    DrawText("SHIFT", 151, ALTURA - 51, 11, GOLD);
    DrawText("Correr", 197, ALTURA - 51, 11, secundario);

    // Controles de esquiva e câmera.
    DesenharPainel(LARGURA - 284, ALTURA - 112, 260, 84);

    DrawRectangleRounded(
        {static_cast<float>(LARGURA - 263),
         static_cast<float>(ALTURA - 91), 29, 25},
        0.18f, 5, Fade(violeta, 0.25f)
    );

    DrawRectangleRoundedLines(
        {static_cast<float>(LARGURA - 263),
         static_cast<float>(ALTURA - 91), 29, 25},
        0.18f, 5, violeta
    );

    DrawText("SPACE", LARGURA - 261, ALTURA - 84, 9, branco);
    DrawText("Esquiva", LARGURA - 224, ALTURA - 88, 12, branco);
    DrawText("Q / E", LARGURA - 263, ALTURA - 55, 11, GOLD);

    DrawText(
        "Camera | Roda: zoom",
        LARGURA - 218, ALTURA - 55, 11, secundario
    );

    DrawText(
        "ALFA DE DESENVOLVIMENTO",
        505, ALTURA - 27, 10,
        Fade(secundario, 0.72f)
    );

    // Mira desenhada por último para permanecer visível.
    DesenharMiraHUD();
}

// ============================================================
// LOOP PRINCIPAL
// ============================================================

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(LARGURA, ALTURA, "ECLIPSE: Ecos dos Colossos");
    SetTargetFPS(60);

    Jogador jogador;
    jogador.posicao = {0.0f, 0.0f, 2.0f};

    Vector3 azhkhar = {0.0f, 0.0f, -16.0f};

    Camera3D camera = {
        {8.0f, 7.0f, 14.0f},
        {0.0f, 4.0f, -5.0f},
        {0.0f, 1.0f, 0.0f},
        55.0f,
        CAMERA_PERSPECTIVE
    };

    Color ceu = {12, 15, 26, 255};

    float tempo = 0.0f;
    float rotacaoCamera = 0.0f;
    float distanciaCamera = 15.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        tempo += dt;

        Vector3 frenteCamera = {
            -sinf(rotacaoCamera), 0.0f, -cosf(rotacaoCamera)
        };

        Vector3 direitaCamera = {
            cosf(rotacaoCamera), 0.0f, -sinf(rotacaoCamera)
        };

        Vector3 movimento = {0.0f, 0.0f, 0.0f};

        if (IsKeyDown(KEY_W)) {
            movimento = Vector3Add(movimento, frenteCamera);
        }

        if (IsKeyDown(KEY_S)) {
            movimento = Vector3Subtract(movimento, frenteCamera);
        }

        if (IsKeyDown(KEY_D)) {
            movimento = Vector3Add(movimento, direitaCamera);
        }

        if (IsKeyDown(KEY_A)) {
            movimento = Vector3Subtract(movimento, direitaCamera);
        }

        float comprimento = Vector3Length(movimento);
        bool movendo = comprimento > 0.001f;

        if (movendo) {
            movimento = Vector3Scale(movimento, 1.0f / comprimento);
        }

        bool correndo =
            IsKeyDown(KEY_LEFT_SHIFT) ||
            IsKeyDown(KEY_RIGHT_SHIFT);

        jogador.recargaEsquiva = fmaxf(
            0.0f, jogador.recargaEsquiva - dt
        );

        jogador.tempoEsquiva = fmaxf(
            0.0f, jogador.tempoEsquiva - dt
        );

        if (IsKeyPressed(KEY_SPACE) &&
            jogador.tempoEsquiva <= 0.0f &&
            jogador.recargaEsquiva <= 0.0f &&
            jogador.resistencia >= CUSTO_ESQUIVA) {

            Vector3 direcao = movimento;

            if (Vector3Length(direcao) < 0.001f) {
                direcao = Frente(jogador.rotacao);
            }

            jogador.direcaoEsquiva = Vector3Normalize(direcao);
            jogador.tempoEsquiva = DURACAO_ESQUIVA;
            jogador.recargaEsquiva = RECARGA_ESQUIVA;

            jogador.resistencia = fmaxf(
                0.0f, jogador.resistencia - CUSTO_ESQUIVA
            );
        }

        bool esquivando = jogador.tempoEsquiva > 0.0f;

        float intensidadeDestino = 0.0f;

        if (movendo) {
            intensidadeDestino = correndo ? 1.65f : 1.0f;
        }

        if (esquivando) {
            intensidadeDestino = 2.2f;
        }

        jogador.intensidadeMovimento = Interpolar(
            jogador.intensidadeMovimento,
            intensidadeDestino, 10.0f, dt
        );

        if (jogador.intensidadeMovimento > 0.05f) {
            float ritmo = correndo ? 14.0f : 9.0f;

            if (esquivando) {
                ritmo = 20.0f;
            }

            jogador.fasePassos += ritmo * dt;
        }

        float inclinacaoDestino = 0.0f;

        if (movendo && correndo && !esquivando) {
            inclinacaoDestino = -5.0f;
        } else if (movendo && !esquivando) {
            inclinacaoDestino = -1.5f;
        }

        jogador.inclinacao = Interpolar(
            jogador.inclinacao,
            inclinacaoDestino, 7.0f, dt
        );

        // Movimento.
        if (esquivando) {
            jogador.posicao = Vector3Add(
                jogador.posicao,
                Vector3Scale(
                    jogador.direcaoEsquiva,
                    VELOCIDADE_ESQUIVA * dt
                )
            );

            float destino = atan2f(
                -jogador.direcaoEsquiva.x,
                -jogador.direcaoEsquiva.z
            ) * RAD2DEG;

            jogador.rotacao = SuavizarAngulo(
                jogador.rotacao, destino, 18.0f, dt
            );
        } else if (movendo) {
            float velocidade = VELOCIDADE_JOGADOR;

            if (correndo && jogador.resistencia > 0.0f) {
                velocidade = VELOCIDADE_CORRIDA;

                jogador.resistencia = fmaxf(
                    0.0f, jogador.resistencia - 22.0f * dt
                );
            } else {
                jogador.resistencia = fminf(
                    100.0f, jogador.resistencia + 14.0f * dt
                );
            }

            jogador.posicao = Vector3Add(
                jogador.posicao,
                Vector3Scale(movimento, velocidade * dt)
            );

            float destino = atan2f(
                -movimento.x, -movimento.z
            ) * RAD2DEG;

            jogador.rotacao = SuavizarAngulo(
                jogador.rotacao, destino, 10.0f, dt
            );
        } else {
            jogador.resistencia = fminf(
                100.0f, jogador.resistencia + 18.0f * dt
            );
        }

        jogador.posicao.x = Clamp(
            jogador.posicao.x, -25.0f, 25.0f
        );

        jogador.posicao.z = Clamp(
            jogador.posicao.z, -28.0f, 20.0f
        );

        // Câmera.
        if (IsKeyDown(KEY_Q)) {
            rotacaoCamera -= 0.9f * dt;
        }

        if (IsKeyDown(KEY_E)) {
            rotacaoCamera += 0.9f * dt;
        }

        distanciaCamera -= GetMouseWheelMove() * 1.2f;
        distanciaCamera = Clamp(distanciaCamera, 9.0f, 23.0f);

        Vector3 destinoCamera = Vector3Add(
            jogador.posicao,
            {
                sinf(rotacaoCamera) * distanciaCamera,
                6.0f,
                cosf(rotacaoCamera) * distanciaCamera
            }
        );

        Vector3 alvoJogador = {
            jogador.posicao.x,
            jogador.posicao.y + 2.0f,
            jogador.posicao.z
        };

        float distanciaColosso = Vector3Distance(
            jogador.posicao, azhkhar
        );

        float pesoColosso = distanciaColosso < 32.0f
            ? 0.14f : 0.0f;

        Vector3 alvoColosso = {
            azhkhar.x,
            azhkhar.y + 7.0f,
            azhkhar.z
        };

        Vector3 alvoCamera = Vector3Lerp(
            alvoJogador, alvoColosso, pesoColosso
        );

        alvoCamera.y += 0.4f;

        camera.position = Suavizar(
            camera.position, destinoCamera, 3.5f, dt
        );

        camera.target = Suavizar(
            camera.target, alvoCamera, 4.0f, dt
        );

        // Renderização 3D.
        BeginDrawing();
        ClearBackground(ceu);

        BeginMode3D(camera);

        DesenharAtmosfera(
            {0.0f, 0.0f, 0.0f},
            jogador.posicao,
            tempo
        );

        DesenharChao();
        DesenharRuinas();
        DesenharColosso(azhkhar, tempo);
        DesenharJogador(jogador, tempo);

        EndMode3D();

        // Interface 2D independente da câmera.
        DesenharHUD(
            jogador,
            movendo,
            correndo,
            esquivando,
            tempo,
            distanciaColosso
        );

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

#include "camera.h"
#include "input.h" 
#include <stdio.h>
#include <math.h>

// Inicializa variáveis globais
int freeCameraMode = FALSE;
int currentCameraMode = CAM_THIRD_PERSON;

float fcAngleH;
float fcAngleV;
float fcX, fcY, fcZ;

float dirX, dirY, dirZ;
typedef struct { float x, y, z; } Vec3;

// Rotaciona um vetor em torno do eixo X (Pitch)
Vec3 camRotateX(Vec3 v, float angleDeg) {
    float rad = angleDeg * RADIAN_FACTOR;
    float c = cosf(rad);
    float s = sinf(rad);
    return (Vec3){ v.x, v.y * c - v.z * s, v.y * s + v.z * c };
}

// Rotaciona um vetor em torno do eixo Y (Yaw)
Vec3 camRotateY(Vec3 v, float angleDeg) {
    float rad = angleDeg * RADIAN_FACTOR;
    float c = cosf(rad);
    float s = sinf(rad);
    return (Vec3){ v.x * c + v.z * s, v.y, -v.x * s + v.z * c };
}

// ==========================================
// ATUALIZAÇÃO DA CÂMARA
// ==========================================

void updateCamera()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, ratio, 0.1, 200); // ratio vem de game.h (extern)

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Prioridade: Se o modo "Câmera Livre" (Debug) estiver ligado, ignora os outros
    if (freeCameraMode) {
        CalculateFreeCamNewPosition();
        gluLookAt(fcX, fcY, fcZ,
                  fcX + dirX, fcY + dirY, fcZ + dirZ,
                  0.0f, 1.0f, 0.0f);
        return; // Sai da função
    }

    // ====================================================================
    // CÁLCULO DO ÂNGULO REAL
    // ====================================================================
    float totalAngle = player.hullAngle + player.turretAngle;

    switch (currentCameraMode) {

        case CAM_FIRST_PERSON: 
        {

            float dist = PIPE_LENGTH + 0.8f; 
            
            Vec3 offset = { 0.0f, 0.0f, -dist };

            offset = camRotateX(offset, player.pipeAngle);

            offset = camRotateY(offset, player.turretAngle);

            offset.y += PIPE_HEIGHT;

            offset = camRotateX(offset, player.pitch);

            offset = camRotateY(offset, player.hullAngle);

            float eyeX = player.x + offset.x;
            float eyeY = player.y + offset.y; 
            float eyeZ = player.z + offset.z;

            Vec3 forward = { 0.0f, 0.0f, -1.0f }; // Vetor frente base
            forward = camRotateX(forward, player.pipeAngle);
            forward = camRotateY(forward, player.turretAngle);

            forward = camRotateX(forward, player.pitch); // A direção roda com a rampa
            forward = camRotateY(forward, player.hullAngle);

            float lookX = eyeX + forward.x;
            float lookY = eyeY + forward.y;
            float lookZ = eyeZ + forward.z;

            Vec3 up = { 0.0f, 1.0f, 0.0f };
            up = camRotateX(up, player.pitch); // O vetor UP inclina com o tanque
            up = camRotateY(up, player.hullAngle);

            gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, up.x, up.y, up.z);
            drawSun();
        }
        break;

        case POSITIONAL_CAM: // --- MODO 2: CÂMERA EXTRA (Vista de cima) ---
        {
            gluLookAt(player.x, player.y + 20.0f, player.z,
                      player.x, player.y, player.z,         
                      0.0f, 0.0f, -1.0f);   
            drawSun();                
        }
        break;

        case CAM_THIRD_PERSON: // --- MODO 0: TERCEIRA PESSOA (Padrão) ---
        default:               
        {
            float camX = player.x + sinf(totalAngle * RADIAN_FACTOR) * CAM_FACTOR_X;
            float camY = CAM_FACTOR_Y - (player.pipeAngle * 0.1f);
            float camZ = player.z + cosf(totalAngle * RADIAN_FACTOR) * CAM_FACTOR_Z;

            if (camY < 1.0f) camY = 1.0f;
            if (camY > 5.0f) camY = 5.0f;

            gluLookAt(camX, camY, camZ,
                      player.x, 0.5f, player.z,
                      0.0f, 1.0f, 0.0f);
            drawSun();
        }
        break;
    }
}

void updateMinimapCamera()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(90, 1.0, 0.1, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Câmera estática no centro do mapa
    gluLookAt(50.0f, 50.0f, 50.0f, 
              50.0f, 0.0f, 50.0f,
              0.0f, 0.0f, -1.0f); 

    drawMapSun();
}

void CalculateFreeCamNewPosition(){
    if (specialKeyStates[GLUT_KEY_LEFT]) fcAngleH -= FREE_CAM_ROT_SPEED;
    if (specialKeyStates[GLUT_KEY_RIGHT]) fcAngleH += FREE_CAM_ROT_SPEED;
    if (specialKeyStates[GLUT_KEY_UP])   fcAngleV += FREE_CAM_ROT_SPEED;
    if (specialKeyStates[GLUT_KEY_DOWN]) fcAngleV -= FREE_CAM_ROT_SPEED;

    if (fcAngleV > 89.0f) fcAngleV = 89.0f;
    if (fcAngleV < -89.0f) fcAngleV = -89.0f;

    float radH = fcAngleH * 3.14159f / 180.0f;
    float radV = fcAngleV * 3.14159f / 180.0f;

    dirX = sinf(radH) * cosf(radV);
    dirY = sinf(radV);
    dirZ = -cosf(radH) * cosf(radV);

    if (keyStates['w'] || keyStates['W']) {
        fcX += dirX * FREE_CAM_SPEED;
        fcY += dirY * FREE_CAM_SPEED;
        fcZ += dirZ * FREE_CAM_SPEED;
    }
    if (keyStates['s'] || keyStates['S']) {
        fcX -= dirX * FREE_CAM_SPEED;
        fcY -= dirY * FREE_CAM_SPEED;
        fcZ -= dirZ * FREE_CAM_SPEED;
    }

    float rightX = sinf(radH - 3.14159f/2.0f);
    float rightZ = -cosf(radH - 3.14159f/2.0f);

    if (keyStates['d'] || keyStates['D']) {
        fcX -= rightX * FREE_CAM_SPEED;
        fcZ -= rightZ * FREE_CAM_SPEED;
    }
    if (keyStates['a'] || keyStates['A']) {
        fcX += rightX * FREE_CAM_SPEED;
        fcZ += rightZ * FREE_CAM_SPEED;
    }
    if (keyStates['q'] || keyStates['Q']) fcY -= FREE_CAM_SPEED;
    if (keyStates['e'] || keyStates['E']) fcY += FREE_CAM_SPEED;
}
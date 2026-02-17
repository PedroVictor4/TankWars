#include <input.h>
#include <camera.h> // Para acessar freeCameraMode e atualizar posições
#include <tank.h>   // Para pegar posição do tanque ao trocar

// Teclas
int keyStates[256] = {0};
int specialKeyStates[256] = {0};

// Teclado
void keyDown(unsigned char key, int x, int y) {
    keyStates[key] = 1;
    
    if (key == ' ' && player.ammo > 0) shootBullet();

    // L: Alternar Câmera Livre (Debug)
    if (key == 'l' || key == 'L') {
        freeCameraMode = !freeCameraMode;
        if (freeCameraMode) {
            // Configurações iniciais da free cam
            fcX = player.x; fcY = player.y + 5.0f; fcZ = player.z + 5.0f;
            fcAngleH = 180.0f; fcAngleV = -20.0f;
        }
    }

    // C: Alternar entre os 3 modos de câmera (0 -> 1 -> 2 -> 0)
    if (key == 'c' || key == 'C') {
        // Se estivermos no modo livre, 'C' desliga o modo livre e volta para a câmera atual
        if (freeCameraMode) {
            freeCameraMode = 0;
        } else {
            // Incrementa o modo
            currentCameraMode++; 
            
            // Se passar de 2, volta para 0
            if (currentCameraMode > 2) {
                currentCameraMode = 0;
            }
        }
    }
}

void keyUp(unsigned char key, int x, int y) { 
    keyStates[key] = 0; 
}

// Teclas Especiais
void specialKeyDown(int key, int x, int y) {
    specialKeyStates[key] = 1;
}

void specialKeyUp(int key, int x, int y) {
    specialKeyStates[key] = 0;
}
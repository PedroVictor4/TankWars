#include <GL/glut.h>
#include "menu.h"
#include "game.h"

extern int currentState; // definido em main.c

GLuint bgTexture;

// ----------------------------------------------------------
// Carregar BMP como textura
// ----------------------------------------------------------
GLuint loadBMP(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erro ao abrir %s\n", filename);
        return 0;
    }

    unsigned char header[54];
    fread(header, 1, 54, file);

    int width  = *(int*)&header[18];
    int height = *(int*)&header[22];
    int size   = 3 * width * height;

    unsigned char *data = (unsigned char*)malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    GLuint textureID;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
                 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    free(data);
    return textureID;
}

// ----------------------------------------------------------
// Desenhar fundo com textura
// ----------------------------------------------------------
void drawBackground() {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, bgTexture);

    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex2f(0,0);
    glTexCoord2f(1,0); glVertex2f(1,0);
    glTexCoord2f(1,1); glVertex2f(1,1);
    glTexCoord2f(0,1); glVertex2f(0,1);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
}

// ----------------------------------------------------------
// Mostrar texto
// ----------------------------------------------------------
void drawText(float x, float y, const char *str, void *font) {
    glRasterPos2f(x, y);
    while (*str)
        glutBitmapCharacter(font, *str++);
}

// ----------------------------------------------------------
// Função de display do menu
// ----------------------------------------------------------
void displayMenu() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawBackground();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(1.0, 1.0, 1.0);

    // Título centralizado
    drawText(0.40f, 0.60f, "TANK WARS", GLUT_BITMAP_TIMES_ROMAN_24);

    // Opção jogar
    drawText(0.42f, 0.45f, "Pressione 1 para Jogar", GLUT_BITMAP_HELVETICA_18);

    glutSwapBuffers();
}

// ----------------------------------------------------------
// Controle de teclas no menu
// ----------------------------------------------------------
void keyboardMenu(unsigned char key, int x, int y) {
    if (key == '1') {
        currentState = 1;   // Ir para o jogo

        glutIdleFunc(NULL); // <--- Adicionar esta linha
        
        // Restaurar callbacks do jogo
        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutTimerFunc(16, timer, 0);

        glutKeyboardUpFunc(keyUp);
        glutKeyboardFunc(keyDown);
        glutSpecialFunc(specialKeyDown);
        glutSpecialUpFunc(specialKeyUp);

        init(); // inicializa o jogo

        // *** ESSENCIAL PARA RESTAURAR O 3D ***
        reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

        glutPostRedisplay();
    }
}

// ----------------------------------------------------------
// Inicialização do Menu
// ----------------------------------------------------------
void initMenu() {
    glClearColor(0,0,0,1);

    bgTexture = loadBMP("tank_background.bmp");

    // Callbacks do menu
    glutDisplayFunc(displayMenu);
    glutKeyboardFunc(keyboardMenu);
    glutIdleFunc(displayMenu); // Para atualizar sempre
}

// -----------------------------------------------------------------------------
// Função principal: desenha tela de game over
// -----------------------------------------------------------------------------
void drawGameOverScreen()
{
    // Desabilita iluminação para texto 2D
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Modo ortográfico (2D)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Fundo preto semi-transparente
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(-1, -1);
    glVertex2f(1, -1);
    glVertex2f(1, 1);
    glVertex2f(-1, 1);
    glEnd();

    // Texto principal - GAME OVER
    glColor3f(1.0f, 0.0f, 0.0f);
    // Título centralizado
    drawText(-0.10f, 0.00f, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);

    // Restaura matrizes
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}

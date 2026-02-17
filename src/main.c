#include <game.h>
#include <input.h>
#include <menu.h>

int currentState = 0; // 0 = MENU, 1 = GAME

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 800);
    glutCreateWindow("Tank Wars");
    if (currentState == 0) {
        initMenu();
    }
    glutMainLoop();
    return 0;
}
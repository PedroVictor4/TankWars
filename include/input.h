#include <projectile.h>

extern int keyStates[256];

extern int specialKeyStates[256];

void keyDown(unsigned char key, int x, int y);

void keyUp(unsigned char key, int x, int y);

void specialKeyDown(int key, int x, int y);

void specialKeyUp(int key, int x, int y);
#ifndef MENU_H
#define MENU_H

void initMenu();
void displayMenu();
void keyboardMenu(unsigned char key, int x, int y);
GLuint loadBMP(const char *filename);
void drawBackground();
void drawGameOverScreen();

#endif
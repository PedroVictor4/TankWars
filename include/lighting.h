#ifndef LIGHTING_H
#define LIGHTING_H

#include <GL/glut.h>

extern unsigned long freezeEndTime;

void drawSun();

void drawMapSun();

void setupLighting();

void updateFreezeLighting();

#endif
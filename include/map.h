#ifndef MAP_H
#define MAP_H

typedef struct vertex {
    float x, y, z;
} vertex;

typedef struct square {
    vertex A, B, C, D;
} square;

#include <GL/glut.h>
#include <mybib.h>
#include "collision.h"

#define MAP_SIZE 100
#define VERTEX_NUM MAP_SIZE + 1

#define MAX_STATIC_COLLIDERS 100

// Defines de Posições
#define CHURCH_X 10.0f
#define CHURCH_Z 18.0f
#define HOUSE_X 15.0f  
#define HOUSE_Z 18.0f
#define HOUSE2_X 19.0f  
#define HOUSE2_Z 9.0f
#define GASSTATION_X 25.0f  
#define GASSTATION_Z 18.0f
#define GASSTATION2_X 30.0f  
#define GASSTATION2_Z 18.0f
#define GASSTATION3_X 25.0f  
#define GASSTATION3_Z 10.0f
#define GASSTATION4_X 30.0f  
#define GASSTATION4_Z 10.0f
#define STORE_X 38.0f  
#define STORE_Z 18.0f

extern square mapCells[MAP_SIZE][MAP_SIZE];
extern float heightMatrix[VERTEX_NUM][VERTEX_NUM];
extern vertex vertexNormals[VERTEX_NUM][VERTEX_NUM];

extern ObjModel churchModel;
extern ObjModel houseModel;
extern ObjModel gasStationModel;
extern ObjModel storeModel;

// Dados dos colisores para o collision.c ler
extern CollisionBox staticColliders[MAX_STATIC_COLLIDERS];
extern int staticColliderCount;

void drawMap();
void initMapCells();
void initHeightMatrix();

// Pega a altura do terreno em qualquer ponto (x, z)
float getTerrainHeight(float x, float z);

// Calcula a inclinação (pitch) do terreno baseada no angulo do objeto
float getTerrainPitch(float x, float z, float angleYaw);

void drawWall(float x, float y, float z, float width, float height, float depth);
void initChurch();
void initHouse();
void initGasStation();
void initStore();

#endif
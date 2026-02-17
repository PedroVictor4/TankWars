#ifndef TANK_H
#define TANK_H

typedef struct {
    float x, y, z;
    int mapCellX, mapCellY, mapCellZ; 

    float hullAngle;
    float pitch;
    float turretAngle;
    float pipeAngle;

    float moveSpeed;
    float tankRotSpeed;
    float turretRotSpeed;
    float pipeInclineSpeed;
    
    int alive;
    int health;
    int ammo;
    int bulletDmg;
    unsigned long lastShootTime;
    int reloadTime;
    int flagReloadCircle;
    int shieldOn;
} Tank;

#include <GL/glut.h>
#include <projectile.h>
#include <input.h>
#include <map.h>
#include <stdio.h>
#include <mybib.h>
#include <utils.h>
#include <collision.h>
#include <camera.h>
#define INITIAL_TANK_CELL_Z 25
#define INITIAL_TANK_CELL_X 25

#define MAX_PIPE_ANGLE 10
#define MIN_PIPE_ANGLE -5

#define TANK_MOVEMENT_SPEED 0.2f

#define TANK_ROT_SPEED 2.0f
#define TURRET_ROT_SPEED 0.5f
#define PIPE_INCLINE_SPEED 0.1f

// Defini isso na mão pq não tenho como saber exatamente onde está o pipe
#define PIPE_LENGTH 3.2
#define PIPE_HEIGHT 1.12

// Altura que o tank deve ficar do chão
#define TANK_GROUND_DISTANCE 0.05f

#define TRUE 1
#define FALSE 0

extern const double RADIAN_FACTOR; // Pra converter de graus pra radiano

extern ObjModel turretModel, pipeModel, hullModel;

extern Tank player;

void drawTank();

void updateTank();

void initTank();

void updateMapCellPos();

#endif
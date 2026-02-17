#ifndef PROJECTILE_H  
#define PROJECTILE_H

// Estrutura bullets
typedef struct {
    unsigned char active;
    float x, y, z;
    
    // Adicionado vetores de velocidade para suportar física 3D (tiro inclinado)
    float vx, vy, vz; 

    // Mantido para desenho
    float horizontal_angle;
    float vertical_angle;
} Bullet;

typedef struct {
    float x, y, z;
    float radius;
    float maxRadius;
    int active;

    float startTime;
} Explosion;

#include <math.h>
#include <time.h>
#include <GL/glut.h>
#include "mybib.h"

// Defines de configuração
#define MAX_BULLETS 100
#define MAX_ENEMY_BULLETS 50 
#define ENEMY_SHOOT_DELAY 1000 

#define BULLET_SPEED 1.2f
#define BULLET_SCALE_CORRECTION 0.15f
#define BULLET_SIDE_CORRECTION 0.07f

// Forward declaration para evitar inclusão circular
struct Enemy; 

extern Explosion explosion;
extern Bullet player_bullet; 
extern Bullet enemy_bullets[MAX_ENEMY_BULLETS]; 
extern ObjModel shellModel;

// Funções de Bala
void initBullet();
void updateBullets(); // Player bullet update
void shootBullet();   // Player shoot
void drawBullet();    // Player draw

// Funções de Inimigo
void initEnemyBullets();
void updateEnemyBullets();
void shootEnemyBullet(struct Enemy *e);
void drawEnemyBullets();

// Funções de Explosão
void startExplosion(float x, float y, float z); 
void updateExplosion();
void drawExplosion();

#endif
#ifndef POWER_UP_H
#define POWER_UP_H

typedef enum {
    PU_AMMO,
    PU_HEALTH,
    PU_SPEED,
    PU_DMG,
    PU_FREEZE,
    PU_SHIELD
} PowerUpType;

typedef struct {
    PowerUpType type;
    float x, y, z;
    float scale;
    int active;
} PowerUpInstance;

#include <mybib.h>
#include <input.h>
#include <collision.h>
#include <tank.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "enemy.h" // Adicionar include para acessar freezeEndTime

#define MAX_POWER_UPS 250

extern ObjModel snowFlake, hermesShoes, fire, healthPack; //Ammo é carregado em projectile.c

extern PowerUpInstance powerUps[MAX_POWER_UPS];

extern int numPUativos;

PowerUpType getRandomPowerUp();

ObjModel getObjModel(PowerUpInstance p);

void applyPowerUpEffect(PowerUpType type);

void drawPowerUps();

float getScale(PowerUpType type);

void spawnPowerUp();

void testePowerUp();

void initPowerUps();

void morreu(); //testar tela de morte

// ------------- FREEZE -------------
void drawSnowFlake(int index);

void freeze();

// ------------- AMMO -------------
void drawAmmo(int index);

void addAmmo();

// ------------- HEALTH -------------
void drawHealthPack(int index);

void heal();

// ------------- INSTA KILL -------------
void drawFire(int index);

void increaseDmg();

// ------------- SPEED -------------
void drawHermesShoes(int index);

void increaseSpeed();

// ------------- SHIELD -------------

void drawShieldPU(int index);

void activateShield();

void drawShield(CollisionBox b);

void drawPlayerShield();

#endif
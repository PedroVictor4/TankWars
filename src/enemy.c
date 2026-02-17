#include "enemy.h"
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>       
#include <time.h>       
#include "collision.h" 
#include <projectile.h> 
#include "map.h"      
#include "tank.h" 

#ifndef RADIAN_FACTOR
#define RADIAN_FACTOR (3.14159265359 / 180.0)
#endif

// VARIÁVEIS GLOBAIS
Enemy enemies[MAX_ENEMIES];
ObjModel enemyHullModel;
ObjModel enemyTurretModel;
ObjModel enemyPipeModel;

unsigned long freezeEndTime = 0;

// Distância mínima entre dois inimigos para não entrelaçar canos
#define SEPARATION_DIST 8.0f 

void initEnemies() {
    if (!loadOBJ("objects/hull.obj", "objects/hull.mtl", &enemyHullModel)) printf("Erro Hull Inimigo\n");
    if (!loadOBJ("objects/turret.obj", "objects/turret.mtl", &enemyTurretModel)) printf("Erro Turret Inimigo\n");
    if (!loadOBJ("objects/pipe.obj", "objects/pipe.mtl", &enemyPipeModel)) printf("Erro Pipe Inimigo\n");

    srand(time(NULL));
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].alive = 1;
        enemies[i].x = (rand() % 100); 
        enemies[i].z = (rand() % 100);
        
        enemies[i].y = getTerrainHeight(enemies[i].x, enemies[i].z) + ENEMY_TANK_GROUND_DISTANCE;
        enemies[i].pitch = 0.0f;
        
        enemies[i].hullAngle = rand() % 360;   
        enemies[i].turretAngle = 0.0f;         
        enemies[i].wanderTimer = 0;
        enemies[i].targetWanderAngle = enemies[i].hullAngle;
        enemies[i].lastShootTime = 0; 
        enemies[i].stuckTimer = 0; 
    }
}

void drawVisionCone(Enemy *e) {
    glPushAttrib(GL_ENABLE_BIT); 
    float totalAngle = e->hullAngle + e->turretAngle;
    float rad = totalAngle * RADIAN_FACTOR;
    float halfFovRad = (ENEMY_VIEW_ANGLE / 2.0f) * RADIAN_FACTOR;
    
    float y = e->y + 1.25f; 
    
    float cx = -sinf(rad) * ENEMY_VIEW_RANGE;
    float cz = -cosf(rad) * ENEMY_VIEW_RANGE;
    float lx = -sinf(rad + halfFovRad) * ENEMY_VIEW_RANGE;
    float lz = -cosf(rad + halfFovRad) * ENEMY_VIEW_RANGE;
    float rx = -sinf(rad - halfFovRad) * ENEMY_VIEW_RANGE;
    float rz = -cosf(rad - halfFovRad) * ENEMY_VIEW_RANGE;

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(2.0f);
    glColor3f(0.0f, 1.0f, 1.0f); 

    glBegin(GL_LINES);
        glVertex3f(e->x, y, e->z); glVertex3f(e->x + cx, y, e->z + cz);
        glVertex3f(e->x, y, e->z); glVertex3f(e->x + lx, y, e->z + lz);
        glVertex3f(e->x, y, e->z); glVertex3f(e->x + rx, y, e->z + rz);
        glVertex3f(e->x + lx, y, e->z + lz); glVertex3f(e->x + rx, y, e->z + rz);
    glEnd();
    glPopAttrib(); 
}

int canSeePlayer(Enemy *e, float px, float pz) {
    float dx = px - e->x;
    float dz = pz - e->z;
    float distSq = dx*dx + dz*dz;
    if (distSq > ENEMY_VIEW_RANGE * ENEMY_VIEW_RANGE) return 0; 

    float totalAngle = e->hullAngle + e->turretAngle;
    float rad = totalAngle * RADIAN_FACTOR;
    float forwardX = -sinf(rad);
    float forwardZ = -cosf(rad);
    float dist = sqrtf(distSq);
    float dirToPlayerX = dx / dist;
    float dirToPlayerZ = dz / dist;
    float dot = forwardX * dirToPlayerX + forwardZ * dirToPlayerZ;
    float cosHalfFov = cosf((ENEMY_VIEW_ANGLE / 2.0f) * RADIAN_FACTOR);
    return (dot > cosHalfFov);
}

void drawEnemyTank(Enemy *e) {
    drawVisionCone(e);
    glColor3f(1.0f, 1.0f, 1.0f); 
    glPushMatrix();
        glTranslatef(e->x, e->y, e->z); 
        glRotatef(e->hullAngle, 0.0f, 1.0f, 0.0f);
        glRotatef(e->pitch, 1.0f, 0.0f, 0.0f);

        glPushMatrix();
            drawModel(&enemyHullModel);
        glPopMatrix();
        glPushMatrix();
            glRotatef(e->turretAngle, 0.0f, 1.0f, 0.0f);
            drawModel(&enemyTurretModel);
            drawModel(&enemyPipeModel);
        glPopMatrix();
    glPopMatrix();
}

void drawEnemies() {
    glEnable(GL_TEXTURE_2D); 
    glPushMatrix();
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) drawEnemyTank(&enemies[i]);
    }
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void updateEnemies(float playerX, float playerZ) {
    unsigned long now = glutGet(GLUT_ELAPSED_TIME);

    if (now < freezeEndTime) {
        return; 
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;

        // Atualiza estado atual
        enemies[i].y = getTerrainHeight(enemies[i].x, enemies[i].z) + ENEMY_TANK_GROUND_DISTANCE;
        enemies[i].pitch = getTerrainPitch(enemies[i].x, enemies[i].z, enemies[i].hullAngle);

        // LÓGICA DE MANOBRA (RÉ)
        if (enemies[i].stuckTimer > 0) {
            enemies[i].stuckTimer--;
            
            float rad = enemies[i].hullAngle * RADIAN_FACTOR;
            float backSpeed = ENEMY_SPEED * 0.8f; 
            
            float nextX = enemies[i].x + sinf(rad) * backSpeed; // + é ré
            float nextZ = enemies[i].z + cosf(rad) * backSpeed;
            float nextPitch = getTerrainPitch(nextX, nextZ, enemies[i].hullAngle);
            float nextY = getTerrainHeight(nextX, nextZ) + ENEMY_TANK_GROUND_DISTANCE;

            // Verifica colisão da ré (Se bater em qualquer coisa, cancela a ré)
            int collisionType = checkEnemyGhostCollision(i, nextX, nextY, nextZ, enemies[i].hullAngle, enemies[i].turretAngle, nextPitch);
            if (collisionType == 0) {
                enemies[i].x = nextX;
                enemies[i].z = nextZ;
            }
            
            enemies[i].hullAngle += 2.0f; 
            continue; 
        }

        float nextX = enemies[i].x;
        float nextZ = enemies[i].z;
        float currentMoveSpeed = ENEMY_SPEED;
        float targetGlobalAngle = 0.0f; 
        int shouldMove = 1; 
        
        int seesPlayer = canSeePlayer(&enemies[i], playerX, playerZ);
        
        if (seesPlayer) {
            // ATIRAR
            if (now - enemies[i].lastShootTime > ENEMY_SHOOT_DELAY) {
                shootEnemyBullet(&enemies[i]);
                enemies[i].lastShootTime = now;
            }
            
            float dx = playerX - enemies[i].x;
            float dz = playerZ - enemies[i].z;
            float distSq = dx*dx + dz*dz;
            
            if (distSq < ENEMY_MIN_COMBAT_DIST * ENEMY_MIN_COMBAT_DIST) shouldMove = 0; 
            
            float targetRad = atan2f(-dx, -dz); 
            targetGlobalAngle = targetRad * (180.0f / 3.14159f);
        } else {
            // WANDER
            enemies[i].wanderTimer--;
            if (enemies[i].wanderTimer <= 0) {
                enemies[i].wanderTimer = 60 + (rand() % 120); 
                float dx = playerX - enemies[i].x;
                float dz = playerZ - enemies[i].z;
                float trueAngleRad = atan2f(-dx, -dz);
                float trueAngleDeg = trueAngleRad * (180.0f / 3.14159f);
                int spread = (int)(360.0f * (1.0f - ENEMY_TRACKING_BIAS));
                if (spread > 0) {
                    int randomOffset = (rand() % spread) - (spread / 2);
                    enemies[i].targetWanderAngle = trueAngleDeg + randomOffset;
                } else {
                    enemies[i].targetWanderAngle = trueAngleDeg;
                }
            }
            targetGlobalAngle = enemies[i].targetWanderAngle;
        }

        // REPULSÃO ENTRE INIMIGOS
        int isAvoiding = 0;
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (i == j || !enemies[j].alive) continue;
            float dx = enemies[j].x - enemies[i].x;
            float dz = enemies[j].z - enemies[i].z;
            float distSq = dx*dx + dz*dz;
            
            if (distSq < SEPARATION_DIST * SEPARATION_DIST) {
                float angleToNeighborRad = atan2f(-dx, -dz);
                float angleAwayDeg = (angleToNeighborRad * (180.0f / 3.14159f)) + 180.0f;
                targetGlobalAngle = angleAwayDeg;
                isAvoiding = 1;
                shouldMove = 1; 
                break; 
            }
        }

        // ROTAÇÃO DO CORPO (HULL)
        float hullDiff = targetGlobalAngle - enemies[i].hullAngle;
        while (hullDiff > 180) hullDiff -= 360;
        while (hullDiff < -180) hullDiff += 360;
        
        float nextHullAngle = enemies[i].hullAngle;
        float rotSpeed = isAvoiding ? ENEMY_ROT_SPEED * 2.0f : ENEMY_ROT_SPEED;

        if (hullDiff > rotSpeed) nextHullAngle += rotSpeed;
        else if (hullDiff < -rotSpeed) nextHullAngle -= rotSpeed;
        else nextHullAngle = targetGlobalAngle; 

        // Checa se a rotação é segura
        // Se retornar 1 (parede) ou 2 (player), não gira.
        float futurePitch = getTerrainPitch(enemies[i].x, enemies[i].z, nextHullAngle);
        int rotCollision = checkEnemyGhostCollision(i, enemies[i].x, enemies[i].y, enemies[i].z, nextHullAngle, enemies[i].turretAngle, futurePitch);
        
        if (rotCollision == 0) {
            enemies[i].hullAngle = nextHullAngle;
        }
        // Se quiser implementar lógica de stuck aqui, descomente:
        /*
            else {
            enemies[i].stuckTimer = ENEMY_STUCK_TIME;
        }
        */

        // ROTAÇÃO DA TORRE
        if (seesPlayer && !isAvoiding) {
            float dx = playerX - enemies[i].x;
            float dz = playerZ - enemies[i].z;
            float pRad = atan2f(-dx, -dz); 
            float pDeg = pRad * (180.0f / 3.14159f);
            float tDiff = pDeg - (enemies[i].hullAngle + enemies[i].turretAngle);
            while (tDiff > 180) tDiff -= 360;
            while (tDiff < -180) tDiff += 360;
            
            float nextTurretAngle = enemies[i].turretAngle;
            if (tDiff > ENEMY_ROT_SPEED) nextTurretAngle += ENEMY_ROT_SPEED;
            else if (tDiff < -ENEMY_ROT_SPEED) nextTurretAngle -= ENEMY_ROT_SPEED;

            // Checa se a rotação da torre é segura
            if (checkEnemyGhostCollision(i, enemies[i].x, enemies[i].y, enemies[i].z, enemies[i].hullAngle, nextTurretAngle, enemies[i].pitch) == 0) {
                 enemies[i].turretAngle = nextTurretAngle;
            }
        } else {
            enemies[i].turretAngle *= 0.90f;
        }

        // MOVIMENTO
        if (shouldMove) {
            float rad = enemies[i].hullAngle * RADIAN_FACTOR;
            
            // SENSOR DE SEGURANÇA (Para PAREDES)
            float probeX = enemies[i].x - sinf(rad) * ENEMY_OBSTACLE_SAFE_DIST;
            float probeZ = enemies[i].z - cosf(rad) * ENEMY_OBSTACLE_SAFE_DIST;
            float probeY = getTerrainHeight(probeX, probeZ) + ENEMY_TANK_GROUND_DISTANCE;
            float probePitch = getTerrainPitch(probeX, probeZ, enemies[i].hullAngle);

            int probeCollision = checkEnemyGhostCollision(i, probeX, probeY, probeZ, enemies[i].hullAngle, enemies[i].turretAngle, probePitch);
            
            // SÓ ENTRA EM MODO RÉ SE FOR COLISÃO TIPO 1 (PAREDE)
            if (probeCollision == 1) {
                shouldMove = 0;
                enemies[i].targetWanderAngle += 180.0f; 
                enemies[i].stuckTimer = ENEMY_STUCK_TIME; 
            }
            // SE FOR TIPO 2 (PLAYER/INIMIGO), NÃO FAZ NADA (APENAS PARA DE ANDAR PELA LÓGICA ABAIXO)
            
            // APLICA MOVIMENTO
            if (shouldMove) {
                nextX -= sinf(rad) * currentMoveSpeed;
                nextZ -= cosf(rad) * currentMoveSpeed;
                float nextY = getTerrainHeight(nextX, nextZ) + ENEMY_TANK_GROUND_DISTANCE; 
                float nextPitch = getTerrainPitch(nextX, nextZ, enemies[i].hullAngle);

                // Verifica colisão REAL do movimento
                int moveCollision = checkEnemyGhostCollision(i, nextX, nextY, nextZ, enemies[i].hullAngle, enemies[i].turretAngle, nextPitch);

                if (moveCollision == 0) {
                    // LIVRE
                    enemies[i].x = nextX;
                    enemies[i].z = nextZ;
                    enemies[i].y = nextY;
                    enemies[i].pitch = nextPitch;
                } else if (moveCollision == 1) {
                    // BATEU NA PAREDE DE PERTO (Ativa Ré)
                    enemies[i].stuckTimer = ENEMY_STUCK_TIME;
                } 
                // Se moveCollision == 2, não faz nada (não anda, não dá ré) -> Comportamento desejado
            }
        }
    }
}
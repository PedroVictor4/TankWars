#include "projectile.h"
#include "collision.h" 
#include "enemy.h"     
#include "game.h"      
#include "tank.h" 
#include "map.h" 

// =========================================================
// GLOBAIS
// =========================================================
Bullet player_bullet;
Bullet enemy_bullets[MAX_ENEMY_BULLETS]; 
ObjModel shellModel;
Explosion explosion;

// Altura aproximada da hitbox da bala
#define BULLET_BOX_HEIGHT 0.3f 

// Sub-steps para evitar Tunneling
#define PHYSICS_SUBSTEPS 4 

#ifndef RADIAN_FACTOR
#define RADIAN_FACTOR (3.14159265359 / 180.0)
#endif

// =========================================================
// FUNÇÕES AUXILIARES
// =========================================================

CollisionBox makeBulletBox(Bullet *b) {
    float halfH = BULLET_BOX_HEIGHT / 2.0f;

    return createHierarchicalBox(
        &shellModel.box, 
        b->x, b->y, b->z,
        BULLET_SCALE_CORRECTION, BULLET_SCALE_CORRECTION, 
        b->horizontal_angle, 0.0f, 
        0.0f, b->vertical_angle,   
        -halfH, halfH              
    );
}

// =========================================================
// PLAYER BULLET
// =========================================================

void shootBullet() {
    unsigned long now = glutGet(GLUT_ELAPSED_TIME);

    if (now - player.lastShootTime < player.reloadTime) return;
    if (player.ammo <= 0) return;

    player.ammo -= 1;
    player.flagReloadCircle = 1; 
    player.lastShootTime = now;

    player_bullet.active = 1;

    player_bullet.horizontal_angle = player.turretAngle + player.hullAngle; 
    player_bullet.vertical_angle = player.pipeAngle + player.pitch;     

    float radY = player_bullet.horizontal_angle * RADIAN_FACTOR;
    float radP = player_bullet.vertical_angle * RADIAN_FACTOR;

    float pivotHeight = 1.25f; 
    float pipeLen = 3.2f;

    float offsetH = pipeLen * cosf(radP); 
    float offsetV = pipeLen * sinf(radP);

    player_bullet.x = player.x - sinf(radY) * offsetH;
    player_bullet.z = player.z - cosf(radY) * offsetH;
    player_bullet.y = (player.y + pivotHeight) + offsetV; 

    player_bullet.vx = -sinf(radY) * cosf(radP) * BULLET_SPEED;
    player_bullet.vz = -cosf(radY) * cosf(radP) * BULLET_SPEED;
    player_bullet.vy = sinf(radP) * BULLET_SPEED;
}

void updateBullets() {
    if (!player_bullet.active) return;

    float stepVx = player_bullet.vx / (float)PHYSICS_SUBSTEPS;
    float stepVy = player_bullet.vy / (float)PHYSICS_SUBSTEPS;
    float stepVz = player_bullet.vz / (float)PHYSICS_SUBSTEPS;

    for (int step = 0; step < PHYSICS_SUBSTEPS; step++) {
        
        player_bullet.x += stepVx;
        player_bullet.y += stepVy;
        player_bullet.z += stepVz;

        CollisionBox bulletBox = makeBulletBox(&player_bullet);

        // 1. Colisão com o MUNDO
        if (checkCollisionWithWorld(&bulletBox) || player_bullet.y < -5.0f) {
            startExplosion(player_bullet.x, player_bullet.y, player_bullet.z);
            player_bullet.active = 0;
            return;
        }

        // 2. Colisão com INIMIGOS
        int hit = 0;
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].alive) continue;

            float ePitch = enemies[j].pitch; 

            CollisionBox eHull = makeEnemyHull(&enemies[j], ePitch);
            CollisionBox eTurret = makeEnemyTurret(&enemies[j], ePitch);
            CollisionBox ePipe = makeEnemyPipe(&enemies[j], ePitch);

            if (checkCollisionOBB(&bulletBox, &eHull) || 
                checkCollisionOBB(&bulletBox, &eTurret) || 
                checkCollisionOBB(&bulletBox, &ePipe)) {
                
                startExplosion(player_bullet.x, player_bullet.y, player_bullet.z);
                enemies[j].alive = 0;     
                player_bullet.active = 0; 
                hit = 1;
                break; 
            }
        }
        if (hit) return; 
    }
}

void drawBullet() {
    if (!player_bullet.active) return;

    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(player_bullet.x, player_bullet.y, player_bullet.z);
    glScalef(BULLET_SCALE_CORRECTION, BULLET_SCALE_CORRECTION, BULLET_SCALE_CORRECTION);
    
    glRotatef(player_bullet.horizontal_angle, 0.0f, 1.0f, 0.0f);
    glRotatef(player_bullet.vertical_angle, 1.0f, 0.0f, 0.0f);

    drawModel(&shellModel);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D); 
}

// =========================================================
// ENEMY BULLETS
// =========================================================

void initEnemyBullets() {
    for(int i=0; i<MAX_ENEMY_BULLETS; i++) {
        enemy_bullets[i].active = 0;
    }
}

void shootEnemyBullet(Enemy *e) {
    int slot = -1;
    for(int i=0; i<MAX_ENEMY_BULLETS; i++) {
        if(!enemy_bullets[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return;

    Bullet *b = &enemy_bullets[slot];
    b->active = 1;
    
    float totalYaw = e->hullAngle + e->turretAngle;
    float totalPitch = e->pitch; 

    b->horizontal_angle = totalYaw;
    b->vertical_angle = totalPitch;

    float radY = totalYaw * RADIAN_FACTOR;
    float radP = totalPitch * RADIAN_FACTOR;
    
    float pipePivotHeight = 1.25f; 
    float pipeLength = 3.2f;

    float offsetH = pipeLength * cosf(radP);
    float offsetV = pipeLength * sinf(radP);

    b->y = (e->y + pipePivotHeight) + offsetV; 
    b->x = e->x - sinf(radY) * offsetH;
    b->z = e->z - cosf(radY) * offsetH;

    float speed = BULLET_SPEED * 0.8f; 
    b->vx = -sinf(radY) * cosf(radP) * speed;
    b->vz = -cosf(radY) * cosf(radP) * speed;
    b->vy = sinf(radP) * speed;
}

void updateEnemyBullets() {
    float pPitch = player.pitch;
    CollisionBox pHull = makePlayerHull(player.x, player.z, player.hullAngle, pPitch);
    CollisionBox pTurret = makePlayerTurret(player.x, player.z, player.hullAngle, pPitch, player.turretAngle);
    CollisionBox pPipe = makePlayerPipe(player.x, player.z, player.hullAngle, pPitch, player.turretAngle, player.pipeAngle);

    for(int i=0; i<MAX_ENEMY_BULLETS; i++) {
        if(!enemy_bullets[i].active) continue;

        Bullet *b = &enemy_bullets[i];

        float stepVx = b->vx / (float)PHYSICS_SUBSTEPS;
        float stepVy = b->vy / (float)PHYSICS_SUBSTEPS;
        float stepVz = b->vz / (float)PHYSICS_SUBSTEPS;

        int collided = 0;

        for (int step = 0; step < PHYSICS_SUBSTEPS; step++) {
            b->x += stepVx;
            b->y += stepVy;
            b->z += stepVz;

            CollisionBox bBox = makeBulletBox(b);

            if(checkCollisionWithWorld(&bBox) || b->y < -5.0f) {
                startExplosion(b->x, b->y, b->z);
                b->active = 0;
                collided = 1;
                break;
            }

            if (checkCollisionOBB(&bBox, &pHull) || 
                checkCollisionOBB(&bBox, &pTurret) || 
                checkCollisionOBB(&bBox, &pPipe)) {
                
                startExplosion(b->x, b->y, b->z);
                b->active = 0; 

                if (player.shieldOn) {
                    player.shieldOn = 0; // desliga escudo caso for acertado
                }
                else {
                    // IMPORTANTE É AQUI QUE O PLAYER LEVA DANO
                    player.health -= 10;
                    //printf("Player Hit! HP: %d\n", player.health);
                }
                
                if(player.health <= 0) player.alive = 0;
                
                collided = 1;
                break;
            }
        }
        if (collided) continue;
    }
}

void drawEnemyBullets() {
    glEnable(GL_TEXTURE_2D);
    for(int i=0; i<MAX_ENEMY_BULLETS; i++) {
        if(!enemy_bullets[i].active) continue;
        
        Bullet *b = &enemy_bullets[i];
        glPushMatrix();
            glTranslatef(b->x, b->y, b->z);
            glScalef(BULLET_SCALE_CORRECTION, BULLET_SCALE_CORRECTION, BULLET_SCALE_CORRECTION);
            glRotatef(b->horizontal_angle, 0.0f, 1.0f, 0.0f);
            glRotatef(b->vertical_angle, 1.0f, 0.0f, 0.0f);
            drawModel(&shellModel); 
        glPopMatrix();
    }
    glDisable(GL_TEXTURE_2D);
}

void initBullet() {
    if (loadOBJ("objects/shell.obj", "objects/shell.mtl", &shellModel)) {
    } else {
        printf("ERRO: Nao foi possivel carregar o modelo do projetil.\n");
    } 
    player_bullet.active = 0;
    initEnemyBullets(); 
}

// =========================================================
// EXPLOSION SYSTEM
// =========================================================

const float EXPLOSION_GROW_TIME = 0.5f;   
const float EXPLOSION_FADE_TIME = 0.5f;   

void startExplosion(float x, float y, float z) {
    explosion.x = x;
    explosion.y = y;
    explosion.z = z;
    explosion.radius = 0.0f;
    explosion.maxRadius = 1.5f;
    explosion.active  = 1;
    explosion.startTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; 
}

void updateExplosion() {
    if (!explosion.active) return;

    float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float t = now - explosion.startTime;

    if (t <= EXPLOSION_GROW_TIME) {
        float k = t / EXPLOSION_GROW_TIME;
        explosion.radius = explosion.maxRadius * k;
        return;
    }

    if (t <= EXPLOSION_GROW_TIME + EXPLOSION_FADE_TIME) {
        float fadeElapsed = t - EXPLOSION_GROW_TIME;
        float k = fadeElapsed / EXPLOSION_FADE_TIME;
        explosion.radius = explosion.maxRadius * (1.0f - k);
        return;
    }

    explosion.radius = 0.0f;
    explosion.active = 0;
}

void drawExplosion() {
    if (!explosion.active) return;

    float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float t = now - explosion.startTime;
    float alpha = 1.0f;

    if (t > EXPLOSION_GROW_TIME) {
        float fadeElapsed = t - EXPLOSION_GROW_TIME;
        alpha = 1.0f - (fadeElapsed / EXPLOSION_FADE_TIME);
        if (alpha < 0.0f) alpha = 0.0f;
    }

    glPushMatrix();
    glTranslatef(explosion.x, explosion.y, explosion.z);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 0.5f, 0.0f, alpha);
    glutSolidSphere(explosion.radius, 20, 20);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}
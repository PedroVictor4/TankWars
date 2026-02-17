#ifndef COLISION_H
#define COLISION_H

// Forward Declaration
struct Enemy; 

typedef struct {
    float x, y, z;
} Point3D;

typedef struct {
    Point3D center;      // Centro da caixa no mundo
    Point3D axis[3];     // Vetores de direção (X, Y, Z rotacionados)
    float halfSize[3];   // Metade da largura, altura e profundidade
    Point3D corners[8];  // Os 8 cantos para desenhar o debug
} CollisionBox;

#include "mybib.h"
#include <power_up.h>

// WIDTH: Quanto menor, mais "fino" fica o colisor (evita bater girando)
// LENGTH: Quanto maior, mais longe ele alcança (corrige bater de frente)

// Base (Hull): Quadrada, encolhe um pouco tudo
#define SCALE_HULL_W   0.90f
#define SCALE_HULL_L   0.90f

// Torre (Turret)
#define SCALE_TURRET_W 0.90f
#define SCALE_TURRET_L 0.90f

// Cano (Pipe): Precisa ser FINO para não bater nos lados girando
// Mas precisa ser LONGO (L=1.0) para bater a ponta corretamente
#define SCALE_PIPE_W   0.3f  
#define SCALE_PIPE_L   1.0f  

// AJUSTE DE ALTURA (Y-RANGE)
// Define onde começa (MIN) e termina (MAX) cada parte no eixo Y do mundo.
#define HULL_Y_MIN 0.0f
#define HULL_Y_MAX 0.9f

// A torre fica em cima da base, então começa quase onde a base termina.
#define TURRET_Y_MIN 0.9f
#define TURRET_Y_MAX 1.4f

// O cano geralmente fica na mesma faixa de altura da torre.
// Importante: Se o PIPE_MIN > HULL_MAX, o cano NUNCA vai bater na base do inimigo!
#define PIPE_Y_MIN   1.0f
#define PIPE_Y_MAX   1.2f

// Função principal que gera a caixa colisora hierárquica
// A partir de uma caixa local (Box) e várias transformações
// Retorna uma CollisionBox pronta para checagem de colisão
// É hierárquica porque considera todas as partes do tanque 
CollisionBox createHierarchicalBox(const Box *localBox, float tx, float ty, float tz,
                                   float scaleW, float scaleL, 
                                   float hullYaw, float terrainPitch, 
                                   float turretYaw, float pipePitch,
                                   float yMinFixed, float yMaxFixed);

int checkCollisionOBB(CollisionBox *a, CollisionBox *b);
// Verifica colisão de uma caixa dinâmica contra todos os objetos estáticos do mapa
int checkCollisionWithWorld(CollisionBox *dynamicBox);
// Verifica se existe uma linha de visão livre entre dois pontos (Raycast)
// Com isso se o jogador se esconder atrás de um objeto o inimigo perde a visão dele
// Retorna 1 se estiver livre, 0 se houver um prédio no caminho.
int checkLineOfSight(Point3D start, Point3D end);

// Funções de Lógica do Jogo
int wouldCollideTank(float nextX, float nextZ, float hullAngleDeg, float nextPitch);
int wouldCollideTurret(float nextTurretAngle);

// NOVA FUNÇÃO ROBUSTA: Verifica se um "Fantasma" do inimigo colide com qualquer coisa
int checkEnemyGhostCollision(int enemyIndex, float x, float y, float z, float hullAngle, float turretAngle, float pitch);

// Debug Visual
void drawDebugBox(CollisionBox b); 
void debugDrawPlayerCollision();
void debugDrawEnemyCollision();
void debugDrawWorldCollisions();

// Helpers
// Player (Usa variáveis globais do tank.c)
CollisionBox makePlayerHull(float x, float z, float hullAngle, float terrainPitch);
CollisionBox makePlayerTurret(float x, float z, float hullAngle, float terrainPitch, float turretLocalAngle);
CollisionBox makePlayerPipe(float x, float z, float hullAngle, float terrainPitch, float turretLocalAngle, float pipeLocalPitch);
// Enemy (Usa a struct Enemy) dados que são vários inimigos
CollisionBox makeEnemyHull(struct Enemy *e, float terrainPitch);
CollisionBox makeEnemyTurret(struct Enemy *e, float terrainPitch);
CollisionBox makeEnemyPipe(struct Enemy *e, float terrainPitch);

// Funções de colisão para POWER UP
CollisionBox getCollisionBox(const Box *localBox, float tx, float ty, float tz, 
    float angleYaw, float anglePitch, 
    float scaleW, float scaleL, 
    float yMinFixed, float yMaxFixed); 

int checkCollisionOBBwithPU(CollisionBox *a, CollisionBox *b);
int checkTankPowerUpCollision(PowerUpInstance *p);
int checkAllPowerUpCollisions(PowerUpInstance powerUps[]);

#endif
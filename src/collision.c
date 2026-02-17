#include "collision.h"
#include "tank.h"
#include "enemy.h"
#include "map.h"
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <GL/glut.h>
#include <power_up.h>

#define RADIAN_FACTOR (3.14159265359 / 180.0)

// MATEMÁTICA VETORIAL 3D

Point3D add(Point3D a, Point3D b) { return (Point3D){a.x + b.x, a.y + b.y, a.z + b.z}; }
Point3D sub(Point3D a, Point3D b) { return (Point3D){a.x - b.x, a.y - b.y, a.z - b.z}; }
float dot(Point3D a, Point3D b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Point3D scaleVec(Point3D v, float s) { return (Point3D){v.x * s, v.y * s, v.z * s}; }

// Produto Vetorial para os eixos das arestas
Point3D crossProduct3D(Point3D a, Point3D b)
{
    return (Point3D){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

Point3D normalizeVector(Point3D v)
{
    float len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len < 0.0001f)
        return (Point3D){0, 0, 0};
    return (Point3D){v.x / len, v.y / len, v.z / len};
}

// ROTAÇÕES
/*
Yaw	    eixo Y	
Pitch	eixo X	
Roll	eixo Z	
*/
// Subir e descer em XZ (pitch) Muito usado no pipe
Point3D rotateX(Point3D p, float angleDeg)
{
    if (fabs(angleDeg) < 0.001f)
        return p;
    float rad = angleDeg * RADIAN_FACTOR;
    float c = cos(rad);
    float s = sin(rad);
    return (Point3D){p.x, p.y * c - p.z * s, p.y * s + p.z * c};
}
// Girar em Y (yaw) apenas
Point3D rotateY(Point3D p, float angleDeg)
{
    if (fabs(angleDeg) < 0.001f)
        return p;
    float rad = angleDeg * RADIAN_FACTOR;
    float c = cos(rad);
    float s = sin(rad);
    return (Point3D){p.x * c + p.z * s, p.y, -p.x * s + p.z * c};
}

// GERAÇÃO DA CAIXA 3D (OBB)
// Box é defindio no mybib.h como:
// typedef struct Box {
//     float minX, minY, minZ;
//     float maxX, maxY, maxZ;
// } Box;
CollisionBox createHierarchicalBox(const Box *localBox, float tx, float ty, float tz,
                                   float scaleW, float scaleL,
                                   float hullYaw, float terrainPitch,
                                   float turretYaw, float pipePitch,
                                   float yMinFixed, float yMaxFixed)
{
    CollisionBox cb;
    // Define o tamanho da caixa antes de girar, o tamanho é afetado pela escala e definido a partir do centro
    float width = (localBox->maxX - localBox->minX) * scaleW;
    float length = (localBox->maxZ - localBox->minZ) * scaleL;
    float height = yMaxFixed - yMinFixed;

    cb.halfSize[0] = width / 2.0f;
    cb.halfSize[1] = height / 2.0f;
    cb.halfSize[2] = length / 2.0f;

    float localCenterY = (yMinFixed + yMaxFixed) / 2.0f;
    // Encontrar a posição local do centro da caixa
    // Basicamente, ele pega a média entre o mínimo e o máximo (o meio) e aplica a escala.
    Point3D localCenter = {
        ((localBox->minX + localBox->maxX) / 2.0f) * scaleW,
        localCenterY,
        ((localBox->minZ + localBox->maxZ) / 2.0f) * scaleL};

    // Lógica de colisão OBB (Oriented Bounding Box, ou Caixa Delimitadora Orientada)
    // Orientada pois ela segue a orientação do tank
    Point3D axis[3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

    localCenter = rotateX(localCenter, pipePitch);
    for (int i = 0; i < 3; i++)
        axis[i] = rotateX(axis[i], pipePitch);

    localCenter = rotateY(localCenter, turretYaw);
    for (int i = 0; i < 3; i++)
        axis[i] = rotateY(axis[i], turretYaw);

    localCenter = rotateX(localCenter, terrainPitch);
    for (int i = 0; i < 3; i++)
        axis[i] = rotateX(axis[i], terrainPitch);

    localCenter = rotateY(localCenter, hullYaw);
    for (int i = 0; i < 3; i++)
        axis[i] = rotateY(axis[i], hullYaw);

    for (int i = 0; i < 3; i++)
        cb.axis[i] = normalizeVector(axis[i]);
    // A posição final do centro da caixa no mundo. Ela é a posição rotacionada + a translação (tx, ty, tz)
    cb.center.x = localCenter.x + tx;
    cb.center.y = localCenter.y + ty;
    cb.center.z = localCenter.z + tz;
    // Calcula os 8 cantos da caixa para facilitar a detecção de colisão depois
    int idx = 0;
    for (int i = -1; i <= 1; i += 2)
    {
        for (int j = -1; j <= 1; j += 2)
        {
            for (int k = -1; k <= 1; k += 2)
            {
                Point3D v = cb.center;
                for (int d = 0; d < 3; d++)
                {
                    float s = (d == 0 ? i : (d == 1 ? j : k)) * cb.halfSize[d];
                    v = add(v, scaleVec(cb.axis[d], s));
                }
                cb.corners[idx++] = v;
            }
        }
    }
    return cb;
}

// COLISÃO SAT 3D (Separating Axis Theorem ou Teorema do Eixo Separador)
// Resumo:
// Imagine que você quer saber se dois objetos colidiram, mas eles estão girados no espaço.
// Uma maneira de fazer isso é olhar para eles de um ângulo específico (um eixo) e ver se as "sombras" (projeções) deles se sobrepõem.
// Se você encontrar pelo menos um ângulo onde as sombras não se tocam, você tem certeza absoluta de que os objetos não estão colidindo.
int getSeparation(CollisionBox *a, CollisionBox *b, Point3D axis)
{
    if (dot(axis, axis) < 0.00001f)
        return 0;

    float minA = FLT_MAX, maxA = -FLT_MAX;
    float minB = FLT_MAX, maxB = -FLT_MAX;

    for (int i = 0; i < 8; i++)
    {
        // Recebe um axis (um vetor de direção). Comprime a caixa 3D inteira em cima dessa única linha.
        // Calcula onde os cantos da caixa "caem" nessa linha (axis).
        float p = dot(a->corners[i], axis);
        if (p < minA)
            minA = p;
        if (p > maxA)
            maxA = p;
    }
    for (int i = 0; i < 8; i++)
    {
        float p = dot(b->corners[i], axis);
        // Encontra o mínimo e máximo dessa projeção para a caixa B
        if (p < minB)
            minB = p;
        if (p > maxB)
            maxB = p;
    }
    return (maxA < minB || maxB < minA);
}
// Testa apenas as faces da caixa, desconsiderando arestas para simplificar
// e melhorar performance. Pode falhar em casos extremos.
// No total, seria 15 eixos (3 de A, 3 de B, 9 cruzamentos)
int checkCollisionOBB(CollisionBox *a, CollisionBox *b)
{
    // Existe um buraco entre os objetos neste ângulo
    // Testa 6 eixos: os 3 de A e os 3 de B
    // Olha para a cena alinhado com o chão do Tanque A. Se vê luz entre os tanques, não colidiu.
    // Olha alinhado com a parede lateral do Tanque A. Se vê luz, não colidiu.
    if (getSeparation(a, b, a->axis[0]))
        return 0;
    if (getSeparation(a, b, a->axis[1]))
        return 0;
    if (getSeparation(a, b, a->axis[2]))
        return 0;

    if (getSeparation(a, b, b->axis[0]))
        return 0;
    if (getSeparation(a, b, b->axis[1]))
        return 0;
    if (getSeparation(a, b, b->axis[2]))
        return 0;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            Point3D axis = crossProduct3D(a->axis[i], b->axis[j]);
            if (getSeparation(a, b, axis))
                return 0;
        }
    }
    return 1;
}
// Verifica colisão de uma caixa dinâmica contra todos os objetos estáticos do mapa
int checkCollisionWithWorld(CollisionBox *dynamicBox)
{
    for (int i = 0; i < staticColliderCount; i++)
    {
        if (checkCollisionOBB(dynamicBox, &staticColliders[i]))
        {
            return 1;
        }
    }
    return 0;
}
// ALGORITMO DE RAYCASTING (VISÃO)
// Retorna 1 se o raio NÃO bater em nada (visão livre).
// Retorna 0 se o raio bater em alguma CollisionBox do mundo (visão obstruída).
int checkLineOfSight(Point3D start, Point3D end)
{
    Point3D dir = sub(end, start);
    float maxDist = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    dir = normalizeVector(dir);
    // Percorre todos os prédios
    for (int i = 0; i < staticColliderCount; i++)
    {
        CollisionBox *box = &staticColliders[i];
        // Transforma o raio para o espaço local da caixa (Slabs Method OBB)
        Point3D diff = sub(box->center, start); // Vetor do inicio do raio ao centro da caixa

        float tMin = -100000.0f;
        float tMax = 100000.0f;
        // Testa interseção nos 3 eixos da caixa
        for (int j = 0; j < 3; j++)
        {
            float e = dot(box->axis[j], diff);
            float f = dot(box->axis[j], dir);

            if (fabs(f) > 0.001f)
            {
                float t1 = (e + box->halfSize[j]) / f; // Plano near
                float t2 = (e - box->halfSize[j]) / f; // Plano far
                if (t1 > t2)
                {
                    float tmp = t1;
                    t1 = t2;
                    t2 = tmp;
                } // Swap
                if (t1 > tMin)
                    tMin = t1;
                if (t2 < tMax)
                    tMax = t2;
                if (tMin > tMax)
                    goto next_box; // Sem interseção neste eixo
                if (tMax < 0)
                    goto next_box; // Caixa está atrás do raio
            }
            else
            {
                if (-e - box->halfSize[j] > 0 || -e + box->halfSize[j] < 0)
                    goto next_box;
            }
        }
        // Se chegou aqui, houve interseção com a caixa 'i'
        if (tMin < maxDist && tMin > 0)
            return 0; // Bloqueado! Bateu num prédio antes de chegar no player.
    next_box:;
    }
    return 1;
}

// (HELPERS)
// Pegam as medidas exatas de cada uma dessas partes (que já estão definidas em variáveis globais ou constantes
// como SCALE_HULL_W, hullModel.box) e preparam a caixa certa.
// Dado um corpo na posição (x, z) com ângulo 'angle', retorna a CollisionBox correta.
// Ela preenche todo o resto (altura, largura, modelo 3D).
// Helpers do PLAYER
CollisionBox makePlayerHull(float x, float z, float hullAngle, float terrainPitch)
{
    return createHierarchicalBox(&hullModel.box, x, player.y, z,
                                 SCALE_HULL_W, SCALE_HULL_L,
                                 hullAngle, terrainPitch,
                                 0.0f, 0.0f,
                                 HULL_Y_MIN, HULL_Y_MAX);
}

CollisionBox makePlayerTurret(float x, float z, float hullAngle, float terrainPitch, float turretLocalAngle)
{
    return createHierarchicalBox(&turretModel.box, x, player.y, z,
                                 SCALE_TURRET_W, SCALE_TURRET_L,
                                 hullAngle, terrainPitch,
                                 turretLocalAngle, 0.0f,
                                 TURRET_Y_MIN, TURRET_Y_MAX);
}

CollisionBox makePlayerPipe(float x, float z, float hullAngle, float terrainPitch, float turretLocalAngle, float pipeLocalPitch)
{
    return createHierarchicalBox(&pipeModel.box, x, player.y, z,
                                 SCALE_PIPE_W, SCALE_PIPE_L,
                                 hullAngle, terrainPitch,
                                 turretLocalAngle, pipeLocalPitch,
                                 PIPE_Y_MIN, PIPE_Y_MAX);
}
// Helpers do INIMIGO
CollisionBox makeEnemyHull(struct Enemy *e, float terrainPitch)
{
    return createHierarchicalBox(&enemyHullModel.box, e->x, e->y, e->z,
                                 ENEMY_SCALE_HULL_W, ENEMY_SCALE_HULL_L,
                                 e->hullAngle, terrainPitch,
                                 0.0f, 0.0f,
                                 HULL_Y_MIN, HULL_Y_MAX);
}

CollisionBox makeEnemyTurret(struct Enemy *e, float terrainPitch)
{
    return createHierarchicalBox(&enemyTurretModel.box, e->x, e->y, e->z,
                                 ENEMY_SCALE_TURRET_W, ENEMY_SCALE_TURRET_L,
                                 e->hullAngle, terrainPitch,
                                 e->turretAngle, 0.0f,
                                 TURRET_Y_MIN, TURRET_Y_MAX);
}

CollisionBox makeEnemyPipe(struct Enemy *e, float terrainPitch)
{
    return createHierarchicalBox(&enemyPipeModel.box, e->x, e->y, e->z,
                                 ENEMY_SCALE_PIPE_W, ENEMY_SCALE_PIPE_L,
                                 e->hullAngle, terrainPitch,
                                 e->turretAngle, 0.0f,
                                 PIPE_Y_MIN, PIPE_Y_MAX);
}

// FUNÇÕES PRINCIPAIS DE COLISÃO DO JOGO
// Verifica se o tanque na próxima posição colidiria com algum inimigo
int wouldCollideTank(float nextX, float nextZ, float hullAngleDeg, float nextPitch)
{
    CollisionBox pHull = makePlayerHull(nextX, nextZ, hullAngleDeg, nextPitch);
    CollisionBox pTurret = makePlayerTurret(nextX, nextZ, hullAngleDeg, nextPitch, player.turretAngle);
    CollisionBox pPipe = makePlayerPipe(nextX, nextZ, hullAngleDeg, nextPitch, player.turretAngle, player.pipeAngle);

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].alive)
            continue;
        // Colisão com o chão para a caixa de colisão do inimig
        // Isso é necessário para posicionar corretamente a caixa de colisão no terreno e verificar a colisão
        float ePitch = getTerrainPitch(enemies[i].x, enemies[i].z, enemies[i].hullAngle);
        CollisionBox eHull = makeEnemyHull(&enemies[i], ePitch);
        CollisionBox eTurret = makeEnemyTurret(&enemies[i], ePitch);
        CollisionBox ePipe = makeEnemyPipe(&enemies[i], ePitch);

        if (checkCollisionOBB(&pHull, &eHull) || checkCollisionOBB(&pHull, &eTurret) || checkCollisionOBB(&pHull, &ePipe))
            return 1;
        if (checkCollisionOBB(&pTurret, &eHull) || checkCollisionOBB(&pTurret, &eTurret) || checkCollisionOBB(&pTurret, &ePipe))
            return 1;
        if (checkCollisionOBB(&pPipe, &eHull) || checkCollisionOBB(&pPipe, &eTurret) || checkCollisionOBB(&pPipe, &ePipe))
            return 1;
    }
    return 0;
}
// Verifica se a torre do tanque colidiria com algum inimigo ao girar
int wouldCollideTurret(float nextTurretAngle)
{
    float pPitch = player.pitch;

    CollisionBox pHull = makePlayerHull(player.x, player.z, player.hullAngle, pPitch);
    CollisionBox pTurret = makePlayerTurret(player.x, player.z, player.hullAngle, pPitch, nextTurretAngle);
    CollisionBox pPipe = makePlayerPipe(player.x, player.z, player.hullAngle, pPitch, nextTurretAngle, player.pipeAngle);

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].alive)
            continue;

        float ePitch = getTerrainPitch(enemies[i].x, enemies[i].z, enemies[i].hullAngle);
        CollisionBox eHull = makeEnemyHull(&enemies[i], ePitch);
        CollisionBox eTurret = makeEnemyTurret(&enemies[i], ePitch);
        CollisionBox ePipe = makeEnemyPipe(&enemies[i], ePitch);

        if (checkCollisionOBB(&pTurret, &eHull) || checkCollisionOBB(&pTurret, &eTurret) || checkCollisionOBB(&pTurret, &ePipe))
            return 1;
        if (checkCollisionOBB(&pPipe, &eHull) || checkCollisionOBB(&pPipe, &eTurret) || checkCollisionOBB(&pPipe, &ePipe))
            return 1;
        if (checkCollisionOBB(&pHull, &eHull) || checkCollisionOBB(&pHull, &eTurret) || checkCollisionOBB(&pHull, &ePipe))
            return 1;
    }
    return 0;
}

// ============================================================
// Retorna o TIPO DE COLISÃO
// 0 = Livre
// 1 = Parede/Mundo (Deve manobrar)
// 2 = Tanque/Player (Deve parar, mas não manobrar)
// ============================================================
int checkEnemyGhostCollision(int enemyIndex, float x, float y, float z, float hullAngle, float turretAngle, float pitch)
{
    // Caixas do "Fantasma" (Inimigo hipotético)
    CollisionBox gHull = createHierarchicalBox(&enemyHullModel.box, x, y, z,
                                               ENEMY_SCALE_HULL_W, ENEMY_SCALE_HULL_L,
                                               hullAngle, pitch,
                                               0.0f, 0.0f,
                                               HULL_Y_MIN, HULL_Y_MAX);

    CollisionBox gTurret = createHierarchicalBox(&enemyTurretModel.box, x, y, z,
                                                 ENEMY_SCALE_TURRET_W, ENEMY_SCALE_TURRET_L,
                                                 hullAngle, pitch,
                                                 turretAngle, 0.0f,
                                                 TURRET_Y_MIN, TURRET_Y_MAX);

    CollisionBox gPipe = createHierarchicalBox(&enemyPipeModel.box, x, y, z,
                                               ENEMY_SCALE_PIPE_W, ENEMY_SCALE_PIPE_L,
                                               hullAngle, pitch,
                                               turretAngle, 0.0f,
                                               PIPE_Y_MIN, PIPE_Y_MAX);

    // Checa contra o MUNDO (Paredes) -> Retorna 1 (Manobrar)
    if (checkCollisionWithWorld(&gHull) || checkCollisionWithWorld(&gTurret) || checkCollisionWithWorld(&gPipe))
    {
        return 1;
    }

    // Checa contra o PLAYER -> Retorna 2 (Parar apenas)
    float pPitch = player.pitch;
    CollisionBox pHull = makePlayerHull(player.x, player.z, player.hullAngle, pPitch);
    CollisionBox pTurret = makePlayerTurret(player.x, player.z, player.hullAngle, pPitch, player.turretAngle);
    CollisionBox pPipe = makePlayerPipe(player.x, player.z, player.hullAngle, pPitch, player.turretAngle, player.pipeAngle);

    if (checkCollisionOBB(&gHull, &pHull) || checkCollisionOBB(&gHull, &pTurret) || checkCollisionOBB(&gHull, &pPipe))
        return 2;
    if (checkCollisionOBB(&gTurret, &pHull) || checkCollisionOBB(&gTurret, &pTurret) || checkCollisionOBB(&gTurret, &pPipe))
        return 2;
    if (checkCollisionOBB(&gPipe, &pHull) || checkCollisionOBB(&gPipe, &pTurret) || checkCollisionOBB(&gPipe, &pPipe))
        return 2;

    // Checa contra OUTROS INIMIGOS -> Retorna 2 (Parar apenas)
    // Se eu quiser impedir que eles fiquem parados de alguma forma poderia retornar 1 aqui
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (i == enemyIndex || !enemies[i].alive)
            continue;

        float oPitch = enemies[i].pitch;
        CollisionBox oHull = makeEnemyHull(&enemies[i], oPitch);
        CollisionBox oTurret = makeEnemyTurret(&enemies[i], oPitch);
        CollisionBox oPipe = makeEnemyPipe(&enemies[i], oPitch);

        if (checkCollisionOBB(&gHull, &oHull) || checkCollisionOBB(&gHull, &oTurret) || checkCollisionOBB(&gHull, &oPipe))
            return 2;
        if (checkCollisionOBB(&gTurret, &oHull) || checkCollisionOBB(&gTurret, &oTurret) || checkCollisionOBB(&gTurret, &oPipe))
            return 2;
        if (checkCollisionOBB(&gPipe, &oHull) || checkCollisionOBB(&gPipe, &oTurret) || checkCollisionOBB(&gPipe, &oPipe))
            return 2;
    }

    return 0; // Livre
}

// DEBUG VISUAL

void drawDebugBox(CollisionBox b)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(2.0f);
    glColor3f(1.0f, 0.0f, 0.0f);

    glBegin(GL_LINES);
    glVertex3f(b.corners[0].x, b.corners[0].y, b.corners[0].z);
    glVertex3f(b.corners[1].x, b.corners[1].y, b.corners[1].z);
    glVertex3f(b.corners[1].x, b.corners[1].y, b.corners[1].z);
    glVertex3f(b.corners[3].x, b.corners[3].y, b.corners[3].z);
    glVertex3f(b.corners[3].x, b.corners[3].y, b.corners[3].z);
    glVertex3f(b.corners[2].x, b.corners[2].y, b.corners[2].z);
    glVertex3f(b.corners[2].x, b.corners[2].y, b.corners[2].z);
    glVertex3f(b.corners[0].x, b.corners[0].y, b.corners[0].z);

    glVertex3f(b.corners[4].x, b.corners[4].y, b.corners[4].z);
    glVertex3f(b.corners[5].x, b.corners[5].y, b.corners[5].z);
    glVertex3f(b.corners[5].x, b.corners[5].y, b.corners[5].z);
    glVertex3f(b.corners[7].x, b.corners[7].y, b.corners[7].z);
    glVertex3f(b.corners[7].x, b.corners[7].y, b.corners[7].z);
    glVertex3f(b.corners[6].x, b.corners[6].y, b.corners[6].z);
    glVertex3f(b.corners[6].x, b.corners[6].y, b.corners[6].z);
    glVertex3f(b.corners[4].x, b.corners[4].y, b.corners[4].z);

    glVertex3f(b.corners[0].x, b.corners[0].y, b.corners[0].z);
    glVertex3f(b.corners[4].x, b.corners[4].y, b.corners[4].z);
    glVertex3f(b.corners[1].x, b.corners[1].y, b.corners[1].z);
    glVertex3f(b.corners[5].x, b.corners[5].y, b.corners[5].z);
    glVertex3f(b.corners[2].x, b.corners[2].y, b.corners[2].z);
    glVertex3f(b.corners[6].x, b.corners[6].y, b.corners[6].z);
    glVertex3f(b.corners[3].x, b.corners[3].y, b.corners[3].z);
    glVertex3f(b.corners[7].x, b.corners[7].y, b.corners[7].z);
    glEnd();

    glEnable(GL_LIGHTING);
}

void debugDrawPlayerCollision()
{
    // o player é definido no tank.h
    float pitch = player.pitch;
    drawDebugBox(makePlayerHull(player.x, player.z, player.hullAngle, pitch));
    drawDebugBox(makePlayerTurret(player.x, player.z, player.hullAngle, pitch, player.turretAngle));
    drawDebugBox(makePlayerPipe(player.x, player.z, player.hullAngle, pitch, player.turretAngle, player.pipeAngle));
}

void debugDrawEnemyCollision()
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].alive)
            continue;
        // Colisão com o chão da caixa de colisão de debug do inimigo 
        float pitch = getTerrainPitch(enemies[i].x, enemies[i].z, enemies[i].hullAngle);
        drawDebugBox(makeEnemyHull(&enemies[i], pitch));
        drawDebugBox(makeEnemyTurret(&enemies[i], pitch));
        drawDebugBox(makeEnemyPipe(&enemies[i], pitch));
    }
}

void debugDrawWorldCollisions()
{
    for (int i = 0; i < staticColliderCount; i++)
    {
        drawDebugBox(staticColliders[i]);
    }
}

// FUNÇÕES DE COLISÃO COM POWER UP

int checkTankPowerUpCollision(PowerUpInstance *p)
{
    // if (!p->active) return 0;  // ignorar PUs já coletados
    ObjModel pu = getObjModel(*p);

    float halfHeight = 0.5f; // metade de p->y
    CollisionBox puBox = getCollisionBox(&pu.box, p->x, p->y, p->z, 0.0f, 0.0f, p->scale, p->scale, p->y - halfHeight, p->y + halfHeight);

    // colisão com o tanque (carcaça do player)
    CollisionBox tankHull = makePlayerHull(player.x, player.z, player.hullAngle, player.pitch);

    if (checkCollisionOBBwithPU(&tankHull, &puBox))
        return 1;

    return 0;
}

int checkAllPowerUpCollisions(PowerUpInstance powerUps[])
{
    for (int i = 0; i < MAX_POWER_UPS; i++)
    {
        if (powerUps[i].active && checkTankPowerUpCollision(&powerUps[i]))
        {
            return i; // retorna o índice do power-up coletado
        }
    }

    return -1; // nenhum power-up tocado
}

CollisionBox getCollisionBox(const Box *localBox, float tx, float ty, float tz,
                             float angleYaw, float anglePitch,
                             float scaleW, float scaleL,
                             float yMinFixed, float yMaxFixed)
{
    CollisionBox cb;

    // Define o tamanho da caixa antes de girar, o tamanho é afetado pela escala e definido a partir do centro
    float width = (localBox->maxX - localBox->minX) * scaleW;
    float length = (localBox->maxZ - localBox->minZ) * scaleL;
    float height = yMaxFixed - yMinFixed;

    cb.halfSize[0] = width / 2.0f;
    cb.halfSize[1] = height / 2.0f;
    cb.halfSize[2] = length / 2.0f;

    float localCenterY = (yMinFixed + yMaxFixed) / 2.0f;
    // Encontrar a posição local do centro da caixa
    // Basicamente, ele pega a média entre o mínimo e o máximo (o meio) e aplica a escala.
    Point3D localPos = {
        ((localBox->minX + localBox->maxX) / 2.0f) * scaleW,
        localCenterY - ty,
        ((localBox->minZ + localBox->maxZ) / 2.0f) * scaleL};
    // Lógica de colisão OBB (Oriented Bounding Box, ou Caixa Delimitadora Orientada)
    // Orientada pois ela segue a orientação do tank
    Point3D axisBase[3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    // A ideia é calcular a orientação dos eixos da caixa aplicando as rotações necessárias.
    for (int i = 0; i < 3; i++)
    {
        Point3D tmp = axisBase[i];
        // Pega os vetores base originais e aplica as rotações anglePitch (inclinação)
        // e angleYaw (direção, virar para esquerda/direita).
        if (fabs(anglePitch) > 0.001f)
            tmp = rotateX(tmp, anglePitch);
        tmp = rotateY(tmp, angleYaw);
        cb.axis[i] = normalizeVector(tmp);
    }
    // Aplica as rotações ao centro local para obter a posição correta no mundo
    Point3D centerRot = localPos;
    if (fabs(anglePitch) > 0.001f)
        centerRot = rotateX(centerRot, anglePitch);
    centerRot = rotateY(centerRot, angleYaw);
    // A posição final do centro da caixa no mundo. Ela é a posição rotacionada + a translação (tx, ty, tz)
    cb.center.x = centerRot.x + tx;
    cb.center.y = centerRot.y + ty;
    cb.center.z = centerRot.z + tz;
    // Calcula os 8 cantos da caixa para facilitar a detecção de colisão depois
    int idx = 0;
    for (int i = -1; i <= 1; i += 2)
    {
        for (int j = -1; j <= 1; j += 2)
        {
            for (int k = -1; k <= 1; k += 2)
            {
                Point3D v = cb.center;
                for (int d = 0; d < 3; d++)
                {
                    float s = (d == 0 ? i : (d == 1 ? j : k)) * cb.halfSize[d];
                    v.x += cb.axis[d].x * s;
                    v.y += cb.axis[d].y * s;
                    v.z += cb.axis[d].z * s;
                }
                cb.corners[idx++] = v;
            }
        }
    }
    return cb;
}

// Testa apenas as faces da caixa, desconsiderando arestas para simplificar
// e melhorar performance. Pode falhar em casos extremos.
// No total, seria 15 eixos (3 de A, 3 de B, 9 cruzamentos)
int checkCollisionOBBwithPU(CollisionBox *a, CollisionBox *b) {
    // Existe um buraco entre os objetos neste ângulo
    // Testa 6 eixos: os 3 de A e os 3 de B
    // Olha para a cena alinhado com o chão do Tanque A. Se vê luz entre os tanques, não colidiu.
    // Olha alinhado com a parede lateral do Tanque A. Se vê luz, não colidiu.
    if(getSeparation(a, b, a->axis[0])) return 0;
    if(getSeparation(a, b, a->axis[1])) return 0;
    if(getSeparation(a, b, a->axis[2])) return 0;

    if(getSeparation(a, b, b->axis[0])) return 0;
    if(getSeparation(a, b, b->axis[1])) return 0;
    if(getSeparation(a, b, b->axis[2])) return 0;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            Point3D axis = crossProduct3D(a->axis[i], b->axis[j]);
            if(getSeparation(a, b, axis)) return 0;
        }
    }

    return 1;
}

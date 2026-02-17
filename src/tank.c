#include "tank.h"
#include "collision.h" // Inclui para usar CollisionBox, macros de escala e a função getCollisionBox
#include "map.h"       // Inclui para usar checkCollisionWithWorld
#include <power_up.h>

Tank player; // Agora tudo é centralizado aqui

const double RADIAN_FACTOR = 3.14159 / 180.0;

ObjModel turretModel, pipeModel, hullModel;

void drawTank()
{
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
        // Usa player.y que agora é dinâmico
        glTranslatef(player.x, player.y, player.z);
        
        // ROTAÇÃO DO HULL ANGLE (YAW)
        glRotatef(player.hullAngle, 0.0f, 1.0f, 0.0f);

        // NOVA ROTAÇÃO DE PITCH (SUBIDA/DESCIDA)
        glRotatef(player.pitch, 1.0f, 0.0f, 0.0f);

        // HULL
        glPushMatrix();
            // Sem glRotate(hull) aqui pois já foi no pai
            drawModel(&hullModel);
        glPopMatrix();

        // TURRET + PIPE
        glPushMatrix();
            glRotatef(player.turretAngle, 0.0f, 1.0f, 0.0f);
            drawModel(&turretModel);

    glRotatef(player.pipeAngle, 1.0f, 0.0f, 0.0f);
    drawModel(&pipeModel);
    glPopMatrix();

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void updateTank() {
    if (freeCameraMode) {
        glutPostRedisplay();
        return;
    }
    float nextX = player.x;
    float nextZ = player.z;
    float nextHullAngle = player.hullAngle;
    
    // inputs de movimento do tanque
    if (keyStates['w'] || keyStates['W'])
    {
        nextX -= sinf(player.hullAngle * RADIAN_FACTOR) * player.moveSpeed;
        nextZ -= cosf(player.hullAngle * RADIAN_FACTOR) * player.moveSpeed;
    }
    if (keyStates['s'] || keyStates['S'])
    {
        nextX += sinf(player.hullAngle * RADIAN_FACTOR) * player.moveSpeed;
        nextZ += cosf(player.hullAngle * RADIAN_FACTOR) * player.moveSpeed;
    }
    if (keyStates['a'] || keyStates['A'])
        nextHullAngle += TANK_ROT_SPEED;
    if (keyStates['d'] || keyStates['D'])
        nextHullAngle -= TANK_ROT_SPEED;

    // ============================================
    // Calcular altura e PITCH na próxima posição
    // ============================================
    float nextY = getTerrainHeight(nextX, nextZ) + TANK_GROUND_DISTANCE;
    float nextPitch = getTerrainPitch(nextX, nextZ, nextHullAngle);

    // Verificação de colisão antes de aplicar o movimento
    
    // Gera as caixas hipotéticas de TODAS as partes na nova posição e NOVA ALTURA
    // Nota: Passamos a hierarquia completa agora.
    
    // Hull
    CollisionBox nextHullBox = makePlayerHull(nextX, nextZ, nextHullAngle, nextPitch);

    // Turret (Na nova posição, com o angulo atual da torre)
    CollisionBox nextTurretBox = makePlayerTurret(nextX, nextZ, nextHullAngle, nextPitch, player.turretAngle);
                                                 
    // Pipe
    CollisionBox nextPipeBox = makePlayerPipe(nextX, nextZ, nextHullAngle, nextPitch, player.turretAngle, player.pipeAngle);

    // Verifica colisão com o MUNDO ESTÁTICO (Prédios, etc)
    int hitWorld = checkCollisionWithWorld(&nextHullBox) || 
                   checkCollisionWithWorld(&nextTurretBox) || 
                   checkCollisionWithWorld(&nextPipeBox);

    // Verifica colisão com INIMIGOS e aplica movimento se livre
    // Passamos o nextPitch para a função wouldCollideTank também
    if (!hitWorld && !wouldCollideTank(nextX, nextZ, nextHullAngle, nextPitch)) {
        player.x = nextX;
        player.z = nextZ;
        player.y = nextY; 
        player.hullAngle = nextHullAngle;
        player.pitch = nextPitch; // Atualiza o pitch
    } else {
        // Se bater, ainda atualizamos a altura e pitch na posição ATUAL
        player.y = getTerrainHeight(player.x, player.z) + TANK_GROUND_DISTANCE;
        player.pitch = getTerrainPitch(player.x, player.z, player.hullAngle);
    }
    
    // Lógica da Torre
    float nextTurretAngle = player.turretAngle;

    if (specialKeyStates[GLUT_KEY_LEFT])
        nextTurretAngle += TANK_ROT_SPEED;
    if (specialKeyStates[GLUT_KEY_RIGHT])
        nextTurretAngle -= TANK_ROT_SPEED;

    // Se houve tentativa de girar a torre
    if (nextTurretAngle != player.turretAngle) {
        
        // Verifica inimigos E mapa usando as funções atualizadas
        // As funções helper dentro de wouldCollide já usam o player.pitch atualizado acima
        int colidiuInimigo = wouldCollideTurret(nextTurretAngle);
        
        // Para o mapa, criamos caixas temporárias
        CollisionBox rotTurretBox = makePlayerTurret(player.x, player.z, player.hullAngle, player.pitch, nextTurretAngle);
        CollisionBox rotPipeBox = makePlayerPipe(player.x, player.z, player.hullAngle, player.pitch, nextTurretAngle, player.pipeAngle);
        
        int colidiuMapa = checkCollisionWithWorld(&rotTurretBox) || checkCollisionWithWorld(&rotPipeBox);

        if (!colidiuInimigo && !colidiuMapa) {
            player.turretAngle = nextTurretAngle;
        }
    }

    // Inclinação do Cano (Cima/Baixo)
    if (specialKeyStates[GLUT_KEY_UP])
    {
        player.pipeAngle += player.pipeInclineSpeed;
        if (player.pipeAngle > MAX_PIPE_ANGLE)
            player.pipeAngle = MAX_PIPE_ANGLE;
    }

    if (specialKeyStates[GLUT_KEY_DOWN])
    {
        player.pipeAngle -= player.pipeInclineSpeed;
        if (player.pipeAngle < MIN_PIPE_ANGLE)
            player.pipeAngle = MIN_PIPE_ANGLE;
    }

    // Verifica colisão do tank com POWER UP
    int puIndex = checkAllPowerUpCollisions(powerUps);
    if (puIndex != -1)
    {
        applyPowerUpEffect(powerUps[puIndex].type);
        powerUps[puIndex].active = 0; // remove do mapa
    }

    updateMapCellPos();
    glutPostRedisplay();
}

void updateMapCellPos()
{

    int posX_A = mapCells[player.mapCellZ][player.mapCellX].A.x;
    int posZ_A = mapCells[player.mapCellZ][player.mapCellX].A.z;
    int posX_D = mapCells[player.mapCellZ][player.mapCellX].D.x;
    int posZ_D = mapCells[player.mapCellZ][player.mapCellX].D.z;

    if (player.x < posX_A && player.mapCellX > 0)
        player.mapCellX--;
    else if (player.x > posX_D && player.mapCellX < 49)
        player.mapCellX++;

    if (player.z < posZ_A && player.mapCellZ > 0)
        player.mapCellZ--;
    else if (player.z > posZ_D && player.mapCellZ < 49)
        player.mapCellZ++;
}

void initTank()
{

    if (!loadOBJ("objects/turret.obj", "objects/turret.mtl", &turretModel))
        printf("ERRO ao carregar modelo da torreta.\n");

    if (!loadOBJ("objects/pipe.obj", "objects/pipe.mtl", &pipeModel))
        printf("ERRO ao carregar modelo do canhão.\n");

    if (!loadOBJ("objects/hull.obj", "objects/hull.mtl", &hullModel))
        printf("ERRO ao carregar modelo da base.\n");

    // ---------------------------
    // Inicializando o Tank player
    // ---------------------------
    player.mapCellX = INITIAL_TANK_CELL_X;
    player.mapCellZ = INITIAL_TANK_CELL_Z;

    player.x = mapCells[player.mapCellZ][player.mapCellX].C.x;
    player.z = mapCells[player.mapCellZ][player.mapCellX].C.z;
    
    // Inicializa a altura correta
    player.y = getTerrainHeight(player.x, player.z) + TANK_GROUND_DISTANCE;
    player.pitch = getTerrainPitch(player.x, player.z, 0.0f);

    player.hullAngle = 0;
    player.turretAngle = 0;
    player.pipeAngle = 0;

    player.moveSpeed = TANK_MOVEMENT_SPEED;
    player.tankRotSpeed = TANK_ROT_SPEED;
    player.turretRotSpeed = TANK_ROT_SPEED;
    player.pipeInclineSpeed = PIPE_INCLINE_SPEED;

    player.health = 100;
    player.alive = 1;
    player.ammo = 100;
    player.bulletDmg = 33;

    player.lastShootTime = 0;
    player.reloadTime = 3000;
    player.flagReloadCircle = 0;

    player.shieldOn = 0;
}

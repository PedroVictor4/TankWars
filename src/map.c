#include "map.h"
#include <stdio.h>
#include <mybib.h>
#include <math.h>
#include "collision.h" 

#define MAX_STATIC_COLLIDERS 100

// Array global que armazena todas as caixas de colisão de prédios, árvores, etc.
CollisionBox staticColliders[MAX_STATIC_COLLIDERS];
int staticColliderCount = 0;

square mapCells[MAP_SIZE][MAP_SIZE];

float heightMatrix[VERTEX_NUM][VERTEX_NUM];

vertex vertexNormals[VERTEX_NUM][VERTEX_NUM];

ObjModel churchModel;
ObjModel houseModel;
ObjModel gasStationModel;
ObjModel storeModel;
ObjModel wallModel;

// Função auxiliar interna para registrar um objeto estático no sistema de colisão
void addStaticCollider(ObjModel *model, float x, float y, float z, float scale, float rotAngleY) {
    if (staticColliderCount >= MAX_STATIC_COLLIDERS) {
        printf("AVISO: Limite de colisores estaticos (MAX_STATIC_COLLIDERS) atingido.\n");
        return;
    }

    // Fundação infinita (-50.0f)
    // Isso garante que o tanque e o pipe batam na parede independente do relevo.
    // O teto (yMax) continua respeitando a altura do modelo para não bater no ar.
    
    float yMin = -50.0f; 
    float yMax = y + (model->box.maxY * scale);

    // Para objetos estáticos (prédios), não temos Pitch do terreno, nem torre/cano girando.
    // Então passamos 0.0f nesses parâmetros.
    CollisionBox cb = createHierarchicalBox(&model->box, x, y, z, 
                                            scale, scale,       // Escala W e L
                                            rotAngleY, 0.0f,    // HullYaw, TerrainPitch
                                            0.0f, 0.0f,         // TurretYaw, PipePitch
                                            yMin, yMax);
    
    staticColliders[staticColliderCount] = cb;
    staticColliderCount++;
}

// Interpolação Bilinear para altura do terreno
/*
Basicamente, pegamos a célula do mapa onde o ponto (x, z), o centro do tank inimigo ou alidado, está localizado.
E calculamos a altura que ele deveria estar baseado na altura dos 4 vértices daquela célula.
*/
float getTerrainHeight(float x, float z) {
    // Verifica limites do mapa
    if (x < 0 || x >= MAP_SIZE || z < 0 || z >= MAP_SIZE) {
        return 0.0f; // Fora do mapa retorna 0
    }

    int cellX = (int)x;
    int cellZ = (int)z;

    // Garante que não estoure o array
    if (cellX >= MAP_SIZE) cellX = MAP_SIZE - 1;
    if (cellZ >= MAP_SIZE) cellZ = MAP_SIZE - 1;

    // Pega a diferença decimal (0.0 a 1.0) dentro da célula
    // Isso é importante pois imagine que um quadrado do mapa é formado por 4 vértices
    /*
    A-----C
    |     |
    |     |
    B-----D
    Eu quero saber o quão proximo de A eu estou (dx) e o quão proximo de B eu estou (dz).
    Assim eu consigo calcular melhor a altura em que o centro do tank deveria estar.
    */
    float dx = x - (float)cellX;
    float dz = z - (float)cellZ;

    // Alturas dos 4 cantos da célula atual
    float hA = mapCells[cellZ][cellX].A.y; // Top-Left
    float hC = mapCells[cellZ][cellX].C.y; // Top-Right (X+1)
    float hB = mapCells[cellZ][cellX].B.y; // Bottom-Left (Z+1)
    float hD = mapCells[cellZ][cellX].D.y; // Bottom-Right (X+1, Z+1)

    // Interpolação no eixo X (Cima e Baixo)
    float heightTop = hA * (1.0f - dx) + hC * dx;
    float heightBottom = hB * (1.0f - dx) + hD * dx;

    // Interpolação no eixo Z (Final)
    float finalHeight = heightTop * (1.0f - dz) + heightBottom * dz;

    return finalHeight;
}

/*
Yaw	    eixo Y	
Pitch	eixo X	
Roll	eixo Z	
*/
float getTerrainPitch(float x, float z, float angleYaw) {
    float rad = angleYaw * (3.14159f / 180.0f);
    
    // Distância de amostragem (quanto maior, mais suave, mas pode atravessar picos agudos)
    float sampleDist = 0.8f; 

    // O tanque anda "para trás" no eixo Z quando o angulo é 0?
    // Baseado no input: nextX -= sin(ang), nextZ -= cos(ang)
    // Isso significa que o vetor "Frente" é (-sin, -cos)
    
    float frontX = x - sinf(rad) * sampleDist;
    float frontZ = z - cosf(rad) * sampleDist;
    
    float backX = x + sinf(rad) * sampleDist;
    float backZ = z + cosf(rad) * sampleDist;

    float hFront = getTerrainHeight(frontX, frontZ);
    float hBack = getTerrainHeight(backX, backZ);

    float dy = hFront - hBack;
    float dist = sampleDist * 2.0f;

    // atan2 retorna radianos. Convertemos para graus.
    // Se a frente é mais alta, o pitch deve ser positivo (nariz pra cima)
    // Retorno do angulo que deve ser aplicado no pitch do tanque
    return atan2f(dy, dist) * (180.0f / 3.14159f);
}

void addHill(float centerX, float centerZ, float radius, float height) {
    for (int z = 0; z < VERTEX_NUM; z++) {
        for (int x = 0; x < VERTEX_NUM; x++) {
            float dx = x - centerX;
            float dz = z - centerZ;
            float distance = sqrt(dx * dx + dz * dz);
            
            if (distance < radius) {
                float factor = (cos(distance / radius * 3.14159f) + 1.0f) / 2.0f;
                heightMatrix[z][x] += height * factor;
            }
        }
    }
}

void initHeightMatrix(){
    // Primeiro, deixa tudo plano (altura 0)
    for (int z = 0; z < VERTEX_NUM; z++) {
        for (int x = 0; x < VERTEX_NUM; x++) {
            heightMatrix[z][x] = 0.0f;
        }
    }

    //ELEVAÇÕES - x, z, raio, altura
    addHill(40, 25, 5.0f, 1.0f);  // colina
    addHill(25, 26, 5.0f, -0.9f);  // buraco
    addHill(10, 27, 5.0f, 1.0f); // colina
}

void drawWall(float x, float y, float z, float width, float height, float depth) {
    
    glPushMatrix();

    glTranslatef(x, y, z);

    // A escala faz o cubo virar um paralelepipedo
    glScalef(width, height, depth);
    glutSolidCube(1.0);

    glPopMatrix();
}

void initChurch() {
    loadOBJ("objects/igreja.obj", "objects/igreja.mtl", &churchModel);
    
    // Adiciona colisor da Igreja
    int cellX = (int)CHURCH_X;
    int cellZ = (int)CHURCH_Z;
    float y = mapCells[cellZ][cellX].A.y;
    addStaticCollider(&churchModel, CHURCH_X, y, CHURCH_Z, 1.0f, 180.0f);
}

void initHouse() {
    loadOBJ("objects/house.obj", "objects/house.mtl", &houseModel);

    // Casa 1
    int h1X = (int)HOUSE_X;
    int h1Z = (int)HOUSE_Z;
    float h1Y = mapCells[h1Z][h1X].A.y;
    addStaticCollider(&houseModel, HOUSE_X, h1Y, HOUSE_Z, 1.0f, 0.0f);

    // Casa 2
    int h2X = (int)HOUSE2_X;
    int h2Z = (int)HOUSE2_Z;
    float h2Y = mapCells[h2Z][h2X].A.y;
    addStaticCollider(&houseModel, HOUSE2_X, h2Y, HOUSE2_Z, 1.0f, 180.0f);
}

void initGasStation() {
    loadOBJ("objects/gasStation.obj", "objects/gasStation.mtl", &gasStationModel);

    // Posto 1
    int g1X = (int)GASSTATION_X;
    int g1Z = (int)GASSTATION_Z;
    float g1Y = mapCells[g1Z][g1X].A.y;
    addStaticCollider(&gasStationModel, GASSTATION_X, g1Y, GASSTATION_Z, 0.7f, 0.0f);

    // Posto 2
    int g2X = (int)GASSTATION2_X;
    int g2Z = (int)GASSTATION2_Z;
    float g2Y = mapCells[g2Z][g2X].A.y;
    addStaticCollider(&gasStationModel, GASSTATION2_X, g2Y, GASSTATION2_Z, 0.7f, 0.0f);

    // Posto 3
    int g3X = (int)GASSTATION3_X;
    int g3Z = (int)GASSTATION3_Z;
    float g3Y = mapCells[g3Z][g3X].A.y;
    addStaticCollider(&gasStationModel, GASSTATION3_X, g3Y, GASSTATION3_Z, 0.7f, 180.0f);

    // Posto 4
    int g4X = (int)GASSTATION4_X;
    int g4Z = (int)GASSTATION4_Z;
    float g4Y = mapCells[g4Z][g4X].A.y;
    addStaticCollider(&gasStationModel, GASSTATION4_X, g4Y, GASSTATION4_Z, 0.7f, 180.0f);
}

void initStore() {
    loadOBJ("objects/store.obj", "objects/store.mtl", &storeModel);

    // Loja
    int sX = (int)STORE_X;
    int sZ = (int)STORE_Z;
    float sY = mapCells[sZ][sX].A.y;
    addStaticCollider(&storeModel, STORE_X, sY, STORE_Z, 1.0f, 180.0f);
}

void initMapCells(){
    initHeightMatrix();

    float coordZ = 0;
    float coordX = 0;

    for (int z = 0; z < MAP_SIZE; z++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            mapCells[z][x].A.x = coordX;
            mapCells[z][x].A.z = coordZ;
            mapCells[z][x].B.x = coordX;
            mapCells[z][x].B.z = coordZ + 1;
            mapCells[z][x].C.x = coordX + 1;
            mapCells[z][x].C.z = coordZ;
            mapCells[z][x].D.x = coordX + 1;
            mapCells[z][x].D.z = coordZ + 1;
            coordX += 1;
        }
        coordZ += 1;
        coordX = 0;
    }

    // Atualiza Ys
    for (int z = 0; z < MAP_SIZE; z++) {
        for (int x = 0; x < MAP_SIZE; x++){
            mapCells[z][x].A.y = heightMatrix[z][x];
            mapCells[z][x].B.y = heightMatrix[z + 1][x];
            mapCells[z][x].C.y = heightMatrix[z][x + 1];
            mapCells[z][x].D.y = heightMatrix[z + 1][x + 1];
        }
    }
}

void drawMap() {
    for (int z = 0; z < MAP_SIZE; z++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            
            float avgHeight = (mapCells[z][x].A.y + mapCells[z][x].B.y + mapCells[z][x].C.y + mapCells[z][x].D.y) / 4.0f;
            float normalizedHeight = fmax(0.0f, fmin(avgHeight, 10.0f)); 
            float minFactor = 0.5f; 
            float maxVariation = 1.0f; 
            float colorFactor = minFactor + (normalizedHeight / 10.0f) * maxVariation;
            
            // Cinza do chão
            glColor3f(0.55f * colorFactor, 0.55f * colorFactor, 0.55f * colorFactor);
            
            glBegin(GL_TRIANGLE_STRIP);
                vertex normalA = vertexNormals[z][x];
                glNormal3f(normalA.x, normalA.y, normalA.z);
                glVertex3f(mapCells[z][x].A.x, mapCells[z][x].A.y, mapCells[z][x].A.z);
                vertex normalB = vertexNormals[z+1][x];
                glNormal3f(normalB.x, normalB.y, normalB.z);
                glVertex3f(mapCells[z][x].B.x, mapCells[z][x].B.y, mapCells[z][x].B.z);
                vertex normalC = vertexNormals[z][x+1];
                glNormal3f(normalC.x, normalC.y, normalC.z);
                glVertex3f(mapCells[z][x].C.x, mapCells[z][x].C.y, mapCells[z][x].C.z);
                vertex normalD = vertexNormals[z+1][x+1];
                glNormal3f(normalD.x, normalD.y, normalD.z);
                glVertex3f(mapCells[z][x].D.x, mapCells[z][x].D.y, mapCells[z][x].D.z);
            glEnd();
        }
    }

    // Cor da parede
    glColor3f(0.70f, 0.30f, 0.10f);
    drawWall(MAP_SIZE/2,0,0,100,6,2);
    drawWall(MAP_SIZE/2,0,MAP_SIZE,100,6,2);
    drawWall(0,0,MAP_SIZE/2,2,6,100);
    drawWall(MAP_SIZE,0,MAP_SIZE/2,2,6,100);

    glEnable(GL_TEXTURE_2D); 

    glPushMatrix();
        int cellX = (int)CHURCH_X; int cellZ = (int)CHURCH_Z;
        float churchY = mapCells[cellZ][cellX].A.y; 
        glTranslatef(CHURCH_X, churchY, CHURCH_Z); 
        glScalef(1.0f, 1.0f, 1.0f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        if (churchModel.faceCount > 0) drawModel(&churchModel);
    glPopMatrix();

    glPushMatrix();
        int h1X = (int)HOUSE_X; int h1Z = (int)HOUSE_Z;
        float houseY = mapCells[h1Z][h1X].A.y; 
        glTranslatef(HOUSE_X, houseY, HOUSE_Z); 
        glScalef(1.0f, 1.0f, 1.0f);
        if (houseModel.faceCount > 0) drawModel(&houseModel);
    glPopMatrix();

    glPushMatrix();
        int h2X = (int)HOUSE2_X; int h2Z = (int)HOUSE2_Z;
        float house2Y = mapCells[h2Z][h2X].A.y; 
        glTranslatef(HOUSE2_X, house2Y, HOUSE2_Z); 
        glScalef(1.0f, 1.0f, 1.0f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f); 
        if (houseModel.faceCount > 0) drawModel(&houseModel);
    glPopMatrix();

    glPushMatrix();
        int g1X = (int)GASSTATION_X; int g1Z = (int)GASSTATION_Z;
        float g1Y = mapCells[g1Z][g1X].A.y; 
        glTranslatef(GASSTATION_X, g1Y, GASSTATION_Z); 
        glScalef(0.7f, 0.7f, 0.7f);
        if (gasStationModel.faceCount > 0) drawModel(&gasStationModel);
    glPopMatrix();

    glPushMatrix();
        int g2X = (int)GASSTATION2_X; int g2Z = (int)GASSTATION2_Z;
        float g2Y = mapCells[g2Z][g2X].A.y; 
        glTranslatef(GASSTATION2_X, g2Y, GASSTATION2_Z); 
        glScalef(0.7f, 0.7f, 0.7f);
        if (gasStationModel.faceCount > 0) drawModel(&gasStationModel);
    glPopMatrix();

    glPushMatrix();
        int g3X = (int)GASSTATION3_X; int g3Z = (int)GASSTATION3_Z;
        float g3Y = mapCells[g3Z][g3X].A.y; 
        glTranslatef(GASSTATION3_X, g3Y, GASSTATION3_Z); 
        glScalef(0.7f, 0.7f, 0.7f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f); 
        if (gasStationModel.faceCount > 0) drawModel(&gasStationModel);
    glPopMatrix();

    glPushMatrix();
        int g4X = (int)GASSTATION4_X; int g4Z = (int)GASSTATION4_Z;
        float g4Y = mapCells[g4Z][g4X].A.y; 
        glTranslatef(GASSTATION4_X, g4Y, GASSTATION4_Z); 
        glScalef(0.7f, 0.7f, 0.7f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f); 
        if (gasStationModel.faceCount > 0) drawModel(&gasStationModel);
    glPopMatrix();

    glPushMatrix();
        int sX = (int)STORE_X; int sZ = (int)STORE_Z;
        float storeY = mapCells[sZ][sX].A.y; 
        glTranslatef(STORE_X, storeY, STORE_Z); 
        glScalef(1.0f, 1.0f, 1.0f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f); 
        if (storeModel.faceCount > 0) drawModel(&storeModel);
    glPopMatrix();
}
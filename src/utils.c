#include <utils.h>
//#include <stdio.h>

vertex calcularNormalTriangulo(vertex p1, vertex p2, vertex p3) {
    vertex v, w, normal;

    // Faço os vetores
    v.x = p2.x - p1.x; 
    v.y = p2.y - p1.y; 
    v.z = p2.z - p1.z;

    w.x = p3.x - p1.x;
    w.y = p3.y - p1.y; 
    w.z = p3.z - p1.z;

    // Calculo a normal (Determinante matriz 3x3)
    normal.x = (v.y * w.z) - (v.z * w.y);
    normal.y = (v.z * w.x) - (v.x * w.z);
    normal.z = (v.x * w.y) - (v.y * w.x);

    float magnitude = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);

    if (magnitude > 0.0f) {
        normal.x /= magnitude; 
        normal.y /= magnitude;
        normal.z /= magnitude;
    }
    return normal;
}

void calcularNormaisDoMapa() {
    // Inicializa a matriz com zeros
    // (Assumindo que VERTEX_NUM é MAP_SIZE + 1)
    for (int z = 0; z <= VERTEX_NUM; z++) {
        for (int x = 0; x <= VERTEX_NUM; x++) {
            vertexNormals[z][x].x = 0.0f;
            vertexNormals[z][x].y = 0.0f;
            vertexNormals[z][x].z = 0.0f;
        }
    }

    // Passo 1: Acumular as normais das faces em cada vértice
    for (int z = 0; z < MAP_SIZE; z++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            
            vertex pA = mapCells[z][x].A;
            vertex pB = mapCells[z][x].B;
            vertex pC = mapCells[z][x].C;
            vertex pD = mapCells[z][x].D;

            // Divide o quad em dois triângulos (A, B, C) e (B, C, D)
            vertex normal1 = calcularNormalTriangulo(pA, pB, pC);
            vertex normal2 = calcularNormalTriangulo(pB, pD, pC);
            //printf("1: %f, %f, %f \n ", normal1.x, normal1.y, normal1.z);
            //printf("2: %f, %f, %f \n ", normal2.x, normal2.y, normal2.z);

            // Acumula a normal do triângulo 1 em seus vértices (A, B, C)
            vertexNormals[z][x].x += normal1.x;     // Vértice A
            vertexNormals[z][x].y += normal1.y;
            vertexNormals[z][x].z += normal1.z;
            
            vertexNormals[z+1][x].x += normal1.x;   // Vértice B
            vertexNormals[z+1][x].y += normal1.y;
            vertexNormals[z+1][x].z += normal1.z;

            vertexNormals[z][x+1].x += normal1.x;   // Vértice C
            vertexNormals[z][x+1].y += normal1.y;
            vertexNormals[z][x+1].z += normal1.z;

            // Acumula a normal do triângulo 2 em seus vértices (B, C, D)
            vertexNormals[z+1][x].x += normal2.x;   // Vértice B
            vertexNormals[z+1][x].y += normal2.y;
            vertexNormals[z+1][x].z += normal2.z;

            vertexNormals[z][x+1].x += normal2.x;   // Vértice C
            vertexNormals[z][x+1].y += normal2.y;
            vertexNormals[z][x+1].z += normal2.z;
            
            vertexNormals[z+1][x+1].x += normal2.x; // Vértice D
            vertexNormals[z+1][x+1].y += normal2.y;
            vertexNormals[z+1][x+1].z += normal2.z;
        }
    }

    for (int z = 0; z <= VERTEX_NUM; z++) {
        for (int x = 0; x <= VERTEX_NUM; x++) {
            vertex* n = &vertexNormals[z][x];
            
            // Calcula a magnitude (comprimento) do vetor
            float mag = sqrt(n->x * n->x + n->y * n->y + n->z * n->z);
            
            // Evita divisão por zero se a magnitude for 0
            if (mag > 0.0f) {
                // Divide cada componente pela magnitude
                n->x /= mag;
                n->y /= mag;
                n->z /= mag;
            }
        }
    }
}

void debugObjModel(ObjModel *model) {
    if (model == NULL) {
        printf("[DEBUG] Erro: O ponteiro do modelo esta NULO!\n");
        return;
    }

    printf("\n========================================\n");
    printf(" RELATORIO DE DEBUG DO OBJMODEL \n");
    printf("========================================\n");

    // 1. Resumo das Contagens
    printf("ESTATISTICAS:\n");
    printf(" - Vertices:  %d\n", model->vertexCount);
    printf(" - Normais:   %d\n", model->normalCount);
    printf(" - TexCoords: %d\n", model->texCoordCount);
    printf(" - Faces:     %d\n", model->faceCount);
    printf(" - Materiais: %d\n", model->materialCount);
    printf(" - Texturas:  %d\n", model->textureCount);
    printf("----------------------------------------");

    // 7. Listar Texturas (Nomes de arquivos)
    if (model->textures && model->textureCount > 0) {
        printf("\n--- TEXTURAS ARQUIVOS (Total: %d) ---\n", model->textureCount);
        for (int i = 0; i < model->textureCount; i++) {
            printf("Texture[%d]: %s (ID: %d)\n", i, model->textures[i].name, model->textures[i].textureID);
        }
    }
    printf("========================================\n");
}
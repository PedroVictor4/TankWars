#include <GL/glut.h> 

// Incluir o header correspondente (se necessário)
#include "lighting.h"

// --- CONSTANTES GLOBAIS DE ILUMINAÇÃO ---
// Definidas como 'const static' para escopo de arquivo e evitar modificações acidentais.

// Valores Padrão (Original)
const static GLfloat AMBIENT_ORIGINAL[] = { 0.5f, 0.5f, 0.5f, 1.0f };
const static GLfloat DIFFUSE_ORIGINAL[] = { 1.0f, 0.95f, 0.9f, 1.0f };
const static GLfloat SKY_COLOR_ORIGINAL[] = { 0.5f, 0.7f, 1.0f, 1.0f }; // Azul claro

// Valores para o Efeito "Congelamento" (Freeze)
const static GLfloat AMBIENT_FROZEN[] = { 0.2f, 0.3f, 0.5f, 1.0f };
const static GLfloat DIFFUSE_FROZEN[] = { 0.4f, 0.6f, 1.0f, 1.0f }; 
const static GLfloat SKY_COLOR_FROZEN[] = { 0.2f, 0.3f, 0.7f, 1.0f }; // Azul escuro

// -------------------------------------------------------------------
// 1. INICIALIZAÇÃO E CONFIGURAÇÃO DA ILUMINAÇÃO
// -------------------------------------------------------------------

/**
 * @brief Configura o sistema de iluminação principal e materiais.
 * * Define cores de luz (ambiente, difusa, especular), material e habilita
 * recursos essenciais do OpenGL.
 */
void setupLighting() {
    // Parâmetros para a luz GL_LIGHT0
    /*GLfloat light_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };  // Deve ser igual ao AMBIENT_ORIGINAL
    GLfloat light_diffuse[] = { 1.0f, 0.95f, 0.9f, 1.0f };  // Deve ser igual ao DIFFUSE_ORIGINAL*/
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    // Propriedades do material (para objetos)
    GLfloat espec_material[] = { 0.1f, 0.1f, 0.1f, 1.0f }; // Componente especular
    GLint espec_brilho = 10; // Brilho/Shininess

    // --- Configurações Iniciais ---
    
    // 1. Cor de Fundo (Céu) - Valor Original
    glClearColor(SKY_COLOR_ORIGINAL[0], SKY_COLOR_ORIGINAL[1], 
                 SKY_COLOR_ORIGINAL[2], SKY_COLOR_ORIGINAL[3]);

    // 2. Modelo de Sombreamento
    glShadeModel(GL_SMOOTH); // Sombreamento Gouraud (suavizado)

    // --- Configuração do Material (Objetos) ---
    glMaterialfv(GL_FRONT, GL_SPECULAR, espec_material);
	glMateriali(GL_FRONT, GL_SHININESS, espec_brilho);
 
    // --- Configuração da Luz (GL_LIGHT0) e Modelo Global ---
    
    // Luz Ambiente GLOBAL (afeta todos os objetos por igual)
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, AMBIENT_ORIGINAL);
    
    // Componentes da GL_LIGHT0
    glLightfv(GL_LIGHT0, GL_AMBIENT, AMBIENT_ORIGINAL);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, DIFFUSE_ORIGINAL);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // --- Habilitar Recursos ---
    glEnable(GL_LIGHTING);       // Ativa o sistema de iluminação
    glEnable(GL_LIGHT0);         // Ativa a primeira fonte de luz
    glEnable(GL_COLOR_MATERIAL); // Permite usar glColor* para definir o material
    glEnable(GL_DEPTH_TEST);     // Habilita teste de profundidade
}

// -------------------------------------------------------------------
// 2. CONTROLE DA POSIÇÃO DA LUZ
// -------------------------------------------------------------------

/**
 * @brief Define a posição da luz GL_LIGHT0 para simular o Sol/Padrão (Fonte pontual).
 *
 * Posição: (1.0, 1.0, 1.0), Tipo: Pontual (w=0.0 desabilitado, mas o w=0.0 
 * o transforma em luz direcional).
 *
 * Nota: Uma posição com W=0.0 define uma luz direcional (vetor).
 */
void drawSun(){
    // Luz Direcional (Simula um Sol distante: w=0.0)
    GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 0.0f }; 
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

/**
 * @brief Define a posição da luz GL_LIGHT0 para o modo Mapa (luz vinda de cima).
 *
 * Posição: (0.0, 1.0, 0.0), Tipo: Direcional (w=0.0)
 */
void drawMapSun(){
    // Luz Direcional vinda de cima (w=0.0)
    GLfloat light_position[] = { 0.0f, 1.0f, 0.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

// -------------------------------------------------------------------
// 3. CONTROLE DO EFEITO DE CONGELAMENTO (FREEZE)
// -------------------------------------------------------------------

/**
 * @brief Atualiza os parâmetros de iluminação para aplicar/reverter um efeito
 * visual de "congelamento" (tons mais frios).
 *
 * O efeito é ativo se o tempo atual for menor que 'freezeEndTime'.
 */
void updateFreezeLighting() {
    unsigned long now = glutGet(GLUT_ELAPSED_TIME);

    if (now < freezeEndTime) {
        // --- APLICAR CONGELAMENTO (Tons Azuis/Frios) ---
        
        // 1. Mudar Luz Ambiente Global (Afeta a cor base de TODOS os objetos)
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, AMBIENT_FROZEN);
        // A luz 0 também tem componente ambiente
        glLightfv(GL_LIGHT0, GL_AMBIENT, AMBIENT_FROZEN); 
        
        // 2. Mudar Luz Difusa (A luz vinda da fonte fica mais azulada)
        glLightfv(GL_LIGHT0, GL_DIFFUSE, DIFFUSE_FROZEN);
        
        // 3. Mudar a Cor de Fundo (Céu para azul escuro)
        glClearColor(SKY_COLOR_FROZEN[0], SKY_COLOR_FROZEN[1], 
                     SKY_COLOR_FROZEN[2], SKY_COLOR_FROZEN[3]);
    } else {
        // --- REVERTER (Garantir que os valores voltem ao padrão) ---

        // 1. Reverter Luz Ambiente Global
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, AMBIENT_ORIGINAL);
        glLightfv(GL_LIGHT0, GL_AMBIENT, AMBIENT_ORIGINAL);
        
        // 2. Reverter Luz Difusa
        glLightfv(GL_LIGHT0, GL_DIFFUSE, DIFFUSE_ORIGINAL);
        
        // 3. Reverter Cor de Fundo (Céu para azul claro original)
        glClearColor(SKY_COLOR_ORIGINAL[0], SKY_COLOR_ORIGINAL[1], 
                     SKY_COLOR_ORIGINAL[2], SKY_COLOR_ORIGINAL[3]);
    }
}
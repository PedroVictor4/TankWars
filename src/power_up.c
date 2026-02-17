#include <power_up.h>

PowerUpInstance powerUps[MAX_POWER_UPS];
int numPUativos = 0;

static int lastPowerUpIndex = 0;
static float lastSpawnTime = 0.0f;

// ------------- TESTE --------------
void testePowerUp()
{
    if (keyStates['m'] || keyStates['M'])
    {
        addAmmo();
    }
    if (keyStates['h'] || keyStates['H'])
    {
        heal();
    }
    if (keyStates['r'] || keyStates['R'])
    {
        increaseSpeed();
    }
    if (keyStates['k'] || keyStates['K'])
    {
        freeze();
    }
}

// ------------- GET SCALE -------------
float getScale(PowerUpType type)
{
    switch (type)
    {
    case PU_AMMO:
        return 0.35f;

    case PU_HEALTH:
        return 0.09f;

    case PU_SPEED:
        return 0.1f;

    case PU_DMG:
        return 0.001f;

    case PU_FREEZE:
        return 0.2f;

    case PU_SHIELD:
        return 0.03f;
    }

    return 0.1f;
}

// ------------- GET RANDOM POWER UP -------------

PowerUpType getRandomPowerUp()
{
    int r = rand() % 6; // seis tipos no enum
    return (PowerUpType)r;
}

// ------------- APPLY EFEITO DO POWER UP -------------

void applyPowerUpEffect(PowerUpType type)
{
    switch (type)
    {
    case PU_AMMO:
        addAmmo();
        break;

    case PU_HEALTH:
        heal();
        break;

    case PU_SPEED:
        increaseSpeed();
        break;

    case PU_DMG:
        increaseDmg();
        break;

    case PU_FREEZE:
        freeze();
        break;

    case PU_SHIELD:
        activateShield();
        break;
    }
}

// ------------- SPAWN POWER UP -------------

void spawnPowerUp()
{
    int secondsToSpawn = 10;
    float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    if (now - lastSpawnTime >= secondsToSpawn)
    {
        lastSpawnTime = now;
        powerUps[lastPowerUpIndex].active = 1;
        numPUativos += 1;
        lastPowerUpIndex += 1;
    }
}

// ------------- DRAW ACTIVE POWER UPS -------------
void drawPowerUps()
{
    for (int i = 0; i < MAX_POWER_UPS; i++)
    {
        if (!powerUps[i].active)
            continue;

        switch (powerUps[i].type)
        {
        case PU_AMMO:
            drawAmmo(i);
            break;
        case PU_HEALTH:
            drawHealthPack(i);
            break;
        case PU_SPEED:
            drawHermesShoes(i);
            break;
        case PU_DMG:
            drawFire(i);
            break;
        case PU_FREEZE:
            drawSnowFlake(i);
            break;
        case PU_SHIELD:
            drawShieldPU(i);
            break;
        }
    }
}

// ------------- INIT -------------
ObjModel snowFlake, hermesShoes, fire, healthPack, shield; // Ammo é carregado em projectile.c

void initPowerUps()
{
    if (loadOBJ("objects/snowFlake.obj", "objects/snowFlake.mtl", &snowFlake))
    {
    }
    else
    {
    }

    if (loadOBJ("objects/hermesShoes.obj", "objects/hermesShoes.mtl", &hermesShoes))
    {
    }
    else
    {
    }

    if (loadOBJ("objects/fire.obj", "objects/fire.mtl", &fire))
    {
    }
    else
    {
    }

    if (loadOBJ("objects/healthPack.obj", "objects/healthPack.mtl", &healthPack))
    {
    }
    else
    {
    }

    if (loadOBJ("objects/shield.obj", "objects/shield.mtl", &shield))
    {
    }
    else
    {
    }

    for (int i = 0; i < MAX_POWER_UPS; i++)
    {
        powerUps[i].type = getRandomPowerUp();
        powerUps[i].x = (rand() % 100); //25
        powerUps[i].y = 1;
        powerUps[i].z = (rand() % 100); //22
        powerUps[i].scale = getScale(powerUps[i].type);
        powerUps[i].active = 0;
    }
}

// ------------- GET OBJECT MODEL -------------

ObjModel getObjModel(PowerUpInstance p)
{
    switch (p.type)
    {
    case PU_AMMO:
        return shellModel;
    case PU_HEALTH:
        return healthPack;
    case PU_SPEED:
        return hermesShoes;
    case PU_DMG:
        return fire;
    case PU_FREEZE:
        return snowFlake;
    case PU_SHIELD:
        return shield;
    }

    return shellModel;
}

// ------------- FREEZE -------------
void drawSnowFlake(int index)
{
    // Modelo
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(powerUps[index].x, powerUps[index].y, powerUps[index].z);
    glScalef(powerUps[index].scale, powerUps[index].scale, powerUps[index].scale);
    drawModel(&snowFlake);
    // drawBox(snowFlake.box);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void freeze()
{
    unsigned long now = glutGet(GLUT_ELAPSED_TIME);
    // 5 segundos = 5000 milissegundos
    freezeEndTime = now + 5000; 
    printf("Inimigos congelados por 5 segundos!\n"); // Debug opcional
}

// ------------- AMMO -------------
void drawAmmo(int index)
{
    // Modelo
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(powerUps[index].x, powerUps[index].y, powerUps[index].z);
    glScalef(powerUps[index].scale, powerUps[index].scale, powerUps[index].scale);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    drawModel(&shellModel);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void addAmmo()
{
    player.ammo += 10;
}

// ------------- HEALTH -------------
void drawHealthPack(int index)
{
    // Modelo
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(powerUps[index].x, powerUps[index].y, powerUps[index].z);
    glScalef(powerUps[index].scale, powerUps[index].scale, powerUps[index].scale);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    drawModel(&healthPack);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void heal()
{
    player.health += 30;
}

// ------------- DMG UPGRADE -------------
void drawFire(int index)
{
    // Modelo
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(powerUps[index].x, powerUps[index].y, powerUps[index].z);
    glScalef(powerUps[index].scale, powerUps[index].scale, powerUps[index].scale);
    //glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
    drawModel(&fire);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void increaseDmg()
{
    player.bulletDmg += 15;
}

// ------------- SPEED -------------
void drawHermesShoes(int index)
{
    // Modelo
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(powerUps[index].x, powerUps[index].y, powerUps[index].z);
    glScalef(powerUps[index].scale, powerUps[index].scale, powerUps[index].scale);
    // glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    drawModel(&hermesShoes);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void increaseSpeed()
{
    if (player.reloadTime > 0)
        player.reloadTime -= 250;
    player.moveSpeed += 0.2;
    player.tankRotSpeed += 0.2;
    player.turretRotSpeed += 0.2;
}

// ------------- SHIELD -------------
void drawShieldPU(int index)
{
    // Modelo
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(powerUps[index].x, powerUps[index].y, powerUps[index].z);
    glScalef(powerUps[index].scale, powerUps[index].scale, powerUps[index].scale);
    //glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    drawModel(&shield);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void activateShield()
{
    player.shieldOn = 1;
}

void drawShield(CollisionBox b)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(2.0f);
    glColor3f(0.0f, 0.0f, 1.0f);

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

void drawPlayerShield()
{
    float pitch = player.pitch;
    drawShield(makePlayerHull(player.x, player.z, player.hullAngle, pitch));
    drawShield(makePlayerTurret(player.x, player.z, player.hullAngle, pitch, player.turretAngle));
    drawShield(makePlayerPipe(player.x, player.z, player.hullAngle, pitch, player.turretAngle, player.pipeAngle));
}

void morreu() {
    player.alive = 0;
}
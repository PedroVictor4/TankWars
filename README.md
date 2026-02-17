# TankWars

  Project Overview:
  This project is an educational endeavor to build a 3D tank-battle game from
  the ground up using C and the legacy OpenGL (fixed-function pipeline). It
  serves as a practical study of key computer graphics and game development
  principles, avoiding high-level game engines to focus on core mechanics. The
  codebase is structured to be modular, with clear separation between rendering,
  game logic, physics, and AI.

  This repository is for educational and demonstrative purposes only.

  Core Technical Features Implemented:


   * Graphics & Rendering:
       * OpenGL Fixed-Function Pipeline: All rendering is managed using legacy
         OpenGL 1.x commands (glBegin, glEnd, glLightfv, glMaterialfv,
         glTranslatef), providing a deep understanding of the foundational
         graphics pipeline before the advent of modern shaders.
       * Custom 3D Model Loader: A bespoke parser in mybib.c loads .obj mesh
         data (vertices, normals, texture coordinates) and .mtl material
         libraries. It correctly applies ambient, diffuse, and specular
         properties.
       * Texture Management: Integrates the stb_image library to load, bind, and
         map textures onto 3D models, with support for formats like PNG and TGA.
       * Per-Vertex Lighting: A Phong-based lighting model is implemented using
         glLightfv, featuring a single directional light source (sun) and
         material properties (shininess, specular color) to create depth and
         realism. A dynamic lighting effect for the "freeze" power-up is
         included.
       * Procedural Terrain: The map is generated from a heightmap, with
         functions to procedurally add hills and depressions. The terrain is
         rendered as a GL_TRIANGLE_STRIP mesh.
       * Smooth Shading on Terrain: Per-vertex normals for the terrain are
         calculated by averaging the normals of adjacent faces. This allows for
         smooth lighting and gradients across the landscape instead of a
         flat-shaded, low-poly look.


   * Physics & Collision:
       * Oriented Bounding Box (OBB) Collision: The core of the physics engine
         uses Oriented Bounding Boxes for all dynamic and static objects. This
         is more accurate than AABB for rotating objects like tanks.
       * Separating Axis Theorem (SAT): Collision detection between OBBs is
         implemented using the 3D Separating Axis Theorem in collision.c. This
         algorithm robustly determines if two rotated boxes are intersecting by
         checking for a separating plane along 15 potential axes.
       * Hierarchical Collision Models: The player and enemy tanks feature a
         hierarchical collision system, with separate-but-linked OBBs for the
         hull, turret, and pipe, allowing for more granular hit detection.
       * Terrain Following: Tanks realistically align to the terrain's slope by
         performing a bilinear interpolation on the heightmap (getTerrainHeight)
         to find the ground level and calculating the surface pitch
         (getTerrainPitch).


   * Game Logic & AI:
       * State Machine: A simple state machine manages the game flow, switching
         between a main menu (menu.c), the main game (game.c), and a game-over
         screen.
       * Enemy AI: The enemy AI in enemy.c operates on a simple
         "sense-think-act" loop.
           * Vision System: Enemies have a defined view cone (angle and
             distance).
           * Line-of-Sight Raycasting: A basic ray-box intersection test
             (checkLineOfSight) is implemented to determine if the enemy's view
             of the player is obstructed by static objects (buildings), allowing
             players to use the environment for cover.
           * Behavioral States: The AI switches between a "Wander" state and an
             "Engage" state when the player is seen. It includes logic to
             maintain a minimum combat distance.
           * Obstacle Avoidance: A simple "stuck" detection mechanism makes the
             AI reverse and turn if it collides with a wall or another tank.
       * Camera System: Includes three distinct camera modes: first-person
         (attached to the cannon), third-person (trailing the tank), and a
         top-down strategic view.

   * Code Structure:
       * include/: Header files defining the interfaces for all modules.
       * src/: C source files containing the implementation for each module
         (e.g., tank.c, enemy.c, collision.c).
       * objects/: Stores .obj and .mtl files for 3D models.
       * textures/: Contains all image assets used for texturing.

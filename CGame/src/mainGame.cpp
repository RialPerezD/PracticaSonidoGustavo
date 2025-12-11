/**
 * @file main.cpp
 * @brief Main file for the game application.
 *
 * This file contains the primary game loop, initialization,
 * and core logic for handling drawing, input, and audio updates.
 */

#pragma once

#include <stdio.h>

#include <esat/window.h>
#include <esat/draw.h>
#include <esat/input.h>
#include <esat/sprite.h>
#include <esat/time.h>

#include <loader.h>
#include <drawableEntity.h>
#include <sound.h>

#include <cstdlib>
#include <time.h>
#include <iostream>
#include <vector>
#include <utility>


 /** @brief Target frames per second for game loop control. */
unsigned char fps = 8;
/** @brief Current time in milliseconds. */
double current_time;
/** @brief Last time recorded in milliseconds. */
double last_time;

// --- Map data ---
/** @brief Structure representing the game board or map. */
Board board;
/** @brief Size of a single tile in pixels. */
int tileSize = 16;

/** @brief Vector of initial grid coordinates (x, y) for enemy entities. */
std::vector<std::pair<int, int>> enemyPos =
{
    {2,40},
    {9,25},
    {56,4},
    {10,5}
};
// --- *** ---

// --- Drawables ---
/** @brief List of non-player drawable entities on the map. */
std::vector<Drawable> drawableList;
// --- *** ---

// --- Player Data ---
/** @brief The number of movement steps the player has remaining before the day cycle changes. */
int stepAmmount = 32;
/** @brief The maximum number of player steps before enemies move during the night cycle. */
int maxStepCounter = 8;
/** @brief Current step counter used to pace enemy movement during the night cycle. */
int stepCounter = 3;
/** @brief The player's main drawable entity. */
Drawable player;
/** @brief Flag indicating if the player has lost the game. */
bool hasLost = false;
// --- *** ---

// --- Day cicle data ---
/** @brief Flag indicating the current time of day (true for day, false for night). */
bool isDay = true;
/** @brief Buffer for text output (e.g., displaying remaining steps). */
char buffer[32];
// --- *** ---

// --- Sound Manager ---
/** @brief Instance of the audio manager for handling sounds and music. */
AudioManager audio;
/** @brief ID for the daytime background music track. */
int backgroundMusic = -1;
/** @brief ID for the nighttime background music track. */
int nightMusic = -1;
/** @brief ID for the tavern music track. */
int tabernMusic = -1;
/** @brief Flag indicating if the player is currently outside. */
bool outside = true;
/** @brief List of sound IDs for individual enemy movement/proximity sounds. */
std::vector<int> enemyMusicIdList = {};
// --- *** ---

/**
 * @brief Loads a sprite from a file and creates a Drawable object.
 *
 * Initializes a sprite and its transformation with optional scaling.
 *
 * @param route The file path to the sprite image.
 * @param scaleX The scaling factor for the sprite on the X-axis (default: 1.0f).
 * @param scaleY The scaling factor for the sprite on the Y-axis (default: 1.0f).
 * @return A Drawable object containing the loaded sprite and transformation.
 */
Drawable LoadSprite(const char* route, float scaleX = 1.f, float scaleY = 1.f) {
    esat::SpriteHandle sprite = esat::SpriteFromFile(route);

    esat::SpriteTransform transform;
    esat::SpriteTransformInit(&transform);
    transform.scale_x = scaleX;
    transform.scale_y = scaleY;

    return Drawable(sprite, transform);
}

/**
 * @brief Updates the position and state of all enemy entities.
 *
 * Enemies attempt to move towards the player's position. Checks for collision
 * with the player and sets the game state to 'lost' upon collision.
 * Also updates the spatial audio position for each enemy.
 */
void UpdateEnemys() {
    // Starts from index 1 because index 0 is typically the background
    for (int i = 1; i < 5; i++) {
        drawableList[i].MoveTowards(player.posX, player.posY, board);
        // IDs for spatial sound sources start from a specific offset (i+2)
        audio.SetSourcePosition(i + 2, drawableList[i].posX, drawableList[i].posY);

        if (drawableList[i].posX == player.posX && drawableList[i].posY == player.posY) {
            hasLost = true;
            // Ensures music switches back to day music or a neutral state after loss if it was night
            if (!isDay) {
                isDay = true; // This will trigger a music change on the next day cycle check or is used for a visual state.
            }
        }
    }
}

/**
 * @brief Draws all sprites and on-screen text elements.
 *
 * Draws the background and enemies (only if it is daytime) and always draws the player.
 * Also draws the remaining steps counter and a "You Lost" message if applicable.
 */
void DrawSprites() {
    if (isDay) {
        // Draw all entities (background and enemies) during the day
        for (auto& d : drawableList) {
            d.transform.x = d.posX * tileSize;
            d.transform.y = d.posY * tileSize;
            esat::DrawSprite(d.sprite, d.transform);
        }
    }
    else {
        // Only draw the background (first element) during the night if entities are hidden
        drawableList[0].transform.x = drawableList[0].posX * tileSize;
        drawableList[0].transform.y = drawableList[0].posY * tileSize;
        esat::DrawSprite(drawableList[0].sprite, drawableList[0].transform);
    }

    // Always draw the player
    player.transform.x = player.posX * tileSize;
    player.transform.y = player.posY * tileSize;
    esat::DrawSprite(player.sprite, player.transform);

    // Draw remaining steps counter
    esat::DrawSetTextSize(20);
    esat::DrawSetFillColor(255, 255, 0);
    sprintf(buffer, "Steps remain %d", stepAmmount);
    esat::DrawText(24 * tileSize, 20, buffer);

    // Draw 'You Lost' message if the game is over
    if (hasLost) {
        esat::DrawSetTextSize(60);
        esat::DrawSetFillColor(255, 255, 255);
        esat::DrawText(18 * tileSize, 25 * tileSize, "You Lost");
    }
}

/**
 * @brief Initializes and loads all drawable entities, including the player and enemies.
 *
 * Loads sprites from files, sets initial positions for enemies based on `enemyPos`,
 * and sets the initial position for the player.
 */
void InitAllDrawables() {
    // Background (index 0)
    drawableList.push_back(
        LoadSprite("../assets/Mapa1.png", 0.5f, 0.5f));

    // Enemies (index 1 to 4)
    for (int i = 0; i < 4; i++) {
        drawableList.push_back(
            LoadSprite("../assets/Dino.png", 0.25f, 0.25f));

        drawableList.at(i + 1).posX = enemyPos[i].first;
        drawableList.at(i + 1).posY = enemyPos[i].second;
    }

    // Player
    player = LoadSprite("../assets/Pj.png");
    player.posX = 31;
    player.posY = 46;
}

/**
 * @brief Checks if a movement to the specified grid coordinates is valid.
 *
 * A move is invalid if the target coordinates are outside the board boundaries
 * or if the cell at the target coordinates is a wall (value 1 in `board.cells`).
 *
 * @param x The target X-coordinate (column).
 * @param y The target Y-coordinate (row).
 * @return True if the move is valid, false otherwise.
 */
bool CanIMoveThere(int x, int y) {
    if (y < 0 || y >= board.height || x < 0 || x >= board.width || board.cells[y * board.width + x] == 1) {
        return false;
    }
    return true;
}

/**
 * @brief Checks if the player is at a special location (e.g., entrance/exit) and handles related state changes.
 *
 * Manages the transition of background music when the player enters or exits a specific area (e.g., a tavern).
 */
void CheckSpecialPlaces() {
    // Entering the tavern
    if (player.posX == 25 && player.posY == 28 && outside) {
        if (isDay) {
            audio.Crossfade(backgroundMusic, tabernMusic, 1.0f);
        }
        else {
            audio.Crossfade(nightMusic, tabernMusic, 1.0f);
        }
        outside = !outside;
    }
    // Exiting the tavern
    else if (player.posX == 25 && player.posY == 30 && !outside) {
        if (isDay) {
            audio.Crossfade(tabernMusic, backgroundMusic, 1.0f);
        }
        else {
            audio.Crossfade(tabernMusic, nightMusic, 1.0f);
        }
        outside = !outside;
    }
}

/**
 * @brief Handles player input for movement.
 *
 * Checks for WASD key presses and attempts to move the player if the target position is valid
 * using `CanIMoveThere()`. Updates game state (step count and enemy movement) after a successful move.
 */
void UpdateInput() {
    bool hasMoved = false;
    // Check for 'W' (Up) movement
    if (esat::IsKeyPressed('W') && CanIMoveThere(player.posX, player.posY - 1)) {
        player.posY--;
        hasMoved = true;
    }
    // Check for 'S' (Down) movement
    else if (esat::IsKeyPressed('S') && CanIMoveThere(player.posX, player.posY + 1)) {
        player.posY++;
        hasMoved = true;
    }
    // Check for 'A' (Left) movement
    else if (esat::IsKeyPressed('A') && CanIMoveThere(player.posX - 1, player.posY)) {
        player.posX--;
        hasMoved = true;
    }
    // Check for 'D' (Right) movement
    else if (esat::IsKeyPressed('D') && CanIMoveThere(player.posX + 1, player.posY)) {
        player.posX++;
        hasMoved = true;
    }

    if (hasMoved) {
        CheckSpecialPlaces();
        stepAmmount--;

        // Enemy movement logic during night cycle
        if (!isDay && stepCounter <= 0) {
            UpdateEnemys();

            stepCounter = maxStepCounter; // Reset counter for next enemy movement
        }
        stepCounter--; // Decrement step counter regardless of day/night
    }
}

/**
 * @brief Toggles the game's day/night cycle.
 *
 * Resets the player's step count, crossfades the background music,
 * starts or stops the ambient enemy sounds based on the new cycle,
 * and decreases the number of steps required for enemies to move (`maxStepCounter`)
 * to increase difficulty.
 */
void ChangeDayCicle() {
    stepAmmount = 32;

    if (isDay) {
        // Transition from Day to Night
        if (outside) { // Only crossfade if outside, otherwise the tabern music keeps playing
            audio.Crossfade(backgroundMusic, nightMusic, 1.5f);
        }

        // Start playing ambient enemy sounds
        for (int i = 0; i < 4; i++) {
            audio.Play(enemyMusicIdList[i], true);
        }
    }
    else {
        // Transition from Night to Day
        if (outside) { // Only crossfade if outside
            audio.Crossfade(nightMusic, backgroundMusic, 1.5f);
        }

        // Stop ambient enemy sounds
        for (int i = 0; i < 4; i++) {
            audio.Stop(enemyMusicIdList[i]);
        }
    }

    // Increase difficulty by reducing steps required for enemy movement
    if (maxStepCounter > 1) maxStepCounter--;

    isDay = !isDay;
}

/**
 * @brief Initializes the configuration for text drawing.
 *
 * Sets the font, blur, fill color, and size for all subsequent text rendering.
 */
void InitTextConfig() {
    esat::DrawSetTextFont("../assets/font.ttf");
    esat::DrawSetTextBlur(0);
    esat::DrawSetFillColor(255, 255, 0);
    esat::DrawSetTextSize(20);
}

/**
 * @brief Initializes the audio manager and loads all necessary sound files.
 *
 * Loads background music tracks (day, night, tavern) and enemy/ambient sounds.
 * Registers 2D spatial sound sources for enemies and an ambient bird sound.
 * Starts playback of the initial background music.
 */
void InitBaseMusic() {
    if (!audio.Init()) {
        printf("Error inicializando OpenAL\n");
    }

    // Load background music tracks
    backgroundMusic = audio.LoadWav("../assets/fondo.wav");
    tabernMusic = audio.LoadWav("../assets/casa.wav");
    nightMusic = audio.LoadWav("../assets/noche.wav");

    // Set volume for night music
    audio.SetVolume(nightMusic, 0.5f);

    // Load, register, and store IDs for enemy spatial sounds
    for (int i = 0; i < 4; i++) {
        int enemyMusicId = audio.LoadWav("../assets/dinoStepMono.wav");
        audio.Register2DSound(
            enemyMusicId,
            enemyPos[i].first,
            enemyPos[i].second,
            10.f // Radius
        );
        enemyMusicIdList.push_back(enemyMusicId);
    }

    // Load, register, and play ambient bird sound
    int bird = audio.LoadWav("../assets/bird.wav");
    audio.Register2DSound(bird, 37, 22, 20.f);
    audio.Play(bird, true);
    audio.SetVolume(bird, 5.0f);

    // Start playing the initial background music (day)
    audio.Play(backgroundMusic, true);
}

/**
 * @brief The main entry point of the application.
 *
 * Initializes the window, draws, game logic, and handles the main game loop,
 * including frame rate capping.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 * @return 0 on successful execution.
 */
int esat::main(int argc, char** argv) {
    srand(time(NULL));
    esat::WindowInit(1024, 768);

    InitTextConfig();
    WindowSetMouseVisibility(true);

    InitAllDrawables();

    InitBaseMusic();

    // Load the board collision map from a black and white image
    BoardFromImage(&board, "../assets/Mapa1_bw.png");

    float dt = 0.125f; // Time step for audio updates

    // Main game loop
    while (esat::WindowIsOpened() && !esat::IsSpecialKeyDown(esat::kSpecialKey_Escape)) {

        last_time = esat::Time();
        esat::DrawBegin();
        esat::DrawClear(0, 0, 0);

        // Update audio state and 2D listener position
        audio.Update(dt);
        audio.UpdateSpatial2D(player.posX, player.posY);

        // Process input if the game is not over
        if (!hasLost) {
            UpdateInput();
        }

        DrawSprites();

        // Check for day/night cycle change trigger
        if (stepAmmount <= 0) {
            ChangeDayCicle();
        }

        esat::DrawEnd();
        // Frame rate capping loop
        do {
            current_time = esat::Time();
        } while ((current_time - last_time) <= 1000.0 / fps);
        esat::WindowFrame();
    }

    audio.Close();

    return 0;
}
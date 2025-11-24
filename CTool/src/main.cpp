
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

unsigned char fps = 8; //Control de frames por segundo
double current_time, last_time;

// --- Map data ---
Board board;
int tileSize = 16;

std::vector<std::pair<int, int>> enemyPos = 
{ 
	{2,40},
	{9,25},
	{56,4},
	{10,5}
};
// --- *** ---

// --- Drawables ---
std::vector<Drawable> drawableList;
// --- *** ---

// --- Player Data ---
int stepAmmount = 32;
int maxStepCounter = 8;
int stepCounter = 3;
Drawable player;
// --- *** ---

// --- Day cicle data ---
bool isDay = true;
char buffer[32];
// --- *** ---

// --- Sound Manager ---
AudioManager audio;
int backgroundMusic = -1;
int nightMusic = -1;
int tabernMusic = -1;
bool outside = true;
std::vector<int> enemyMusicIdList = {};
// --- *** ---


Drawable LoadSprite(const char* route, float scaleX = 1.f, float scaleY = 1.f) {
	esat::SpriteHandle sprite = esat::SpriteFromFile(route);

	esat::SpriteTransform transform;
	esat::SpriteTransformInit(&transform);
	transform.scale_x = scaleX;
	transform.scale_y = scaleY;

	return Drawable(sprite, transform);
}


void UpdateEnemys() {
	for (int i = 1; i < 5; i++) {
		drawableList[i].MoveTowards(player.posX, player.posY, board);
	}
}


void DrawSprites() {
	if (isDay) {
		for (auto& d : drawableList) {
			d.transform.x = d.posX * tileSize;
			d.transform.y = d.posY * tileSize;
			esat::DrawSprite(d.sprite, d.transform);
		}
	}

	player.transform.x = player.posX * tileSize;
	player.transform.y = player.posY * tileSize;
	esat::DrawSprite(player.sprite, player.transform);

	sprintf(buffer, "Steps remain %d", stepAmmount);
	esat::DrawText(24*tileSize, 20, buffer);
}


void InitAllDrawables() {
	//Background
	drawableList.push_back(
		LoadSprite("../assets/Mapa1.png", 0.5f, 0.5f));

	//Enemys
	for (int i = 0; i < 4; i++) {
		drawableList.push_back(
			LoadSprite("../assets/Dino.png", 0.25f, 0.25f));

		drawableList.at(i + 1).posX = enemyPos[i].first;
		drawableList.at(i + 1).posY = enemyPos[i].second;
	}

	//Player
	player = LoadSprite("../assets/Pj.png");
	player.posX = 31;
	player.posY = 46;
}


bool CanIMoveThere(int x, int y) {
	if (y < 0 || y >= board.height || x < 0 || x >= board.width|| board.cells[y * board.width + x] == 1) {
		return false;
	}
	return true;
}


void CheckSpecialPlaces() {
	if (player.posX == 25 && player.posY == 28 && outside) {
		audio.Crossfade(backgroundMusic, tabernMusic, 5.0f);
		outside = !outside;
	} else if (player.posX == 25 && player.posY == 30 && !outside) {
		audio.Crossfade(tabernMusic, backgroundMusic, 5.0f);
		outside = !outside;
	}
}


void UpdateInput() {
	bool hasMoved = false;
	if (esat::IsKeyPressed('W') && CanIMoveThere(player.posX, player.posY - 1)) {
		player.posY--;
		hasMoved = true;
	} else if (esat::IsKeyPressed('S') && CanIMoveThere(player.posX, player.posY + 1)){
		player.posY++;
		hasMoved = true;
	} else if (esat::IsKeyPressed('A') && CanIMoveThere(player.posX - 1, player.posY)) {
		player.posX--;
		hasMoved = true;
	} else if (esat::IsKeyPressed('D') && CanIMoveThere(player.posX + 1, player.posY)) {
		player.posX++;
		hasMoved = true;
	}

	if (hasMoved) {
		CheckSpecialPlaces();
		stepAmmount--;

		if (!isDay && stepCounter <= 0) {
			UpdateEnemys();

			if(maxStepCounter > 1) maxStepCounter--;
			stepCounter = maxStepCounter;
		}
		stepCounter--;
	}
}


void ChangeDayCicle() {
	stepAmmount = 32;

	if (isDay) {
		audio.Crossfade(backgroundMusic, nightMusic, 3.f);

		for (int i = 0; i < 4; i++) {
			audio.Play(enemyMusicIdList[i], true);
		}
	} else {
		audio.Crossfade(nightMusic, backgroundMusic, 3.f);

		for (int i = 0; i < 4; i++) {
			audio.Stop(enemyMusicIdList[i]);
		}
	}

	isDay = !isDay;
}


void InitTextConfig() {
	esat::DrawSetTextFont("../assets/font.ttf");
	esat::DrawSetTextBlur(0);
	esat::DrawSetFillColor(255, 255, 0);
	esat::DrawSetTextSize(20);
}


void InitBaseMusic() {
	if (!audio.Init()) {
		printf("Error inicializando OpenAL\n");
	}

	backgroundMusic = audio.LoadWav("../assets/fondo.wav");
	tabernMusic = audio.LoadWav("../assets/casa.wav");
	nightMusic = audio.LoadWav("../assets/noche.wav");

	audio.SetVolume(nightMusic, 0.5f);

	for (int i = 0; i < 4; i++) {
		int enemyMusicId = audio.LoadWav("../assets/a.wav");
		audio.Register2DSound(
			enemyMusicId,
			enemyPos[i].first,
			enemyPos[i].second,
			10.f
		);
		enemyMusicIdList.push_back(enemyMusicId);
	}

	int bird = audio.LoadWav("../assets/bird.wav");
	audio.Register2DSound(bird, 37, 22, 20.f);
	audio.Play(bird, true);

	//audio.Play(backgroundMusic, true);
}


int esat::main(int argc, char** argv) {
	srand(time(NULL));
	esat::WindowInit(1024, 768);
	
	InitTextConfig();
	WindowSetMouseVisibility(true);

	InitAllDrawables();

	InitBaseMusic();

	BoardFromImage(&board, "../assets/Mapa1_bw.png");

	float dt = 0.125f;

	while (esat::WindowIsOpened() && !esat::IsSpecialKeyDown(esat::kSpecialKey_Escape)) {

		last_time = esat::Time();
		esat::DrawBegin();
		esat::DrawClear(0, 0, 0);

		audio.Update(dt);
		audio.UpdateSpatial2D(player.posX, player.posY);

		UpdateInput();

		DrawSprites();

		if (stepAmmount <= 0) {
			ChangeDayCicle();
		}

		esat::DrawEnd();
		do {
			current_time = esat::Time();
		} while ((current_time - last_time) <= 1000.0 / fps);
		esat::WindowFrame();
	}

	audio.Close();
	
	return 0;
}
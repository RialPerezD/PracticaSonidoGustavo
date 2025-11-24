#include <esat/sprite.h>
#include <loader.h>

class Drawable {
public:
	int posX;
	int posY;

	esat::SpriteHandle sprite;
	esat::SpriteTransform transform;

	Drawable() {};
	Drawable(esat::SpriteHandle spr, esat::SpriteTransform trs) {
		sprite = spr;
		transform = trs;
		posX = 0;
		posY = 0;
	}

	bool MoveTowards(int targetX, int targetY, const Board& board);
};
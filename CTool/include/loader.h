#ifndef __LOADER_H__
#define __LOADER_H__ 1

#include <esat/sprite.h>
#include <vector>


struct Board {
    int width;
    int height;
    std::vector<int> cells;

    void init(int w, int h) {
        width = w;
        height = h;
        cells.resize(width * height);
    }

    int& cell(int row, int col) {
        return cells[row * width + col];
    }
};


static void BoardFromImage(Board* board, const char* filename) {
    if (filename == nullptr || board == nullptr) return;

    esat::SpriteHandle handle = esat::SpriteFromFile(filename);
    if (handle == nullptr) return;

    board->init(esat::SpriteWidth(handle), esat::SpriteHeight(handle));

    for (int col = 0; col < esat::SpriteWidth(handle); ++col) {
        for (int row = 0; row < esat::SpriteHeight(handle); ++row) {
            unsigned char outRGBA[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
            esat::SpriteGetPixel(handle, col, row, outRGBA);

            if ((outRGBA[0] == 0xFF) &&
                (outRGBA[1] == 0xFF) &&
                (outRGBA[2] == 0xFF))
            {
                board->cell(row, col) = 0;
            } else {
                board->cell(row, col) = 1;
            }
        }
    }
}

#endif

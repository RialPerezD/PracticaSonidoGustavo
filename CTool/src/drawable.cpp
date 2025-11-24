#include "../include/drawableEntity.h"
#include <unordered_map>
#include <queue>

struct Node {
    int x, y;
    float g, f;
    bool operator>(const Node& other) const { return f > other.f; }
};

bool Drawable::MoveTowards(int tx, int ty, const Board& board) {

    int w = board.width;
    int h = board.height;

    auto inBounds = [&](int x, int y) {
        return x >= 0 && x < w && y >= 0 && y < h;
        };

    auto isWalkable = [&](int x, int y) {
        return board.cells[y * w + x] == 0;
        };

    auto heuristic = [&](int x, int y) {
        return abs(tx - x) + abs(ty - y);   // Manjattan
        };

    // Si ya estoy en el destino
    if (posX == tx && posY == ty) return true;

    // A*
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    std::unordered_map<int, float> gScore;
    std::unordered_map<int, int> cameFrom;

    auto hash = [&](int x, int y) { return y * w + x; };

    int start = hash(posX, posY);
    int goal = hash(tx, ty);

    gScore[start] = 0;
    Node startNode;
    startNode.x = posX;
    startNode.y = posY;
    startNode.g = 0;
    startNode.f = heuristic(posX, posY);

    open.push(startNode);

    int dirs[4][2] = {
        {1,0},{-1,0},{0,1},{0,-1}
    };

    bool found = false;

    while (!open.empty()) {
        Node cur = open.top(); open.pop();
        int curHash = hash(cur.x, cur.y);

        if (curHash == goal) {
            found = true;
            break;
        }

        for (auto& d : dirs) {
            int nx = cur.x + d[0];
            int ny = cur.y + d[1];

            if (!inBounds(nx, ny)) continue;
            if (!isWalkable(nx, ny)) continue;

            int nh = hash(nx, ny);
            float tentative = gScore[curHash] + 1;

            if (!gScore.count(nh) || tentative < gScore[nh]) {
                gScore[nh] = tentative;
                float f = tentative + heuristic(nx, ny);

                open.push({ nx, ny, tentative, f });
                cameFrom[nh] = curHash;
            }
        }
    }

    if (!found) return false;

    // --- volver por camino desde goal hacia start ---
    std::vector<int> path;
    int current = goal;
    while (current != start) {
        path.push_back(current);
        current = cameFrom[current];
    }
    std::reverse(path.begin(), path.end());

    // Pillar solo el PRIMER PASO
    if (!path.empty()) {
        int next = path[0];
        int nx = next % w;
        int ny = next / w;

        posX = nx;
        posY = ny;
        return true;
    }

    return false;
}

/**
 * @file drawableEntity.cpp
 * @brief Implementation of pathfinding logic for Drawable entities.
 */

#include "../include/drawableEntity.h"
#include <unordered_map>
#include <queue>
#include <vector>
#include <cmath> // For abs and std::sqrt
#include <algorithm> // For std::reverse

 /**
  * @brief Represents a node in the A* search algorithm.
  *
  * Stores grid coordinates, the cost from the start (g), and the estimated
  * total cost to the goal (f = g + h).
  */
struct Node {
    int x, y; /**< Grid coordinates. */
    float g, f; /**< Cost from start (g) and total estimated cost (f). */
    /**
     * @brief Comparison operator for the priority queue.
     * @return True if this node's estimated total cost is greater than the other node's.
     */
    bool operator>(const Node& other) const { return f > other.f; }
};

/**
 * @brief Calculates the next step for the entity to move towards a target position using A* search.
 *
 * This function implements the A* pathfinding algorithm on the game board (grid)
 * to find the shortest path from the entity's current position to the target position.
 * The entity then moves one step along the calculated path.
 *
 * @param tx The target X-coordinate (column).
 * @param ty The target Y-coordinate (row).
 * @param board The game board structure, containing dimensions and cell traversability data.
 * @return True if a move was made (a path was found or the entity is already at the target), false otherwise.
 */
bool Drawable::MoveTowards(int tx, int ty, const Board& board) {

    int w = board.width;
    int h = board.height;

    /**
     * @brief Lambda function to check if coordinates are within the board boundaries.
     */
    auto inBounds = [&](int x, int y) {
        return x >= 0 && x < w && y >= 0 && y < h;
        };

    /**
     * @brief Lambda function to check if a grid cell is walkable (not a wall).
     * Assumes cell value 0 is walkable.
     */
    auto isWalkable = [&](int x, int y) {
        return board.cells[y * w + x] == 0;
        };

    /**
     * @brief Heuristic function: Manhattan distance.
     * Provides an admissible estimate of the cost from (x, y) to the target (tx, ty).
     */
    auto heuristic = [&](int x, int y) {
        return (float)(abs(tx - x) + abs(ty - y)); // Manhattan Distance
        };

    // Check if the entity is already at the destination
    if (posX == tx && posY == ty) return true;

    // A* Pathfinding implementation
    /** @brief Priority queue for nodes to be evaluated, sorted by lowest f-score. */
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    /** @brief Map storing the cost from the start node to a node (g-score). Key is grid hash. */
    std::unordered_map<int, float> gScore;
    /** @brief Map storing the predecessor of each node in the shortest path found so far. Key/Value are grid hashes. */
    std::unordered_map<int, int> cameFrom;

    /**
     * @brief Lambda function to convert 2D coordinates to a unique 1D hash (index).
     */
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


    /** @brief Array defining the four possible movement directions (right, left, down, up). */
    int dirs[4][2] = {
        {1,0},{-1,0},{0,1},{0,-1}
    };

    bool found = false;

    // Main A* loop
    while (!open.empty()) {
        Node cur = open.top(); open.pop();
        int curHash = hash(cur.x, cur.y);

        if (curHash == goal) {
            found = true;
            break;
        }

        // Check neighbors
        for (auto& d : dirs) {
            int nx = cur.x + d[0];
            int ny = cur.y + d[1];

            // Boundary and walkability checks
            if (!inBounds(nx, ny)) continue;
            if (!isWalkable(nx, ny)) continue;

            int nh = hash(nx, ny);
            float tentative = gScore[curHash] + 1; // Cost to move to neighbor is 1

            // If a shorter path to the neighbor is found
            if (!gScore.count(nh) || tentative < gScore[nh]) {
                gScore[nh] = tentative;
                float f = tentative + heuristic(nx, ny);

                open.push({ nx, ny, tentative, f });
                cameFrom[nh] = curHash; // Record the current node as the predecessor
            }
        }
    }

    // If no path was found
    if (!found) return false;

    // --- Reconstruct Path (Backtracking) ---
    std::vector<int> path;
    int current = goal;
    // Trace back from goal to start using the cameFrom map
    while (current != start) {
        path.push_back(current);
        // This is safe because 'start' must have an entry in cameFrom if 'goal' was reached
        // unless goal == start, which is handled at the beginning.
        current = cameFrom[current];
    }
    std::reverse(path.begin(), path.end()); // Path is now ordered from start to goal

    // Take only the FIRST STEP
    if (!path.empty()) {
        int next = path[0];
        // Convert hash back to coordinates
        int nx = next % w;
        int ny = next / w;

        // Apply the movement to the entity's position
        posX = nx;
        posY = ny;
        return true;
    }

    return false; // Should not be reached if found is true and goal != start
}
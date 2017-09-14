#pragma once

#include <vector>

struct raycast {
    bool hit, yhit;
    float distance;
};

typedef std::vector<std::vector<bool>> maze;

raycast DDA_raycast(maze &map, float x, float y, float dx, float dy);
maze make_maze(unsigned size);

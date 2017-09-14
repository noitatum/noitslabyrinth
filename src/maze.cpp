#include <maze.h>
#include <random>
#include <algorithm>

using namespace std;

struct disjoint_sets {

    disjoint_sets(unsigned count) {
        set.reserve(count);
        for (unsigned i = 0; i < count; i++)
            set.push_back(node{i, 0});
    }

    unsigned seek_root(unsigned v) {
        unsigned p = set[v].parent;
        // Flaten tree
        if (p != v)
            set[v].parent = seek_root(p);

        return set[v].parent;
    }

    bool join_nodes(unsigned v1, unsigned v2) {
        unsigned p1, p2;
        if ((p1 = seek_root(v1)) == (p2 = seek_root(v2)))
            return false;
        if (set[p1].rank < set[p2].rank)
            set[p1].parent = p2;
        else {
            set[p2].parent = p1;
            if (set[p1].rank == set[p2].rank)
                set[p1].rank++;
        }
        return true;
    }

    private:
    struct node {
        unsigned parent, rank;
    };
    vector<node> set;
};

maze make_maze(unsigned size) {
    struct edge {
            unsigned x1, y1, x2, y2;
    };
    vector<edge> edges, tree;
    for (unsigned i = 0; i < size - 1; i++)
        for (unsigned j = 0; j < size; j++)
            edges.push_back((edge) {i, j, i + 1, j});
    for (unsigned i = 0; i < size; i++)
        for (unsigned j = 0; j < size - 1; j++)
            edges.push_back((edge) {i, j, i, j + 1});
    random_device rd;
    minstd_rand g(rd());
    shuffle(edges.begin(), edges.end(), g);
    disjoint_sets set(size * size);
    for (edge e : edges) {
        unsigned a = e.y1 * size + e.x1, b = e.y2 * size + e.x2;
        if (set.join_nodes(a, b))
            tree.push_back(e);
    }
	const unsigned psize = 2, msize = size * psize + 1;
	maze map(msize, vector<bool>(msize, false));
	for (unsigned i = 0; i < msize; i += psize) {
		for (unsigned j = 0; j < msize; j++) {
			map[i][j] = true;
			map[j][i] = true;
		}
	}
	// Create spanning tree at output
	for (edge e : tree) {
		if (e.x1 == e.x2) {
			for (unsigned j = 1; j < psize; j++)
				map[e.x1 * psize + j][e.y1 * psize + psize] = false;
		} else {
			for (unsigned j = 1; j < psize; j++)
				map[e.x1 * psize + psize][e.y1 * psize + j] = false;
		}
	}
    return map;
}

raycast DDA_raycast(maze &map, float x, float y, float dx, float dy) {
	//which box of the map we're in
	int mapX = x, mapY = y;
	//length of ray from current position to next x or y-side
	float sideDistX, sideDistY;
    // Total length of the ray
    float dlength = sqrt(dy * dy + dx * dx);
	//length of ray from one x or y-side to next x or y-side
	float deltaDistX = sqrt(1 + (dy * dy) / (dx * dx));
	float deltaDistY = sqrt(1 + (dx * dx) / (dy * dy));

	//what direction to step in x or y-direction (either +1 or -1)
	int stepX, stepY;

	//calculate step and initial sideDist
	if (dx < 0) {
		stepX = -1;
		sideDistX = (x - mapX) * deltaDistX;
	} else {
		stepX = 1;
		sideDistX = (mapX + 1.0 - x) * deltaDistX;
	}
	if (dy < 0) {
		stepY = -1;
		sideDistY = (y - mapY) * deltaDistY;
	} else {
		stepY = 1;
		sideDistY = (mapY + 1.0 - y) * deltaDistY;
	}
    while (sideDistX < dlength || sideDistY < dlength) {
        if (sideDistX < sideDistY) {
            mapX += stepX;
            if (map[mapX][mapY])
                return {true, false, sideDistX};
            sideDistX += deltaDistX;
        } else {
            mapY += stepY;
            if (map[mapX][mapY])
                return {true, true, sideDistY};
            sideDistY += deltaDistY;
        }
    }
    return {false, false, dlength};
}

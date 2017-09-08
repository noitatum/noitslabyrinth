#include <string.h>
#include <stdio.h>
#include <SDL2/SDL.h>

typedef struct nodo_set {
    unsigned padre, rango;
} nodo_set;

unsigned buscar_raiz(nodo_set* set, unsigned v) {
    // Buscamos su raiz y la devolvemos, tambien aplanamos el arbol
    unsigned p = set[v].padre;
    if (p != v)
        set[v].padre = buscar_raiz(set, p);

    return set[v].padre;
}

unsigned unir_nodos(nodo_set* set, unsigned v1, unsigned v2) {
    unsigned p1, p2;
    // Estan en el mismo arbol, ya estan unidos
    if ((p1 = buscar_raiz(set, v1)) == (p2 = buscar_raiz(set, v2)))
        return 0;
    // p1 y p2 no estan en el mismo arbol, unirlos
    // Asignamos el padre al que tenga rango mas chico o a p1 si son iguales
    // En ese caso el rango de p1 tiene que aumentar
    if (set[p1].rango < set[p2].rango)
        set[p1].padre = p2;
    else {
        set[p2].padre = p1;
        if (set[p1].rango == set[p2].rango)
            set[p1].rango++;
    }
    return 1;
}

typedef struct order {
    unsigned value;
    unsigned index;
} order;

// Arregla el heap hacia abajo
void heapify(order* arr, unsigned n, unsigned i) {
    unsigned l = i * 2, r = i * 2 + 1, max = i;
    if (l < n && arr[l].value > arr[max].value)
        max = l;
    if (r < n && arr[r].value > arr[max].value)
        max = r;
    if (max != i) {
        order t = arr[i];
        arr[i] = arr[max];
        arr[max] = t;
        heapify(arr, n, max);
    }
}

// Ordena el array
void heapsort(order* arr, unsigned n) {
    for (unsigned i = n / 2 - 1; i != -1; i--)
        heapify(arr, n, i);
    for (unsigned i = n - 1; i > 0; i--) {
        order t = *arr;
        *arr = arr[i];
        arr[i] = t;
        heapify(arr, i, 0);
    }
}

SDL_Window* init_sdl() {
    SDL_Init(SDL_INIT_VIDEO);
    return SDL_CreateWindow("AMAZ3D", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 800, 600, 0);
}

typedef struct edge {
    unsigned x1, y1, x2, y2;
} edge;

void terminal_output(edge* tree, unsigned size) {
    char output[size + 1][size * 2 + 2];
    // Initialize output
    memset(output, '_', sizeof(output));
    for (unsigned i = 1; i < size + 1; i++)
        for (unsigned j = 0; j < size + 1; j++)
            output[i][j * 2] = '|';
    for (unsigned i = 0; i < size; i++)
        output[i][size * 2 + 1] = '\n';
    output[size][size * 2 + 1] = 0;
    // Create spanning tree at output
    for (unsigned i = 0; i < size * size - 1; i++) {
        edge e = tree[i];
        if (e.x1 == e.x2)
            output[e.y1 + 1][e.x1 * 2 + 1] = ' ';
        else
            output[e.y1 + 1][e.x1 * 2 + 2] = '_';
    }
    // Print output
    puts((char*) output);
}

void render_output(SDL_Renderer* r, edge* tree, unsigned size) {
    const unsigned t = 10;
    SDL_RenderClear(r);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    for (unsigned i = 0; i <= size; i++) {
        SDL_RenderDrawLine(r, t, t + i * t, t + size * t, t + i * t);
        SDL_RenderDrawLine(r, t + i * t, t, t + i * t, t + size * t);
    }
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    // Create spanning tree at output
    for (unsigned i = 0; i < size * size - 1; i++) {
        edge e = tree[i];
        unsigned x1 = t * 2 + e.x1 * t, y1 = t * 2 + e.y1 * t;
        if (e.x1 == e.x2)
            SDL_RenderDrawLine(r, x1 - 1, y1, x1 - t + 1, y1);
        else
            SDL_RenderDrawLine(r, x1, y1 - 1, x1, y1 - t + 1);
    }
    SDL_RenderPresent(r);
}

void render_output2(SDL_Renderer* r, edge* tree, unsigned size) {
    const unsigned tsize = 5, psize = 5;
    char map[size * psize + 1][size * psize + 1];
    memset(map, 0, sizeof(map));
    for (unsigned i = 0; i <= size; i++) {
        for (unsigned j = 0; j <= size * psize; j++) {
            map[i * psize][j] = 1;
            map[j][i * psize] = 1;
        }
    }
    // Create spanning tree at output
    for (unsigned i = 0; i < size * size - 1; i++) {
        edge e = tree[i];
        if (e.x1 == e.x2) {
            for (unsigned j = 1; j < psize; j++)
                map[e.x1 * psize + j][e.y1 * psize + psize] = 0;
        } else {
            for (unsigned j = 1; j < psize; j++)
                map[e.x1 * psize + psize][e.y1 * psize + j] = 0;
        }
    }
    SDL_RenderClear(r);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    for (unsigned i = 0; i <= size * psize; i++) {
        for (unsigned j = 0; j <= size * psize; j++) {
            if (map[i][j]) {
                SDL_Rect rect = {i * tsize, j * tsize, tsize, tsize};
                SDL_RenderFillRect(r, &rect);
            }
        }
    }
    SDL_RenderPresent(r);
}

int main(int argc, char** argv) {
    SDL_Window* w = init_sdl();
    SDL_Renderer* r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);
    unsigned size = 10;
    if (argc == 2)
        sscanf(argv[1], "%u", &size);
    const unsigned edge_count = size * (size - 1) * 2;
    FILE* rng = fopen("/dev/urandom", "r");
    order kruskal[edge_count];
    nodo_set set[size * size];
    edge edges[edge_count], tree[size * size - 1];
    // Initialize set
    for (unsigned i = 0; i < size * size; i++)
        set[i] = (nodo_set) {i, 0};
    // Initialize edges
    unsigned k = 0;
    for (unsigned i = 0; i < size - 1; i++)
        for (unsigned j = 0; j < size; j++, k++)
            edges[k] = (edge) {i, j, i + 1, j};
    for (unsigned i = 0; i < size; i++)
        for (unsigned j = 0; j < size - 1; j++, k++)
            edges[k] = (edge) {i, j, i, j + 1};
    // Initialize kruskal and sort
    fread(kruskal, sizeof(kruskal), 1, rng);
    for (unsigned i = 0; i < edge_count; i++)
        kruskal[i].index = i;
    heapsort(kruskal, edge_count);
    for (unsigned i = 0, j = 0; i < edge_count; i++) {
        edge e = edges[kruskal[i].index];
        unsigned a = e.y1 * size + e.x1, b = e.y2 * size + e.x2;
        if (unir_nodos(set, a, b))
            tree[j++] = e;
    }
    render_output2(r, tree, size);
    terminal_output(tree, size);
    getchar();
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);
    SDL_Quit();
}

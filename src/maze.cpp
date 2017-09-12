#include <gl3w.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <stdarg.h>

using namespace std;

#define COUNT(arr) (sizeof(arr) / sizeof(arr[0]))
#define LIMIT(a, min, max) ((a) < (min) ? (min) : (a) > (max) ? (max) : (a))

constexpr float PI = acos(-1);

// settings
unsigned int WIDTH = 800, HEIGHT = 600;
const float sensitivity = 0.003f;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f, fov = 45.0f;
float lastX = WIDTH / 2.0, lastY = HEIGHT / 2.0;

// timing
float deltaTime = 0.0f, lastFrame = 0.0f;

// mouse
bool mouse_press = false;

// maze
float maze_roll = 0.0f, maze_pitch = 0.0f;

float vertices[] = {
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

    -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

    -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
    0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
    0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

    -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f
};

float grass_verts[] = {
    // First Triangle
    -0.5f, 0.0f,-0.5f, 0.0f, 0.0f,
    -0.5f, 0.0f, 0.5f, 0.0f, 1.0f,
     0.5f, 0.0f,-0.5f, 1.0f, 0.0f,
    // Second Triangle
    -0.5f, 0.0f, 0.5f, 0.0f, 1.0f,
     0.5f, 0.0f,-0.5f, 1.0f, 0.0f,
     0.5f, 0.0f, 0.5f, 1.0f, 1.0f,
};

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

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 5.0 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -=
            glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos +=
            glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    if (mouse_press) {
        maze_roll += xoffset;
        maze_pitch += yoffset;
        const float max = PI / 6;
        maze_pitch = LIMIT(maze_pitch, -max, max);
        maze_roll = LIMIT(maze_roll, -max, max);
        return;
    }
    yaw += xoffset;
    pitch += yoffset;
    const float max = PI / 2 - 0.01f;
    pitch = LIMIT(pitch, -max, max);
    glm::vec3 front(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
    cameraFront = glm::normalize(front);
}

void mouse_button_callback(GLFWwindow*, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        mouse_press = (action == GLFW_PRESS);
}

void scroll_callback(GLFWwindow*, double, double yoffset) {
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}

unsigned load_texture(const char* path) {
    unsigned texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    unsigned char* data
        = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return texture;
}

void panic(const char* format, ...) {
    va_list l;
    va_start(l, format);
    vfprintf(stderr, format, l);
    va_end(l);
}

vector<vector<bool>> make_maze(unsigned size) {
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
    mt19937 g(rd());
    shuffle(edges.begin(), edges.end(), g);
    disjoint_sets set(size * size);
    for (edge e : edges) {
        unsigned a = e.y1 * size + e.x1, b = e.y2 * size + e.x2;
        if (set.join_nodes(a, b))
            tree.push_back(e);
    }
	const unsigned psize = 2, msize = size * psize + 1;
	vector<vector<bool>> map(msize, vector<bool>(msize, false));
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

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window =
        glfwCreateWindow(WIDTH, HEIGHT, "Maze", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (gl3wInit())
        panic("Failed to initialize OpenGL\n");

    if (!gl3wIsSupported(3, 3))
        panic("OpenGL 3.3 is not supported\n");

    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
           glGetString(GL_SHADING_LANGUAGE_VERSION));

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    Shader ourShader("src/camera.vert", "src/camera.frag");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    unsigned int VBO[2], VAO[2];
    glGenVertexArrays(COUNT(VAO), VAO);
    glGenBuffers(COUNT(VBO), VBO);
    // Cubes
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Grass
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grass_verts), grass_verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load and create a texture
    // -------------------------
    stbi_set_flip_vertically_on_load(true);
    unsigned tx_wall = load_texture("./textures/wall_0.png");
    unsigned tx_youmu = load_texture("./textures/youmu.png");
    unsigned tx_grass = load_texture("./textures/grass_0.png");
    unsigned tx_marble = load_texture("./textures/marble_0.png");
    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    vector<vector<bool>> maze = make_maze(10);
    float msize = maze.size() / 2;

    float player_x = 1 - msize, player_y = 1 - msize;
    float player_vx = 0, player_vy = 0;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tx_wall);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tx_youmu);

        ourShader.use();

        glm::mat4 projection = glm::perspective(
            glm::radians(fov), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view =
            glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        ourShader.setMat4("view", view);

        // Maze rotation transformation
        glm::mat4 maze_rot(1);
        maze_rot = rotate(maze_rot, maze_roll, glm::vec3(0, 0, 1));
        maze_rot = rotate(maze_rot, maze_pitch, glm::vec3(1, 0, 0));

        // Player acceleration
        const float gravity = 0.1, max_vel = 0.1, psize = 0.25;
        player_x += player_vx;
        player_y += player_vy;
        player_vx += (-maze_roll / (PI / 2)) * gravity;
        player_vy += (maze_pitch / (PI / 2)) * gravity;
        player_vx = LIMIT(player_vx, -max_vel, max_vel);
        player_vy = LIMIT(player_vy, -max_vel, max_vel);

        // Render maze
        glBindVertexArray(VAO[0]);
        for (unsigned int i = 0; i < maze.size(); i++) {
            for (unsigned int j = 0; j < maze[0].size(); j++) {
                if (maze[i][j]) {
                    glm::mat4 model = translate(maze_rot,
                                      glm::vec3(i - msize, 0, j - msize));
                    ourShader.setMat4("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
            }
        }
        // Render skybox
        {
            glm::mat4 model(1);
            model = scale(model, glm::vec3(maze.size() * 2));
            ourShader.setMat4("model", model);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tx_marble);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // Render player
        {
            glm::mat4 model = maze_rot;
            model = translate(model, glm::vec3(player_x, -psize, player_y));
            model = scale(model, glm::vec3(0.5));
            ourShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // Render floor
        glm::mat4 model = maze_rot;
        model = translate(model, glm::vec3(0, -0.5, 0));
        model = scale(model, glm::vec3(maze.size()));
        ourShader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tx_grass);
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLES, 0, 6);



        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(COUNT(VAO), VAO);
    glDeleteBuffers(COUNT(VBO), VBO);
    glfwTerminate();
    return 0;
}



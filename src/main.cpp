#include <gl3w.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <maze.h>

#include <shader.h>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <stdarg.h>

using namespace std;

#define LIMIT(a, min, max) ((a) < (min) ? (min) : (a) > (max) ? (max) : (a))

constexpr float PI = acos(-1);

// settings
unsigned int WIDTH = 800, HEIGHT = 600;
const float sensitivity = 0.003f;

// camera
glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cam_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cam_up = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f, fov = 45.0f;
float xlast = WIDTH / 2.0, ylast = HEIGHT / 2.0;

// timing
float deltaTime = 0.0f, lastFrame = 0.0f;

// mouse
bool mouse_press = false;

// maze
float maze_roll = 0.0f, maze_pitch = 0.0f;

const vector<float> wall_verts = {
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

const vector<float> floor_verts = {
    // First Triangle
    -0.5f, 0.0f,-0.5f, 0.0f, 0.0f,
    -0.5f, 0.0f, 0.5f, 0.0f, 1.0f,
     0.5f, 0.0f,-0.5f, 1.0f, 0.0f,
    // Second Triangle
    -0.5f, 0.0f, 0.5f, 0.0f, 1.0f,
     0.5f, 0.0f,-0.5f, 1.0f, 0.0f,
     0.5f, 0.0f, 0.5f, 1.0f, 1.0f,
};

void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float cam_speed = 10.0 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam_pos += cam_speed * cam_front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam_pos -= cam_speed * cam_front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam_pos -= glm::normalize(glm::cross(cam_front, cam_up)) * cam_speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam_pos += glm::normalize(glm::cross(cam_front, cam_up)) * cam_speed;
}

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    float xoffset = xpos - xlast;
    float yoffset = ylast - ypos;
    xlast = xpos;
    ylast = ypos;

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
    cam_front = glm::normalize(front);
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
    _Exit(-1);
}

class vabo {
    unsigned vao, vbo, vcount;

    public:
        vabo(const vector<unsigned> &counts, const vector<float> &attrs) {
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, attrs.size() * sizeof(float),
                         attrs.data(), GL_STATIC_DRAW);
            unsigned sum = 0;
            for (unsigned a : counts)
                sum += a;
            vcount = attrs.size() / sum;
            for (unsigned i = 0, partial = 0; i < counts.size(); i++) {
                glVertexAttribPointer(i, counts[i], GL_FLOAT, GL_FALSE,
                                      sum * sizeof(float),
                                      (void*) (uintptr_t) partial);
                glEnableVertexAttribArray(i);
                partial += counts[i] * sizeof(float);
            }
        }

        ~vabo() {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
        }

        void use_vao() {
            glBindVertexArray(vao);
        }

        void draw_all() {
            glDrawArrays(GL_TRIANGLES, 0, vcount);
        }
};

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Maze", NULL, NULL);
    if (!window)
        panic("Failed to create GLFW window");
    // Create context and callbacks
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // Initialize gl3w
    int error = gl3wInit();
    if (error)
        panic("Failed to initialize OpenGL, gl3w error: %i\n", error);
    if (!gl3wIsSupported(3, 3))
        panic("OpenGL 3.3 is not supported\n");
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
           glGetString(GL_SHADING_LANGUAGE_VERSION));
    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    // Compile shader
    Shader shader("src/camera.vert", "src/camera.frag");
    // Set up vertex and buffer data with attributes
    vabo vabo_cube({3, 2}, wall_verts);
    vabo vabo_floor({3, 2}, floor_verts);
    // Load textures
    stbi_set_flip_vertically_on_load(true);
    unsigned tx_wall = load_texture("./textures/wall_0.png");
    unsigned tx_grass = load_texture("./textures/grass_0.png");
    unsigned tx_marble = load_texture("./textures/marble_0.png");
    shader.use();
    shader.setInt("texture0", 0);
    // Create maze, initialize variables
    vector<vector<bool>> maze = make_maze(10);
    float msize = maze.size() / 2;
    float player_x = 1, player_y = 1;
    float player_vx = 0, player_vy = 0;
    // Render loop
    shader.use();
    while (!glfwWindowShouldClose(window)) {
        // Delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Keyboard input
        process_input(window);
        // Clear screen and depth buffers
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Set projection based on fov and screen width
        glm::mat4 projection = glm::perspective(
            glm::radians(fov), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        shader.setMat4("projection", projection);
        // Camera/View transformation
        glm::mat4 view = glm::lookAt(cam_pos, cam_pos + cam_front, cam_up);
        shader.setMat4("view", view);
        // Maze rotation transformation
        glm::mat4 maze_rot(1);
        maze_rot = rotate(maze_rot, maze_roll, glm::vec3(0, 0, 1));
        maze_rot = rotate(maze_rot, maze_pitch, glm::vec3(1, 0, 0));
        // Player collision
        const float gravity = 0.1, max_vel = 0.1, psize = 0.25;
        bool y_hit = false, x_hit = false;
        raycast rays[4];
        rays[0] = DDA_raycast(maze, player_x + psize, player_y + psize,
                              player_vx, player_vy);
        rays[1] = DDA_raycast(maze, player_x + psize, player_y + psize * 3,
                              player_vx, player_vy);
        rays[2] = DDA_raycast(maze, player_x + psize * 3, player_y + psize,
                              player_vx, player_vy);
        rays[3] = DDA_raycast(maze, player_x + psize * 3, player_y + psize * 3,
                              player_vx, player_vy);
        for (unsigned i = 0; i < 4; i++) {
            if (rays[i].hit && !rays[i].yhit)
                x_hit = true;
            if (rays[i].hit && rays[i].yhit)
                y_hit = true;
        }

        // Player movement and bounce
        if (x_hit)
            player_vx = -player_vx / 3;
        if (y_hit)
            player_vy = -player_vy / 3;
        player_x += player_vx;
        player_y += player_vy;
        // Player acceleration
        player_vx += (-maze_roll / (PI / 2)) * gravity;
        player_vy += (maze_pitch / (PI / 2)) * gravity;
        player_vx = LIMIT(player_vx, -max_vel, max_vel);
        player_vy = LIMIT(player_vy, -max_vel, max_vel);
        // Render maze
        vabo_cube.use_vao();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tx_wall);
        for (unsigned int i = 0; i < maze.size(); i++) {
            for (unsigned int j = 0; j < maze[0].size(); j++) {
                if (maze[i][j]) {
                    glm::mat4 model = translate(maze_rot,
                                      glm::vec3(i - msize, 0, j - msize));
                    shader.setMat4("model", model);
                    vabo_cube.draw_all();
                }
            }
        }
        // Render skybox
        glm::mat4 model(1);
        model = scale(model, glm::vec3(maze.size() * 2));
        shader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tx_marble);
        vabo_cube.draw_all();
        // Render player
        glm::vec3 player_pos(player_x - msize, -psize, player_y - msize);
        model = maze_rot;
        model = translate(model, player_pos);
        model = scale(model, glm::vec3(0.5));
        shader.setMat4("model", model);
        vabo_cube.draw_all();
        // Render floor, avoid Z fighting
        model = maze_rot;
        model = translate(model, glm::vec3(0, -0.501, 0));
        model = scale(model, glm::vec3(maze.size()));
        shader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tx_grass);
        vabo_floor.use_vao();
        vabo_floor.draw_all();
        // Render and poll
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}



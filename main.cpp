#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

// ─── Window ──────────────────────────────────────────────────────────────────
const int SCR_W = 1024, SCR_H = 768;

// ─── Sphere geometry ─────────────────────────────────────────────────────────
struct Mesh {
    GLuint vao, vbo, ebo;
    int indexCount;
};

Mesh buildSphere(int stacks, int slices) {
    std::vector<float>    verts;
    std::vector<unsigned> idx;

    for (int i = 0; i <= stacks; ++i) {
        float phi = M_PI * i / stacks;          // 0 … π
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.f * M_PI * j / slices;
            float x = sinf(phi) * cosf(theta);
            float y = cosf(phi);
            float z = sinf(phi) * sinf(theta);
            // pos
            verts.push_back(x); verts.push_back(y); verts.push_back(z);
            // normal (same as pos for unit sphere)
            verts.push_back(x); verts.push_back(y); verts.push_back(z);
            // uv
            verts.push_back((float)j / slices);
            verts.push_back((float)i / stacks);
        }
    }
    for (int i = 0; i < stacks; ++i)
        for (int j = 0; j < slices; ++j) {
            int a = i*(slices+1)+j, b = a+slices+1;
            idx.push_back(a); idx.push_back(b); idx.push_back(a+1);
            idx.push_back(b); idx.push_back(b+1); idx.push_back(a+1);
        }

    Mesh m;
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*4, verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()*4, idx.data(), GL_STATIC_DRAW);
    int stride = 8*sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    m.indexCount = (int)idx.size();
    return m;
}

// ─── Shader helpers ──────────────────────────────────────────────────────────
static std::string readFile(const char* path) {
    std::ifstream f(path);
    if (!f) { std::cerr << "Cannot open: " << path << "\n"; return ""; }
    std::ostringstream ss; ss << f.rdbuf(); return ss.str();
}

static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    int ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512]; glGetShaderInfoLog(s, 512, nullptr, log);
        std::cerr << "Shader error:\n" << log << "\n";
    }
    return s;
}

static GLuint buildProgram(const char* vsPath, const char* fsPath) {
    std::string vsrc = readFile(vsPath), fsrc = readFile(fsPath);
    GLuint vs = compileShader(GL_VERTEX_SHADER,   vsrc.c_str());
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsrc.c_str());
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs); glAttachShader(prog, fs);
    glLinkProgram(prog);
    int ok; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512]; glGetProgramInfoLog(prog, 512, nullptr, log);
        std::cerr << "Link error:\n" << log << "\n";
    }
    glDeleteShader(vs); glDeleteShader(fs);
    return prog;
}

// ─── Texture loader ──────────────────────────────────────────────────────────
static GLuint loadTexture(const char* path) {
    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &ch, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << "\n";
        // Return a 1x1 fallback texture
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        unsigned char fallback[4] = {200, 100, 50, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, fallback);
        return tex;
    }
    GLenum fmt = (ch == 4) ? GL_RGBA : (ch == 3 ? GL_RGB : GL_RED);
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return tex;
}

// ─── Main ────────────────────────────────────────────────────────────────────
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* win = glfwCreateWindow(SCR_W, SCR_H, "Solar System - Phong Shading", nullptr, nullptr);
    if (!win) { std::cerr << "GLFW window failed\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr << "GLEW init failed\n"; return -1; }
    glEnable(GL_DEPTH_TEST);

    // Shaders
    GLuint prog = buildProgram("vertex.glsl", "fragment.glsl");

    // Geometry
    Mesh sphere = buildSphere(64, 64);

    // Textures
    GLuint sunTex   = loadTexture("textures/sun.jpg");
    GLuint earthTex = loadTexture("textures/earth.jpg");

    // Uniform locations
    GLint uModel      = glGetUniformLocation(prog, "uModel");
    GLint uView       = glGetUniformLocation(prog, "uView");
    GLint uProj       = glGetUniformLocation(prog, "uProj");
    GLint uLightPos   = glGetUniformLocation(prog, "uLightPos");
    GLint uViewPos    = glGetUniformLocation(prog, "uViewPos");
    GLint uTexture    = glGetUniformLocation(prog, "uTexture");
    GLint uIsLight    = glGetUniformLocation(prog, "uIsLightSource");
    GLint uAmbient    = glGetUniformLocation(prog, "uAmbientStrength");
    GLint uSpecStr    = glGetUniformLocation(prog, "uSpecularStrength");
    GLint uShininess  = glGetUniformLocation(prog, "uShininess");

    // Camera
    glm::vec3 camPos(0.f, 8.f, 18.f);
    glm::mat4 view = glm::lookAt(camPos, glm::vec3(0), glm::vec3(0,1,0));
    glm::mat4 proj = glm::perspective(glm::radians(45.f), (float)SCR_W/SCR_H, 0.1f, 100.f);

    // Light at sun center (world origin)
    glm::vec3 lightPos(0.f, 0.f, 0.f);

    float orbitAngle = 0.f;
    float selfRotAngle = 0.f;

    while (!glfwWindowShouldClose(win)) {
        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(win, true);

        float t = (float)glfwGetTime();
        orbitAngle   = t * 0.5f;   // orbit speed
        selfRotAngle = t * 1.2f;   // earth self-rotation

        glClearColor(0.02f, 0.02f, 0.08f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(prog);
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3fv(uLightPos, 1, glm::value_ptr(lightPos));
        glUniform3fv(uViewPos,  1, glm::value_ptr(camPos));
        glUniform1i(uTexture, 0);
        glActiveTexture(GL_TEXTURE0);

        // ── Sun ──────────────────────────────────────────────────────────────
        glm::mat4 sunModel = glm::mat4(1.f);
        sunModel = glm::scale(sunModel, glm::vec3(2.5f));
        sunModel = glm::rotate(sunModel, t * 0.1f, glm::vec3(0,1,0)); // slow self-rotation
        glUniformMatrix4fv(uModel,  1, GL_FALSE, glm::value_ptr(sunModel));
        glUniform1i(uIsLight,  1);
        glUniform1f(uAmbient,  1.0f);    // fully bright (emissive)
        glUniform1f(uSpecStr,  0.0f);
        glUniform1f(uShininess, 1.f);
        glBindTexture(GL_TEXTURE_2D, sunTex);
        glBindVertexArray(sphere.vao);
        glDrawElements(GL_TRIANGLES, sphere.indexCount, GL_UNSIGNED_INT, 0);

        // ── Earth ─────────────────────────────────────────────────────────────
        float orbitRadius = 7.5f;
        glm::vec3 earthPos(
            orbitRadius * cosf(orbitAngle),
            0.f,
            orbitRadius * sinf(orbitAngle)
        );
        glm::mat4 earthModel = glm::mat4(1.f);
        earthModel = glm::translate(earthModel, earthPos);
        earthModel = glm::rotate(earthModel, selfRotAngle, glm::vec3(0,1,0));
        earthModel = glm::scale(earthModel, glm::vec3(1.0f));
        glUniformMatrix4fv(uModel,  1, GL_FALSE, glm::value_ptr(earthModel));
        glUniform1i(uIsLight,  0);
        glUniform1f(uAmbient,  0.08f);
        glUniform1f(uSpecStr,  0.5f);
        glUniform1f(uShininess, 64.f);
        glBindTexture(GL_TEXTURE_2D, earthTex);
        glDrawElements(GL_TRIANGLES, sphere.indexCount, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &sphere.vao);
    glDeleteBuffers(1, &sphere.vbo);
    glDeleteBuffers(1, &sphere.ebo);
    glDeleteProgram(prog);
    glfwTerminate();
    return 0;
}

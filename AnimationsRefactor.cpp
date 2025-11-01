#include <GL/glew.h>
#include <iostream>
#include <string>
#include <vector>
//ifglm==1.0.1
#define GLM_ENABLE_EXPERIMENTAL
//
#define GLM_FORCE_INTRINSICS
#define GLM_FORCE_AVX2
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <random>
// #include <memory>
#include <vector>
// #include <algorithm>
// #include <chrono>
// #include <thread>
#include <string_view>
#include <mutex>
#include <sstream>
// #include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define sqrt2 1.41421
#define underS 0.1
// g++-14 -std=c++26 -O3 -msse4.2 -mavx2 -ffast-math AnimationsRefactor.cpp -o parsing  -DSHM -I/usr/include/freetype2/ -lGL -lGLU -lGLEW -lglfw -lm -lfreetype -lassimp
// clang++-20 -std=c++26 -O3 -msse4.2 -mavx2 -ffast-math AnimationsRefactor.cpp -o parsing  -DSHM -I/usr/include/freetype2/ -lGL -lGLU -lGLEW -lglfw -lm -lfreetype -lassimp


//////////////////////////////////////////////////////////////////////
// Уровни логирования
enum class LogLevel {
    ERROR = 0,
    WARNING,
    INFO,
    DEBUG
};

// Структура для описания каждого вывода
struct LogOutput {
    std::ostream* stream; // указатель на поток (например, std::cout или файл)
    std::ofstream fileStream; // если вывод в файл
    LogLevel level; // уровень для этого вывода
    bool isFile; // флаг, что это файл
};

class Logger {
public:
    static void init(LogLevel level = LogLevel::INFO, std::string_view filename = "", LogLevel fileLevel = LogLevel::DEBUG) {
        auto& inst = getInstance();
        inst.setLogLevel(level);
        if (!filename.empty()) {
            inst.addOutputFile(std::string(filename), fileLevel);
        }
    }

    static void setLevel(LogLevel level) {
        getInstance().setLogLevel(level);
    }

    static void addOutputFile(std::string_view filename, LogLevel level = LogLevel::DEBUG) {
        getInstance().addFileOutput(std::string(filename), level);
    }

    // Логирование сообщения с уровнем
    static void log(LogLevel level, std::string_view message) {
        getInstance().logMessage(level, message);
    }

    template<typename T>
    static void log(LogLevel level, std::string_view message, const T& arg) {
        std::ostringstream oss;
        oss << message << arg;
        getInstance().logMessage(level, oss.str());
    }

    template<typename T, typename... Args>
    static void log(LogLevel level, std::string_view message, const T& first, const Args&... rest) {
        std::ostringstream oss;
        oss << message << first;
        (oss << ... << rest);
        getInstance().logMessage(level, oss.str());
    }

    static void shutdown() {
        auto& inst = getInstance();
        for (auto& out : inst.outputs) {
            if (out.isFile && out.fileStream.is_open()) {
                out.fileStream.close();
            }
        }
        inst.outputs.clear();
    }

private:
    LogLevel currentLevel = LogLevel::INFO;
    std::vector<LogOutput> outputs;
    std::mutex mutex_;

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    Logger() = default;

    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        currentLevel = level;
    }

    void addFileOutput(std::string_view filename, LogLevel level) {
        LogOutput out;
        out.fileStream.open(std::string(filename));
        out.stream = &out.fileStream;
        out.level = level;
        out.isFile = true;
        outputs.push_back(std::move(out));
    }

    void logMessage(LogLevel level, std::string_view message) {
        if (level > currentLevel) return;

        std::lock_guard<std::mutex> lock(mutex_);
        std::string prefix = levelToString(level);
        std::string t{"[" + prefix + "] " + std::string(message)};
        std::string_view output{t};

        // Вывод в консоль
        std::cout << output << std::endl;

        // Вывод в файлы
        for (auto& file : outputs) {
            if (file.isFile) {
                file.fileStream << output << std::endl;
            }
        }
    }

    std::string levelToString(LogLevel level) {
        switch (level) {
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::INFO: return "INFO";
        case LogLevel::DEBUG: return "DEBUG";
        default: return "UNKNOWN";
        }
    }
};
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//getrand from x to y
int GetRand(int a,int b){

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_int_distribution<> distrib(a, b);

    return distrib(gen);
}
//////////////////////////////////////////////////////////////////////




typedef unsigned int uint;
typedef unsigned char byte;

inline glm::mat4 assimpToGlmMatrix(aiMatrix4x4 mat)
{
    glm::mat4 m;
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            m[x][y] = mat[y][x];
        }
    }
    return m;
}
inline glm::vec3 assimpToGlmVec3(aiVector3D vec)
{
    return glm::vec3(vec.x, vec.y, vec.z);
}

inline glm::quat assimpToGlmQuat(aiQuaternion quat)
{
    glm::quat q;
    q.x = quat.x;
    q.y = quat.y;
    q.z = quat.z;
    q.w = quat.w;

    return q;
}
inline unsigned int createShader(const char *vertexStr, const char *fragmentStr)
{
    int success;
    char info_log[512];
    uint
        program = glCreateProgram(),
        vShader = glCreateShader(GL_VERTEX_SHADER),
        fShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vShader, 1, &vertexStr, 0);
    glCompileShader(vShader);
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vShader, 512, 0, info_log);
        std::cout << "vertex shader compilation failed!\n"
                  << info_log << std::endl;
    }
    glShaderSource(fShader, 1, &fragmentStr, 0);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fShader, 512, 0, info_log);
        std::cout << "fragment shader compilation failed!\n"
                  << info_log << std::endl;
    }

    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, 0, info_log);
        std::cout << "program linking failed!\n"
                  << info_log << std::endl;
    }
    glDetachShader(program, vShader);
    glDeleteShader(vShader);
    glDetachShader(program, fShader);
    glDeleteShader(fShader);

    return program;
}

/*
opengl skeletal animation demo
*/
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
inline GLFWwindow *initWindow(int &windowWidth, int &windowHeight)
{
    if (!glfwInit())
    {

        // std::cerr << "Failed to initialize GLFW" << std::endl;
        Logger::log(LogLevel::ERROR,"Failed to initialize glfwInit()" +std::to_string(__LINE__) + " " + __FILE__);
        // Handle error, e.g., throw an exception or exit
        // return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Create the window
    // window->width = width;
    // window->height = height;
    // window->title = title;
    GLFWwindow *window = glfwCreateWindow(800, 600, " window->title.c_str()", NULL, NULL);
    if (!window)
    {
        // std::cerr << "Failed to create GLFW window" << std::endl;
        Logger::log(LogLevel::ERROR,"Failed to create GLFW window window" +std::to_string(__LINE__)+ " " + __FILE__);
        glfwTerminate(); // Terminate GLFW if window creation fails
        // Handle error
        // return;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    // glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glfwSetCursorPosCallback(window, mouse_callback);
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        // std::cerr << "Failed to initialize GLEW" << std::endl;
        Logger::log(LogLevel::ERROR,"Failed to initialize GLEW glewInit()" +std::to_string(__LINE__)+ " " + __FILE__);
        // Handle error, e.g., throw an exception or exit
        // return;
    }

    glEnable(GL_DEPTH_TEST);
    return window;
}

bool firstMouse = true;
float yaw = -135.0f; // подобрано // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;
int animation = 0;
// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 50.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
// timing
float deltaTime = 0.0f; // time between current frame and last frame
const char *vertexShaderSource = R"(
	#version 460 core
	layout (location = 0) in vec3 position; 
	layout (location = 1) in vec3 normal;
	layout (location = 2) in vec2 uv;
	layout (location = 3) in vec4 boneIds;
	layout (location = 4) in vec4 boneWeights;

	out vec2 tex_cord;
	out vec3 v_normal;
	out vec3 v_pos;
	out vec4 bw;

	uniform mat4 bone_transforms[100];
	uniform mat4 view_projection_matrix;
	uniform mat4 model_matrix;

	void main()
	{
		bw = vec4(0);
		if(int(boneIds.x) == 1)
		bw.z = boneIds.x;
		//boneWeights = normalize(boneWeights);
		mat4 boneTransform  =  mat4(0.0);
		boneTransform  +=    bone_transforms[int(boneIds.x)] * boneWeights.x;
		boneTransform  +=    bone_transforms[int(boneIds.y)] * boneWeights.y;
		boneTransform  +=    bone_transforms[int(boneIds.z)] * boneWeights.z;
		boneTransform  +=    bone_transforms[int(boneIds.w)] * boneWeights.w;
		vec4 pos =boneTransform * vec4(position, 1.0);
		gl_Position = view_projection_matrix * model_matrix * pos;
		v_pos = vec3(model_matrix * boneTransform * pos);
		tex_cord = uv;
		v_normal = mat3(transpose(inverse(model_matrix * boneTransform))) * normal;
		v_normal = normalize(v_normal);
	}

	)";
const char *fragmentShaderSource = R"(
	#version 460 core

	in vec2 tex_cord;
	in vec3 v_normal;
	in vec3 v_pos;
	in vec4 bw;
	out vec4 color;

	uniform sampler2D diff_texture;

	vec3 lightPos = vec3(0.2, 1.0, -3.0);
	
	void main()
	{
		vec3 lightDir = normalize(lightPos - v_pos);
		float diff = max(dot(v_normal, lightDir), 0.2);
		vec3 dCol = diff * texture(diff_texture, tex_cord).rgb; 
		color = vec4(dCol, 1);
	}
	)";

// vertex of an animated model
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 boneIds = glm::vec4(0);
    glm::vec4 boneWeights = glm::vec4(0.0f);
};

// structure to hold bone tree (skeleton)
struct Bone
{
    int id = 0; // position of the bone in final upload array
    std::string name = "";
    glm::mat4 offset = glm::mat4(1.0f);
    std::vector<Bone> children = {};
};

// sturction representing an animation track
struct BoneTransformTrack
{
    std::vector<float> positionTimestamps = {};
    std::vector<float> rotationTimestamps = {};
    std::vector<float> scaleTimestamps = {};

    std::vector<glm::vec3> positions = {};
    std::vector<glm::quat> rotations = {};
    std::vector<glm::vec3> scales = {};
};

// structure containing animation information
struct Animation
{
    float duration = 0.0f;
    float ticksPerSecond = 1.0f;
    std::unordered_map<std::string, BoneTransformTrack> boneTransforms = {};
};

struct ModelVB
{
    GLuint vao, vbo, ebo;
    GLuint textureID;
};

struct ModelVI
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

struct Skeletons
{
    std::vector<Bone> skeleton;
};

struct Animations
{
    std::vector<Animation> animations;
};
//////////////////////////////////////////////////////////
struct Pose
{
    std::vector<glm::mat4> pose;
};

struct PrecomputedAnimation
{
    int start;
    int end;
    std::vector<Pose> poses;
};

struct Animator
{
    int Number;
    std::vector<PrecomputedAnimation> pAnimations;
};
//////////////////////////////////////////////////////////
struct ModelLocs
{
    GLuint *Model;
    GLuint *BonesT;
    GLuint *Texture;
};
//////////////////////////////////////////////////////////
struct Model
{
    std::string name;
    ModelVB *modelVB;
    ModelVI *modelVI;
    Skeletons *skeletons;
    Animations *animations;

    GLuint *shader;
    ModelLocs *locs;

    glm::vec3 pos;
    glm::vec3 rA{180, 0, 0};
    glm::quat orientation;
    glm::vec3 sc{.05f, .05f, .05f};
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    float speed;
    int pseudoTimer;//unittime quant
    // bool gotoPatrol;
    bool patrol;//state
    bool agro;//state
    bool agrostart;//state
    bool agroend;//state
    glm::mat4 modelMatrix;

    unsigned int boneCount = 0;
    glm::mat4 globalInverseTransform;
    glm::mat4 identity;
    std::vector<glm::mat4> currentPose;

    Animator *animtor;
    unsigned int it = 0;

    int frame = 0;

    // Время для анимации
    float currentTime = 0.0f;
    bool playing = true;
    float playbackSpeed = 30.f;
    bool loop = true;
};
////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// table AnimationModel
struct AnimationModel
{
    std::vector<Model> models;
};
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
struct ModelOnLevel
{
    int n = 0;
    std::vector<Model> instances;
};
///////////////////////////////////////////////////////////
// a recursive function to read all bones and form skeleton
bool readSkeleton(Bone &boneOutput, aiNode *node, std::unordered_map<std::string, std::pair<int, glm::mat4>> &boneInfoTable)
{

    if (boneInfoTable.find(node->mName.C_Str()) != boneInfoTable.end())
    { // if node is actually a bone
        boneOutput.name = node->mName.C_Str();
        boneOutput.id = boneInfoTable[boneOutput.name].first;
        boneOutput.offset = boneInfoTable[boneOutput.name].second;

        for (int i = 0; i < node->mNumChildren; i++)
        {
            Bone child;
            readSkeleton(child, node->mChildren[i], boneInfoTable);
            boneOutput.children.push_back(child);
        }
        return true;
    }
    else
    { // find bones in children
        for (int i = 0; i < node->mNumChildren; i++)
        {
            if (readSkeleton(boneOutput, node->mChildren[i], boneInfoTable))
            {
                return true;
            }
        }
    }
    return false;
}

void loadMeshBones(const aiMesh *mesh,
                   std::vector<Vertex> &verticesOutput,
                   std::vector<uint> &indicesOutput,
                   std::unordered_map<std::string, std::pair<int, glm::mat4>> &boneInfo,
                   uint &nBoneCount)
{

    verticesOutput = {};
    indicesOutput = {};
    // load position, normal, uv

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        // process position
        Vertex vertex;
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        // process normal
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;
        // process uv
        glm::vec2 vec;
        vec.x = mesh->mTextureCoords[0][i].x;
        vec.y = mesh->mTextureCoords[0][i].y;
        vertex.uv = vec;

        vertex.boneIds = glm::ivec4(0);
        vertex.boneWeights = glm::vec4(0.0f);

        verticesOutput.push_back(vertex);
    }

    // load boneData to vertices
    boneInfo.clear();
    std::vector<uint> boneCounts;
    boneCounts.resize(verticesOutput.size(), 0);
    nBoneCount = mesh->mNumBones;

    // loop through each bone
    for (uint i = 0; i < nBoneCount; i++)
    {
        aiBone *bone = mesh->mBones[i];
        glm::mat4 m = assimpToGlmMatrix(bone->mOffsetMatrix);
        boneInfo[bone->mName.C_Str()] = {i, m};

        // loop through each vertex that have that bone
        for (int j = 0; j < bone->mNumWeights; j++)
        {
            uint id = bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;
            boneCounts[id]++;
            switch (boneCounts[id])
            {
            case 1:
                verticesOutput[id].boneIds.x = i;
                verticesOutput[id].boneWeights.x = weight;
                break;
            case 2:
                verticesOutput[id].boneIds.y = i;
                verticesOutput[id].boneWeights.y = weight;
                break;
            case 3:
                verticesOutput[id].boneIds.z = i;
                verticesOutput[id].boneWeights.z = weight;
                break;
            case 4:
                verticesOutput[id].boneIds.w = i;
                verticesOutput[id].boneWeights.w = weight;
                break;
            default:
                // std::cout << "err: unable to allocate bone to vertex" << std::endl;
                break;
            }
        }
    }

    // normalize weights to make all weights sum 1
    for (int i = 0; i < verticesOutput.size(); i++)
    {
        glm::vec4 &boneWeights = verticesOutput[i].boneWeights;
        float totalWeight = boneWeights.x + boneWeights.y + boneWeights.z + boneWeights.w;
        if (totalWeight > 0.0f)
        {
            verticesOutput[i].boneWeights = glm::vec4(
                boneWeights.x / totalWeight,
                boneWeights.y / totalWeight,
                boneWeights.z / totalWeight,
                boneWeights.w / totalWeight);
        }
    }

    // load indices
    for (int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace &face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indicesOutput.push_back(face.mIndices[j]);
    }
}
void loadSkeleton(const aiScene *scene, Bone &skeletonOutput, std::unordered_map<std::string, std::pair<int, glm::mat4>> &boneInfo)
{
    // Предположим, что readSkeleton заполняет скелетную иерархию
    if (readSkeleton(skeletonOutput, scene->mRootNode, boneInfo))
    {
        // std::cout << "YES skeleton" << std::endl;
    }
}

void loadAnimation(const aiScene *scene, Animation &animation, int i)
{
    // loading  first Animation
    // std::cout << "scene->mNumAnimations " << scene->mNumAnimations << std::endl;
    aiAnimation *anim = scene->mAnimations[i];
    // std::cout << "Name " << scene->mAnimations[i]->mName.C_Str() << std::endl;
    if (anim->mTicksPerSecond != 0.0f)
        animation.ticksPerSecond = anim->mTicksPerSecond;
    else
        animation.ticksPerSecond = 1.0f;

    animation.duration = anim->mDuration;
    animation.boneTransforms = {};

    // load positions rotations and scales for each bone
    //  each channel represents each bone
    for (int i = 0; i < anim->mNumChannels; i++)
    {
        aiNodeAnim *channel = anim->mChannels[i];
        BoneTransformTrack track;
        for (int j = 0; j < channel->mNumPositionKeys; j++)
        {
            track.positionTimestamps.push_back(channel->mPositionKeys[j].mTime);
            track.positions.push_back(assimpToGlmVec3(channel->mPositionKeys[j].mValue));
        }
        for (int j = 0; j < channel->mNumRotationKeys; j++)
        {
            track.rotationTimestamps.push_back(channel->mRotationKeys[j].mTime);
            track.rotations.push_back(assimpToGlmQuat(channel->mRotationKeys[j].mValue));
        }
        for (int j = 0; j < channel->mNumScalingKeys; j++)
        {
            track.scaleTimestamps.push_back(channel->mScalingKeys[j].mTime);
            track.scales.push_back(assimpToGlmVec3(channel->mScalingKeys[j].mValue));
        }
        animation.boneTransforms[channel->mNodeName.C_Str()] = track;
    }
}

void createVertexArray(GLuint &vao, GLuint &vbo, GLuint &ebo, std::vector<Vertex> &vertices, std::vector<uint> indices)
{

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, uv));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, boneIds));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, boneWeights));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), &indices[0], GL_STATIC_DRAW);
    glBindVertexArray(0);
}

uint createTexture(std::string filepath)
{
    uint textureId = 0;
    int width, height, nrChannels;
    byte *data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 4);
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureId;
}

std::pair<uint, float> getTimeFraction(std::vector<float> &times, float &dt)
{
    uint segment = 0;
    if (dt <= times.front())
    {
        return {0, 0.0f};
    }
    // Если dt больше последнего времени, возвращаем последний сегмент и 1.0
    if (dt >= times.back())
    {
        return {(uint)times.size(), 1.0f};
    }
    while (dt > times[segment])
        segment++;
    float start = times[segment - 1];
    float end = times[segment];
    float frac = (dt - start) / (end - start);
    return {segment, frac};
}

void getPoseSIMD(
    Animation &animation,
    Bone &skeleton,
    float &dt,
    std::vector<glm::mat4> &output,
    glm::mat4 &parentTransform,
    glm::mat4 &globalInverseTransform)
{
    BoneTransformTrack &btt = animation.boneTransforms[skeleton.name];

    dt = fmod(dt, animation.duration);
    auto fp = getTimeFraction(btt.positionTimestamps, dt);
    uint idx = fp.first;
    float frac = fp.second;

    // Проверка границ
    if (idx == 0)
        idx = 1;
    if (idx >= btt.positions.size())
    {
        // std::cout << "btt" << std::endl;
        idx = btt.positions.size() - 1;
    }

    // std::cout<<"TestAfter2 "<<idx << " "<<frac<<std::endl;
    if (btt.positions.empty() || btt.scales.empty() || btt.rotations.empty())
    {
        std::cerr << "Empty bone transform track for " << skeleton.name << std::endl;
        return;
    }
    // Интерполяция позиции
    glm::vec3 pos1 = btt.positions[idx - 1];
    glm::vec3 pos2 = btt.positions[idx];
    glm::vec3 position = glm::mix(pos1, pos2, frac);

    // std::cout<<"TestAfter3 "<<idx << " "<<frac<<std::endl;
    // Интерполяция масштаба
    glm::vec3 scale1 = btt.scales[idx - 1];
    glm::vec3 scale2 = btt.scales[idx];
    glm::vec3 scale = glm::mix(scale1, scale2, frac);

    // std::cout<<"TestAfter4 "<<idx << " "<<frac<<std::endl;

    glm::quat rot1 = btt.rotations[idx - 1];
    glm::quat rot2 = btt.rotations[idx];

    // std::cout<<"TestAfter5 "<<idx << " "<<frac<<std::endl;
    glm::quat rotation = glm::slerp(rot1, rot2, frac);

    // std::cout<<"TestAfter6 "<<idx << " "<<frac<<std::endl;
    // Создаем локальную матрицу трансформации
    glm::mat4 positionMat = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotationMat = glm::toMat4(rotation);
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
    glm::mat4 localTransform = positionMat * rotationMat * scaleMat;

    // std::cout<<"TestAfter7 "<<idx << " "<<frac<<std::endl;
    glm::mat4 globalTransform = parentTransform * localTransform;
    output[skeleton.id] = globalInverseTransform * globalTransform * skeleton.offset;

    // std::cout<<"TestAfter8 "<<idx << " "<<frac<<std::endl;
    for (Bone &child : skeleton.children)
    {
        // std::cout<<"TestAfter9 "<<idx << " "<<frac<<std::endl;
        getPoseSIMD(animation, child, dt, output, globalTransform, globalInverseTransform);
        // std::cout<<"TestAfter10 "<<idx << " "<<frac<<std::endl;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateModel(Model *model)
{
    model->locs = new ModelLocs;
    model->modelVB = new ModelVB;
    model->modelVI = new ModelVI;
    model->skeletons = new Skeletons;
    model->animations = new Animations;
    model->animtor = new Animator;
}
void updateModel(Model *model, float deltaTime, int h)
{
    int t = abs(h);
    // model->animtor->pAnimations[t].end;
    if (model->currentTime > model->animtor->pAnimations[t].end)
    {
        if (model->loop && h > 0)
        {
            model->currentTime = 1.0f;
        }
        else if (!model->loop && h > 0)
        {
            model->currentTime = model->animtor->pAnimations[t].end;
            model->playing = false;
        }
        else if (model->loop && h < 0)
        {
            model->currentTime = model->animtor->pAnimations[t].end;
        }
        else if (!model->loop && h < 0)
        {
            model->currentTime = 1.0f;
            model->playing = false;
        }
    }
    if (model->currentTime < 0)
    {
        if (model->loop)
        {
            model->currentTime = model->animtor->pAnimations[t].end;
        }
        else
        {
            model->currentTime = 1.0f;
            model->playing = false;
        }
    }
    if (!model->playing)
        return;

    // Обновляем время
    if (h > 0)
    {
        model->currentTime += deltaTime * model->playbackSpeed;
        model->it = glm::clamp(int(model->currentTime), 0, (int)model->animtor->pAnimations[t].end - 1);
    }
    else if (h < 0)
    {
        model->currentTime -= deltaTime * model->playbackSpeed;
        model->it = glm::clamp(int(model->currentTime), 0, (int)model->animtor->pAnimations[t].end - 1);
    }
    // model->animtor->pAnimations[t].poses[model->it];model->precAnim[t][model->it];
    model->currentPose = model->animtor->pAnimations[t].poses[model->it].pose;
}
void updateModelL(Model *model, float deltaTime, int h)
{
    if (model->currentTime > model->animations->animations[h].duration)
    {
        if (model->loop)
        {
            model->currentTime = 1.0f;
        }
        else
        {
            model->currentTime = model->animations->animations[h].duration;
            model->playing = false;
        }
    }
    if (!model->playing)
        return;

    // Обновляем время
    model->currentTime += deltaTime * model->playbackSpeed;
}
void drawModel(Model *model)
{
    glUniformMatrix4fv(*model->locs->BonesT, model->boneCount, GL_FALSE, glm::value_ptr(model->currentPose[0]));

    glBindVertexArray(model->modelVB->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, model->modelVB->textureID);
    glUniform1i(*model->locs->Texture, 0);

    glDrawElements(GL_TRIANGLES, model->modelVI->indices.size(), GL_UNSIGNED_INT, 0);
}
void DeleteModel(Model *model)
{

    delete model->locs;
    glDeleteBuffers(1, &model->modelVB->vbo);
    glDeleteVertexArrays(1, &model->modelVB->vao);
    glDeleteBuffers(1, &model->modelVB->ebo);
    glDeleteTextures(1, &model->modelVB->textureID);
    delete model->modelVB;
    delete model->modelVI;
    delete model->skeletons;
    delete model->animations;
    delete model->animtor;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void processInput(GLFWwindow *window, Model *model);
////////////////////////////////////////////////////////////////////////////////////////////////////////
// testFunctions
//standardFUnction
void CreateInstancesOnLevel(ModelOnLevel *ms, AnimationModel *TableAnimationModel, uint &shader, int n)
{
    ms->n = n;
    // modelsOnLevel.instances = instances;
    ms->instances.resize(n);
    int l = underS * ms->n;
    // std::cout << (float)(sqrt(2)) << std::endl;
    int tC = 0;
    int kC = 0;
    int ccc = 0;
    for (int i = 0; i < l; ++i)
    {
        for (int j = 0; j < l; ++j)
        {
            // if (kC == 13)
            //     kC = 0;

            Model enemy = TableAnimationModel->models[0];
            // std::cout << ccc << std::endl;

            enemy.name = "Enemy" + std::to_string(ccc);
            // enemy.shader = &shader;

            glm::vec3 pos = glm::vec3(5.0f, 0.0f, 5.0f);
            glm::quat newRotation = glm::angleAxis(glm::radians(enemy.rA.x), glm::vec3(1.0f, 0.0f, 0.0f)); // типо прямо
            //   newRotation = glm::angleAxis(glm::radians(enemy.rA.y), glm::vec3(0.0f, 1.0f, 0.0f));
            //   newRotation = glm::angleAxis(glm::radians(enemy.rA.z), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 rotationMatrix = glm::toMat4(newRotation);
            glm::mat4 modelMatrix(1.0f);
            modelMatrix = glm::translate(modelMatrix, pos);
            modelMatrix = modelMatrix * rotationMatrix; // Apply rotation
            // modelMatrix = glm::rotate(modelMatrix, glm::radians(enemy.rot.x), glm::vec3(0.0f, 0.0f, 1.0f));
            // modelMatrix = glm::rotate(modelMatrix, glm::radians(enemy.rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
            // modelMatrix = glm::rotate(modelMatrix, glm::radians(enemy.rot.z), glm::vec3(1.0f, 0.0f, 0.0f));
            modelMatrix = glm::scale(modelMatrix, enemy.sc);
            enemy.pos = pos;
            enemy.pseudoTimer = 0;
            enemy.orientation = newRotation;
            enemy.frame = 12;
            enemy.speed = 10;
            enemy.modelMatrix = modelMatrix;

            ms->instances[ccc] = enemy;

            // kC++;
            ccc++;
        }
    }
}
//stressTest10000
void CreateInstancesOnLevel10000(ModelOnLevel *ms, AnimationModel *TableAnimationModel, uint &shader)
{
    ms->n = 10000;
    // modelsOnLevel.instances = instances;
    ms->instances.resize(10000);
    int l = underS * ms->n;
    // std::cout << (float)(sqrt(2)) << std::endl;
    int tC = 0;
    int kC = 0;
    int ccc = 0;
    for (int i = 0; i < 100; ++i)
    {
        for (int j = 0; j < 100; ++j)
        {
            // if (kC == 13)
            //     kC = 0;

            Model enemy = TableAnimationModel->models[0];
            // std::cout << ccc << std::endl;

            enemy.name = "Enemy" + std::to_string(ccc);
            // enemy.shader = &shader;

            glm::vec3 pos = glm::vec3(5.0f, 0.0f, 5.0f);
            glm::quat newRotation = glm::angleAxis(glm::radians(enemy.rA.x), glm::vec3(1.0f, 0.0f, 0.0f)); // типо прямо
            //   newRotation = glm::angleAxis(glm::radians(enemy.rA.y), glm::vec3(0.0f, 1.0f, 0.0f));
            //   newRotation = glm::angleAxis(glm::radians(enemy.rA.z), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 rotationMatrix = glm::toMat4(newRotation);
            glm::mat4 modelMatrix(1.0f);
            modelMatrix = glm::translate(modelMatrix, pos);
            modelMatrix = modelMatrix * rotationMatrix; // Apply rotation
            // modelMatrix = glm::rotate(modelMatrix, glm::radians(enemy.rot.x), glm::vec3(0.0f, 0.0f, 1.0f));
            // modelMatrix = glm::rotate(modelMatrix, glm::radians(enemy.rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
            // modelMatrix = glm::rotate(modelMatrix, glm::radians(enemy.rot.z), glm::vec3(1.0f, 0.0f, 0.0f));
            modelMatrix = glm::scale(modelMatrix, enemy.sc);
            enemy.pos = pos;
            enemy.pseudoTimer = 0;
            enemy.orientation = newRotation;
            enemy.frame = 12;
            enemy.speed = 10;
            enemy.modelMatrix = modelMatrix;

            ms->instances[ccc] = enemy;

            // kC++;
            ccc++;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void loadModelB(Model *model, const std::string s, GLuint *modelLoc, GLuint *BonesTLoc, GLuint *TextureLoc)
{

    Assimp::Importer importer;
    const char *filePath = s.c_str();
    const aiScene *scene = importer.ReadFile(filePath, aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        // std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        std::string err="ERROR::ASSIMP::";
        err+=importer.GetErrorString() +std::to_string(__LINE__);
        Logger::log(LogLevel::ERROR,err);
        return;
    }

    aiMesh *mesh = scene->mMeshes[0];

    uint nBoneCount = 0;
    // Загрузка меша и весов
    std::unordered_map<std::string, std::pair<int, glm::mat4>> boneInfo;
    loadMeshBones(mesh, model->modelVI->vertices, model->modelVI->indices, boneInfo, nBoneCount);
    model->boneCount = nBoneCount;
    for (int i = 0; i < scene->mNumAnimations; ++i)
    {
        // Загрузка скелета
        Bone skeleton;
        loadSkeleton(scene, skeleton, boneInfo);
        // Загрузка анимации
        Animation animation;
        loadAnimation(scene, animation, i);

        model->skeletons->skeleton.push_back(skeleton);
        model->animations->animations.push_back(animation);
    }
    // as the name suggests just inverse the global transform
    model->globalInverseTransform = assimpToGlmMatrix(scene->mRootNode->mTransformation);
    model->globalInverseTransform = glm::inverse(model->globalInverseTransform);

    // std::cout << animation.duration << std::endl;
    createVertexArray(model->modelVB->vao, model->modelVB->vbo, model->modelVB->ebo, model->modelVI->vertices, model->modelVI->indices);
    model->modelVB->textureID = createTexture("man/diffuse.png");

    model->identity = glm::mat4(1.0f);

    // currentPose is held in this vector and uploaded to gpu as a matrix array uniform
    model->currentPose.resize(model->boneCount, model->identity); // use this for no animation

    model->locs->Model = modelLoc;
    model->locs->BonesT = BonesTLoc;
    model->locs->Texture = TextureLoc;

    importer.FreeScene();
}

void LoadAnimationModel(Model &model, const std::string s, GLuint *modelLoc, GLuint *BonesTLoc, GLuint *TextureLoc)
{

    loadModelB(&model, s, modelLoc, BonesTLoc, TextureLoc);

    model.animtor->Number = model.animations->animations.size();
    model.animtor->pAnimations.resize(model.animtor->Number);
    float lastTime = glfwGetTime();
    for (int h = 0; h < model.animtor->Number; ++h)
    {
        PrecomputedAnimation precomputeAnimation;
        precomputeAnimation.start = 1.0f;
        precomputeAnimation.end = model.animations->animations[h].duration;
        precomputeAnimation.poses.resize(model.animations->animations[h].duration);

        unsigned int duration = model.animations->animations[h].duration;

        for (unsigned int i = 0; i < duration; ++i)
        {
            // std::cout << "Frame: " << i << std::endl;
            float currentTime = glfwGetTime();
            deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            updateModelL(&model, deltaTime, h);

            float o = static_cast<float>(i); // или используйте real time, если нужно

            getPoseSIMD(
                model.animations->animations[h],
                model.skeletons->skeleton[h],
                o,
                model.currentPose,
                model.identity,
                model.globalInverseTransform);

            Pose pose;
            pose.pose = model.currentPose;
            precomputeAnimation.poses[i] = pose;
        }
        model.animtor->pAnimations[h] = precomputeAnimation;
    }
}
///////////////////////////////////////////////////////////////////////////
// struct Patrol
// {
//     glm::vec3 min,max;
// };

// void uniquePatrol(Patrol *patrol,glm::vec3 pos,int w,int h){

// }

// struct Player
// {
//     int index;
//     glm::vec3 pos;
//     glm::vec3 rA{180, 0, 0};
//     glm::quat orientation;
//     glm::vec3 sc{.05f, .05f, .05f};
//     glm::vec3 front{0.0f, 0.0f, -1.0f};
// };

// struct Creature
// {
//     int index;
//     glm::vec3 pos;
//     glm::vec3 rA{180, 0, 0};
//     glm::quat orientation;
//     glm::vec3 sc{.05f, .05f, .05f};
//     glm::vec3 front{0.0f, 0.0f, -1.0f};
// };
//////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    Logger::init(LogLevel::DEBUG, "", LogLevel::WARNING);
    // init
    int windowWidth, windowHeight;
    GLFWwindow *window = initWindow(windowWidth, windowHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    uint shader = createShader(vertexShaderSource, fragmentShaderSource);

    // get all shader uniform locations
    uint viewProjectionMatrixLocation = glGetUniformLocation(shader, "view_projection_matrix");
    uint modelMatrixLocation = glGetUniformLocation(shader, "model_matrix");
    uint boneMatricesLocation = glGetUniformLocation(shader, "bone_transforms");
    uint textureLocation = glGetUniformLocation(shader, "diff_texture");

    // initialize projection view and model matrix
    glm::mat4 projectionMatrix = glm::perspective(75.0f, (float)800 / 600, 0.01f, 10000.0f);

    glm::mat4 viewMatrix;

    glm::mat4 viewProjectionMatrix;

    float lastTime = glfwGetTime();

    Model model;
    AnimationModel TableAnimationModel;
    TableAnimationModel.models.push_back(model);
    CreateModel(&TableAnimationModel.models[0]);
    std::string str = "Animations/TestCharacter/characterTest3.fbx";
    LoadAnimationModel(TableAnimationModel.models[0], str, &modelMatrixLocation, &boneMatricesLocation, &textureLocation);

    ModelOnLevel modelsOnLevel;

    CreateInstancesOnLevel(&modelsOnLevel, &TableAnimationModel, shader, 100);
    // CreateInstancesOnLevel10000(&modelsOnLevel, &TableAnimationModel, shader);
    // std::cout << sizeof(TableAnimationModel.models[0].animtor->pAnimations[0]) << std::endl;
    float angle2 = 1;
    int kl = 0;
    glm::vec3 enemyForward = glm::vec3(0.0f, 0.0f, 1.0f);
    float angle1 = 45.0f;
    modelsOnLevel.instances[0].name ={"Player"};
    // modelsOnLevel.instances[0].name = "Player";
    modelsOnLevel.instances[0].pos = modelsOnLevel.instances[0].modelMatrix[3];
    // for (auto &a : modelsOnLevel.instances)
    //     std::cout << a.name << std::endl;
    // srand(time(nullptr));
    int closePP = 0;
    while (!glfwWindowShouldClose(window))
    {
        glm::vec3 objectPos = glm::vec3(modelsOnLevel.instances[0].pos);
        float distanceBehind = 40.0f;
        cameraPos = objectPos + modelsOnLevel.instances[0].front * distanceBehind + glm::vec3(0.0f, -7, 0.0f);

        modelsOnLevel.instances[0].frame = animation;
        viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        viewProjectionMatrix = projectionMatrix * viewMatrix;
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        // input
        // -----

        processInput(window, &modelsOnLevel.instances[0]);

        // if(angle==360)angle=0;
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        glUniformMatrix4fv(viewProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

        // modelsOnLevel.instances[1].modelMatrix = glm::translate(modelsOnLevel.instances[1].modelMatrix, enemyForward);
        for (auto &a : modelsOnLevel.instances)
        {
            if (a.name != "Player")
            {
                a.pos += a.speed * a.front * deltaTime;
                if (a.pseudoTimer == 0)
                {
                    a.patrol = true;
                    float tr = GetRand(0,360);
                    // std::cout << tr << std::endl;
                    Logger::log(LogLevel::INFO,a.name+" Generate new Angle to "+std::to_string(tr));
                    a.rA.y += glm::degrees(tr);
                    // Создаем кватернионы по осям
                    glm::quat pitchQuat = glm::angleAxis(glm::radians(a.rA.x), glm::vec3(1.0f, 0.0f, 0.0f)); // типо прямо
                    glm::quat yawQuat = glm::angleAxis(glm::radians(a.rA.y), glm::vec3(0.0f, 1.0f, 0.0f));
                    glm::quat combinedRotation = yawQuat * pitchQuat;
                    a.front = glm::normalize(glm::rotate(combinedRotation, glm::vec3(0, 0, -1)));
                    a.frame = 12;
                    a.speed = 10 + rand() % 5;
                }
                if (a.pseudoTimer == 1000)
                {
                    a.frame = 10;
                    a.agro = true;
                    a.agrostart = true;
                    glm::vec3 directionToPlayer = glm::normalize(modelsOnLevel.instances[0].pos - a.pos);
                    // float dotResult = glm::dot(enemyForward, directionToPlayer);
                    // dotResult = glm::clamp(dotResult, -1.0f, 1.0f); // безопасность

                    // // Используем atan2 для определения угла с учетом направления
                    // float angle4 = atan2(glm::cross(enemyForward, directionToPlayer).y, glm::dot(enemyForward, directionToPlayer));

                    // modelsOnLevel.instances[1].rA.y += glm::degrees(angle4);
                    // enemyForward = directionToPlayer;
                    float interpolation_factor = 0.1f; // Коэффициент интерполяции

                    glm::vec3 forward = glm::normalize(a.pos + a.front);

                    glm::quat current_quaternion = glm::rotation(forward, glm::normalize(a.front));
                    glm::quat target_quaternion = glm::rotation(forward, glm::normalize(directionToPlayer));

                    glm::quat new_quaternion = glm::slerp(current_quaternion, target_quaternion, interpolation_factor);

                    new_quaternion = glm::normalize(new_quaternion);
                    a.front = directionToPlayer;
                    a.rA.y += glm::degrees(glm::axis(new_quaternion).y * 3);
                    a.speed = 20 + rand() % 5;
                }
                if (a.pseudoTimer > 1000 && glm::distance(a.pos, modelsOnLevel.instances[0].pos) <= 1.0f)
                { // batle begin :)
                    a.agroend = true;
                    a.pseudoTimer = 0;
                    continue;
                }
                a.pseudoTimer++;
                // std::cout<<kl<<std::endl;
            }
        }
        for (auto &m : modelsOnLevel.instances)
        {

            updateModel(&m, deltaTime, m.frame);

            glm::mat4 modelMatrix = glm::mat4(1.0f);

            // Создаем кватернионы по осям
            glm::quat newRotationX = glm::angleAxis(glm::radians(m.rA.x), glm::vec3(0.0f, 0.0f, 1.0f)); // типо обратно типо относительно камеры наверно
            glm::quat newRotationY = glm::angleAxis(glm::radians(m.rA.y), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat newRotationZ = glm::angleAxis(glm::radians(m.rA.z), glm::vec3(1.0f, 0.0f, 0.0f));

            // Комбинируем вращения (порядок важен)
            m.orientation = newRotationZ * newRotationY * newRotationX;

            // Конвертируем кватернион в матрицу
            glm::mat4 rotationMatrix = glm::toMat4(m.orientation);

            // Создаем матрицы трансформаций
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m.pos);
            glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m.sc);

            // Собираем итоговую модельную матрицу
            modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            // }

            drawModel(&m);
        }
        // angle+=5.0f;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    Logger::shutdown();
    DeleteModel(&TableAnimationModel.models[0]);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window, Model *model)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(20.5 * deltaTime);
    // glm::vec3 objectPos = glm::vec3(p[3]);
    bool idle = true;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        // model->modelMatrix = glm::translate(model->modelMatrix, cameraFront);
        model->pos += cameraSpeed * cameraFront;
        cameraPos += cameraSpeed * cameraFront;
        animation = 12;
        idle = false;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        // model->modelMatrix = glm::translate(model->modelMatrix, cameraFront * (-1.0f));
        model->pos -= cameraSpeed * cameraFront;
        cameraPos -= cameraSpeed * cameraFront;
        animation = -12;
        idle = false;
    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        // model->modelMatrix = glm::translate(model->modelMatrix, glm::normalize(glm::cross(cameraFront, cameraUp)) * (-1.0f));
        model->pos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        animation = 4;
        idle = false;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        // model->modelMatrix = glm::translate(model->modelMatrix, glm::normalize(glm::cross(cameraFront, cameraUp)));
        model->pos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        animation = 8;
        idle = false;
    }
    if (idle)
    {
        animation = 0;
    }
}
// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Создаем кватернионы по осям
    glm::quat pitchQuat = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yawQuat = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));

    // Объединяем вращения (порядок важен, обычно yaw * pitch)
    glm::quat combinedRotation = yawQuat * pitchQuat;

    // Применяем к базовому вектору направления
    cameraFront = glm::normalize(glm::rotate(combinedRotation, glm::vec3(1, 0, -1)));
}


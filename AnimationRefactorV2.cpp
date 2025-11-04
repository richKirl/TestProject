//code based on this code https://github.com/hasinaxp/skeletal_animation-_assimp_opengl
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
#include <vector>
#include <string_view>
#include <mutex>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define sqrt2 1.41421
#define underS 0.1
// g++-14 -std=c++26 -O3 -msse4.2 -mavx2 -ffast-math AnimationsRefactor{V2}.cpp -o parsing  -DSHM -I/usr/include/freetype2/ -lGL -lGLU -lGLEW -lglfw -lm -lfreetype -lassimp
// clang++-20 -std=c++26 -O3 -msse4.2 -mavx2 -ffast-math AnimationsRefactor{V2}.cpp -o parsing  -DSHM -I/usr/include/freetype2/ -lGL -lGLU -lGLEW -lglfw -lm -lfreetype -lassimp
// or cmake


//////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
///
enum class LogLevel {
    ERROR = 0,
    WARNING,
    INFO,
    DEBUG
};

// Структура для описания каждого вывода
////////////////////////////////////////////////////////////
/// \brief The LogOutput class
///
struct LogOutput {
    std::ostream* stream; // указатель на поток (например, std::cout или файл)
    std::ofstream fileStream; // если вывод в файл
    LogLevel level; // уровень для этого вывода
    bool isFile; // флаг, что это файл
};
////////////////////////////////////////////////////////////
/// \brief The Logger class
///
class Logger {
public:
    //инициализация
    ////////////////////////////////////////////////////////////
    /// \brief init
    /// \param level
    /// \param filename
    /// \param fileLevel
    ///
    static void init(LogLevel level = LogLevel::INFO, std::string_view filename = "", LogLevel fileLevel = LogLevel::DEBUG) {
        auto& inst = getInstance();
        inst.setLogLevel(level);
        if (!filename.empty()) {
            inst.addOutputFile(std::string(filename), fileLevel);
        }
    }
    //установка уровня логирования
    ////////////////////////////////////////////////////////////
    /// \brief setLevel
    /// \param level
    ///
    static void setLevel(LogLevel level) {
        getInstance().setLogLevel(level);
    }
    //добавление файла в который будет производиться лог
    ////////////////////////////////////////////////////////////
    /// \brief addOutputFile
    /// \param filename
    /// \param level
    ///
    static void addOutputFile(std::string_view filename, LogLevel level = LogLevel::DEBUG) {
        getInstance().addFileOutput(std::string(filename), level);
    }
    //добавление потока например std::cout
    ////////////////////////////////////////////////////////////
    /// \brief addOutputStream
    /// \param s
    /// \param level
    ///
    static void addOutputStream(std::ostream *s, LogLevel level = LogLevel::DEBUG) {
        getInstance().addOutput(s, level);
    }

    // Логирование сообщения с уровнем
    ////////////////////////////////////////////////////////////
    /// \brief log
    /// \param level
    /// \param message
    ///
    static void log(LogLevel level, std::string_view message) {
        getInstance().logMessage(level, message);
    }
    ////////////////////////////////////////////////////////////
    /// \brief log
    /// \param level
    /// \param message
    /// \param arg
    ///
    template<typename T>
    static void log(LogLevel level, std::string_view message, const T& arg) {
        std::ostringstream oss;
        oss << message << arg;
        getInstance().logMessage(level, oss.str());
    }
    ////////////////////////////////////////////////////////////
    /// \brief log
    /// \param level
    /// \param message
    /// \param first
    /// \param rest
    ///
    template<typename T, typename... Args>
    static void log(LogLevel level, std::string_view message, const T& first, const Args&... rest) {
        std::ostringstream oss;
        oss << message << first;
        (oss << ... << rest);
        getInstance().logMessage(level, oss.str());
    }

    //закрытие файла/ов
    ////////////////////////////////////////////////////////////
    /// \brief shutdown
    ///
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
    ////////////////////////////////////////////////////////////
    /// \brief getInstance
    /// \return
    ///
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    Logger() = default;

    //установка уровня логирования типом логирования
    ////////////////////////////////////////////////////////////
    /// \brief setLogLevel
    /// \param level
    ///
    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        currentLevel = level;
    }
    //добавление потока для логирования например std::cout
    ////////////////////////////////////////////////////////////
    /// \brief addOutput
    /// \param s
    /// \param level
    ///
    void addOutput(std::ostream *s, LogLevel level) {
        LogOutput out;
        // out.fileStream.open(std::string(filename));
        out.stream = s;
        out.level = level;
        out.isFile = false;
        outputs.push_back(std::move(out));
    }
    //добавление файла для логирования
    ////////////////////////////////////////////////////////////
    /// \brief addFileOutput
    /// \param filename
    /// \param level
    ///
    void addFileOutput(std::string_view filename, LogLevel level) {
        LogOutput out;
        out.fileStream.open(std::string(filename));
        out.stream = &out.fileStream;
        out.level = level;
        out.isFile = true;
        outputs.push_back(std::move(out));
    }
    //логирование
    ////////////////////////////////////////////////////////////
    /// \brief logMessage
    /// \param level
    /// \param message
    ///
    void logMessage(LogLevel level, std::string_view message) {
        if (level > currentLevel) return;

        std::lock_guard<std::mutex> lock(mutex_);
        std::string prefix = levelToString(level);
        std::string t{"[" + prefix + "] " + std::string(message)};
        std::string_view output{t};

        // Вывод в файлы
        for (auto& file : outputs) {
            if (file.isFile) {
                file.fileStream << output << std::endl;
            }
            if(file.stream){
                *file.stream << output << std::endl;
            }
        }
    }
    //получение строкового литерала уровня из типа уровня
    ////////////////////////////////////////////////////////////
    /// \brief levelToString
    /// \param level
    /// \return
    ///
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
////////////////////////////////////////////////////////////
#define outputInfo(...) Logger::log(LogLevel::INFO,__VA_ARGS__)
#define outputWarning(...) Logger::log(LogLevel::WARNING,__VA_ARGS__)
#define outputError(...) Logger::log(LogLevel::ERROR,__VA_ARGS__)
#define outputDebug(...) Logger::log(LogLevel::DEBUG,__VA_ARGS__)
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//getrand from x to y
////////////////////////////////////////////////////////////
/// \brief GetRand
/// \param a
/// \param b
/// \return
///
int GetRand(int a,int b){

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_int_distribution<> distrib(a, b);

    return distrib(gen);
}
//////////////////////////////////////////////////////////////////////




typedef unsigned int uint;
typedef unsigned char byte;
////////////////////////////////////////////////////////////
/// \brief assimpToGlmMatrix
/// \param mat
/// \return
///
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
////////////////////////////////////////////////////////////
/// \brief assimpToGlmVec3
/// \param vec
/// \return
///
inline glm::vec3 assimpToGlmVec3(aiVector3D vec)
{
    return glm::vec3(vec.x, vec.y, vec.z);
}
////////////////////////////////////////////////////////////
/// \brief assimpToGlmQuat
/// \param quat
/// \return
///
inline glm::quat assimpToGlmQuat(aiQuaternion quat)
{
    glm::quat q;
    q.x = quat.x;
    q.y = quat.y;
    q.z = quat.z;
    q.w = quat.w;

    return q;
}
////////////////////////////////////////////////////////////
/// \brief createShader
/// \param vertexStr
/// \param fragmentStr
/// \return
///
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
        std::cout << "vertex shader compilation failed!\n" << info_log << std::endl;
    }
    glShaderSource(fShader, 1, &fragmentStr, 0);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fShader, 512, 0, info_log);
        std::cout << "fragment shader compilation failed!\n" << info_log << std::endl;
    }

    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, 0, info_log);
        std::cout << "program linking failed!\n" << info_log << std::endl;
    }
    glDetachShader(program, vShader);
    glDeleteShader(vShader);
    glDetachShader(program, fShader);
    glDeleteShader(fShader);

    return program;
}
/////////////////////////////////////////////////////////////////////
//this lines only for this test
int windowWidth, windowHeight;
//
bool firstMouse = true;
float yaw = -315.0f;// !// подобрано // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;
int animation = 0;
// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 50.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);// !
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
// timing
float deltaTime = 0.0f; // time between current frame and last frame
//////////////////////////////////////////////////////////////////////
/*
opengl skeletal animation demo
*/
////////////////////////////////////////////////////////////
/// \brief framebuffer_size_callback
/// \param window
/// \param width
/// \param height
///
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
////////////////////////////////////////////////////////////
/// \brief framebuffer_size_callback
/// \param window
/// \param width
/// \param height
///
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    windowWidth=width;
    windowHeight=height;
}
////////////////////////////////////////////////////////////
/// \brief mouse_callback
/// \param window
/// \param xpos
/// \param ypos
///
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
////////////////////////////////////////////////////////////
/// \brief initWindow
/// \param windowWidth
/// \param windowHeight
/// \return
///
inline GLFWwindow *initWindow(int &windowWidth, int &windowHeight)//snippet window
{
    if (!glfwInit())
    {

        Logger::log(LogLevel::ERROR,"Failed to initialize glfwInit()" +std::to_string(__LINE__) + " " + __FILE__);
        // Handle error, e.g., throw an exception or exit
        // return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    windowWidth = 800;
    windowHeight = 600;
    GLFWwindow *window = glfwCreateWindow(800, 600, " window->title.c_str()", NULL, NULL);
    if (!window)
    {
        Logger::log(LogLevel::ERROR,"Failed to create GLFW window window" +std::to_string(__LINE__)+ " " + __FILE__);
        glfwTerminate(); // Terminate GLFW if window creation fails

    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glfwSetCursorPosCallback(window, mouse_callback);
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        Logger::log(LogLevel::ERROR,"Failed to initialize GLEW glewInit()" +std::to_string(__LINE__)+ " " + __FILE__);
    }

    glEnable(GL_DEPTH_TEST);
    return window;
}

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
////////////////////////////////////////////////////////////
/// \brief The Vertex class
///
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 boneIds = glm::vec4(0);
    glm::vec4 boneWeights = glm::vec4(0.0f);
};

// structure to hold bone tree (skeleton)
////////////////////////////////////////////////////////////
/// \brief The Bone class
///
struct Bone
{
    glm::mat4 offset = glm::mat4(1.0f);
    std::string name = "";
    std::vector<Bone> children = {};
    int id = 0; // position of the bone in final upload array
};

// sturction representing an animation track
////////////////////////////////////////////////////////////
/// \brief The BoneTransformTrack class
///
struct BoneTransformTrack
{
    std::vector<float> positionTimestamps = {};
    std::vector<float> rotationTimestamps = {};
    std::vector<float> scaleTimestamps = {};

    std::vector<glm::vec3> positions = {};
    std::vector<glm::quat> rotations = {};
    std::vector<glm::vec3> scales = {};
};


////////////////////////////////////////////////////////////
/// \brief The Animation class
///structure containing animation information
struct Animation
{
    std::unordered_map<std::string, BoneTransformTrack> boneTransforms = {};
    float duration = 0.0f;
    float ticksPerSecond = 1.0f;
};
////////////////////////////////////////////////////////////
/// \brief The ModelVB class
///
struct ModelVB
{
    GLuint vao, vbo, ebo;
    GLuint textureID;
};
////////////////////////////////////////////////////////////
/// \brief The ModelVI class
///
struct ModelVI
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};
////////////////////////////////////////////////////////////
/// \brief The Skeletons class
///
struct Skeletons
{
    std::vector<Bone> skeleton;
};
////////////////////////////////////////////////////////////
/// \brief The Animations class
///
struct Animations
{
    std::vector<Animation> animations;
};
//////////////////////////////////////////////////////////
/// \brief The Pose class
///
struct Pose
{
    std::vector<glm::mat4> pose;
};
//////////////////////////////////////////////////////////
/// \brief The PrecomputedAnimation class
///
struct PrecomputedAnimation
{
    std::vector<Pose> poses;
    int start;
    int end;
};
//////////////////////////////////////////////////////////
/// \brief The FrameName class
///
struct FrameName{
    std::unordered_map<std::string_view,int> frames;
};
//////////////////////////////////////////////////////////
/// \brief The Animator class
///
struct Animator
{
    std::vector<PrecomputedAnimation> pAnimations;
    int Number;
};
//////////////////////////////////////////////////////////
/// \brief The ModelLocs class
///
struct ModelLocs
{
    GLuint *Model;
    GLuint *BonesT;
    GLuint *Texture;
};
//////////////////////////////////////////////////////////
/// \brief The Model class
///
struct Model
{
    glm::mat4 globalInverseTransform;
    glm::mat4 identity;
    std::vector<glm::mat4> currentPose;

    ModelVB *modelVB;
    ModelVI *modelVI;
    Skeletons *skeletons;
    Animations *animations;

    GLuint *shader;
    ModelLocs *locs;

    FrameName frameName;
    Animator *animtor;

    unsigned int boneCount = 0;
    unsigned int it = 0;

    // Время для анимации
    float currentTime = 0.0f;
    float playbackSpeed = 30.f;
    bool playing = true;
    bool loop = true;
};

////////////////////////////////////////////////////////////
/// \brief The Patrol class
///
struct Patrol
{
    glm::vec3 pos,min,max;//area for patrol
    float distance;//distance from patrol
    bool gotoPatrol;//state
    bool patrol;//state
    bool idle;
    bool agro;//state
    bool agrostart;//state
    bool agroend;//state
    bool battle;//battlemode
};


///////////////////////////////////////////////////////////////////
/// \brief The Creature class
///
struct Creature
{
    std::string name;
    glm::quat orientation;
    glm::vec3 pos;
    glm::vec3 rA{180, 0, 0};//from model - example blender
    glm::vec3 sc{.05f, .05f, .05f};//from model - example blender
    glm::vec3 front{0.0f, 0.0f, 1.0f};//! //forward way
    Model *model;
    Patrol *patrolBehavior;

    float speed;
    int pseudoTimer;//unittime quant
    int frame;
};
///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// table AnimationModel
///////////////////////////////////////////////////////////
/// \brief The AnimationModel class
///
struct AnimationModel
{
    std::vector<Model> models;
};
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///
// struct ModelOnLevel
// {
//     int n = 0;
//     std::vector<Model> instances;
// };
struct ModelOnLevel
{
    std::vector<Creature> instances;
    int n = 0;
};
///////////////////////////////////////////////////////////
// a recursive function to read all bones and form skeleton
///////////////////////////////////////////////////////////
/// \brief readSkeleton
/// \param boneOutput
/// \param node
/// \param boneInfoTable
/// \return
///
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
///////////////////////////////////////////////////////////
/// \brief loadMeshBones
/// \param mesh
/// \param verticesOutput
/// \param indicesOutput
/// \param boneInfo
/// \param nBoneCount
///
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
///////////////////////////////////////////////////////////
/// \brief loadSkeleton
/// \param scene
/// \param skeletonOutput
/// \param boneInfo
///
void loadSkeleton(const aiScene *scene, Bone &skeletonOutput, std::unordered_map<std::string, std::pair<int, glm::mat4>> &boneInfo)
{
    // Предположим, что readSkeleton заполняет скелетную иерархию
    if (readSkeleton(skeletonOutput, scene->mRootNode, boneInfo))
    {
        // std::cout << "YES skeleton" << std::endl;
    }
}
///////////////////////////////////////////////////////////
/// \brief loadAnimation
/// \param scene
/// \param animation
/// \param i
///
void loadAnimation(const aiScene *scene, Animation &animation, int i)
{
    // loading  first Animation
    // std::cout << "scene->mNumAnimations " << scene->mNumAnimations << std::endl;
    aiAnimation *anim = scene->mAnimations[i];
    // std::cout << "Name " << scene->mAnimations[i]->mName.C_Str() << std::endl;
    std::string_view temp=scene->mAnimations[i]->mName.C_Str();
    size_t np=temp.find_last_of("|");
    outputInfo("Name: ",temp.substr(np+1));
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
///////////////////////////////////////////////////////////
/// \brief loadAnimation
/// \param scene
/// \param animation
/// \param i
///
void loadAnimation(const aiScene *scene, Animation &animation,FrameName &frames, int i)
{
    // loading  first Animation
    // std::cout << "scene->mNumAnimations " << scene->mNumAnimations << std::endl;
    aiAnimation *anim = scene->mAnimations[i];
    // std::cout << "Name " << scene->mAnimations[i]->mName.C_Str() << std::endl;
    std::string_view temp=scene->mAnimations[i]->mName.C_Str();
    size_t np=temp.find_last_of("|");
    outputInfo("Name: ",temp.substr(np+1));
    frames.frames.emplace(temp.substr(np+1),i);
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
///////////////////////////////////////////////////////////
/// \brief createVertexArray
/// \param vao
/// \param vbo
/// \param ebo
/// \param vertices
/// \param indices
///
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
///////////////////////////////////////////////////////////
/// \brief createTexture
/// \param filepath
/// \return
///
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
///////////////////////////////////////////////////////////
/// \brief getTimeFraction
/// \param times
/// \param dt
/// \return
///
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
///////////////////////////////////////////////////////////
/// \brief getPoseSIMD
/// \param animation
/// \param skeleton
/// \param dt
/// \param output
/// \param parentTransform
/// \param globalInverseTransform
///
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
    // if (idx >= btt.positions.size())
    // {
    //     idx = btt.positions.size() - 1;
    // }cov

    // if (btt.positions.empty() || btt.scales.empty() || btt.rotations.empty())
    // {
    //     std::cerr << "Empty bone transform track for " << skeleton.name << std::endl;
    //     return;
    // }cov
    // Интерполяция позиции
    glm::vec3 pos1 = btt.positions[idx - 1];
    glm::vec3 pos2 = btt.positions[idx];
    glm::vec3 position = glm::mix(pos1, pos2, frac);

    // Интерполяция масштаба
    glm::vec3 scale1 = btt.scales[idx - 1];
    glm::vec3 scale2 = btt.scales[idx];
    glm::vec3 scale = glm::mix(scale1, scale2, frac);

    glm::quat rot1 = btt.rotations[idx - 1];
    glm::quat rot2 = btt.rotations[idx];

    glm::quat rotation = glm::slerp(rot1, rot2, frac);

    // Создаем локальную матрицу трансформации
    glm::mat4 positionMat = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotationMat = glm::toMat4(rotation);
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
    glm::mat4 localTransform = positionMat * rotationMat * scaleMat;

    glm::mat4 globalTransform = parentTransform * localTransform;
    output[skeleton.id] = globalInverseTransform * globalTransform * skeleton.offset;

    for (Bone &child : skeleton.children)
    {
        getPoseSIMD(animation, child, dt, output, globalTransform, globalInverseTransform);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief CreateModel
/// \param model
///
void CreateModel(Model *model)
{
    model->locs = new ModelLocs;
    model->modelVB = new ModelVB;
    model->modelVI = new ModelVI;
    model->skeletons = new Skeletons;
    model->animations = new Animations;
    model->animtor = new Animator;
}
///////////////////////////////////////////////////////////
/// \brief updateModel
/// \param model
/// \param deltaTime
/// \param h (>0 вперёд, <0 в обратном порядке)
///
void updateModel(Creature *model, float deltaTime, int h)
{
    int t = abs(h);
    if (model->model->currentTime > model->model->animtor->pAnimations[t].end)
    {
        if (model->model->loop && h > 0)
        {
            model->model->currentTime = 1.0f;
        }
        else if (!model->model->loop && h > 0)
        {
            model->model->currentTime = model->model->animtor->pAnimations[t].end;
            model->model->playing = false;
        }
        else if (model->model->loop && h < 0)
        {
            model->model->currentTime = model->model->animtor->pAnimations[t].end;
        }
        else if (!model->model->loop && h < 0)
        {
            model->model->currentTime = 1.0f;
            model->model->playing = false;
        }
    }
    if (model->model->currentTime < 0)
    {
        if (model->model->loop)
        {
            model->model->currentTime = model->model->animtor->pAnimations[t].end;
        }
        else
        {
            model->model->currentTime = 1.0f;
            model->model->playing = false;
        }
    }
    if (!model->model->playing)
        return;

    // Обновляем время
    if (h > 0)
    {
        model->model->currentTime += deltaTime * model->model->playbackSpeed;
        model->model->it = glm::clamp(int(model->model->currentTime), 0, (int)model->model->animtor->pAnimations[t].end - 1);
    }
    else if (h < 0)
    {
        model->model->currentTime -= deltaTime * model->model->playbackSpeed;
        model->model->it = glm::clamp(int(model->model->currentTime), 0, (int)model->model->animtor->pAnimations[t].end - 1);
    }
    model->model->currentPose = model->model->animtor->pAnimations[t].poses[model->model->it].pose;
}
///////////////////////////////////////////////////////////
/// \brief updateModelL
/// \param model
/// \param deltaTime
/// \param h
/// update for unpack animations
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
///////////////////////////////////////////////////////////
/// \brief drawModel
/// \param model
///
void drawModel(Creature *model)
{
    glUniformMatrix4fv(*model->model->locs->BonesT, model->model->boneCount, GL_FALSE, glm::value_ptr(model->model->currentPose[0]));

    glBindVertexArray(model->model->modelVB->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, model->model->modelVB->textureID);
    glUniform1i(*model->model->locs->Texture, 0);

    glDrawElements(GL_TRIANGLES, model->model->modelVI->indices.size(), GL_UNSIGNED_INT, 0);
}
///////////////////////////////////////////////////////////
/// \brief DeleteModel
/// \param model
///
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
void processInput(GLFWwindow *window, Creature *model);
///////////////////////////////////////////////////////////
/// \brief rotateOrientationFromCurrentTo
/// \param from
/// \param to
/// \return
///
glm::quat rotateOrientationFromCurrentTo(glm::quat &from,glm::quat &to){
    glm::quat current = from;

    glm::quat rotD=to*glm::inverse(current);

    return rotD*current;
}
//////////////////////////////////////////////////
/// \brief setOrientToPoint
/// \param a
/// \param point
///
void setOrientToPoint(Creature *a,glm::vec3 point){
    glm::vec3 directionToPlayer = glm::normalize(point - a->pos);
    glm::quat pitchQuat = glm::angleAxis(glm::radians(a->rA.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat temp =  glm::quatLookAt(directionToPlayer, glm::vec3(0,1,0));
    glm::quat current_quaternion = a->orientation * glm::vec3(0,1,0);

    glm::quat yawQuat = temp;
    glm::quat combinedRotation = yawQuat *pitchQuat;
    glm::quat final = rotateOrientationFromCurrentTo(current_quaternion,combinedRotation);

    a->orientation = final;
    a->front = a->orientation * glm::vec3(0,0,1);//!
}
//////////////////////////////////////////////////
/// \brief setOrientToAngle
/// \param a
/// \param angle
///
void setOrientToAngle(Creature *a,float angle){
    a->rA.y = glm::degrees(angle);
    // Создаем кватернионы по осям
    glm::quat pitchQuat = glm::angleAxis(glm::radians(a->rA.x), glm::vec3(1.0f, 0.0f, 0.0f)); // типо прямо
    glm::quat yawQuat = glm::angleAxis(glm::radians(a->rA.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat combinedRotation = yawQuat*pitchQuat;//aply transform
    glm::quat final = rotateOrientationFromCurrentTo(a->orientation,combinedRotation);
    a->orientation = final;
    a->front = a->orientation * glm::vec3(0,0,1);//!
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
// testFunctions
//standardFUnction
///////////////////////////////////////////////////////////
/// \brief CreateInstancesOnLevel
/// \param ms
/// \param TableAnimationModel
/// \param shader
/// \param n
/// testFunction create *Creature* *Patrol* and set *Model*
void CreateInstancesOnLevel(ModelOnLevel *ms, AnimationModel *TableAnimationModel, uint &shader, int n)
{
    ms->n = n;
    ms->instances.resize(n);
    int l = underS * ms->n;
    int ccc = 0;
    for (int i = 0; i < l; ++i)
    {
        for (int j = 0; j < l; ++j)
        {
            Model *enemy = new Model;
            Patrol *patrol = new Patrol;
            patrol->gotoPatrol=true;
            patrol->patrol=false;
            patrol->agro=false;
            patrol->agrostart=false;
            patrol->agroend=false;
            patrol->battle=false;
            Creature creature;
            *enemy = TableAnimationModel->models[0];
            creature.model= enemy;
            creature.patrolBehavior=patrol;
            creature.name = "Creature" + std::to_string(ccc);

            glm::vec3 pos = glm::vec3(5.0f, 0.0f, 5.0f);
            glm::quat newRotation = glm::angleAxis(glm::radians(creature.rA.x), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::quat current = glm::quat(1,0,0,0);

            glm::quat final = rotateOrientationFromCurrentTo(current,newRotation);

            creature.pos = pos;
            creature.pseudoTimer = 0;
            creature.orientation = final;
            creature.frame = 12;
            creature.speed = 10;

            ms->instances[ccc] = creature;
            ccc++;
        }
    }
}

//stressTest10000
//////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief CreateInstancesOnLevel10000
/// \param ms
/// \param TableAnimationModel
/// \param shader
/// stress test function for 10000 models
void CreateInstancesOnLevel10000(ModelOnLevel *ms, AnimationModel *TableAnimationModel, uint &shader)
{
    ms->n = 10000;
    ms->instances.resize(10000);

    int ccc = 0;
    for (int i = 0; i < 100; ++i)
    {
        for (int j = 0; j < 100; ++j)
        {

            Model *enemy = new Model;
            Patrol *patrol = new Patrol;
            patrol->gotoPatrol=true;
            patrol->patrol=false;
            patrol->agro=false;
            patrol->agrostart=false;
            patrol->agroend=false;
            patrol->battle=false;
            Creature creature;
            *enemy = TableAnimationModel->models[0];
            creature.model= enemy;
            creature.patrolBehavior=patrol;
            creature.name = "Creature" + std::to_string(ccc);

            glm::vec3 pos = glm::vec3(5.0f, 0.0f, 5.0f);
            glm::quat newRotation = glm::angleAxis(glm::radians(creature.rA.x), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::quat current = glm::quat(1,0,0,0);

            glm::quat final = rotateOrientationFromCurrentTo(current,newRotation);

            creature.pos = pos;
            creature.pseudoTimer = 0;
            creature.orientation = final;
            creature.frame = 12;
            creature.speed = 10;

            ms->instances[ccc] = creature;
            ccc++;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief loadModelB
/// \param model
/// \param s
/// \param modelLoc
/// \param BonesTLoc
/// \param TextureLoc
///
void loadModelB(Model *model, const std::string s, GLuint *modelLoc, GLuint *BonesTLoc, GLuint *TextureLoc)
{

    Assimp::Importer importer;
    const char *filePath = s.c_str();
    const aiScene *scene = importer.ReadFile(filePath, aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
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
    FrameName frameName;
    for (int i = 0; i < scene->mNumAnimations; ++i)
    {
        // Загрузка скелета
        Bone skeleton;
        loadSkeleton(scene, skeleton, boneInfo);
        // Загрузка анимации
        Animation animation;
        loadAnimation(scene, animation,frameName, i);

        model->skeletons->skeleton.push_back(skeleton);
        model->animations->animations.push_back(animation);
    }
    model->frameName=frameName;
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief LoadAnimationModel
/// \param model
/// \param s
/// \param modelLoc
/// \param BonesTLoc
/// \param TextureLoc
///
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
/// \brief The Patrol class
///[0 0 1 1 1 1 1 1] behavior compress system
/// 0-0 - reserved
/// 1-0 - reserved
/// 2-1 - goto
/// 3-1 - patrol
/// 4-1 - agro
/// 5-1 - agrotart
/// 6-1 - agroend
/// 7-1 - battle
///
//////////////////////

// struct BehBytes{
//     uint8_t bytes;
// };
// void InitBeh(BehBytes *b){

// }


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
// };//


//////////////////////////////////////////////////////////////////////////////
/// \brief main
/// \param argc
/// \param argv
/// \return
///
int main(int argc, char **argv)
{
    Logger::init(LogLevel::DEBUG, "", LogLevel::WARNING);
    Logger::addOutputStream(&std::cout,LogLevel::DEBUG);
    // init

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

    modelsOnLevel.instances[0].name="Player";

    // modelsOnLevel.instances[0].pos = modelsOnLevel.instances[0].modelMatrix[3];

    outputInfo(" Hello World"," Hello World");
    // outputInfo("Hello World","j");

    while (!glfwWindowShouldClose(window))
    {
        glm::vec3 objectPos = glm::vec3(modelsOnLevel.instances[0].pos);
        float distanceBehind = 40.0f;
        cameraPos = objectPos + modelsOnLevel.instances[0].front * distanceBehind + glm::vec3(0.0f, -7, 0.0f);

        modelsOnLevel.instances[0].frame = animation;

        projectionMatrix = glm::perspective(75.0f, (float)windowWidth/windowHeight, 0.01f, 10000.0f);
        viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        viewProjectionMatrix = projectionMatrix * viewMatrix;
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        // input
        // -----



        processInput(window, &modelsOnLevel.instances[0]);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        glUniformMatrix4fv(viewProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

        for (auto &a : modelsOnLevel.instances)
        {
            if (a.name != "Player")
            {
                if (a.pseudoTimer > 1000 && glm::distance( modelsOnLevel.instances[0].pos,a.pos) <= 1.0f)
                { // batle begin :)
                    a.patrolBehavior->agroend = true;
                    a.patrolBehavior->agro = false;
                    a.patrolBehavior->agrostart = false;
                    a.patrolBehavior->gotoPatrol=true;//for this test
                    a.pseudoTimer = 0;
                    continue;
                }
                a.pos += a.speed * a.front * deltaTime;


                if (a.patrolBehavior->gotoPatrol)
                {
                    a.patrolBehavior->agroend=false;
                    a.patrolBehavior->gotoPatrol=false;
                    a.patrolBehavior->patrol = true;
                    float tr = GetRand(0,360);//anglereposition
                    // Logger::log(LogLevel::INFO,a.name+" Generate new Angle to "+std::to_string(tr));
                    //outputInfo(LogLevel::INFO,a.name+" Generate new Angle to "+std::to_string(tr));

                    setOrientToAngle(&a,tr);
                    a.frame = a.model->frameName.frames["walking"];
                    a.speed = 10 + GetRand(1,5);
                }
                else if (a.pseudoTimer == 1000)
                {
                    a.frame = a.model->frameName.frames["run"];
                    a.patrolBehavior->agro = true;
                    a.patrolBehavior->patrol = false;
                    a.patrolBehavior->agrostart = true;

                    setOrientToPoint(&a,modelsOnLevel.instances[0].pos);
                    a.speed = 20 + GetRand(1,5);
                }

                else if (a.patrolBehavior->agrostart){

                    setOrientToPoint(&a,modelsOnLevel.instances[0].pos);
                }
                a.pseudoTimer++;
            }
        }
        for (auto &m : modelsOnLevel.instances)
        {

            updateModel(&m, deltaTime, m.frame);

            glm::mat4 modelMatrix = glm::mat4(1.0f);

            // Конвертируем кватернион в матрицу
            glm::mat4 rotationMatrix = glm::toMat4(m.orientation);

            // Создаем матрицы трансформаций
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m.pos);
            glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m.sc);

            // Собираем итоговую модельную матрицу
            modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

            drawModel(&m);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    Logger::shutdown();
    for(auto &a:modelsOnLevel.instances){
        delete a.model;
        delete a.patrolBehavior;
    }
    DeleteModel(&TableAnimationModel.models[0]);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
/////////////////////////////////////////////////////////////////
/// \brief processInput
/// \param window
/// \param model
///
void processInput(GLFWwindow *window, Creature *model)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(20.5 * deltaTime);
    bool idle = true;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        model->pos += cameraSpeed * cameraFront;
        cameraPos += cameraSpeed * cameraFront;
        animation = model->model->frameName.frames["walking"];
        idle = false;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        model->pos -= cameraSpeed * cameraFront;
        cameraPos -= cameraSpeed * cameraFront;
        animation = -model->model->frameName.frames["walking"];
        idle = false;
    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        model->pos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        animation = model->model->frameName.frames["left_strafe"];
        idle = false;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        model->pos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        animation = model->model->frameName.frames["right_strafe"];
        idle = false;
    }
    if (idle)
    {
        animation = model->model->frameName.frames["idle"];
    }
}
// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
/////////////////////////////////////////////////////////////////////
/// \brief mouse_callback
/// \param window
/// \param xposIn
/// \param yposIn
///
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
    else if (pitch < -89.0f)
        pitch = -89.0f;

    // Создаем кватернионы по осям
    glm::quat pitchQuat = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yawQuat = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));

    // Объединяем вращения (порядок важен, обычно yaw * pitch)
    glm::quat combinedRotation = yawQuat * pitchQuat;

    // Применяем к базовому вектору направления
    cameraFront = combinedRotation * glm::vec3(1,0,-1);//!
}
///////////////////////////////////////////////////////////////////////////////////////////


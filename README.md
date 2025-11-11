# TestProject

<img width="1920" height="1019" alt="image" src="https://github.com/user-attachments/assets/f7762ab9-8f8b-483b-b7af-d59e444a5c5b" />

fix - orientation and set simple AI, but need fixes global orient, so yes in this time it global bug [01]

now AnimationsRefactor - basic waveGenerator :) typicaly have some bugs on neworientations but nice 

<details>
  <summary>Stress test 10000</summary>

<img width="793" height="621" alt="image" src="https://github.com/user-attachments/assets/0d1ec439-953e-4342-9372-ea0726a852af" />

after [01]

<img width="804" height="633" alt="testProgram1" src="https://github.com/user-attachments/assets/50f3da09-ff53-4ebd-bf0a-00f9d912fa62" />

</details>


<details>
  <summary>Progress</summary>

Detail current status

<details>
  <summary>Gentoo based tests</summary>


2000 000 particles (update base system with particle system)

<img width="1917" height="1016" alt="image" src="https://github.com/user-attachments/assets/647b39e1-d19e-4b74-bbc7-3cdd508df2a4" />

<img width="1920" height="1018" alt="image" src="https://github.com/user-attachments/assets/0a70d030-80a8-4c5c-beb9-65ad2c395aea" />



Tracking Motion (From point A to point B on the Route), double light(main scene, main model) with shadows, with npc-orientation! its just work :)
notice on this chapter:

<details>
  <summary>Code</summary>

```
from a to b Route(but route have points-steps local position from-to) need know orientation on local points
glm::quat temp =  glm::quatLookAt(dirr, glm::vec3(0,1,0));
obj.q = glm::slerp(obj.q,temp,0.1f);
1 step of Route (subsystem of routing on the mesh - navigation system - A* or dejkstra )
```

</details>

Yellow backcolor for debug space npc(silhouette)
<img width="1918" height="1018" alt="image" src="https://github.com/user-attachments/assets/75226699-4250-4eb1-9f54-27f322f83ac5" />

</details>

<details>
  <summary>Ubuntu based test</summary>

Ray-pickicng (ubuntu based test), slice-object aabb-metada for BVH-tree(or universal binary tree) and pointer to model, scene also binary tree 
shadows calculate 2time in main shader and inDepth shader, shadows realize in this time from frustum and have pixelize effect

<details>
  <summary>Code</summary>

```
enum OBJTYPES
{
    STATIC,
    DINAMIC,
    TERRAIN,
    SKELETALANIMATION,
    LIGHT,
};

struct PASSES{
    std::vector<Shader *> shader;
};

struct RenderObject{
    Object3D *ptr;
    glm::vec3 *pos;
    GLuint *VAO;
    OBJTYPES type;
    size_t size;
    GLuint *textureID;
    Picker *picker;//пока так
};
template<typename T>
struct SceneNode {
    T object;
    SceneNode* left;
    SceneNode* right;

    SceneNode(const T& obj) : object(obj), left(nullptr), right(nullptr) {}
};
template<typename T>
SceneNode<T>* insertNode(SceneNode<T>* root, const T& obj)
{
    if (root == nullptr)
        return new SceneNode<T>(obj);

    // Вставляем по какому-то критерию, например, по size
    if (obj.size < root->object.size)
        root->left = insertNode(root->left, obj);
    else
        root->right = insertNode(root->right, obj);

    return root;
}
template<typename T>
void renderTreePASS(const SceneNode<T>* root, Shader *shader,float dt)
{
    if (root == nullptr)
        return;

    // Обработка левого поддерева
    renderTreePASS(root->left, shader,dt);

    // Рендер текущего объекта
    const T& obj = root->object;

    if (obj.VAO && *obj.VAO)
        glBindVertexArray(*obj.VAO);
    else
        return; // или обработка ошибки

    if (obj.textureID && *obj.textureID)
        glBindTexture(GL_TEXTURE_2D, *obj.textureID);

    glm::mat4 model = glm::mat4(1.f);
    if (obj.type == OBJTYPES::TERRAIN && obj.pos) {
        model = glm::translate(glm::mat4(1.f), *obj.pos);
        shader->setInt("type", 0);
        shader->setVec3("objectColor", glm::vec3(0.4f, 0.8f, 0.4f));
        shader->setMat4("model", model);
        // Предполагается, что шейдер уже активен
        glDrawElements(GL_TRIANGLES, obj.size, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    } else if (obj.type == OBJTYPES::DINAMIC && obj.ptr) {
        glm::mat4 translate = glm::translate(glm::mat4(1.f), obj.ptr->position);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), obj.ptr->size);//тест размера бокса х2y x1.5xz для покрытия ray-picking
        model = translate * scaleMatrix;
        shader->setInt("type", 0);
        shader->setMat4("model", model);
        // Предполагается, что шейдер уже активен
        glDrawElements(GL_TRIANGLES, obj.size, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    } else if (obj.type == OBJTYPES::SKELETALANIMATION && obj.ptr){
        updateModel(&obj.ptr->creature, dt, obj.ptr->creature.frame);
        glm::mat4 rotationMatrix = glm::toMat4(obj.ptr->creature.orientation);
        glm::mat4 translate = glm::translate(glm::mat4(1.f), obj.ptr->creature.pos);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), obj.ptr->creature.sc);
        model = translate * rotationMatrix * scaleMatrix;
        shader->setInt("type", 1);
        if(obj.picker->selectedObject==obj.ptr){//можно и в выборе обработать просто перед выбором сбрасывать выбор/и в если промах сбрасывать выбор, ну у меня пока так
            shader->setVec3("objectColor", glm::vec3(0.8f, 0.4f, 0.4f));
        }
        else shader->setVec3("objectColor", glm::vec3(0.4f, 0.8f, 0.4f));
        shader->setMat4("model", model);
        drawModel(&obj.ptr->creature);
        glBindVertexArray(0);
    }
    // Обработка правого поддерева
    renderTreePASS(root->right, shader,dt);
}
```
</details>

left object moved from route system from center map from high-to lowland, right object started from diagonal and moving on lowland map only, for this test trigger light have right object

<img width="1854" height="1009" alt="image" src="https://github.com/user-attachments/assets/4896b1c7-6bef-4461-86c9-3c172694ecc6" />

</details>

</details>


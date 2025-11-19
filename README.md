# TestProject

<details>
  <summary>Links</summary>
<br>
Resources:

Animation:

animation based on this code github.com/hasinaxp/skeletal_animation-_assimp_opengl

Some Concepts

Noise Generation

youtube.com/watch?v=qChQrNWU9Xw

libnoise.sourceforge.net/noisegen/index.html

stuffwithstuff.com/robot-frog/3d/hills/hill.html

quatertnion

wikipedia.org/wiki/Quaternion

euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/transforms/index.html

danceswithcode.net/engineeringnotes/quaternions/quaternions.html

morphing in shader

youtube.com/watch?v=KEMZR3unWTE

base concepts

mbsoftworks.sk/tutorials/opengl4/

learnopengl.com

</details>
<br>

Lets Begin Immersive adventure

visual presentation core concept maybe like engine idk...

https://github.com/user-attachments/assets/0ae2109e-2f45-4eb8-947e-d2abffee0b8f


https://github.com/user-attachments/assets/31dccebf-bdf5-4cb0-8fd0-e84e7b2ad06a



https://github.com/user-attachments/assets/5287663e-255d-45e9-858b-78331faae6aa


https://github.com/user-attachments/assets/8346d4c7-0747-45cf-b559-0f26619e585d

current Project Pipeline we have terrain-navigation system, 2xTree - for render PASS(scene and sceneUI), 2xTree - bvh for collision object in world, and tree for collision 2D - ui, particle engine on G-shader via transform feedback

<img width="3294" height="576" alt="image" src="https://github.com/user-attachments/assets/21f1e8c0-f562-46ad-9f31-0f1c8d3c4162" />





<br>

fix - orientation and set simple AI, but need fixes global orient, so yes in this time it global bug [01]

now AnimationsRefactor - basic waveGenerator :) typicaly have some bugs on neworientations but nice 

<details>
  <summary>Stress test 10000</summary>
<br>
<img width="793" height="621" alt="image" src="https://github.com/user-attachments/assets/0d1ec439-953e-4342-9372-ea0726a852af" />

after [01]

<img width="804" height="633" alt="testProgram1" src="https://github.com/user-attachments/assets/50f3da09-ff53-4ebd-bf0a-00f9d912fa62" />

</details>


<details>
  <summary>Progress</summary>
<br>
Detail current status
<br>
<details>
  <summary>Gentoo based tests</summary>
<br>
2000 000 particles (update base system with particle system)

<img width="1917" height="1016" alt="image" src="https://github.com/user-attachments/assets/647b39e1-d19e-4b74-bbc7-3cdd508df2a4" />

<img width="1920" height="1018" alt="image" src="https://github.com/user-attachments/assets/0a70d030-80a8-4c5c-beb9-65ad2c395aea" />



Tracking Motion (From point A to point B on the Route), double light(main scene, main model) with shadows, with npc-orientation! its just work :)
notice on this chapter:

<details>
  <summary>Code</summary>
<br>
  
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
<br>
Ray-pickicng (ubuntu based test), slice-object aabb-metada for BVH-tree(or universal binary tree) and pointer to model, scene also binary tree 
shadows calculate 2time in main shader and inDepth shader, shadows realize in this time from frustum and have pixelize effect

left object moved from route system from center map from high-to lowland, right object started from diagonal and moving on lowland map only, for this test trigger light have right object

<img width="1854" height="1009" alt="image" src="https://github.com/user-attachments/assets/4896b1c7-6bef-4461-86c9-3c172694ecc6" />

</details>

</details>


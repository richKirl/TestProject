# TestProject

fix - orientation and set simple AI, but need fixes global orient, so yes in this time it global bug [01]

now AnimationsRefactor - basic waveGenerator :) typicaly have some bugs on neworientations but nice 



stress test 10000 :)
<img width="793" height="621" alt="image" src="https://github.com/user-attachments/assets/0d1ec439-953e-4342-9372-ea0726a852af" />

after [01]

<img width="804" height="633" alt="testProgram1" src="https://github.com/user-attachments/assets/50f3da09-ff53-4ebd-bf0a-00f9d912fa62" />



Progress

gentoo based test

Tracking Motion (From point A to point B on the Route), double light(main scene, main model) with shadows, with npc-orientation! its just work :)
notice on this chapter:
```
from a to b Route(but route have points-steps local position from-to) need know orientation on local points
glm::quat temp =  glm::quatLookAt(dirr, glm::vec3(0,1,0));
obj.q = glm::slerp(obj.q,temp,0.1f);
1 step of Route (subsystem of routing on the mesh - navigation system - A* or dejkstra )
```
Yellow backcolor for debug space npc(silhouette)
<img width="1918" height="1018" alt="image" src="https://github.com/user-attachments/assets/75226699-4250-4eb1-9f54-27f322f83ac5" />

Ray-pickicng (ubuntu based test), slice-object container aabb<bvh>(for cube in this time sizex2) and pointer to model

<img width="1854" height="1009" alt="image" src="https://github.com/user-attachments/assets/4896b1c7-6bef-4461-86c9-3c172694ecc6" />


<img width="1853" height="1012" alt="image" src="https://github.com/user-attachments/assets/a912130e-dd2d-455f-820c-c0a2a3e2e4d7" />

another test (just a test without integration to system Engine, this solution used the library assimp, now its only pre-integration test, because need testing some ideas)

<img width="798" height="595" alt="image" src="https://github.com/user-attachments/assets/68e99096-ea18-4148-a947-32b6750eddfb" />

<img width="793" height="621" alt="image" src="https://github.com/user-attachments/assets/8b3a210d-328f-4a6e-a354-d8aaa4a40942" />

stress test with optimisation with ASSIMP loader - 400 skeletal models with self animation and status update all memory 430 mb :) no broke, no load, fast work - unbelievable
<img width="787" height="620" alt="image" src="https://github.com/user-attachments/assets/2a97ee08-a668-4d84-8560-0ca5494c447a" />

1000 models

<img width="796" height="634" alt="image" src="https://github.com/user-attachments/assets/fa5695e6-a25a-478c-8f78-45d6d32934d3" />

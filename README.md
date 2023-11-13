# CG2023

## OverView

基于 OpenGL 实现一个“流浪地球”主题的 3D 模拟游戏

## Todo List

设计有三种视野移动的方式：

- 上帝视野，可以自由进行切换，在移动近远景临界区时会进行场景的切换
- Car 视野：在近场景下可以选择汽车视角，通过对汽车的驾驶来改变视野
- Aircraft 视野：在远场景下可以选择飞行器视野，通过控制飞行器来改变视野

### Stage 1

完成基本设定的基本模型的载入

- [x] car: 贴图有些丑陋，如果找到更好的可以换一个新的 model
- [ ] Ground: 近景的地面场景的搭建, 山/水面/雪地/草地 ...?
- [ ] Tree: 在 Ground 中添加一些树的元素
- [ ] Elevator: 搭建一个太空电梯的场景
- [ ] Planetary Power Station
- [ ] Galaxy: Sun Earth Moon Jupiter 基本的星系
- [ ] Aircraft (飞行器)
- [ ] SpaceStation(空间站)

### Stage 2

将基本模型组成整个大的场景，增加基本的功能等

- [ ] 增加星球之间的光照条件，在基本的贴图上让最终呈现的效果更酷炫
- [ ] 雪景实现
- [ ] 增加月球爆炸的效果
- [ ] 增加地球移动时行星发动器的推进的效果
- [ ] 木星引力弹弓的轨道计算
- [ ] 碰撞检测实现：近场景下车辆的移动，远场景下飞行器与星球的碰撞检测
- [ ] 根据视野的变化自动进行近远景的切换
- [ ] ...



## Fundamental Element

### near

- Ground
- Elevator(太空电梯)
- Tree
- 行星发动机
- ...

### far

- Galaxy: Sun Earth Moon Jupiter
- Aircraft (飞行器)
- SpaceStation(空间站)
- ...

## Code specifications

将场景中的基本元素封装成一个 **class**, 在 mainLoop 中直接对封装好的类进行调用

```c++
class Car {

public:
    Model carModel;
    Shader carShader;


    Car(): carModel("truck/TruckM.obj"), carShader("shaders/nano.vs", "shaders/nano.fs") {
        ...
    }

    void Draw() {
        ...
    }


};
```

在开发过程中通过 flag 的设置来切换远场景和近场景，便于两个场景的分别开发和调试：

```c++
bool flag = true;
    
    while (!glfwWindowShouldClose(window))
    {
        
        ...
          
        if(flag) {
            // close shot
            car.Draw();
        } else {
            // near shot
        }
        
       
      ...
    }
```


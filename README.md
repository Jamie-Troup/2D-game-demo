# 2D-game-demo

![Screenshot from 2024-01-26 00-10-46](https://github.com/Jamie-Troup/2D-game-demo/assets/150353016/4dd45f8f-663e-42aa-9ce5-98eb184114c7)

__Demo video__ -> https://jamie-troup.github.io/2D-game-demo/

A project based off Dave Churchill's 4th year computer science course in C++ programming.

# Concepts it introduces

- ECS design pattern -> really interesting alternative to OO design that prevents inhertiance complications down the line and gives the programmer/designer a lot of freedom (e.g. by how easy it is to add/remove components). Made my own entity manager class which followed the factory pattern

- RAII -> Encapsulates every resource in a class, tying the lifetime of the class to the life of the resource. Thus initialising the class allocates the resource, and you can use it's destructor to handle memory management (if needed). This project uses shared_ptrs to handle heap management

- Dear ImGui -> Has an actual useful GUI

- Make files -> My previous C/C++ projects had never been larger than a main.cpp or included any large 3rd party libraries, so for this one I was introduced to make files. I am by no means an expert, it certainly led to a few head scratches, yet I am a lot more comfortable with this C++ project management/compilation technique now

- Const correctness -> No expert, but am getting better

- Operator overloading, Templates/generics, More SFML

# Things to add (apart from the TODOs in code)

- Scene mangager

- Audio & Textures

- Animation
  
# If you want to try it

To build it, you need to install SFML version 2.5+ and Dear ImGui to your system (and make if you dont already have it) and then use the makefile provided. May need adjustments, was coded on Debian and worked with g++ there.

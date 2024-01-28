# 2D-game-demo

![Screenshot from 2024-01-26 00-10-46](https://github.com/Jamie-Troup/2D-game-demo/assets/150353016/4dd45f8f-663e-42aa-9ce5-98eb184114c7)

Demo video -> https://jamie-troup.github.io/2D-game-demo/

This is a cover of assignment 2 from Dave Churchills 4th year computer science course in gameplay programming. As it is self taught, I did not have access to the substantial skeleton code he provides his university students and so made the entire project from scratch (lots of freeze framing on his videos to type out sections of his skeleton code). Overall I liked this, I believe it gave me a deeper understanding of C++, for example by making me research topics such as the compilation process (preprocessing, compilation, linking) and the proper use of header files. However, it did add substantial time to the length of the project and so in the interest of getting a job the code is not as neat/efficient as I would like.

# Concepts it introduces

- ECS design pattern -> really interesting alternative to OO design that prevents inhertiance complications down the line and gives the programmer/designer a lot of freedom (e.g. by how easy it is to add/remove components). Made my own entity manager class which followed the factory pattern.

- RAII -> still a bit shaky on this, in this instance I took it to mean encapsulating every resource used within a class, thus the lifetime of each resource is bound to the lifetime of the object that contains it. Every resource class' destructor will be used to free the resource that is allocated upon resource initialisation. In this project, we used shared pointers for this, which keep a count of how many different scopes have access to them and destruct when it reaches 0.

- More Dear ImGui -> I completed assignment 1, which introduced this, however assignment 2 brings in tables, tabs and requires a deeper understanding of dear imgui. Its a powerful tool...

- More SFML -> a 2D openGL wrapper in C++, prevents the programmer from having to repetively write openGL functions and provides useful classes to get a 2D project up and running quickly.

- Make files -> My previous C/C++ projects had never been larger than a main.cpp or included any large 3rd party libraries, so for this one I was introduced to make files. I am by no means an expert, it certainly led to a few head scratches, yet I am a lot more comfortable with this C++ project management/compilation technique now.

- Collisions -> This uses the simplest collision detection technique (circle-circle). The course goes on to introduce more accurate techniques.

- Iterator invalidation -> came across this concept. It reared its head when deleting/adding entities and was resolved by using a one frame delay in adding entities and an extra "toDelete" loop. 

- Operator overloading -> another cool feature I played with during this project. Very fun to defining [] on a homemade array class, both for indexing and setting values of the array.

- Templates/generics -> This project implemented its own Vec2 class, which through templates can create a vector of any datatype.

- Const correctness -> Still by no means an expert but a lot more comfortable to use const in the right places (whilst knowing what it means).

# Things to improve on

- Audio & Textures -> As I said earlier, this project was made following a university course which introduces game audio and textures in assignment 3. Looking forward to it!

- Code neatness -> Due to time constraints I would like at some point to go over this code base and fix the no doubt numerous memory leaks and useless if statements. At some point...

- Game understanding -> by not having access to the official skeleton code of the project I ended up altering the order of some of the systems in the game loop. I am still not 100% sure this didn't introduce logical inconsistencies in the game.

# Notes

To build it, you need to install SFML version 2.5+ and Dear ImGui to your system (and make if you dont already have it) and then use the makefile provided. May need adjustments, was coded on Debian and worked with g++ there.

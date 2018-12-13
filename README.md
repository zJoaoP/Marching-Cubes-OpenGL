# Marching-Cubes-OpenGL
Implementation of the Marching-Cubes Algorithm with OpenGL using the C language.

# About the Code
The objective of the code is to receive a set of points, normalize their values (between 0 and 1) and generate a surface
which represents these points. The level of detail of this surface is given by a value L, which corresponds to the size of the Marching Cubes. However, greatly reducing the value of L can cause holes in the mesh.
For this case, the best values are between [0.02, 0.03].

As it is an academic work (For the discipline of Computer Graphics by the Federal University of Bahia),
there were very strict specifications regarding the organization of the source code. Because of this, unfortunately it was not possible
organize and document in a more user friendly way.

# Run, Forrest! Run!
<ul>
<li> gcc main.c -lm -lGL -lGLU -lglut -o YourFileName </ li>
<li> ./YourFileName PointCloudFile lut.txt L </ li>
</ ul>

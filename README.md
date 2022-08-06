# Encryption and compression algorithms in compiled binaries
The main goal of this repository is to document my journey through learning how to recognize some of the most popular algorithms in assembly (x64) from C++ compiled binaries.

## Roadmap
- [X] RC4
- [X] AES
- [ ] RSA
- [ ] Salsa20
- [ ] 3DES
- [ ] Deflate
- [ ] LZMA

## Why not compile this in C?
Since the C++ compiler usually adds a level of abstraction to the main function, given by the VC runtime libraries, like init routines for exception handling, env variables, etc..., it makes the binary more realistic since good coding practices suggest the use of runtime libraries.
It will also give you a feel for how the C++ compiler works under the hood, how it decides which call standard is the best, when to inline, etc...

## Structure of the repository
Each algorithm will have its own folder with its own project. 
The algorithms will be made in C just in case someone wants to compile them with a C compiler to see the differences with a C++ program.  
Each folder will contain a Readme regarding my personal research on the specific algorithm, along with project files. 

## Updates
Most famous algorithms will probably be added with time, if you have any requests for a new algorithm please open an issue to let me know.

# Info
Compilation was done using Visual Studio 2022, but results shouldn't change much as long as one's using a modern c++ compiler.  
Debugging was done using x64dbg, without .pdb files to make the research more meaningful.  
Any decompilation seen is done using Hex-Rays decompiler from IDA Pro.  

## Coming in the future
Hashing algorithms!

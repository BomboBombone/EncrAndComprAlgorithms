# Encryption and compression algorithms in compiled binaries
The main goal of this repository is to document my journey through learning how to recognize some of the most popular algorithms in assembly (x64) from C++ compiled binaries, since the C++ compiler usually adds a level of difficulty for anyone that wants to reverse engineer the binary, given usually by the VC runtime libraries, that could eventually (if one really wanted) be stripped out, but the excersise is meant to be realistic, and therefore a bit harder.

## Structure of the repository
Each algorithm will have its own folder with its own project. 
The algorithms will be made in C just in case someone wants to compile them with a C compiler to see the differences with a C++ program.  
Each folder will contain a Readme regarding my personal research on the specific algorithm, along with project files. 

## Updates
Most famous algorithms will probably be added with time, if you have any requests for a new algorithm please open an issue to let me know.

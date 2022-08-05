# Finding main
First of all, one can't do much but notice that there's a lot of seemingly junk code, which is in reality added by the C++ compiler.
Such code is composed of basically all the initialization functions, for things such as calling global object constructors, getting env variables, etc.

![image](https://user-images.githubusercontent.com/88135556/182349212-288c8726-5034-4c0d-9968-d087d20dd503.png)

As one can see from the screenshot above, it's relatively easy to spot the real main function of the program, it's usually the first call instruction after the program
gets argc and argv, in figure it's represented as the breakpointed instruction.

# Analysing main function
![image](https://user-images.githubusercontent.com/88135556/182354661-1dbaa61b-736a-46d0-9825-b7852bd98ee8.png)
Zones marked as 1 are (obviously) malloc calls, whilst the ones marked with 2 are std::cout calls (you can see that from the first argument passed being a basic_osstream.

Now we can start making a comparison with our source code:  

![image](https://user-images.githubusercontent.com/88135556/182355173-0bd8ea43-c945-49f0-9f35-c613096307cd.png)

We can already identify a couple areas of interest, in particular the mallocs and std::cout.
This can help us find the EncryptDecrypt call, as well the algorithm object!

We know that C/C++ compilers will use the __cdecl or fastcall conventions by default (for Windows at least), therefore we can derive the function arguments by having a couple concepts in mind:
1. Fastcall will probably used if the number of arguments is small, due to the convenience of not having to store data on the stack
2. If the program is register starved it may need to store local copies of some on the stack, therefore leaving a trail of push and pop instructions (only if using fastcall).
3. If instead the program uses the cdecl convention, all arguments will be passed on the stack, from right to left

![image](https://user-images.githubusercontent.com/88135556/182359009-95c8543a-8857-488f-9a0c-25208325bbde.png)

This function is of particular interest because of its placement and number of args.

![image](https://user-images.githubusercontent.com/88135556/182359883-3e44b848-9a5b-452e-b617-f38ba1ba687e.png)

Analysing it a bit further reveals (as expected) that malloc returns its value on rax. 
The first malloc's result, which is probably the buffer for the encrypted mes, is transferred to r8, which seems to be the first register of choice for fastcalls,
at least when the number of args > 2 and therefore rcx and rdx are not enough.

![image](https://user-images.githubusercontent.com/88135556/182372838-36e317cf-6374-449d-afb9-98bcd83a3f7f.png)

There are 4 arguments, and the convention is fastcall (x64).
The first argument should be a pointer to the algorithm object on the stack, and is passed in rcx. 
Now that we know every function's name, we can finally study how they work internally.

First of all I wanted to know when my algorithm object would have been initialized since I didn't see any call or jumps (at least to other routines).
As it turns out, the compiler decided to inline my RC4Factory function, probably due to the fact that the function would make a useless copy on return,
but since both the local variable and the returned value are on the stack, it makes sense to make them the same variable and inline the whole thing.

This is the routine equivalent to RC4Factory:

![image](https://user-images.githubusercontent.com/88135556/182440192-6aab08a8-433d-4443-b455-be2c09ed5129.png)

Then there was another thing that really caught my attention:

![image](https://user-images.githubusercontent.com/88135556/182440447-afaf32e4-897c-42c9-bcb1-35dd1176032c.png)

This routine here didn't make any sense to me at first, since it used floating point registers to do some weird loop.
On further inspection it's rather easy to see that the reason for xmm0 and xmm1 being used is that they are really just big af, and the routine just needs to
copy a piece of stack (rdx) into another location near it (rax), not really sure why, maybe to make space for something else.

Now that these 2 pieces are solved we can finally go on and reverse the EncryptDecrypt function!
Before diving in the disassembly I keep in mind that the KSA function does some initialization for RC4, in particular it initializes a fixed size (256) array, the SBox.
Therefore I should be able to identify this specific function just from this 256 bytes array being first initialized with numbers 0 to 255, since such
array will need to be initialized every time when performing encryption or decryption.

These are the arguments being passed:  

![image](https://user-images.githubusercontent.com/88135556/182442144-8ce11732-fde6-4c57-95e9-7ba20c0279b3.png)

It seems really obvious that second and fourth args are connected, in particular second is our message, and fourth is meslen!
Therefore the other 2 must be the algorithm object and the recently allocated buffer for the encrypted message. 
Which one is which can be easily deduced by checking where the result of the first malloc was put, in this case the 3rd argument.

Now we can finally make an assumption on the function prototype being like:
EncryptDecrypt(RC4* algorithm, char* mes, char* ciph, int sz)

Which is in fact really similar to the source code!

Now the IMPORTANT part, also known as "How do we know this is RC4 without looking at source code?".
It's really easy to answer such question:
1. Look for a routine initializing the SBox
2. Look for the usage of a single xor just before the end of the loop

By pressing G inside x64dbg we can access the graph mode, which will help us a lot recognizing the algorithm, just look at this!

Easily spotted SBox initialization routine:

![image](https://user-images.githubusercontent.com/88135556/182443469-b983521a-891e-4a8b-af40-b77e7984a763.png)

And here is the ending xor before looping:

![image](https://user-images.githubusercontent.com/88135556/182443752-ac529cd3-c26b-4bfd-800c-5fd702482a0d.png)

## Key takeaways
1. Be aware of the C++ runtime code when reversing
2. If unsure about some routine take your time to analyse it (unless the binary is obfuscated since the routine may be junk code)
3. To recognize RC4 look for the encryption function by keeping track of your original message
4. When such function has been found, look for the SBox init routine and the xor at the end of the loop to confirm it is RC4
5. Enjoy!

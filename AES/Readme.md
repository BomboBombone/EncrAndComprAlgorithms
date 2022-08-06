# A small intro to AES (since it's a lot more complicated than RC4)
I won't go deep into the details of the implementation since it's a bit complicated (and even I cannot fully understand the math behind it).
What really interests us are the reverse engineering details to recognize the algorithm!

AES stands for Advanced Encryption Standard, and as other algorithms also do, it usually operates in different "modes",
which use the underlying AES algorithm and applies it differently to tackle different issues, like:
1. Timing attacks
2. Patterns in the algorithm

Be aware that other attack vectors, like cache attacks, exist, therefore custom implementations may be required depending on
your usage of the algorithm. If you need something simple you can copy paste my source.

Also keep in mind that this is a block cipher, which usually needs padding in the data to make the length a multiple of
BLOCK_SIZE.

Check out the [different five modes](https://www.highgo.ca/2019/08/08/the-difference-in-five-modes-in-the-aes-encryption-algorithm/) of AES.

## Key elements
There exists a constant key that is used for encryption/decryption, and you may want to find that!
The algorithm will usually have (may be a variant of) one of the five modes of operation described above.
The algorithm uses a couple constants, like substitution box, which can be scanned for!
(Side note) This is the last time any source references will be used, therefore IDA Pro will be used for the next algorithm.

# Let's start reversing!
Since I suppose you followed the first steps into reversing RC4, it'll be a breeze for you to find all the test
function calls!

In case you did not, don't worry, here's the TEST_ALL function call:

![image](https://user-images.githubusercontent.com/88135556/183219090-bd3db1d2-2b74-4786-94a9-ed40f55045c3.png)

Taking a small peek inside the function, we can already see how the compiler did some inlining:

![image](https://user-images.githubusercontent.com/88135556/183219424-620494e4-6aad-4cbd-9b8d-4d01335651a1.png)

The 2 sections in red translate well into each other, and it looks like the AESFactory call has been inline once again,
even though this time I created a pointer inside the heap instead of the stack like in RC4.
This basically shows us that the C++ compiler (MSVC at least) seems to inline functions that are short, and 
perhaps with a small amount of arguments.

From now on, to make this experience more meaningful, I won't be using source code for direct reference,
but I will use general concepts about any AES implementation as references.

From simple debugging we can see some output on the console, which lets us know the encryption must be happening
before the call to std::cout

![image](https://user-images.githubusercontent.com/88135556/183221241-da8d272e-2d5f-4409-8f45-d838afce892e.png)

*Side note*
In a real program you'd look for some clue on where the encryption starts,
like a console print, or a specific function call, and where it ends, so that you basically have a fixed number of instructions to dissect.

Let's dive deeper!
As we can see the last unnamed function is just a free() wrapper.

![image](https://user-images.githubusercontent.com/88135556/183245395-9f61bed7-c1a8-428f-94db-137a4e9639aa.png)

Instead, the first unnamed function already looks very interesting from a first glance.

![image](https://user-images.githubusercontent.com/88135556/183245569-569d3373-fc24-4e71-9191-d24b2ef93c5f.png)

It has some for loops, shifting and xoring, all operations required for the encryption process!

Looks like we found gold, but how do we make sure?

We should keep in mind that the algorithm uses some constants, which you could theoretically look for statically,
but if the program's creators wanted to make it a bit more difficult to guess the algorithm used, then
the sbox and other constant arrays will be allocated dynamically on the stack (which is not really the case here,
but I'm trying to cover a decent amount of possible cases).

That's why here simply doing a scan on the binary would've revealed that this is, in fact, AES, by simply
doing a pattern scan for the first 5-10 bytes of the sbox.

*We will assume in this case that the sbox has been allocated dynamically at some point during program execution and therefore
it could not be found using the method described above.*

Pressing G we can toggle graph mode, and immediatly see the function's control flow.
In particular this loop right here:

![image](https://user-images.githubusercontent.com/88135556/183248246-5bd0d69c-0f4e-42d1-81c7-187b5fd91245.png)

Looks like an initialization function for an array.
This is the memory pointed by RDI at the end of the loop:

![image](https://user-images.githubusercontent.com/88135556/183249119-9eb2a832-bc8e-42bd-bca8-c431464e6732.png)

16 bytes are copied from [rsi+rcx] to [rcx].
Just looking at the value we could be tempted to deduce this is some kind of array initialization function like:

```
for(unsigned char i = 0; i < 16; i++){
	out_array[i] = i;
}
```

But such code would roughly translate to this:

```
xor eax, eax

loop:
lea rcx, qword ptr ds:[out_array + eax]
mov [rcx], eax
inc eax
cmp eax, 0x11
jl loop
```

Hopefully you can see how the 2 routines are quite different from each other now.
Now let's see what else this routine has got for us!

![image](https://user-images.githubusercontent.com/88135556/183249654-4bc672a9-99da-4a8d-b264-9cda9d0ee838.png)

It's now evident rdi holds a pointer to some structure important to the routine,
since its members are being accessed quite a lot now.
We will call this *struct A*.

Values in the struct at this point:

![image](https://user-images.githubusercontent.com/88135556/183249696-3a5489e7-d30d-463a-a8c8-b3166eb81cf8.png)

Now stuff gets a bit complicated. 
At this point we hypothesize we're not sure which algorithm this is, and we're trying to figure it out.
Here we have a quite big routine:

![image](https://user-images.githubusercontent.com/88135556/183255863-c6f1e858-eb01-4eaf-9d3e-56e29c4869f0.png)

It seems to use the array copied before to do some stuff, and it basically fills an array with such data:

![image](https://user-images.githubusercontent.com/88135556/183255936-0661424d-07e4-47cd-b96f-d24243a97212.png)

We can see here that the array being filled is adjacent to the one copied before.
That's basically all the function does, and it does it by taking single bytes, and combining them in
words (4 bytes) of data, which get manipulated somehow.

After the function call this is what we're left with: 

![image](https://user-images.githubusercontent.com/88135556/183256084-f4409f80-abf4-4d17-9844-c814d3a4447f.png)

A big messy array that doesn't seem to have any special meaning... unless?
Unless it does!
We know that encryption must occur at some point now, and this is not it because our message was never accessed, therefore it must be
some initialization function for the algorithm being used.

This is actually a common pattern in a lot of ciphers, being them block or stream ones (RC4 has 
for instance to initialize an array based on the key, which then is used as a substitution box).

The only option remaining is that this is in fact the Encryption function:

![image](https://user-images.githubusercontent.com/88135556/183256335-e972802c-6646-4427-a2b9-2ea874c1e0d0.png)

If it really is, we should be able to find somewhere in here the S-Box being read from.
But wait, didn't you say there are multiple operation modes?
Yes, I did, and I'm very proud of you for remembering about it!

It's actually decently important since most ciphers have "suggested modes of operation"
(suggested by the NIST) and therefore it's extremely probable that this is just a wrapper for the specific 
mode of operation, which then inside contains the actual encryption function call.

*Side note:*
This is actually pretty nice because if one wanted to go as far as recognizing the mode of operation for whatever reason,
we could actually do it by analyzing how the wrapper is structured and how it uses the encrypted data
(refer back to the link on the 5 modes of operation).

If you have any doubts regarding the implementation check out [how to implement AES](https://blog.nindalf.com/posts/implementing-aes/).

Back to reversing now.
Inside the function we can find the highligthed loop being the first instructions to (apparently) access our plaintext message.
This should be enough confirmation that this is, in fact, the main encryption function.

The function continues as such:

![image](https://user-images.githubusercontent.com/88135556/183257866-6ee3bac0-317f-410a-9502-f9feba856d4e.png)

The highligthed text is the array generated in memory, and it's a permutation of our plaintext:

![image](https://user-images.githubusercontent.com/88135556/183257935-54455e65-5d24-4876-91a8-b10fa1e6c274.png)

The highligthed function here does another permutation on the same array:

![image](https://user-images.githubusercontent.com/88135556/183258130-7c8a6d70-dc74-4bfb-bbcd-b180fb9fddd4.png)

![image](https://user-images.githubusercontent.com/88135556/183258144-c4d392a9-065c-47a1-9139-3957803bc562.png)

Here we have the first couple instructions of the routine:

![image](https://user-images.githubusercontent.com/88135556/183260668-0382e957-381e-44c5-b4df-0228e8dbf307.png)

and by inspecting briefly the code and variables being accessed, we can see that 

![image](https://user-images.githubusercontent.com/88135556/183260928-e1d500ea-6335-493f-b41d-e2d9c779d79a.png)

this is the global variable being accessed on instruction 4:
```
lea rdi, qword ptr ds:[7FF718674490]
```

and it's the infamous AES sbox, therefore confirming the hypothesis.

## Exercise 1 (Easy)
If you followed through the paper with attention you should now be able to find the key expansion function pretty easily,
and therefore you should be able to find the encryption key.

## Exercise 2 (Very hard)
Keep on reversing and find out the mode of execution used in each function.

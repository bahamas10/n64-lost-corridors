Lost Corridors
==============

A C implementation of the Recursive Backtracking Maze Generation
Algorithm designed specifically for the N64 using [libdragon](https://libdragon.dev)

![intro](/screenshots/intro.png)

![maze](/screenshots/maze.png)

How To
-------

JUST GIVE ME THE z64 ROM FILE:

http://files.daveeddy.com/n64/roms/lost-corridors.z64

---

If you want to compile the rom file you can do:

```
$ ./compile
    [CC] src/main.c
/n64_toolchain/bin/mips64-elf-gcc -c -MMD      -march=vr4300 -mtune=vr4300 -I/n64_toolchain/mips64-elf/include -falign-functions=32    -ffunction-sections -fdata-sections -g -ffile-prefix-map="/libdragon"= -DN64 -O2 -Wall -Werror -Wno-error=deprecated-declarations -fdiagnostics-color=always -std=gnu99 -o build/main.o src/main.c
    [CC] src/maze.c
/n64_toolchain/bin/mips64-elf-gcc -c -MMD      -march=vr4300 -mtune=vr4300 -I/n64_toolchain/mips64-elf/include -falign-functions=32    -ffunction-sections -fdata-sections -g -ffile-prefix-map="/libdragon"= -DN64 -O2 -Wall -Werror -Wno-error=deprecated-declarations -fdiagnostics-color=always -std=gnu99 -o build/maze.o src/maze.c
    [CC] src/ryb2rgb.c

... snipped ...
$ open lost-corridors.z64
```

Check out the song here:

https://www.youtube.com/watch?v=RCjLs9koZQg

Algorithm
---------

More Info: https://weblog.jamisbuck.org/2010/12/27/maze-generation-recursive-backtracking

License
-------

MIT License

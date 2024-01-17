Lost Corridors
==============

A C implementation of the Recursive Backtracking Maze Generation
Algorithm designed specifically for the N64 using [libdragon](https://libdragon.dev)

![intro](/screenshots/intro.png)


How To
-------

**JUST GIVE ME THE z64 ROM FILE**:

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

This will create `lost-corridors.z64` which can be used in an N64 emulator or
flash cart.

Controls
--------

In the menu

- `start` - start the maze illustration

In the maze

- `A` - restart the maze
- `B` - go back to the main menu
- `C-up` - speed up
- `C-down` - speed down
- `C-left` - disable randomized colors
- `C-right` - enable and randomize colors

More Info
---------

My JavaScript Implementation: https://bahamas10.github.io/lost-corridors/

My song "Lost Corridors": https://www.youtube.com/watch?v=RCjLs9koZQg

The Algorithm: https://weblog.jamisbuck.org/2010/12/27/maze-generation-recursive-backtracking

Screenshots
-----------

Maze in progress with static color

![maze-start-no-color](/screenshots/maze-start-no-color.png)

Maze almost done with randomized colors

![maze-middle-color](/screenshots/maze-middle-color.png)

Maze finished with randomized colors

![maze-end-color](/screenshots/maze-end-color.png)

License
-------

MIT License

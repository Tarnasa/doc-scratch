# Chess C++ Client

This is the root of you AI. Stay out of the joueur/ folder, it does most of the heavy lifting to play on our game servers. Your AI, and the game objects it manipulates are all in `games/chess/`, with your very own AI living in `games/chess/ai.h` and `games/chess/ai.cpp` files for you to make smarter.

## How to Run

This client has been tested and confirmed to work on the Campus rc##xcs213 Linux machines, but it can work on your own Windows/Linux/Mac machines if you desire.

Also make sure **NOT** to try to compile this in your Missouri S&T S-Drive. This is not a fault with the client, but rather the school's S-Drive implimentation changing some file permissions during run time. We cannot control this. Instead, we recommend cloning your repo outside the S-Drive and use an SCP program like [WinSCP](https://winscp.net/eng/download.php) to edit the files in Windows using whatever IDE you want if you want to code in Windows, but compile in Linux.

### Linux

```
make
./testRun MyOwnGameSession
```

If you are on your own machine, make sure you've installed boost. The `libboost-all-dev` package should be up to date. You'll also need the `cmake` package for make to work.

### Windows

Be aware that getting this C++ client to build on Windows, while possible, can be a headache.

For Windows, Boost has a simple way to [compile from source using bootstrap](http://www.boost.org/doc/libs/1_58_0/more/getting_started/windows.html). You'll need to do that. This client does work with VC++, you can create a solution at the root or ask a dev for the sln file. Just add the directory you built Boost in the Project's linker configuration. You'll also need to set command line arguments like the other clients.

## Other notes

The initial `make` step may take upwards of 2 minutes. You should see a percent progress updating on your screen, but it will be slow. Subsequent `make`s should be only a few seconds if you don't `make clean`.

## Additions

The files starting with "Skaia" in the games/chess directory are the core of the chess library.
The Skaia.h file contains the Position data structure, and functions to convert from Skaia's notation into the notation that SIG-Game's framework expects.
The SkaiaAction.h file contains a structure which represents an action.
The SkaiaPiece.h file contains a structure which represents a piece on the board.
The SkaiaState.h file contains a structure and a buttload of functions for manipulating a state of the game.
The SkaiaState_internal.cpp contains definitions for functions which aren't too interesting.
The SkaiaState.cpp contains definitions for funcitons that do alot of wacky stuff.

Things to note:
The moves that a piece can make are actually a member of that piece, that way, when they need to be updated, only that piece's moves need be changed.
Every square on the board contains a bitset which tells which other pieces can attack that square.

These two caches add alot of complexity to the code, I'm sorry.


# MK - F417XX3

My [Ludum Dare #34](http://ludumdare.com/compo/) entry.


## Dependencies

The game depends on [SDL2](https://www.libsdl.org/),
[GFraMe](https://github.com/SirGFM/GFraMe) and
[c_synth](https://github.com/SirGFM/c_synth).

SDL2 is available on Ubuntu through the package manager. Just run the following,
to install it:

```
$ sudo apt-get install libsdl2-2.0.0-dev
```

The others libs must be cloned/downloaded and manually built. Check the README
on each lib repository (though it's usually just runnig '$ sudo make install').


## Compiling

Run:

```
$ make RELEASE=yes
$ cp ./bin/Linux/game .
```

Before running the game, download the missing sound effects from the TAG 1.0.0!


## Controls

'F', 'Left shoulder button' - Move left leg

'J', 'Right shoulder button' - Move right leg


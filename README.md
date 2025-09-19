# VIMS3D
Very Impressive Mesh System (3D renderer for fx-9860G series)

# Building
Build this program/game engine using the official fx-9860G SDK (gint port is in the making).

# Usage
The software contains a small demo, some assets (a tank mesh, sample textures, sample code).
Debug info available by pressing `F3`.
Look around with the arrow keys, move with 8, 4, 6, and 2, float up/down with 3 and 9.
Summon the Big Animated Marker Arrow with `VARS`

![tank_in_forest_demo](https://github.com/user-attachments/assets/35169bff-45cb-464e-b886-e05219bbd029)

Tank in forest demo

# Software
Some code is self-documenting, I think.

Main things to know:
Define and add world objects in `init()` in `game.c`.
Manipulate them in their respective tick functions in `worldobj.c`.
Define textures in `texturemap.c`.
All of these files have some example code.

The engine is kind of limited, mostly due to memory constraints.
The number of objects at once is limited by the size of the heap (48kbytes, of which 16kbytes are for the depth buffer)
Also the engine is kinda buggy, there are some rendering errors, this is still in alpha :)

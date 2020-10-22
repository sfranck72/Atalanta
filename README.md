# Atalanta
MSX game by sfranck72

From a BASIC listing in a spanish book (NEUTRONIA game).  
Adapted to Fusion-C 1.2  
Need a *MSX2* minimum (SCREEN 5).

2 dsk versions : 
  - Française.
  - English.

How to play :
Insert the ATALAN_E.dsk (ATALAN_F.dsk for French version) in a MSX emulator drive.  
There's a autoexec that launch the game.  
Manually, you can launch the game by typing ATALANTA and <entry> in DOS mode.  

I inserted some goodies in the disk :
- Launch BLOAD in DOS mode (for a pixelart picture).
- Launch PALETTE in DOS mode to modify a palette file in PL5 format to a Fusion format palette (DEV tool).

Code : 
Open the "main.c" file to see my nasty Fusion code (my first program in C).  
When I compile via the "compil.bat" it update the "dsk" directory.  
If you want to compil the french version, you must change the name : main-french.c > main.c  
Note :The compil mode in Fusion-C need the "fusion-c" library wich is a directory near the "dsk" directory (not provided in this repo).  

Long life MSX.  
sfranck72...oct 2020...somewhere in France.

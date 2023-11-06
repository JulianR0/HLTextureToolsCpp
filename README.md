# Half-Life Texture Tools++
This is a port of [yuraj11's Half-Life Texture Tools](https://github.com/yuraj11/HL-Texture-Tools) to the C++ language.

The original program is bundled within .NET Framework and C#, which binds it to only the Windows ecosystem. While Linux users can use WINE to run Windows applications, the mileage may vary, and the end result is an application unable to run, or an application that doesn't work as intended.

To maximize application compatibility, the program had to be rewritten in a new language that can be compiled across various operating systems, along with the use of a cross-platform GUI tool to ensure that the program is able to run natively on multiple platforms.

Thus, the goal of this project. The aim is to provide a tool for WAD/Sprite manipulation for Half-Life 1 based games that can run natively on either Windows or Linux. There aren't many tools for the Linux lovers of the GoldSrc community, let's fix that!

## Status

**ALPHA**. The project is currently on a primitive state and can only be used to view WAD/Sprite files. Manipulation tools will come up next, followed by UI updates and the final step of code refactoring.

Currently, the tool can do the following:

- [X] Open and view WAD textures and Sprites.
- [X] Animate sprites.
- [X] Show transparent textures (Starting with '{') and transparent sprites (Alphatest only).
- [ ] Extract WAD from BSP.
- [ ] Extract textures from WAD.
- [ ] Create new WADs.
- [ ] Create new Sprites.
- [ ] Edit color palette.
- [ ] Edit sprite type.
- [ ] Rename WAD textures.
- [ ] Other miscellaneous UI tools.

Once these tasks has been accomplished, the project will move on to **BETA** status.

## Building

*TODO: Build instructions.*

You'll need wxWidgets 3.2 or newer to compile.

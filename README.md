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
- [X] Extract WAD from BSP.
- [X] Extract textures from WAD.
- [X] Extract sprite frames.
- [ ] Create new WADs.
- [ ] Create new Sprites.
- [ ] Edit color palette.
- [ ] Edit sprite type.
- [ ] Rename WAD textures.
- [ ] Other miscellaneous UI tools.

Once these tasks has been accomplished, the project will move on to **BETA** status.

# Building

You'll need wxWidgets 3.2 or newer to compile.

## Linux

First, get the necessary dependencies based on your distro:

*NOTE: Your distro might have these packages under a slighly different name, tweak as needed.*

### Ubuntu 23.04+

`sudo apt install g++ make git libwxbase3.2-1 libwxgtk3.2-dev libpcre2-32-0`

### Ubuntu 20.04+

Older Ubuntu distributions do not contain wxWidgets 3.2 on their official repositories. Unofficial repositories will be used for compilation.

Add wxWidgets 3.2 repos following [these instructions](https://docs.codelite.org/wxWidgets/repo320)

You may now proceed to install the dependencies:

`sudo apt install g++ make git libwxbase3.2unofficial-dev libwxgtk3.2unofficial-dev libpcre2-32-0`

### Fedora 39+

`sudo dnf install git make gcc-c++ wxBase-devel wxGTK-devel pcre2`

### Arch Linux

`sudo pacman -S git make gcc wxwidgets-common wxwidgets-gtk3 pcre2`

### openSUSE (Leap 15.5+)

`sudo zypper install git make gcc-c++ wxWidgets-3_2-devel libpcre2-32-0`

Now you can clone the repository and compile.

```
git clone https://github.com/JulianR0/HLTextureToolsCpp.git
cd HLTextureToolsCpp/HLTT
make
```

### If using unofficial repositories

If compilation fails due to missing include files, try the following command *(Ubuntu 20.04+)*:

`sudo ln -sv /usr/include/wx-3.2/wx /usr/include/wx`

Then try again.

## Windows

You need Visual Studio 2013 or newer.

[Download the wxWidgets library](https://www.wxwidgets.org/downloads/) from their website. Use the pre-built, 32-bit binaries for simplicity. [Follow the installation instructions](https://docs.wxwidgets.org/3.2.1/plat_msw_binaries.html) as provided by the website.

Create a `$(WXWIN)` environment variable as requested by the instructions above. It is required by the solution file, and so, needed for compilation.

Open the solution file in Visual Studio and change the active configuration to **"Release"**.

If using Visual Studio 2015 or newer, you'll need to tweak the project properties a bit to make use of the VS2015+ wxWidgets binaries. Otherwise, you can skip the steps below and compile now.

The solution file was created in VS2013: Opening the file under newer Visual Studio versions will initially prompt to update the compiler toolsets, say **"Yes"** to this dialog.

On the solution explorer, right-click HLTT then go to properties, edit the following setting:

`Linker --> General --> Additional Library Directories:` Replace **vc120_dll** with **vc14x_dll**.

You should now be able to compile.

**The program will only run while executed from the IDE.** To make it launch from outside it, you'll need to copy a few DLLs from the wxWidgets directory.

Copy the "wxbase32u_vc120.dll" and "wxmsw32u_core_vc120.dll" files from the wxWidgets binaries *(Use vc14x DLLs if compiled under VS2015 or newer)* to the root folder of HLTT.exe, and the program should run now.

# nes-emulator
My attempt at making an emulator for the Nintendo Entertainment System.

## Motives
I really want to learn much more about low-level programming, specifically programming in C. Upon searching for project ideas, I came across a [gameboy emulator](https://cturt.github.io/cinoop.html) that really caught my eye. As a huge fan of the Nintendo Entertainment System, I thought I would try my hand at making an emulator for it.

## Legal
I am in no way advocating the use of illegally obtained software with this emulator. I will not and cannot supply any software to use with this emulator. Any use of this emulator is under the assumption that the owner of the software that is being used has given permission to use it freely. All the information used in the development of this emulator was legally obtained and is credited in this file. **It is illegal to run any software on this emulator for which you do not have permission to use.**

## Status

### CPU - MOS 6502 Processor
The CPU used in the NES is for all intents and purposes, fully emulated. The only edge case left to implement is the passing of an extra clock cycle whenever a page in memory is crossed during the execution of specific instructions. Additionally, there are 105 opcodes that were not utilized by the NES, and consequently perform determinable, yet arbitrary operations. I have implemented some of these "illegal" opcodes, but not all. I decided not to continue implementing these for now, as there are only a handful of games that have ever used any of these illegal opcodes.

### PPU - NTSC 2C02 Processor
The Picture Processing Unit is undoubtedly the biggest challenge for me in this project. This emulated processor is in a state such that graphic data is being transferred via NTSC standard, but not correctly. There is still debugging to be had between acquiring and processing the graphic data within all the lookup tables of the NES, and accurately displaying the information to the screen. Furthermore, while the memory mapped registers between the CPU and the PPU are established, they mostly have no effect. Some flags have been utilized for various purposes, but for the most part, the memory mapped I/O of the NES has yet to be fully realized.

### Display Window
I am using the SDL2 library to display a window for the emulator. This component of the full emulator is, in my opinion, not far from completion. There are some issues that need to be resolved somewhere between the PPU processing graphic data and the display receiving and displaying this data. For now, it is hard to say whether this bug is coming from one component or the other (or both!). However, as far as a barebones yet functional display, this is close.

### APU - Audio Processing Unit
This component has not been started, or even researched yet. It is much more important to me that the previous three components are all fully functional before I even begin to look into the APU.

### MMC - Memory Management Controllers
These components are actually stored within a game cartridge, and allow for the switching of program & graphical data into & out of the memory of the NES, as needed. As of now, I have only implemented the NROM memory mapper, which is for simple games that actually don't contain and type of switching hardware in the cartridge. My feelings on this component are similar to that of the APU, except this component is more functional. Implementing NROM was required for me to test any games. After completion of the CPU and the PPU, I plan to implement at least the four most popular memory mappers, as that will cover the majority of games for the NES.


# Nand2Tetris Course Project

## About the Course

The Nand2Tetris course [https://www.nand2tetris.org/] is an educational initiative that guides students through the process of building a modern computer system from the ground up. Starting with elementary logic gates constructed in Hardware Description Language (HDL), the course progresses through several stages including the creation of basic and complex circuits, a Harvard Architecture cpu, machine language, an assembler, virtual machine, and a high-level language with compiler and an operating system. This hands-on approach demystifies how a computer works and provides comprehensive knowledge of computer architecture.

## Repository Overview

This repository contains all my work and experiments conducted as part of the Nand2Tetris course. While the course itself offers a structured approach to understanding the workings of computer systems, I have taken additional steps to enrich this learning by leveraging my background in C programming. Below are the key components and experiments included in this repository:

### Course Components and File Structure

- **Basic Gates**: Implementation of foundational logic gates and circuits using HDL.
- **Arithmetic Logic**: Arithmetic circuits including adders and incrementers.
- **Sequential Logic**: Creation of memory components like RAM and the PC(program counter).
- **Processor**: Construction of a central processing unit (CPU) and an integrated computer system using the components built in earlier modules.
- **Assembly**: Development and testing of assembly-level programs for the custom hardware.
- **Tools**: Custom development tools for the unique hard-/soft-ware.
- **To-Do**: Modules of the second part of the course I have not done yet.

### Extensions and Experiments
Side project that are not directly part of the course and not might not be refined duo to course deadlines:
- **C Integration with Assembly**: Developed a C header library to allow Hack assembly files to be compiled by a C compiler
- **Multithreaded Parsing**: Implemented low-level multi-threaded parsing where different threads perform unique tasks such as reading into a ring buffer, utilizing SIMD for token building, and performing binary translation.
- **CUDA Exploration**: Experimented with CUDA to potentially accelerate certain processes, exploring how GPU computing can be integrated within the assembler pipeline.
- **Advanced Optimizations**: Engaged in advanced optimization techniques such as carry look-ahead addition, which, while not necessary for the course outcomes, provided deeper insights into high-performance computing.


## Skills and Technologies

- **Languages**: HDL, Assembly, C, Verilog, CUDA
- **Tools**: Custom Assemblers, Simulators Debugging Tools from [https://www.nand2tetris.org/]
- **Concepts**: Boolean Logic, Computer Architecture, Assembler pipeline, Multithreading, GPU Computing

## Learning Outcomes

Through this project, I have not only adhered to the structured learning path provided by Nand2Tetris but also ventured into areas that bridged my existing knowledge with new challenges. Each component in this repository stands as both following course guidelines and transcending them to explore new technological.

## How to Use This Repository

Each directory in this repository is structured to reflect different aspects of the course and my additional experiments. There is general information in the doc directory and some modules have their own readme.

To experiemnt with the diffrent simulators or emulators check the official course website for guidence [https://www.nand2tetris.org/]

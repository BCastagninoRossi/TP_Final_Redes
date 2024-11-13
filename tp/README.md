# C Networking Project

This project is a C networking project that includes modules for text parsing, TCP server creation, and more. The project has been restructured to have a professional and well-organized structure.

## Folder Structure

The project is divided into the following subfolders based on responsibility:

- `parser`: Contains code for text parsing.
- `server`: Contains code for TCP server creation.
- `include`: Contains header (.h) files.
- `src`: Contains the main source code.
- `utils`: Contains auxiliary functions.

## Makefile

The `Makefile` is organized to reflect the new structure, separating dependencies and targets based on the defined modules (e.g., parser, server, utils). It follows standard compilation conventions and allows for building the entire project or individual modules independently.

## Documentation

Documentation files, such as `README.md`, are located in the root directory. Additional documentation can be found in the `docs` folder.

## Preservation of Functionality

The functionality in the files located in `tp/src` has been preserved, as everything is working correctly. Only the necessary changes to reorganize the project have been made while preserving its functionality.

## Removal of Redundancies

Redundant files like `tp/src/pepe` and `tp/src/nc` have been removed, and all configuration files point to the reorganized structure.

## Goal

The goal of this restructuring is to make the project easy to maintain, extend, and understand for any developer. The project is now ready for use in a professional environment and adheres to a standard structure for C projects.

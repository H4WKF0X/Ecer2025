# ECER 2026 Robotics Codebase

This repository contains the C codebase for the ECER 2026 robotics competition, developed for the KIPR Wombat controller. The software architecture is designed with a strict focus on modularity, hardware abstraction, and dynamic on-device calibration.

## Project Structure

The codebase is separated into distinct functional directories to separate reusable library code from executable routines:

* `config/`: Contains the standalone calibration program used to set up the robot before a run.
* `data/`: The target directory for the generated hardware configuration files (e.g., `robot_config.txt`). This directory is typically git-ignored to prevent merge conflicts between different physical robots.
* `lib/`: The core reusable libraries.
    * `include/`: Header files defining the public APIs (`drive.h`, `servo.h`, `config_parser.h`).
    * `src/`: C implementations of the libraries.
* `run/`: Contains the executable main programs (the actual contest routines) that utilise the libraries.

## Video Reference

The specific programs and contest routines demonstrated in our technical video can be found in the `run/` directory. These files contain the high-level logic that utilizes our modular libraries to perform the pipe collection and dispensing tasks shown.

## Core Features and Modularity

The system avoids hardcoded hardware parameters in the main execution logic. Instead, it relies on a modular library system:

1.  **Config Parser:** Reads a configuration file at runtime to dynamically assign motor ports, sensor ports, hardware limits, and light thresholds.
2.  **Drive Library:** Handles all locomotion. Features include constant acceleration (ramping) to prevent wheel slip, modular sensor-based movement (e.g., driving until specific line detections), and state-machine-based rotation for precise line-hugging. Functions are decoupled from hardcoded stopping commands, allowing for seamless movement chaining.
3.  **Servo Library:** Provides clamped, safe movement ranges and smooth, incremental sweeping functions to prevent mechanical strain or abrupt jerking of the manipulator arms.

## On-Device Calibration

To account for changing ambient lighting conditions and potential hardware swaps during the competition, calibration is performed directly with information from the Wombat controller. 

Executing the `main_calibrate` program initiates an interactive setup routine on the device. The user is prompted to input current hardware ports, read live sensor values for black/white thresholds, and define the sensor offset geometry. This data is exported to `data/robot_config.txt`. 

When the main contest program is executed, it parses this text file via the `config_parser` library. This ensures that sensor degradation, port changes, or lighting variations can be resolved in seconds without requiring the code to be recompiled.

## Build and Deployment

Currently, this repository serves as a structured development and version control environment. Because local cross-compilation is not yet fully configured, deployment is handled by manually copying the required library functions and execution logic directly into the KIPR IDE onto the Wombat controller.

The immediate roadmap includes integrating the `wombat-cross` toolchain. Implementing this toolchain will eliminate the need for manual code transfer, allowing the entire modular directory structure (`lib/`, `config/`, `run/`) to be compiled natively on a standard PC and deployed directly to the ARM-based controller as a complete executable.

For reference on the planned cross-compilation environment, see:
[PRIArobotics/wombat-cross](https://github.com/PRIArobotics/wombat-cross)

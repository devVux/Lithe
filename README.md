

# Introduction
**Lithe** is a 2D, simple, cross-platform, lightweight C++ game engine built for learning purposes.

---


## Install Dependencies
The engine supports **Windows**, **MacOS**, and **Linux**.

Before building the project, install the required dependency by running the following command, replacing "*your_preset*" with one of the following ones based on your operating system:

| OS      | Debug         | Release         |
| ------- | ------------- | --------------- |
| Windows | `x64-debug`   | `x64-release`   |
| MacOS   | `macos-debug` | `macos-release` |
| Linux   | `linux-debug` | `linux-release` |

```bash
cmake --preset "your_preset"
```

---

# Build the Project
To compile the project, you can choose one of the default presets (as shown in the table above) and running the following command:
```bash
cmake --build --preset "your_preset"
```

Alternatively, you can create a custom build by defining a `CMakeUserPresets.json` file.


#!/usr/bin/env python3
import json
from pathlib import Path

# Define your matrix
CONFIG = {
    "version": 6,
    "platforms": {
        "windows": {
            "condition": {"type": "equals", "lhs": "${hostSystemName}", "rhs": "Windows"},
            "compilers": {
                "MSVC": {
                    "cacheVariables": {
                        "CMAKE_C_COMPILER": "cl",
                        "CMAKE_CXX_COMPILER": "cl",
                        "CMAKE_TOOLCHAIN_FILE": "./vendor/vcpkg/scripts/buildsystems/vcpkg.cmake",
                        "VCPKG_MANIFEST_MODE": "ON"
                    }
                },
                "clang": {
                    "cacheVariables": {
                        "CMAKE_C_COMPILER": "clang-cl",
                        "CMAKE_CXX_COMPILER": "clang-cl",
                        "CMAKE_TOOLCHAIN_FILE": "./vendor/vcpkg/scripts/buildsystems/vcpkg.cmake",
				        "VCPKG_MANIFEST_MODE": "ON"
                    }
                }
            }
        },
        "linux": {
            "condition": {"type": "equals", "lhs": "${hostSystemName}", "rhs": "Linux"},
            "compilers": {
                "clang": {
                    "cacheVariables": {
                        "CMAKE_C_COMPILER": "clang",
                        "CMAKE_CXX_COMPILER": "clang++",
                        "CMAKE_TOOLCHAIN_FILE": "./vendor/vcpkg/scripts/buildsystems/vcpkg.cmake",
				        "VCPKG_MANIFEST_MODE": "ON"
                    }
                }
            }
        }
    },
    "generator": "Ninja Multi-Config",
    "buildTypes": ["Debug", "Release"],
}

def generate_presets():
    configure_presets = []
    build_presets = []
    
    for platform, pdata in CONFIG["platforms"].items():
        for compiler, cdata in pdata["compilers"].items():
            preset_name = f"{platform}-{compiler}"
            
            # Configure preset
            configure_presets.append({
                "name": preset_name,
                "displayName": f"{platform.title()} ({compiler})",
                "generator": CONFIG["generator"],
                "binaryDir": f"${{sourceDir}}/out/build/{compiler.lower()}",
                "installDir": f"${{sourceDir}}/out/install/{compiler.lower()}",
                "condition": pdata["condition"],
                "cacheVariables": {
                    **cdata["cacheVariables"],
                    "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
                }
            })
            
            # Build presets for each type
            for build_type in CONFIG["buildTypes"]:
                build_name = f"{preset_name}-{build_type.lower()}"
                build_presets.append({
                    "name": build_name,
                    "displayName": f"{platform.title()} {compiler} ({build_type})",
                    "configurePreset": preset_name,
                    "configuration": build_type
                })

    
    return {
        "version": CONFIG["version"],
        "cmakeMinimumRequired": {"major": 3, "minor": 25, "patch": 0},
        "configurePresets": configure_presets,
        "buildPresets": build_presets
    }

if __name__ == "__main__":
    presets = generate_presets()
    output_path = Path("CMakePresets.json")
    
    with output_path.open("w") as f:
        json.dump(presets, f, indent=2)
    
    print(f"Generated {output_path} with {len(presets['configurePresets'])} configure presets")
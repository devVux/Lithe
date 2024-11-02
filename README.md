
## Build the project
The engine supports Windows, MacOS and Linux. To compile the project choose one of the available default presets:

| OS      | Debug       | Release       |
| ------- | ----------- | ------------- |
| Windows | x64-debug   | x64-release   |
| MacOS   | macos-debug | macos-release |
| Linux   | linux-debug | linux-release |

Or create your custom build by creating a `CMakeUserPresets.json` file.

<br>

To install the dependencies run:
<details open>
	<summary>Windows</summary>
	```bash
		./scripts/setup.bat
	```
</details>
<details>
	<summary>MacOS/Linux</summary>
	```bash
		./scripts/setup.sh
	```
</details>

<br>

To start building simply run:
<details open>
	<summary>Windows</summary>
	```bash
		cmake --build --preset x64-release
	```
</details>
<details>
	<summary>MacOS</summary>
	```bash
		cmake --build --preset macos-release
	```
</details>
<details>
	<summary>Linux</summary>
	```bash
		cmake --build --preset linux-release
	```
</details>

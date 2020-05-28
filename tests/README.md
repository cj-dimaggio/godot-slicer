# Tests

Mostly as an experiment in implementation, Slicer includes a simple suite of unit tests. Configuration details can be found in the [SCSub file](./SCsub) found in this folder. Currently tests are only available on Unix environments, as they make use of the Godot headless server for initializing the Godot environment.

## Building
The tests can be built with the scons option `slicer_tests`, such as:

```bash
scons slicer_tests=yes
```

`slicer_tests` is also available as an alias if you wish to only target building the test binarry:

```bash
scons slicer_tests=yes slicer_tests
```

To speed up build times during development the `slicer_shared` option can be used to separate the building of the main test binary and the test files themselves. Both test and Slicer logic can be built as dynamic libraries using a command such as:

```bash
scons platform=osx slicer_shared=yes slicer_tests=yes slicer-test-shared slicer-shared
```

## Running
The testing binary will be built in Godot's `./bin/` folder with the naming format: `./bin/test-slicer.{os}.tools.{arch}`. Running this binary will start the test suite.

## Adding new tests
You should be able to add new test files by simply adding a file with the `test.cpp` prefix into this folder. the SCsub file will run a Glob to try to find all test files at build.

## Example Integration with VSCode
If you happen to use VSCode, the config files I used for my own personal debugging looked like (based on [Godot documentation](https://docs.godotengine.org/en/stable/development/cpp/configuring_an_ide/visual_studio_code.html)):

*tasks.json**
```json
{
    "version": "2.0.0",
    "tasks": [
        // Builds the Slicer in dynamic library mode so you don't need to wait for linking
        {
            "label": "build-slicer",
            "type": "shell",
            "command": "scons",
            "group": "build",
            "args": [
                "slicer_shared=yes",
                "target=debug",
                "-j",
                "4",
                "slicer-shared"
            ],
            "problemMatcher": "$msCompile"
        },
        // Builds the Slicer tests and Slicer logic in dynamic library mode so you don't need to wait for linking
        {
            "label": "build-slicer-tests",
            "type": "shell",
            "command": "scons",
            "group": "build",
            "args": [
                "target=debug",
                "skip_running_tests=yes",
                "slicer_tests=yes",
                "slicer_shared=yes",
                "-j",
                "4",
                "slicer-shared",
                "slicer-test-shared",
            ],
            "problemMatcher": "$msCompile"
        }
    ]
}
```

**launch.json**
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Slicer Debug Test Project",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/bin/godot.osx.tools.64",
            "args": ["--path", "/path/to/some/test/project"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "lldb",
            "preLaunchTask": "build-slicer"
        },
        {
            "name": "Slicer Debug Tests",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/bin/test-slicer.osx.tools.64",
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "lldb",
            "preLaunchTask": "build-slicer-tests"
        }
    ]
}
```


# How to build

## This is one Visual Studio Solution

- This repo uses the MACHINE_WIDE concept. No submodules but in one place, used by everything on one machine.
- Please checkout the [MACHINE_WIDE](https://github.com/dbj-data/machine_wide) repo somewhere on your machine.
  - we do it in `d:\machine_wide`
- Of course building from VStudio you will just [need to know this](https://docs.microsoft.com/en-us/cpp/build/reference/i-additional-include-directories?view=msvc-170) :wink:


## Why don't you use CMake you morons?

  - CMake is not a maker it is a "meta maker" :wink: 
    - it makes make files or Visual Studio solutions, for mostly multi OS projects.
  - This is single OS, Visual Studio project.
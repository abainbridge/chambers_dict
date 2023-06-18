# chambers_dict
Alternative GUI for the Chambers 21st Century Dictionary

Loads 10x more quickly. Uses half as much RAM. Is portable to Linux.

Depends on https://github.com/abainbridge/deadfrog-lib

The Visual Studio build uses a trick to reduce executable size by
linking against a version of msvcrt.dll that ships with every version
of Windows since 95. This means we can dynamically link without having
to force the user to install a "Visual C++ Redistributable Package".
Instructions on how this works: https://stackoverflow.com/a/39737730/66088


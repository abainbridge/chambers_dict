# chambers_dict
Alternative GUI for the Chambers 21st Century Dictionary

* Loads 10x more quickly.
* Uses half as much RAM.
* Uses one fifth as much disk space.
* Is portable to Linux.
* GUI is scalable for different screen DPIs.

Depends on https://github.com/abainbridge/deadfrog-lib

The Visual Studio build uses a cut-down libc/crt and win32 headers in order
to reduce executable size. https://github.com/abainbridge/vs_libc_replacement
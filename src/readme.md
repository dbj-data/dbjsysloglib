
This is implementation of a DBJ DLL Component.

Notes

- the def file you see in here is required and its contents are always the same
  - yes it is jkust copied from project to project, its name is arbitrary
  - Visual Studio def file processing is not automatic 
  - Go to Project > Properties > Linker > Input > Module Definition File and enter the name of the DEF file. Repeat this step for each configuration and platform, or do it all at once by selecting Configuration = All Configurations, and Platform = All Platforms.

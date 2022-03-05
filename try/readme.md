
- This VS project is not referencing VS project "dbjsyslogclient" in this same solution. 
- That is deliberate
	- To use dbj-dll you do not to need lib to use it
	- dbj-dll is always dynamicaly loaded
	- Huh? Simple, please see the `trydbjsyslogclient.c`
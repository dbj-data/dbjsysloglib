
- This VS project is not referencing VS project "dbjsyslogclient" in [this same solution](https://github.com/dbj-data/dbjsysloglib). 
- That is deliberate
	- To use dbj-dll you do not to need lib to use it
	    - thus you fo not need to wonder how to make it
	- dbj-dll component is always dynamicaly loaded
	- Huh? Simple, please see the `trydbjsyslogclient.c`

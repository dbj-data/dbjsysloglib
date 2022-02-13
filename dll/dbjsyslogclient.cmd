@rem
@rem 2022 Q1: Not in use but good candidate to resurect
@rem
@rem (c) 2021 by dbj@dbj.org
@rem 
@rem
@echo off
@cls
setlocal

@rem Setup VS2019
@rem change to your local path, if need be
call "D:\PROD\programs\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

set "sourcebasename=dbjsyslogclient"
set "sourcefilename=%sourcebasename%"
set "build=..\Build"
@md %build% 2>nul

@set "INCLUDE=.\..;%INCLUDE%"

@rem this is debug build
@rem /Zi -- debug builds + pdb file -- remove this switch for release builds
@rem /TC -- threat all as C
clang-cl %sourcefilename%.c src\dbjsyslog.c /nologo /MDd /D_DEBUG /Zi /TC /W3 /Fo%build%\ /Fe%build%\%sourcebasename%.dll /link /dll /def:%sourcefilename%.def

@echo.
@echo Done building: %build%\%sourcebasename%.dll
@echo.

endlocal
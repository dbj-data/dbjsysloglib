
# dbjsysloglib 
### standard C implementation

(For good and mature syslog server please check: https://www.paessler.com/free_syslog_server )

The project we started from is Copyright (c) 2008 Secure Endpoints Inc.


> ## Disclaimer: <br/>
> #### This is Open Source Proof Of Concept (POC). <br/>
> #### Use at your own risk. We do.


Currently [RFC3164](https://tools.ietf.org/html/rfc3164) is followed. Plan is to develop a support for the newer one: 
[RFC5424](https://tools.ietf.org/html/rfc5424).

### Usage and the API

Loging and syslog are cool and best way to develop resilient software.

But be carefull not to introduce too much hard-wired dependancies by using the dbjsysloglib (syslog).

We recomend using the set of macros found at the botom of the `dbjsyslog.h`,  in order to 
decouple a bit more.

```cpp
// ERROR
#define DBJ_LOG_ERR(...) 
// CRITICAL
#define DBJ_LOG_CRT(...) 
// ALERT
#define DBJ_LOG_LRT(...) 
// WARNING
#define DBJ_LOG_WRG(...) 
// EMERGENCY
#define DBJ_LOG_MCY(...) 
// DEBUG
#define DBJ_LOG_DBG(...) 
// INFO
#define DBJ_LOG_INF(...) 
// NOTICE
#define DBJ_LOG_NTC(...)
```

Use these macros. They are active if DBJ_SYSLOGLIB is defined before 
including `dbjsyslog.h`, if not they are `__noop`.

<span id="features">

### What are the actual syslog features?

Please make sure you have read once the RFC's 3164 and [5424](https://tools.ietf.org/html/rfc5424) . (Links are at the top of the doc.)

The dbjsysloglib behaviour is standard, as described on  numerous pages on the web.
`syslog.h` header in here is 99% standard and you are free to browse through it to understand all the options.

#### Async behaviour and Multi Threading

This API is asynchronous as much as windows sockets are asynchronous.

This C API is not, resilient in presence of mutliple threads. Althugh the core implementation is `syslog.c` uses critical section.

The C++ API is resilient in the presence of multiple threads.

##### dbjsysloglib SysLog Messages are composed following the RFC3164

Make sure you syslog server can understand them. Which is the case with almost all of them. 
This is simply because there are milions of devices in use (and will be for a long time to come) who are emitting 
syslog messages in the RFC3164 format. Note: Devices on the network, not PC's or laptops.

> This is a static library, built to be used in the Windows client or server.

Built with Microsoft (R) C/C++ Optimizing Compiler Version 19.15.26732.1 or better.
(In 2020Q1, standard C++ was C++17)



> Copyright by dbj@dbj.org, serverside.systems, a division of dbj.systems ltd.

Licence is [here](LICENSE.md).

---------------------------------------------------------------------  

[![dbj();](http://dbj.org/wp-content/uploads/2015/12/cropped-dbj-icon-e1486129719897.jpg)](http://www.dbj.org "dbj")  


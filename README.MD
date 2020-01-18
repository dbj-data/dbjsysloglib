
# dbjsysloglib 
### standard C API 

(or use: https://www.paessler.com/free_syslog_server ?)

> The project we started from is Copyright (c) 2008 Secure Endpoints Inc.


> Disclaimer: <br/>
> This is Open Source Proof Of Concept (POC). <br/>
> Use at your own risk.
> We do


Currently [RFC3164](https://tools.ietf.org/html/rfc3164) is followed. Plan is to develop a support for the newer one: 
[RFC5424](https://tools.ietf.org/html/rfc5424).

### Usage and the API


<span id="features">

### What are the actual syslog features?

Please make sure you have read once the RFC's 3164 and [5424](https://tools.ietf.org/html/rfc5424) . (Links are at the top of the doc.)

The dbjsysloglib behaviour is standard, as described on  numerous pages on the web.
`syslog.h` header in here is 99% standard and you are free to browse through it to understand all the options.


#### Decouple the logging from your code

Loging and syslog are cool and best way to develop resilient software.

But be carefull not to introduce too much hard-wired dependancies on the dbjsysloglib (syslog).

We recomend using the set of macros found at the botom of the dbjsysloglib .h  ,  in order to 
decouple a bit more.

```cpp
#define DBJ_LOG_ERR(...) 
#define DBJ_LOG_CRT(...) 
#define DBJ_LOG_LRT(...) 
#define DBJ_LOG_WRG(...) 
#define DBJ_LOG_MCY(...) 
#define DBJ_LOG_DBG(...) 
#define DBJ_LOG_INF(...) 
#define DBJ_LOG_NTC(...)
```

Use these macros. They are active if DBJ_SYSLOGLIB is defined before 
including dbjsysloglib .h 

Alternatively one can avoid the macros. 
dbjsysloglib .h  is quite simple in order to understand how to do that.

In case you do not want to use dbjsysloglib any more just place these macros (non functional versions)
in one of your header files.

#### std::error_code 

Is logged by using that macro above. Example usage.

```cpp
std::error_code err = some_function() ;_
if (err)
   DBJ_LOG_STD_ERR(err,__FILE__) ;
```
Above logs an syslog error, the std::error_code message, and user defined prompt with it.

In case you are wondering what is std::error_code, it is in standard C++ since years ago.
You do not need to fully implement your own error category.
Using [`std::errc`](https://en.cppreference.com/w/cpp/error/errc) , is quite sufficient in most situations.

#### Async behaviour and Multi Threading

This API is asynchronous as much as windows sockets are asynchronous.
But all the impotrtant functions in here are capable of sequential working, 
in the presence of multiple threads.

This API is resilient in presence of mutliple threads.

##### WARNING: dbjsysloglib SysLog Messages are composed following the RFC3164

Make sure you syslog server can understand them. Which is the case with almost all of them. 
This is simply because there are milions of devices in use (nad will be for a long time to come) who are emitting 
syslog messages in the RFC3164 format. Device on the network, not PC's or laptops.

> This is a static library. 

> Built with Microsoft (R) C/C++ Optimizing Compiler Version 19.15.26732.1 or better.
> (In 2020Q1, standard C++ was C++17)



> Copyright by dbj@dbj.org, serverside.systems, a division of dbj.systems ltd.

> Licence is [here](./LICENSE.MD).

---------------------------------------------------------------------  

[![dbj();](http://dbj.org/wp-content/uploads/2015/12/cropped-dbj-icon-e1486129719897.jpg)](http://www.dbj.org "dbj")  


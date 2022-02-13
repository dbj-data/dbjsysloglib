
# dbjsysloglib 

There are many Windows Syslog servers with associated GUI app.

![](./media/syslogwatcher.com.jpg)

For our Windows server side deliverables we use "[SysLog Watcher](https://syslogwatcher.com/)".

(For another good and mature syslog server please check: https://www.paessler.com/free_syslog_server )

The project we started from is Copyright (c) 2008 Secure Endpoints Inc.

### The Requirement

Currently [RFC3164](https://tools.ietf.org/html/rfc3164) is followed. Plan is to develop a support for the newer one: 
[RFC5424](https://tools.ietf.org/html/rfc5424).

This is implemented in standard C as [DBJ DLL Component](https://github.com/dbj-data/dbj-dll).

### Usage and the API

Please review the [simple one file example application](https://github.com/dbj-data/dbjsysloglib/blob/master/try/trydbjsyslogclient.c).


<span id="features">

## What are the actual syslog features?

Please make sure you have read once the RFC's 3164 and [5424](https://tools.ietf.org/html/rfc5424) . (Links are at the top of the doc.)

The dbjsysloglib behaviour is standard syslog, as described on numerous pages on the web.
`syslog.h` header in here is 99% standard and you are free to browse through it to understand all the options.

### Sort-of-a async behaviour and Multi Threading

This API is asynchronous as much as windows UDP sockets are asynchronous.

This C API is resilient in presence of multiple threads.  That is not asynchronous API it simple locks on each call.

Since at the end it used Windows UDP sockets the full process of sending is very quick and controls returns to the caller almost instantaneously.

Please note UDP sockets are not guaranteed delivery. But, UDP is very good solution to target the `localhost:514`, standard Syslog port.


#### dbjsysloglib SysLog Messages are composed following the RFC3164

Make sure your syslog server of choice can understand them. Which is the case with almost all of them. 

This is simply because there are millions of devices in use (and will be for a long time to come) who are emitting 
syslog messages in the RFC3164 format. Note: Devices on the network, not PC's or laptops.

> This is a DLL, built to be used in the Windows client or server.

> Copyright by dbj@dbj.org, serverside.systems, a division of dbj.systems ltd.

Licence is [here](LICENSE.md).

---------------------------------------------------------------------  

[![dbj();](https://dbj.org/wp-content/uploads/2015/12/cropped-dbj-icon-e1486129719897.jpg)](http://www.dbj.org "dbj")  


# Netwatchdog
![Logo](./Icon_256.png)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](./LICENSE)

## Table of Contents
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [Build](#build)  
  - [Usage](#usage)  
    - [Server](#server)  
    - [Client](#client)  
  - [Contribute](#contribute)
  - [License](#license)
  - [Dependencies](#dependencies)

## Introduction

Netwatchdog is a simple application that is used to monitor network connection drops.

## Build

Netwatchdog uses CMake to generate necessary configuration files

```
mkdir build
cd build
cmake ..
```

## Usage

The usage of the client and the server is quite simple. Running the application is enough to use the default settings.

### Server

```
NetWatchdog - ZeroMQ based network monitoring tool.
Usage: C:\git\NetWatchdog\bin\Debug\netwatchdogd.exe [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -p,--port INT               The port to use [defaults to 32000]
  -i,--identity TEXT          Identity
  --host TEXT                 Listening address for the server [defaults to *]
```

### Client

```
NetWatchdog - ZeroMQ based network monitoring tool.
Usage: C:\git\NetWatchdog\bin\Debug\netwatchdogc.exe [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -p,--port INT               The port to use [defaults to 32000]
  -i,--identity TEXT          Identity
  -c,--clientCount UINT       Number of clients to spawn.
  --host TEXT                 Host to connect to
```

## License

Netwatchdog is licensed under the [MIT license](https://opensource.org/license/mit)

## Dependencies

- [Cereal](https://uscilab.github.io/cereal/)
- [CLI11](https://cliutils.github.io/CLI11/book/)
- [cppzmq](https://zeromq.org/)
- [stduuid](https://github.com/mariusbancila/stduuid)
- [TOML11](https://queue.cppget.org/toml11)
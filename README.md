# Netwatchdog
![Logo](./Icon_256.png)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](./LICENSE)

## Table of Contents
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [Build](#build)  
  - [Contribute](#contribute)
  - [License](#license)

## Introduction

Netwatchdog is a simple application that is used to monitor network connection drops.

## Build

Netwatchdog uses CMake to generate necessary configuration files

```
mkdir build
cd build
cmake ..
```

## License

Netwatchdog is licensed under the [MIT license](https://opensource.org/license/mit)

## Dependencies

- [Cereal](https://uscilab.github.io/cereal/)
- [CLI11](https://cliutils.github.io/CLI11/book/)
- [cppzmq](https://zeromq.org/)
- [stduuid](https://github.com/mariusbancila/stduuid)
- [TOML11](https://queue.cppget.org/toml11)
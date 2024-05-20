# Docker

The main README for the project is found here [README.md](../README.md)

## Building Docker Image

All docker images, along with the GitHub actions, are based upon Ubuntu.

Assuming from the root of the project, and using Linux as a host environment. Once the project has been compiled in Release, following the build information in the root [README.md](../README.md), the following steps can be executed (depending on the applicaiton/docker to be built).

0.1 can be replaced with the correct version.

### Netwatchdog Server

```
mkdir tmp/server
cd ./tmp/server
cp ../../bin/netwatchdogd .
cp ../../docker/server/* .
docker build -t viperman1271/netwatchdog-server:0.1 .
```

### Client

```
mkdir tmp/client
cd ./tmp/client
cp ../../bin/netwatchdogc .
cp ../../docker/client/* .
docker build -t viperman1271/netwatchdog-client:0.1 .
```

### Web Server

```
mkdir tmp/web
cd ./tmp/web
cp ../../bin/web .
cp ../../docker/web/* .
docker build -t viperman1271/netwatchdog-web:0.1 .
```
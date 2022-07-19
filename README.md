# MoonHex
 A hex editor for romhackers

## Overview

### Appearance under Linux (Ubuntu 18) (Outdated picture)
![Main Screen Linux](https://i.imgur.com/Qv7SIpP.png)
### Appearance under Wndows (8.1)
![Main Screen Windows](https://i.imgur.com/778x62u.png)

### Features

* Table Suppport
* Offset navigation
* Font size customization
* Zoom in/out with mouse wheel

## Building

### Linux

```shell
git clone --recurse-submodules https://github.com/Moonslate/MoonHex.git
mkdir build
cd build
cmake ..
make
```

### Windows

You need an envoirment variable pointing to wxWidgets

```shell
git clone --recurse-submodules https://github.com/Moonslate/HM-Studio.git
Open folder with Visual Studio or VS Code
Select a configuration either Release or Debug
Hit Build
```

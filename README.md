# c’t-Demo: Micro-Webservice

## Für c’t-Leser

Dieses Repository enthält den Quellcode für den in c’t 7–9/2023 entwickelten Webservice. Um es auf Ihren Rechner zu bekommen, müssen Sie es klonen:

```
git clone https://github.com/ct-Open-Source/micro-webservice.git
cd micro-webservice
```

Der Code für den [1. Teil](https://www.heise.de/select/ct/2023/7/2303016374942185339) befindet sich im Branch „part1“. Sie checken ihn aus mit

```
git checkout part1
```

Der Code für den [2. Teil](https://www.heise.de/select/ct/2023/8/2303413085906187110) befindet sich im Branch „part2“. Sie checken ihn aus mit

```
git checkout part2
```

Der Code für den [3. Teil](https://www.heise.de/select/ct/2023/9/2305915484770006026) befindet sich im Branch „part3“. Sie checken ihn aus mit

```
git checkout part3
```

**Vor dem Ausprobieren von Teil 3 unter Windows oder Windows Subsystem for Linux bitte [Issue #4](https://github.com/ct-Open-Source/micro-webservice/issues/4) beachten!**

## Systemvoraussetzungen

Der Webservice benötigt das [Boost](https://www.boost.org/)-Framework, das Sie wie folgt installieren können.

### Ubuntu

```
sudo apt install libboost-dev
```

Ggf. sind noch die Pakete `libblkid-dev`, `e2fslibs-dev`, `libboost-all-dev` und `libaudit-dev` zu installieren.

### macOS 13 Ventura

```
brew install libboost-dev
```

### WSL/Windows 11

```
mkdir -p ~/tmp
cd ~/tmp
wget https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/boost_1_82_0.tar.gz
cd boost_1_82_0
./bootstrap.sh
./b2 cxxflags=-std=c++17
mkdir -p ~/dev
./b2 install --prefix=/home/<youruserid>/dev
```

### Windows 11

Windows-Nutzer müssen die benötigten Bibliotheken Boost und GMP aus dem Quellcode übersetzen.

#### Boost

- [Quellcode](https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/) der aktuellen Boost-Version (derzeit 1.82.0) herunterladen und entpacken
- Im Visual Studio Developer Command Prompt in das entpackte Verzeichnis wechseln und dann eingeben:

```
bootstrap
md %HOMEPATH%\dev
b2 cxxflags=-std=c++17
b2 install --prefix=%HOMEPATH%\dev
```

#### GMP

```
git clone https://github.com/gx/gmp.git
cd gmp
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=%HOMEPATH%\dev ..
cmake --build . --config Release --target ALL_BUILD
cmake --build . --config Release --target RUN_TESTS
cmake --build . --config Release --target INSTALL
cd %HOMEPATH%\dev\gmp\lib
copy libgmp-13.lib gmp.lib
copy libgmpxx-9.lib gmpxx.lib
```

## Webservice kompilieren

### macOS, Linux

Zum Erzeugen der Build-Dateien kommt [CMake](https://cmake.org/) zum Einsatz:

```
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

Wenn Sie statt eines Release ein Binary zum Debuggen erzeugen wollen, wählen Sie „Debug“ statt „Release“.

Falls CMake die Boost-Bibliothek nicht finden kann, müssen Sie den Pfad (beispielsweise /opt/boost-1_81_0) dorthin in der Umgebungsvariable `BOOST_ROOT` angeben:

```
BOOST_ROOT=/opt/boost-1_81_0
```

Nun können Sie den Compiler anwerfen:

```
cmake --build .
```

Es entsteht das Binary `micro-webservice`, das Sie mit

```
./micro-webservice
```

aufrufen können.

### WSL/Windows 11

```
cd micro-webservice
mkdir -p build
cd build
BOOST_ROOT=~/dev/boost_1_82_0 cmake -DCMAKE_BUILD_TYPE=Release ..
```


### Windows

```
cd micro-webservice
md build
cd build
set GMP_ROOT=%HOMEPATH%\dev\gmp
set BOOST_ROOT=%HOMEPATH%\dev\boost_1_82_0
cmake --fresh ..
cmake --build . --config Release
```

Bevor Sie die im Verzeichnis Release entstandene EXE-Datei `micro-webservice.exe` aufrufen können, müssen Sie noch die DLL der GMP-Bibliothek dorthin kopieren:

```
copy %HOMEPATH%\dev\gmp\bin\libgmp-13.dll Release
```

_Copyright ©️ 2023 [Oliver Lau](mailto:ola@ct.de), [Heise](https://www.heise.de/) Medien GmbH & Co. KG_

--- 

### Nutzungshinweise

Diese Software wurde zu Lehr- und Demonstrationszwecken geschaffen und ist nicht für den produktiven Einsatz vorgesehen. Heise Medien und der Autor haften daher nicht für Schäden, die aus der Nutzung der Software entstehen, und übernehmen keine Gewähr für ihre Vollständigkeit, Fehlerfreiheit und Eignung für einen bestimmten Zweck.

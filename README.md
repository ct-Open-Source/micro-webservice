# c’t-Demo: Micro-Webservice

## Für c’t-Leser

Dieses Repository enthält den Quellcode für den in c’t 7/2023, S. 140, entwickelten Micro-Webservice. Um es auf Ihren Rechner zu bekommen, müssen Sie es klonen:

```
git clone https://github.com/ct-Open-Source/micro-webservice.git
cd micro-webservice
```

Der Code für den 1. Teil befindet sich im Branch „part1“. Sie checken ihn aus mit

```
git checkout part1
```

## Systemvoraussetzungen

Der Webservice benötigt das [Boost](https://www.boost.org/)-Framework, das Sie wie folgt installieren können.

### Ubuntu

```
sudo apt install libboost-dev
```

### macOS 13 Ventura

```
brew install libboost-dev
```

## Kompilieren

Zum Erzeugen der Build-Dateien kommt [CMake](https://cmake.org/) zum Einsatz:

```
CMAKE_BUILD_TYPE=Release cmake .
```

Wenn Sie statt eines Release ein Binary zum Debuggen erzeugen wollen, wählen Sie „Debug“ statt „Release“.

Falls CMake die Boost-Bibliothek nicht finden kann, müssen Sie den Pfad (beispielsweise /opt/boost-1_81_0) dorthin in der Umgebungsvariable `BOOST_ROOT` angeben:

```
BOOST_ROOT=/opt/boost-1_18_0
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


_Copyright ©️ 2023 [Oliver Lau](mailto:ola@ct.de), [Heise](https://www.heise.de/) Medien GmbH & Co. KG_

--- 

### Nutzungshinweise

Diese Software wurde zu Lehr- und Demonstrationszwecken geschaffen und ist nicht für den produktiven Einsatz vorgesehen. Heise Medien und der Autor haften daher nicht für Schäden, die aus der Nutzung der Software entstehen, und übernehmen keine Gewähr für ihre Vollständigkeit, Fehlerfreiheit und Eignung für einen bestimmten Zweck.

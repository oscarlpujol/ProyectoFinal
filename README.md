# ProyectoFinal
## Proyecto de la asignatura SDG2 - Calidad del aire en un quirófano

### Messages exchanged
- start condition = "StartCond"
- stop condition = "StopCond"
- ACK = "ACK"
- XCK = "XCK"

### Interruption activation (keys from the keyboard)
- Power On/ Power Off = **Space**
- TVOC and CO2 measure now (skip timeout) = **m**
- H2 and ethanol measure now = **r**

### How I make it work?

1. Open two terminals (one will work as the sensor and the other as the Iris
2. Git clone the project
```
$ git clone
$ cd ProyectoFinal
```
3. Firstly, start the Iris
```
$ cd Iris
$ make
$ ./iris
```
4. Secondly, start the sensor
```
cd Sensor
$ make
$ ./sensor
```
5. Use the keys from the keyboard to start the system and ask for different measures

cl /EHsc /c ./src/SerialMonitor.cpp /Fo:./src/SerialMonitor.obj
cl /EHsc /c ./lib/SerialCOM.cpp /Fo:./lib/SerialCOM.obj
cl /EHsc ./src/SerialMonitor.obj ./lib/SerialCOM.obj /link /Fe:./serialmonitor.exe

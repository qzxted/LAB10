# Лабораторна робота №10


## Структура проєкту

* include/Operator.h, Call.h, Database.h, IOperatorRepository.h, SQLiteOperatorRepository.h, TestData.h, CallCommand.h, CallContext.h, CallObserver.h, CallState.h, CallStateHistory.h, IPriorityStrategy.h, PriorityStrategies.h.
* src/main.cpp, SQLiteOperatorRepository.cpp, TestData.cpp, CallCommand.cpp, CallContext.cpp, CallObserver.cpp, CallState.cpp, PriorityStrategies.cpp, Tests.cpp.
* CMakeLists.txt
* CMakePresets.json
* vcpkg.json


## Збірка та запуск

1. Відкрийте папку проєкту у Visual Studio з увімкненою підтримкою CMake та vcpkg.
2. Оберіть пресет x64-windows і виконайте повну збірку.
3. Запустіть зібрані exe:
  * CallCenterApp.exe
  * CallCenterTests.exe
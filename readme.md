Qt версии 5.15 или Qt 6.x.  Модули: Core, Widgets, Gui

Компилятор с поддержкой C++17:  GCC 7+ / Clang 6+ / MSVC 2019+

CMake версии 3.5+


Использование

    Главное окно:
        Визуализация пути (красная линия)
        Датчики (желтые треугольники)
        Оси вагонов (красные/зеленые круги)

    Таблица данных:
        Отображает параметры каждой зафиксированной оси
        Обновляется в реальном времени

    Управление:
        Кнопка "Блокировка состава" - останавливает движение
        Кнопка "Сброс" - возвращает систему в начальное состояние


Linux Ubuntu 

Установка зависимостей 

    sudo apt update
    sudo apt install -y build-essential cmake qt5-default

Сборка проекта 

    https://github.com/Victor-161/pricel.git
    cd pricel
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc)

Запуск приложения 

    ./build/pricel3

Возможные проблемы и решения: 

    1)Ошибка "Cannot find Qt5":
        export QT_DIR=/usr/lib/x86_64-linux-gnu/qt5
        cmake -DCMAKE_PREFIX_PATH=$QT_DIR ..
     2)Отсутствие прав:
        chmod +x pricel3
     3) Зависимости не найдены:
        sudo apt install -y libgl1-mesa-dev

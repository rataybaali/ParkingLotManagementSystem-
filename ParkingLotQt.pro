QT -= gui
CONFIG += c++17 console
CONFIG -= app_bundle

# Target executable name
TARGET = ParkingLotQt

# Disable fpermissive warning as error
QMAKE_CXXFLAGS += -fpermissive
QMAKE_CXXFLAGS_WARN_ON -= -Werror

# Source Files
SOURCES += \
    main.cpp \
    MainWindow.cpp \
    ParkingLot.cpp \
    ParkingSlot.cpp \
    User.cpp \
    Vehicle.cpp

# Header Files
HEADERS += \
    MainWindow.h \
    ParkingLot.h \
    ParkingSlot.h \
    User.h \
    Vehicle.h

QT += widgets
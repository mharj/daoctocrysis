TEMPLATE = app
TARGET = qtheight
QT += core gui xml sql
HEADERS += qtheight.h qtmpak.h
SOURCES += main.cpp qtheight.cpp qtmpak.cpp
CONFIG  += qt warn_on release console
LIBS += -L/usr/lib -L/usr/lib/X11 -lfreetype -lz -lMagick -lWand -Wl,-Bsymbolic-functions 
INCLUDEPATH += /usr/include/ImageMagick

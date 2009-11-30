TEMPLATE = app
TARGET = qtheight
QT += core gui xml sql
HEADERS += qtheight.h qtmpak.h qt_dempak.h main_window.h
SOURCES += main.cpp qtheight.cpp qtmpak.cpp qt_dempak.cpp main_window.cpp
CONFIG  += qt warn_on release console
LIBS += -L/usr/lib -L/usr/lib/X11 -lfreetype -lz -lMagick++ -lMagickWand -lMagickCore -Wl,-Bsymbolic-functions 
INCLUDEPATH += /usr/include/ImageMagick
FORMS += main_window.ui

######################################################################
# Automatically generated by qmake (3.1) Sun Jul 7 19:35:59 2019
######################################################################

QT += sql widgets multimedia network multimediawidgets
TEMPLATE = app
TARGET = ClientAp
INCLUDEPATH += .

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += audiorecorder.h \
           camera.h \
           chatwidgit.h \
           database.h \
           dataconfig.h \
           filereceiver.h \
           filesender.h \
           imagesettings.h \
           login.h \
           qaudiolevel.h \
           register.h \
           trace.h \
           videosettings.h \
           videowidgetsurface.h \
           widget.h
FORMS += audiorecorder.ui \
         camera.ui \
         chatwidgit.ui \
         dataconfig.ui \
         filereceiver.ui \
         filesender.ui \
         imagesettings.ui \
         login.ui \
         register.ui \
         videosettings.ui \
         widget.ui
SOURCES += audiorecorder.cpp \
           camera.cpp \
           chatwidgit.cpp \
           database.cpp \
           dataconfig.cpp \
           filereceiver.cpp \
           filesender.cpp \
           imagesettings.cpp \
           login.cpp \
           main.cpp \
           qaudiolevel.cpp \
           register.cpp \
           trace.cpp \
           videosettings.cpp \
           videowidgetsurface.cpp \
           widget.cpp
RESOURCES += Resource.qrc

DISTFILES += \
    image/User.png

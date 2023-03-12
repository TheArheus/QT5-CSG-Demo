QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets openglwidgets opengl

CONFIG += c++20

###############################
#DEFINES += QT_NODLL
#DEFINES += QT_STATIC_BUILD

#CONFIG += static

#QMAKE_CFLAGS_DEBUG += /MTd
#QMAKE_CXXFLAGS_DEBUG += /MTd
#QMAKE_CFLAGS_DEBUG -= -MDd
#QMAKE_CXXFLAGS_DEBUG -= -MDd

#QMAKE_CFLAGS_RELEASE += /MT
#QMAKE_CXXFLAGS_RELEASE += /MT
#QMAKE_CFLAGS_RELEASE -= -MD
#QMAKE_CXXFLAGS_RELEASE -= -MD
###############################

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mat_h.hpp \
    mesh.cpp \
    openglrenderwidget.cpp

HEADERS += \
    mainwindow.h \
    mat_h.hpp \
    mesh.h \
    openglrenderwidget.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ..\\assets\\mesh.frag.glsl \
    ..\\assets\\mesh.vert.glsl

CONFIG += create_prl link_prl

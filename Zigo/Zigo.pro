#-------------------------------------------------
#
# Project created by QtCreator 2015-12-05T13:46:10
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Zigo
TEMPLATE = app

INCLUDEPATH += modules/include

LIBS += -pthread -lssl -lcrypto -ldl

QMAKE_CXXFLAGS += -Wall -Wno-write-strings -Wno-reorder

SOURCES += main.cpp\
        mainwindow.cpp \
    modules/src/client.cpp \
    modules/src/client_node.cpp \
    modules/src/crypto.cpp \
    modules/src/file.cpp \
    modules/src/heartbeat.cpp \
    modules/src/job.cpp \
    modules/src/logger.cpp \
    modules/src/message.cpp \
    modules/src/peer.cpp \
    modules/src/seeder.cpp \
    modules/src/seeder_job.cpp \
    modules/src/seeder_node.cpp \
    modules/src/server.cpp \
    modules/src/settings.cpp \
    modules/src/steganography.cpp \
    modules/src/thread.cpp \
    modules/src/thread_pool.cpp \
    modules/src/udp_socket.cpp

HEADERS  += mainwindow.h \
    modules/include/client.h \
    modules/include/client_node.h \
    modules/include/crypto.h \
    modules/include/file.h \
    modules/include/heartbeat.h \
    modules/include/job.h \
    modules/include/logger.h \
    modules/include/message.h \
    modules/include/network_exceptions.h \
    modules/include/peer.h \
    modules/include/seeder.h \
    modules/include/seeder_job.h \
    modules/include/seeder_node.h \
    modules/include/server.h \
    modules/include/settings.h \
    modules/include/steganography.h \
    modules/include/thread.h \
    modules/include/thread_pool.h \
    modules/include/udp_socket.h \
    permission.h

FORMS    += mainwindow.ui

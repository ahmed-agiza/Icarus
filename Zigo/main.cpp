#include "mainwindow.h"
#include <QApplication>

#include "server.h"
#include "client.h"
#include <QDebug>

#include <iostream>
using std::endl;

int main(int argc, char *argv[])
{

    if (argc < 4) {
        fprintf(stderr, "Usage %s <ServerPort> <SeederIP> <SeederPort>.\n", argv[0]);
        exit(1);
    }

    uint16_t serverPort = (uint16_t) atoi(argv[1]);
    const char *seederIP = argv[2];
    uint16_t seederPort = (uint16_t) atoi(argv[3]);


    QApplication a(argc, argv);
    MainWindow w(serverPort, (char *)seederIP, seederPort);
    w.show();

    /*try {
        Server *server = new Server(9999);
        server->start();
        //server->wait();
    } catch (NetworkException &e){
        fprintf(stderr, "%s\n", e.what());
    } catch (const char *e) {
        fprintf(stderr, "%s\n", e);
    } catch (char const *e) {
        fprintf(stderr, "%s\n", e);
    }

   try {
        Client *client = new Client("ahmed", "127.0.0.1", 9999);
        int clientRC = client->start();
        //client->wait();
        char terminationMessage[LOG_MESSAGE_LENGTH];
        sprintf(terminationMessage, "Client terminated with code %d.", clientRC);
        Logger::info(terminationMessage);
    } catch (NetworkException &e){
        Logger::error(e.what());
    } catch (const char *e) {
        Logger::error(e);
    }*/

    return a.exec();
}

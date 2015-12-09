#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QProgressBar>
#include "string.h"
#include "QStack"
#include "dialogeditimage.h"
#include <vector>
#include "server.h"
#include "client_manager.h"
#include "heartbeat.h"
#include "peer.h"
#include "steganography.h"
#include <map>
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(uint16_t serverPort, char *seederIp, uint16_t seederPort, QWidget *parent = 0);
    ~MainWindow();

    void _loadPeerData(Peer &peer);
private slots:

    void on_btnConnect_clicked();

    void on_btnSearch_clicked();

    void on_btnNewImg_clicked();

    void on_btnBack_clicked();

    void on_btnSendImg2_clicked();

    //void on_lstPeers_itemDoubleClicked(QListWidgetItem *item);

    void on_btnDecrypt_clicked();

    void on_tblImgs_doubleClicked(const QModelIndex &index);

    void setStatus(QString);

    void on_tblShardImg_doubleClicked(const QModelIndex &index);

    void on_tblUrImg_doubleClicked(const QModelIndex &index);

    void onSearchResults();
    void onAddressSearchResults(int peerIndex);
    void onRSAResults(int peerIndex);
    void onKeysResults(int peerIndex);
    void onHeartBeatConnected();
    void onHeartBeatDisconnected();

    void updateNotify();

    void updateProgress(float);

    void onFileUpdateComplete(int);


    int connectToSeeder(char *username);
    void fetchPeers();
    void setImgNum();
    QPixmap decrypt(QString senderId, QString path, const char *prefix = "/recv", char *stegkey = "123456");
    void getPeerData(QString peerID);
    QImage getImage (QString imgId, QString peerID);
    void updateImageViews(QString fileId, int newCount);
    void on_lstPeers_doubleClicked(const QModelIndex &index);
signals:
    void searchComplete();
    void adressSearchComplete(int);
    void rsaSearchComplete(int);
    void keysSearchComplete(int);
    void heartbeatConnected();
    void heartbearDisconnected();
    void progressUpdate(float);
    void fileUpdateComplete(int);
private:
    Ui::MainWindow *ui;
    QStack<int> _previousWidgetStack;
    int notify;
    vector <Peer> _peers;
    QString _peerName;
    QString _peerIP;
    QString _peerID;
    QString _peerPort;
    QString _peerRSA;
    QString _peerSteg;
    QString _imagePath;
    QString _imageId;
    HeartBeat * _heartbeat;
    Server *_server;
    Peer *_currentPeer;
    ClientManager _clients;
    char _stegKey[2048];
    const char *_publicRSA;
    const char *_privateRSA;
    char _username[128];
    uint16_t _serverPort;
    char _seederIp[128];
    uint16_t _seederPort;
    char _appId[128];
    QProgressBar *_progress;
    ConnectionState _currentState;

    vector<QString> _sentFiles;
    vector<QString> _recvFiles;

    vector<QString> _sentFilesIds;
    vector<QString> _recvFilesIds;

    vector<QString> _myImages;
    vector<QString> _myImageIds;
    vector<QString> _myImageOwnerIds;

    static void *_heartBeatDisconnectionWrapper(Thread* heartBeat, void *mainWindow);
    void _heartBeatDisconnected(HeartBeat *heartBeat);
    static void *_heartBeatConnectionWrapper(Thread* heartBeat, void *mainWindow);
    void _heartBeatConnected(HeartBeat *heartBeat);
    static void *_queryResultsWrapper(Thread* heartBeat, void *mainWindow);
    static void *_addressResultsWrapper(Thread* heartBeat, void *mainWindow);
    static void *_rsaResultsWrapper(Thread* heartBeat, void *mainWindow);
    static void *_keyResultsWrapper(Thread* heartBeat, void *mainWindow);
    static void _progressListenerWrapper(float progress, void *mainWindow);
    static void *_updateCompleteWrapper(Thread *thread, void *mainWindow);

    void _emitUpdateComplete(int);
    void _emitPorgress(float);
    void _emitResults();
    void _emitAddress(int peerIndex);
    void _emitRSA(int peerIndex);
    void _emitKeys(int peerIndex);
    void _getClientAddress(Peer &peer);
    void _getClientRSA(Peer &peer);
    void _getClientKeys(Peer &peer);
    void _refreshStatus();

    int _getTotalViews(QString senderId, QString path, const char *prefix = "/recv", char *stegkey = "123456") const;
    int _getRemainingViews(QString sender, QString id, const char *prefix = "/recv", char *stegkey = "123456") const;

};

#endif // MAINWINDOW_H

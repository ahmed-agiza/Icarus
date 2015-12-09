#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QListWidgetItem"
#include "QTableWidgetItem"
#include "QString"
#include <string>
#include "QStringList"
#include "QMessageBox"
#include "QFileDialog"
#include "QFile"
#include "vector"
#include "QDebug"
#include "dialogquery.h"
#include "dialognewimg.h"
#include "QStatusBar"
#include "dialogeditimage.h"
#include <QPixmap>
#include <QDebug>
#include <QSettings>

using namespace std;


MainWindow::MainWindow(uint16_t serverPort, char *seederIp, uint16_t seederPort, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    if (false) //if logged in before
        ui->stackedWidget->setCurrentIndex(1);
    else
        ui->stackedWidget->setCurrentIndex(0);
    ui->btnBack->setVisible(false);
    ui->btnNewImg->setVisible(false);
    _serverPort = serverPort;
    strcpy(_seederIp, seederIp);
    _seederPort = seederPort;
    _currentPeer = 0;
    connect(this, SIGNAL(searchComplete()), this, SLOT(onSearchResults()));
    connect(this, SIGNAL(adressSearchComplete(int)), this, SLOT(onAddressSearchResults(int)));
    connect(this, SIGNAL(rsaSearchComplete(int)), this, SLOT(onRSAResults(int)));
    connect(this, SIGNAL(keysSearchComplete(int)), this, SLOT(onKeysResults(int)));
    connect(this, SIGNAL(heartbeatConnected()), this, SLOT(onHeartBeatConnected()));
    connect(this, SIGNAL(heartbearDisconnected()), this, SLOT(onHeartBeatDisconnected()));
    connect(this, SIGNAL(progressUpdate(float)), this, SLOT(updateProgress(float)));
    connect(this, SIGNAL(fileUpdateComplete(int)), this, SLOT(onFileUpdateComplete(int)));
    _currentState = Connecting;
    _progress = new QProgressBar;
    _progress->setMaximumHeight(16);
    _progress->setMaximumWidth(200);
    _progress->setTextVisible(true);
    statusBar()->addPermanentWidget(_progress, 0);
    _progress->hide();

}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::connectToSeeder(char *username) {
    strcpy(_username, username);

    _heartbeat = new HeartBeat(_username, _seederIp, _seederPort, _serverPort);
    _heartbeat->addDoneCallback(_heartBeatDisconnectionWrapper, this);
    _heartbeat->setConnectedCallback(_heartBeatConnectionWrapper, this);
    int heartBeatRC =_heartbeat->start();
    _server = new Server(_serverPort);
    QSettings settings("CSCE-485", "Zigo", this);
    memset((char *)_stegKey, 0, 2048);
    if (!settings.contains("STEG_KEY")) {
        Crypto::generateRandomString((char *)_stegKey, 128);
        settings.setValue("STEG_KEY", QByteArray(_stegKey));
    } else {
        strcpy((char *)_stegKey, settings.value("STEG_KEY").toString().toLocal8Bit().data());
    }

    _server->setStegKey(_stegKey);
    _publicRSA = _server->getPublicRSA();
    _privateRSA = _server->getPrivateRSA();
    strcpy(_appId, _heartbeat->getId());
    qDebug() << "App ID: " << _appId;
    int serverRC =_server->start();
    return !((heartBeatRC == 1) && (serverRC == 1));
}

void MainWindow::fetchPeers()
{


    //get contacts from local database and store name and id in peers vector
}

void MainWindow::setImgNum()
{
    // get the number


    notify = 1;
    updateNotify();
}
QPixmap MainWindow::decrypt(QString senderId, QString path, const char *prefix, char *stegkey) {
    char imagePath[PATH_MAX], imageDir[PATH_MAX],
         imageName[PATH_MAX], decryptionPath[PATH_MAX],
         decryptionName[PATH_MAX];

    char *command = senderId.toLocal8Bit().data();
    char *buf1 = path.toLocal8Bit().data();
    char buf2[1024];
    memset(buf2, 0, 1024);

    memset(imageDir, 0, PATH_MAX);
    memset(imagePath, 0, PATH_MAX);
    memset(imageName, 0, PATH_MAX);
    memset(decryptionName, 0, PATH_MAX);
    memset(decryptionPath, 0, PATH_MAX);

    strcpy(imageDir, "storage/");
    strcat(imageDir, command);
    strcpy(imagePath, imageDir);
    strcat(imagePath, prefix);
    strcat(imagePath, "/");
    strcat(imageName, _appId);
    strcat(imageName, "-");
    strcat(imageName, buf1);
    strcat(imagePath, imageName);
    strcpy(decryptionName, imageName);
    strcat(decryptionName, ".decr");
    strcpy(decryptionPath, imageDir);
    strcat(decryptionPath, "/");
    strcat(decryptionPath, decryptionName);
    printf("Image directory: %s\n", imageDir);
    printf("Image path: %s\n", imagePath);
    fflush(stdout);
    if (!File::exists(imagePath)) {
        fprintf(stderr, "Image does not exist!\n");
        return QPixmap();
    }

    char updatedCount[32];
    int decryptRC = Steganography::decryptImage(imageDir, imagePath, decryptionName, buf2, 1024, stegkey);
    if(decryptRC) {
        fprintf(stderr, "Failed to decrypt the image.\n");
        return QPixmap();
    }
    int currentViewCount;
    int totalViewCount;
    char owner[256];
    if(sscanf(buf2, "%[^;]%*c%d;%d;", owner, &totalViewCount, &currentViewCount) != 3) {
        fprintf(stderr, "Invalid image!\n");
        File::remove(decryptionPath);
        return QPixmap();
    }
    if (currentViewCount == 0) {
        QMessageBox::information(this, "Views Count Excceeded", "You can no longer view this image.");
        printf("You can no longer view this image.\n");
        File::remove(imagePath);
        File::remove(decryptionPath);
        return QPixmap();
    }
    printf("Current view count: %d\n", currentViewCount);
    currentViewCount--;
    sprintf(updatedCount, "%s;%d;%d;", owner, totalViewCount, currentViewCount);
    int updateRC = Steganography::updateImageData(imageDir, imagePath, updatedCount, stegkey);
    if(updateRC) {
        fprintf(stderr, "Failed to update the image!\n");
        File::remove(decryptionPath);
        return QPixmap();
    }
    QPixmap result(decryptionPath);
    File::remove(decryptionPath);
    result = result.scaled(ui->lblImage->size());
    if (currentViewCount == 0) {
        printf("You can no longer view this image.\n");
        QMessageBox::information(this, "Views Count Excceeded", "You can no longer view this image.");
        File::remove(imagePath);
        File::remove(decryptionPath);
        return QPixmap();
    }
    return result;
}

void MainWindow::on_btnConnect_clicked()
{
    QString username = ui->txtUsrName->text();
    if (username.isEmpty()) {
        QMessageBox::warning(this, tr("Application"),tr("Enter a Username!"));
        return;
    }
    QByteArray user = username.toLocal8Bit();
    if (connectToSeeder(user.data()) != 0) {
        QMessageBox::warning(this, tr("Application"),tr("Error Connecting!\n Please try again."));
        ui->txtUsrName->setText("");
        return;
    }


    ui->stackedWidget->setCurrentIndex(1);

    ui->btnNewImg->setVisible(true);
    ui->btnBack->setVisible(true);


}

void MainWindow::updateNotify()
{
    QString y = QString::number(notify);
    QString x= "Image (" + y + ")";
    ui->btnNewImg->setText(x);
}

void MainWindow::updateProgress(float val) {
    _progress->show();
    _progress->setValue(val);
    if (val >= 99) {
        _progress->hide();
        _loadPeerData(*_currentPeer);
    }
}

void MainWindow::onFileUpdateComplete(int peerIndex) {
    char buf[2048];
    memset(buf, 0, 2048);
    Peer &peer = _peers[peerIndex];
    const char *ip = peer.getPeerAddress();
    uint16_t port = peer.getPortNumber();
    Client *client = _clients.get(peer.getId(), _username, ip, port);
    client->fetchResults(buf);
    qDebug() << buf;
}

void MainWindow::on_btnSearch_clicked()
{
    QString searched = ui->txtSearch->text();
    if(!searched.isEmpty()){
        _heartbeat->registerListener(_queryResultsWrapper, (void *)this);
        _heartbeat->queryUsername((char*)searched.toStdString().c_str());
    }
    else {
        QMessageBox::warning(this, tr("Application"),tr("Enter a name to search for"));
    }
}

void MainWindow::on_btnNewImg_clicked()
{
    _previousWidgetStack.push(ui->stackedWidget->currentIndex());
    _myImages.clear();
    _myImageIds.clear();
    _myImageOwnerIds.clear();
    vector<FileInfo> recvFiles = File::getAllFiles(Receive);
    ui->tblImgs->clearContents();
    ui->tblImgs->setRowCount(0);

    for(unsigned int i = 0; i < recvFiles.size(); i++) {
        if (strcmp(recvFiles[i].recepientId, _appId) != 0)
            continue;
        QString filePath = QString(QByteArray(recvFiles[i].filePath));
        QString fileId = QString(QByteArray(recvFiles[i].fileId));
        QString owner = QString(QByteArray(recvFiles[i].userId));
        if (filePath.trimmed().isEmpty() || fileId.trimmed().isEmpty() || owner.trimmed().isEmpty())
            continue;

        _myImages.push_back(filePath);
        _myImageIds.push_back(fileId);
        _myImageOwnerIds.push_back(owner);
        ui->tblImgs->insertRow(i);
        QTableWidgetItem *itemId = new QTableWidgetItem(fileId);
        QTableWidgetItem *itemOwner = new QTableWidgetItem(owner);
        ui->tblImgs->setItem(i, 0, itemOwner);
        ui->tblImgs->setItem(i, 1, itemId);

    }
    ui->stackedWidget->setCurrentIndex(4);
    ui->tblImgs->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblImgs->resizeColumnsToContents();
    ui->tblImgs->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::on_btnBack_clicked()
{
    if (!_previousWidgetStack.empty()) {
        ui->stackedWidget->setCurrentIndex(_previousWidgetStack.pop());
    }
}


void MainWindow::on_btnSendImg2_clicked() {
    DialogNewImg *dialog =new DialogNewImg;
    dialog->setPeerName(_peerName);
    dialog->setPeerID(_peerID);
    dialog->setPeerIP(_peerIP);
    dialog->setPeerPort(_peerPort);
    dialog->setPeerLabel(_peerName);
    int code = dialog->exec();
    QString imagePath = dialog->getImagePath();
    int viewCount = dialog->getViewCount();
    if(code == QDialog::Accepted && !imagePath.trimmed().isEmpty()) {
        const char *pathData = imagePath.toLocal8Bit().data();
        Peer &peer = *_currentPeer;
        printf("Address: %s\n", peer.getPeerAddress());
        printf("Exists.\n");
        fflush(stdout);


        if (!File::exists(pathData)) {
            QMessageBox::critical(this, "Error", "Image does not exist.");
            return;
        }


        char extraData[256];
        memset(extraData, 0, 256);
        sprintf(extraData, "%s;%d;%d;", _appId, viewCount, viewCount);



        char userFilePath[PATH_MAX];
        char fileName[PATH_MAX];
        char encryptedFilePath[PATH_MAX];
        char finalFilePath[PATH_MAX];
        char fileId[PATH_MAX];
        char originalHash[64];
        char coverFile[] = "cover.jpg";

        Crypto::md5HashFile((char *)pathData, originalHash);
        memset(userFilePath, 0, PATH_MAX);
        memset(fileName, 0, PATH_MAX);
        memset(encryptedFilePath, 0, PATH_MAX);
        memset(finalFilePath, 0, PATH_MAX);
        memset(fileId, 0, PATH_MAX);
        sprintf(userFilePath, "storage/%s/sent", peer.getId());
        File::joinPaths(encryptedFilePath, userFilePath, originalHash);
        strcat(fileId, peer.getId());
        strcat(fileId, "-");
        strcat(fileId, originalHash);
        qDebug() << "PeerID: " << peer.getId();
        qDebug() << "Hash: " << originalHash;

        int rc = Steganography::encryptImage(userFilePath, pathData, coverFile, originalHash, extraData, "12345");
        if(rc) {
            QMessageBox::critical(this, "Error", "Failed to encrypt the selected file.");
            return;
        }

        File::joinPaths(finalFilePath, userFilePath, fileId);
        File::rename(encryptedFilePath, finalFilePath);

        qDebug() << "File path: " << finalFilePath;



        Client *client = _clients.get(_currentPeer->getId(), _currentPeer->getUsername(), _currentPeer->getPeerAddress(), _currentPeer->getPortNumber());
        client->registerUpdateListener(_progressListenerWrapper, this);
        client->sendFile(finalFilePath, fileId);

    }
}


void MainWindow::on_btnDecrypt_clicked() {
    QPixmap decrypted = decrypt(_peerID, _imageId, "/recv", (char *)_stegKey);
    ui->lblImage->setPixmap(decrypted);
}

void MainWindow::on_tblImgs_doubleClicked(const QModelIndex &index)
{

    /*qDebug () << index.row() << ", " << index.column() << ": " << index.isValid();

    if (!index.isValid()) {
        qDebug() << "Invalid index!" << endl;
        return;
    }
    QTableWidgetItem *item = ui->tblImgs->item(index.row(), index.column());
    if (item) {
        if (item->column()== 2)
        {
            _previousWidgetStack.push(ui->stackedWidget->currentIndex());
            ui->stackedWidget->setCurrentIndex(2);
            notify=0;
            updateNotify();

        }

    }*/

    _imagePath =  _myImages[index.row()];
    _imageId =  _myImageIds[index.row()];
    _peerID = _myImageOwnerIds[index.row()];

    ui->stackedWidget->setCurrentIndex(2);
    QPixmap imageMap = QPixmap(_imagePath);
    imageMap = imageMap.scaled(ui->lblImage->size(), Qt::KeepAspectRatio);
    ui->lblImage->setPixmap(imageMap);




}

void MainWindow::setStatus(QString x)
{
    ui->statusBar->showMessage(x);
}

void MainWindow::on_tblShardImg_doubleClicked(const QModelIndex &index)
{
    QTableWidgetItem *item = ui->tblShardImg->item(index.row(), 0);
    if (!index.isValid()) {
        qDebug() << "Invalid index!" << endl;
        return;
    }
    if (item) {
       /*QString fileId = _recvFilesIds[index.row()];
       DialogEditImage* a = new DialogEditImage;
       a->setPeerLabel(_peerName);
       int totalViews = _getTotalViews(_peerID, fileId, "/recv", (char *)_stegKey);
       a->exec();*/
       _previousWidgetStack.push(ui->stackedWidget->currentIndex());
       _imagePath =  _recvFiles[index.row()];
       _imageId =  _recvFilesIds[index.row()];
       ui->stackedWidget->setCurrentIndex(2);
       QPixmap imageMap = QPixmap(_imagePath);
       imageMap =imageMap.scaled(ui->lblImage->size(), Qt::KeepAspectRatio);
       ui->lblImage->setPixmap(imageMap);

    }
}

void MainWindow::on_tblUrImg_doubleClicked(const QModelIndex &index)
{
    qDebug() << index.row();
    QTableWidgetItem *item = ui->tblUrImg->item(index.row(), index.column());
    if (!index.isValid()) {
        qDebug() << "Invalid index!" << endl;
        return;
    }
    if (item) {
        qDebug() << item;
       QString fileId = _sentFilesIds[index.row()];
       qDebug() << fileId;
       DialogEditImage* a = new DialogEditImage;
       a->setPeerLabel(_peerName);
       a->setImageId(fileId);
       a->setSenderId(_peerID);
       qDebug() << "File ID: " << fileId;
       int totalViews = _getTotalViews(_peerID, fileId, "/sent", _peerSteg.toLocal8Bit().data());
       a->setImgViews(totalViews);
       if (a->exec() == QDialog::Accepted) {
            int newCount = a->getImgViews();
            qDebug() << newCount;
            updateImageViews(fileId, newCount);
       }
    } else
        qDebug() << "!!";
}

void MainWindow::onSearchResults() {
    char buf[2048];
    _heartbeat->fetchResults(buf);
    printf("Buffer: %s\n", buf);
    fflush(stdout);
    PeersMap results = Peer::fromStringList(buf);
    DialogQuery *x = new DialogQuery(results);
    x->setSearchName(ui->txtSearch->text());
    x->setData();
    x->exec();
    _peerName= x->getChoice();
    _peerID= x->getChosenID();
    _peerIP = x->getChosenIP();
    _peerPort = x->getChosenPort();
    if(_peerID.length() < 5)
        return;
    Peer a;
    QByteArray array = _peerName.toLocal8Bit();
    a.setUsername(array.data());
    array = _peerID.toLocal8Bit();
    a.setId(array.data());
    a.setPortNumber(_peerPort.toInt());
    array = _peerIP.toLocal8Bit();
    a.setPeerAddress(array.data());
    bool found = false;
    for(int i = 0; i < _peers.size(); i++) {
        if(strcmp(_peers[i].getId(), a.getId()) == 0) {
            found = true;
            break;
        }
    }
    if (found)
        return;
     _peers.push_back(a);
    //      ui->lstPeers->insertItem(ui->lstPeers->size(),_peerName);
    int size = int(ui->lstPeers->count());
    ui->lstPeers->insertItem(size, _peerName);
}

void MainWindow::onAddressSearchResults(int peerIndex) {
    char buf[2048];
    memset(buf, 0, 2048);
    Peer &peer = _peers[peerIndex];
    _heartbeat->fetchResults(buf);
    char *result = buf + 1;
    Peer temp = Peer::fromString(result);
    peer.setPeerAddress(temp.getPeerAddress());
    peer.setPortNumber(temp.getPortNumber());
    _getClientKeys(peer);
}

void MainWindow::_loadPeerData(Peer &peer)
{
    vector<FileInfo> sentFiles;
    vector<FileInfo> recvFiles;
    File::getUserFiles((char *) peer.getId(), Send, sentFiles);
    File::getUserFiles((char *) peer.getId(), Receive, recvFiles);
    _sentFiles.clear();
    _recvFiles.clear();
    _sentFilesIds.clear();
    _recvFilesIds.clear();
    ui->tblUrImg->clearContents();
    ui->tblShardImg->clearContents();
    ui->tblUrImg->setRowCount(0);
    ui->tblShardImg->setRowCount(0);

    for(unsigned int i = 0; i < sentFiles.size(); i++) {
        QString filePath = QString(QByteArray(sentFiles[i].filePath));
        QString fileId = QString(QByteArray(sentFiles[i].fileId));
        if (filePath.trimmed().isEmpty() || fileId.trimmed().isEmpty())
            continue;
        _sentFiles.push_back(filePath);
        _sentFilesIds.push_back(fileId);
        ui->tblUrImg->insertRow(i);
        QTableWidgetItem *item = new QTableWidgetItem(fileId);
        ui->tblUrImg->setItem(i, 0, item);
    }

    for(unsigned int i = 0; i < recvFiles.size(); i++) {
        QString filePath = QString(QByteArray(recvFiles[i].filePath));
        QString fileId = QString(QByteArray(recvFiles[i].fileId));
        if (filePath.isEmpty() || fileId.isEmpty())
            continue;
        _recvFiles.push_back(filePath);
        _recvFilesIds.push_back(fileId);
        ui->tblShardImg->insertRow(i);
        QTableWidgetItem *item = new QTableWidgetItem(fileId);
        ui->tblShardImg->setItem(i, 0, item);

    }
}

void MainWindow::onRSAResults(int peerIndex) {
    char buf[2048];
    memset(buf, 0, 2048);
    Peer &peer = _peers[peerIndex];
    const char *ip = peer.getPeerAddress();
    uint16_t port = peer.getPortNumber();
    Client *client = _clients.get(peer.getId(), _username, ip, port);
    client->fetchResults(buf);
    peer.setRSA(buf);
    QString peerId(QByteArray(peer.getId()));
    QString username(QByteArray(peer.getUsername()));
    QString peerAddress(QByteArray(peer.getPeerAddress()));
    QString rsa(QByteArray(peer.getRSA()));
    QString steg(QByteArray(peer.getStegKey()));
    QString peerPort = QString::number(peer.getPortNumber());
    _currentPeer = &(_peers[peerIndex]);
    _peerName = username;
    _peerIP = peerAddress;
    _peerID = peerId;
    _peerPort = peerPort;
    _peerRSA = rsa;
    _peerSteg = steg;

    _previousWidgetStack.push(ui->stackedWidget->currentIndex());
    ui->lblPeerName->setText("Peer: "+_peerName);
    //Scan images!-------------------------------------------
    ui->stackedWidget->setCurrentIndex(3);
    _loadPeerData(*_currentPeer);
}

void MainWindow::onKeysResults(int peerIndex)
{
    char buf1[2048], buf2[2048], buf3[2048];
    memset(buf1, 0, 2048);
    memset(buf2, 0, 2048);
    memset(buf3, 0, 2048);
    Peer &peer = _peers[peerIndex];
    const char *ip = peer.getPeerAddress();
    uint16_t port = peer.getPortNumber();
    Client *client = _clients.get(peer.getId(), _username, ip, port);
    client->fetchResults(buf1);
    if(sscanf(buf1, "%[^;]%*c%[^;];", buf2, buf3) != 2) {
        fprintf(stderr, "Invalid reply %s\n", buf1);
        return;
    }

    peer.setStegKey(buf2);
    peer.setRSA(buf3);
    QString peerId(QByteArray(peer.getId()));
    QString username(QByteArray(peer.getUsername()));
    QString peerAddress(QByteArray(peer.getPeerAddress()));
    QString rsa(QByteArray(peer.getRSA()));
    QString steg(QByteArray(peer.getStegKey()));
    QString peerPort = QString::number(peer.getPortNumber());
    _currentPeer = &(_peers[peerIndex]);
    _peerName = username;
    _peerIP = peerAddress;
    _peerID = peerId;
    _peerPort = peerPort;
    _peerRSA = rsa;
    _peerSteg = steg;

    _previousWidgetStack.push(ui->stackedWidget->currentIndex());
    ui->lblPeerName->setText("Peer: "+_peerName);
    //Scan images!-------------------------------------------
    ui->stackedWidget->setCurrentIndex(3);
    _loadPeerData(*_currentPeer);
}

void MainWindow::onHeartBeatConnected() {
    _currentState = Connected;
}

void MainWindow::onHeartBeatDisconnected() {
    _currentState = Disconnected;
    _refreshStatus();
    qDebug() << "Reconnecting..";
    _heartbeat->autoRestart();
}

void MainWindow::on_lstPeers_doubleClicked(const QModelIndex &index)
{
    Peer &peer = _peers[index.row()];
    ui->tblShardImg->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblShardImg->resizeColumnsToContents();
    ui->tblShardImg->horizontalHeader()->setStretchLastSection(true);
    ui->tblUrImg->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblUrImg->resizeColumnsToContents();
    ui->tblUrImg->horizontalHeader()->setStretchLastSection(true);
    _peerName = peer.getUsername();
    ui->lblPeerName->setText(_peerName);
    _peerID = peer.getUsername();
    QByteArray a (peer.getPeerAddress());
    _peerIP = QString(a);
    _peerPort = QString::number(_peers [index.row()].getPortNumber()) ;

    QString peerId(QByteArray(peer.getId()));
    QString username(QByteArray(peer.getUsername()));
    QString peerAddress(QByteArray(peer.getPeerAddress()));
    QString rsa(QByteArray(peer.getRSA()));
    QString steg(QByteArray(peer.getStegKey()));
    QString peerPort = QString::number(peer.getPortNumber());
    _currentPeer = &(_peers[index.row()]);

    if(peerAddress.isEmpty() || _currentPeer->getPortNumber() <= 0)
        _getClientAddress(peer);
    else if (rsa.isEmpty() || steg.isEmpty())
        _getClientKeys(peer);
    else {
        _peerName = username;
        _peerIP = peerAddress;
        _peerID = peerId;
        _peerPort = peerPort;
        _peerRSA = rsa;
        _peerSteg = steg;

        _previousWidgetStack.push(ui->stackedWidget->currentIndex());
        ui->lblPeerName->setText("Peer: " + _peerName);
        //Scan images!-------------------------------------------
        ui->stackedWidget->setCurrentIndex(3);
        _loadPeerData(*_currentPeer);
    }

}

void *MainWindow::_heartBeatDisconnectionWrapper(Thread *heartBeat, void *mainWindow)
{
    MainWindow *window = static_cast<MainWindow *>(mainWindow);
    window->_heartBeatDisconnected(static_cast<HeartBeat *>(heartBeat));
    return (void *)heartBeat;
}

void MainWindow::_heartBeatDisconnected(HeartBeat *) {
    emit heartbearDisconnected();
}

void *MainWindow::_heartBeatConnectionWrapper(Thread *heartBeat, void *mainWindow)
{
    MainWindow *window = static_cast<MainWindow *>(mainWindow);
    window->_heartBeatConnected(static_cast<HeartBeat *>(heartBeat));
    return (void *)heartBeat;
}



void MainWindow::_heartBeatConnected(HeartBeat *heartBeat) {
    emit heartbeatConnected();
}

void *MainWindow::_queryResultsWrapper(Thread *thread, void *mainWindow)
{
    HeartBeat *heartBeat = static_cast<HeartBeat *>(thread);
    static_cast<MainWindow*>(mainWindow)->_emitResults();
    return (void *)heartBeat;
}

void *MainWindow::_addressResultsWrapper(Thread *thread, void *mainWindow) {
    HeartBeat *heartBeat = static_cast<HeartBeat *>(thread);
    char peerId[128];
    strcpy(peerId, heartBeat->getSearchParam());

    int index = -1;
    MainWindow *window = static_cast<MainWindow*>(mainWindow);
    vector<Peer> &peers = window->_peers;
    for(unsigned int i = 0; i < peers.size(); i++) {
        if(strcmp(peerId, peers[i].getId()) == 0) {
            index = i;
            qDebug() << "Found!" << i;
            break;
        }
    }
    window->_emitAddress(index);
    return (void *)thread;
}

void *MainWindow::_rsaResultsWrapper(Thread *thread, void *mainWindow)
{
    Client *client = static_cast<Client *>(thread);
    int index = -1;
    MainWindow *window = static_cast<MainWindow*>(mainWindow);
    vector<Peer> &peers = window->_peers;
    for(unsigned int i = 0; i < peers.size(); i++) {
        if(client->getConnectionPort() == peers[i].getPortNumber() && strcmp(peers[i].getPeerAddress(), client->getAddress()) == 0) {
            index = i;
            qDebug() << "Found!" << i;
            break;
        }
    }
    window->_emitRSA(index);
    return (void *)client;
}

void *MainWindow::_keyResultsWrapper(Thread *thread, void *mainWindow)
{
    qDebug() << "_keyResultsWrapper(Thread *thread, void *mainWindow)";
    Client *client = static_cast<Client *>(thread);
    int index = -1;
    MainWindow *window = static_cast<MainWindow*>(mainWindow);
    vector<Peer> &peers = window->_peers;
    for(unsigned int i = 0; i < peers.size(); i++) {
        if(client->getConnectionPort() == peers[i].getPortNumber() && strcmp(peers[i].getPeerAddress(), client->getAddress()) == 0) {
            index = i;
            qDebug() << "Found!" << i;
            break;
        }
    }
    window->_emitKeys(index);
    return (void *)client;
}

void *MainWindow::_updateCompleteWrapper(Thread *thread, void *mainWindow)
{
    Client *client = static_cast<Client *>(thread);
    int index = -1;
    MainWindow *window = static_cast<MainWindow*>(mainWindow);
    vector<Peer> &peers = window->_peers;
    for(unsigned int i = 0; i < peers.size(); i++) {
        if(client->getConnectionPort() == peers[i].getPortNumber() && strcmp(peers[i].getPeerAddress(), client->getAddress()) == 0) {
            index = i;
            qDebug() << "Found!" << i;
            break;
        }
    }
    window->_emitUpdateComplete(index);
    return (void *)client;
}

void MainWindow::_progressListenerWrapper(float progress, void *mainWindow) {
    qDebug() << "==" << progress;
    static_cast<MainWindow *>(mainWindow)->_emitPorgress(progress);
}

void MainWindow::_emitUpdateComplete(int index) {
    emit fileUpdateComplete(index);
}

void MainWindow::_emitPorgress(float progress) {
    emit progressUpdate(progress);
}



void MainWindow::_emitResults()
{
    emit searchComplete();
}

void MainWindow::_emitAddress(int peerIndex)
{
    emit adressSearchComplete(peerIndex);
}

void MainWindow::_emitRSA(int peerIndex) {
    emit rsaSearchComplete(peerIndex);
}

void MainWindow::_emitKeys(int peerIndex)
{
    qDebug() << "Emit" << peerIndex;
    emit keysSearchComplete(peerIndex);
}

void MainWindow::_getClientAddress(Peer &peer) {
    _heartbeat->registerListener(_addressResultsWrapper, (void *)this);
    _heartbeat->queryId((char *)peer.getId());
}

void MainWindow::_getClientRSA(Peer &peer) {
    const char *ip = peer.getPeerAddress();
    uint16_t port = peer.getPortNumber();
    Client *client = _clients.get(peer.getId(), _username, ip, port);
    client->registerListener(_rsaResultsWrapper, (void *)this);
    client->queryRSA();

}

void MainWindow::_getClientKeys(Peer &peer)
{
    const char *ip = peer.getPeerAddress();
    uint16_t port = peer.getPortNumber();
    Client *client = _clients.get(peer.getId(), _username, ip, port);
    client->registerListener(_keyResultsWrapper, (void *)this);
    client->queryKeys();
}

void MainWindow::_refreshStatus() {
    if (_currentState == Connecting) {
        setStatus("Connecting..");
    } else if (_currentState == Connected) {
        setStatus("Connected");
    } else if (_currentState == Disconnected) {
        setStatus("Disconnected");
    }
}

int MainWindow::_getTotalViews(QString sender, QString id, const char *prefix, char *stegkey) const {
    char imagePath[PATH_MAX], imageDir[PATH_MAX],
            imageName[PATH_MAX], decryptionPath[PATH_MAX],
            decryptionName[PATH_MAX], buf2[1024];
    char *command = sender.toLocal8Bit().data();
    char *buf1 = id.toLocal8Bit().data();
    qDebug() << command;
    qDebug() << buf1;

    memset(imageDir, 0, PATH_MAX);
    memset(imagePath, 0, PATH_MAX);
    memset(imageName, 0, PATH_MAX);
    memset(buf2, 0, 1024);
    memset(decryptionName, 0, PATH_MAX);
    memset(decryptionPath, 0, PATH_MAX);

    strcpy(imageDir, "storage/");
    strcat(imageDir, command);
    strcpy(imagePath, imageDir);
    strcat(imagePath, prefix);
    strcat(imagePath, "/");
    strcat(imageName, _appId);
    strcat(imageName, "-");
    strcat(imageName, buf1);
    strcat(imagePath, imageName);
    printf("Image directory: %s\n", imageDir);
    printf("Image path: %s\n", imagePath);
    fflush(stdout);
    if (!File::exists(imagePath)) {
        fprintf(stderr, "Image does not exist!\n");
        return -1;
    }
    int rc = Steganography::getImageData(imageDir, imagePath, buf2, 2048, stegkey);
    printf("Image data: %s\n", buf2);
    if(rc) {
        fprintf(stderr, "Failed to decrypt the image.\n");
        return -1;
    }
    int currentViewCount;
    int totalViewCount;
    char owner[256];
    if(sscanf(buf2, "%[^;]%*c%d;%d;", owner, &totalViewCount, &currentViewCount) != 3) {
        fprintf(stderr, "Invalid image!\n");
        File::remove(decryptionPath);
        return -1;
    }
    printf("Total count: %d\nCurrent count: %d\n", totalViewCount, currentViewCount);
    fflush(stdout);
    return totalViewCount;
}

int MainWindow::_getRemainingViews(QString sender, QString id, const char *prefix, char *stegkey) const {
    char imagePath[PATH_MAX], imageDir[PATH_MAX],
            imageName[PATH_MAX], decryptionPath[PATH_MAX],
            decryptionName[PATH_MAX], buf2[1024];
    char *command = sender.toLocal8Bit().data();
    char *buf1 = id.toLocal8Bit().data();

    memset(imageDir, 0, PATH_MAX);
    memset(imagePath, 0, PATH_MAX);
    memset(imageName, 0, PATH_MAX);
    memset(buf2, 0, 1024);
    memset(decryptionName, 0, PATH_MAX);
    memset(decryptionPath, 0, PATH_MAX);

    strcpy(imageDir, "storage/");
    strcat(imageDir, command);
    strcpy(imagePath, imageDir);
    strcat(imagePath, prefix);
    strcat(imagePath, "/");
    strcat(imageName, _appId);
    strcat(imageName, "-");
    strcat(imageName, buf1);
    strcat(imagePath, imageName);
    printf("Image directory: %s\n", imageDir);
    printf("Image path: %s\n", imagePath);
    if (!File::exists(imagePath)) {
        fprintf(stderr, "Image does not exist!\n");
        return -1;
    }
    int rc = Steganography::getImageData(imageDir, imagePath, buf2, 2048, stegkey);
    printf("Image data: %s\n", buf2);
    if(rc) {
        fprintf(stderr, "Failed to decrypt the image.\n");
        return -1;
    }
    int currentViewCount;
    int totalViewCount;
    char owner[256];
    if(sscanf(buf2, "%[^;]%*c%d;%d;", owner, &totalViewCount, &currentViewCount) != 3) {
        fprintf(stderr, "Invalid image!\n");
        File::remove(decryptionPath);
        return -1;
    }
    printf("Total count: %d\nCurrent count: %d\n", totalViewCount, currentViewCount);
    fflush(stdout);
    return totalViewCount - currentViewCount;

}
void MainWindow::getPeerData(QString peerID)
{
    //get data to put in tables
}

QImage MainWindow::getImage(QString imgId, QString peerID)
{
    //get image
}

void MainWindow::updateImageViews(QString fileId, int newCount)
{
    char *buf1 = fileId.toLocal8Bit().data();
    char buf2[64];


    sprintf(buf2, "%d", newCount);

    Client *client = _clients.get(_currentPeer->getId(), _currentPeer->getUsername(), _currentPeer->getPeerAddress(), _currentPeer->getPortNumber());
    client->registerListener(_updateCompleteWrapper, this);
    client->updateImage(buf1, buf2);
}

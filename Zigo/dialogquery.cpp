#include "dialogquery.h"
#include "ui_dialogquery.h"
#include"mainwindow.h"
DialogQuery::DialogQuery(PeersMap &peers, QWidget *parent):
    QDialog(parent),
    ui(new Ui::DialogQuery),
    _peers(peers)
{
    ui->setupUi(this);


}

void DialogQuery::setData()
{
    typedef map<char *, Peer, StringCompare> ::iterator it_type;
    int i=0;
    for(it_type iterator = _peers.begin(); iterator != _peers.end(); iterator++)
    {
        QString key (iterator->first);
        _found x;
        QByteArray array = key.toLocal8Bit();
        QByteArray a(_peers[array.data()].getUsername());
        ui->lstResult->insertItem(i,QString(a));
        x._choiceName = QString(a);
        x._choiceID = key;
        QByteArray b(_peers[array.data()].getPeerAddress());
        x._choiceIP = QString(b);
        x._choicePort = QString::number(_peers[array.data()].getPortNumber());
        _peersdata.push_back(x);
        i++;
    }
}
QString DialogQuery::getChoice()
{
    return _chosen._choiceName;
}
QString DialogQuery::getChosenID()
{
    return _chosen._choiceID;
}
QString DialogQuery::getChosenPort()
{
    return _chosen._choicePort;
}
QString DialogQuery::getChosenIP()
{
    return _chosen._choiceIP;
}
DialogQuery::~DialogQuery()
{
    delete ui;
}
void DialogQuery::setSearchName(QString name)
{
    _search = name;
}
void DialogQuery::getData()
{
    // update _peers peersmap
}

void DialogQuery::on_lstResult_doubleClicked(const QModelIndex &index)
{
    QListWidgetItem *item = ui->lstResult->item(index.row());
    _chosen._choiceName = item->text();
    _chosen._choiceIP=_peersdata[index.row()]._choiceIP;
    _chosen._choiceID = _peersdata[index.row()]._choiceID;
    _chosen._choicePort=_peersdata[index.row()]._choicePort;
    this->close();

}

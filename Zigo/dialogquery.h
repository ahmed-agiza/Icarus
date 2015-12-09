#ifndef DIALOGQUERY_H
#define DIALOGQUERY_H

#include <QDialog>
#include <QString>
#include <QListWidgetItem>
#include"peer.h"
#include<vector>
namespace Ui {
class DialogQuery;
}

class DialogQuery : public QDialog
{
    Q_OBJECT

public:
    explicit DialogQuery(PeersMap &peers, QWidget *parent = 0);
    void setData();
    QString getChoice();
    ~DialogQuery();
    void setSearchName(QString name);
    QString getChosenID();
    QString getChosenIP();
    QString getChosenPort();
    void getData();
private slots:
    void on_lstResult_doubleClicked(const QModelIndex &index);

private:
    Ui::DialogQuery *ui;
    PeersMap &_peers;
    QString _search;
    struct _found{
        QString _choiceID;
        QString _choiceIP;
        QString _choicePort;
        QString _choiceName;
    };
    std::vector<_found> _peersdata;
    _found _chosen;

};

#endif // DIALOGQUERY_H

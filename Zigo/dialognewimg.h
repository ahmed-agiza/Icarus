#ifndef DIALOGNEWIMG_H
#define DIALOGNEWIMG_H

#include <QDialog>
#include <QString>
#include "peer.h"
namespace Ui {
class DialogNewImg;
}

class DialogNewImg : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewImg(QWidget *parent = 0);

    void setPeerLabel(QString);

    QString getImgPath();

    int getViews();
    void setPeerName(QString a);
    void setPeerID(QString a);
    void setPeerIP(QString a);
    void setPeerPort(QString a);

    void setPeer(QString);

    QString getImagePath();
    int getViewCount();

    ~DialogNewImg();

private slots:
    void on_btnUpload_clicked();

    void on_btnOk_clicked();

    void on_btnCancel_clicked();



private:
    Ui::DialogNewImg *ui;
    int views;
    QString path;
    Peer x;

};

#endif // DIALOGNEWIMG_H

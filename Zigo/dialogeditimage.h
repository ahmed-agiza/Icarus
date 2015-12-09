#ifndef DIALOGEDITIMAGE_H
#define DIALOGEDITIMAGE_H

#include <QDialog>

namespace Ui {
class DialogEditImage;
}

class DialogEditImage : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditImage(QWidget *parent = 0);
    ~DialogEditImage();
    void setImgLabel(QString img);
    void setPeerLabel(QString peer);
    int getImgViews();
    void setImgViews(int views);
    void setSenderId(QString id);
    void setImageId(QString id);
private slots:
    void on_btnOkEdit_clicked();

    void on_btnCancel_clicked();

private:
    Ui::DialogEditImage *ui;
    QString _fileId;
    QString _senderId;
};

#endif // DIALOGEDITIMAGE_H

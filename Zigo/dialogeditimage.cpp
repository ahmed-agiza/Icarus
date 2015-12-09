#include "dialogeditimage.h"
#include "ui_dialogeditimage.h"
#include "qdebug.h"

DialogEditImage::DialogEditImage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditImage)
{
    ui->setupUi(this);
}

DialogEditImage::~DialogEditImage()
{
    delete ui;
}

void DialogEditImage::on_btnOkEdit_clicked()
{
    accept();
}

void DialogEditImage::on_btnCancel_clicked()
{
    reject();
}
void DialogEditImage::setImgLabel(QString img)
{
    ui->lblImgName->setText(img);
}
void DialogEditImage::setPeerLabel(QString peer)
{
    ui->lblPeer->setText(peer);
}
int DialogEditImage::getImgViews()
{
    return ui->txtViews->text().toInt();
}
void DialogEditImage::setImgViews(int views)
{
    ui->txtViews->setText(views < 0? QString("0") : QString::number(views));
}

void DialogEditImage::setSenderId(QString id)
{
    _senderId = id;
}

void DialogEditImage::setImageId(QString id)
{
    _fileId = id;
}

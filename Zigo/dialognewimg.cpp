#include "dialognewimg.h"
#include "ui_dialognewimg.h"
#include "QFileDialog"

DialogNewImg::DialogNewImg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewImg)
{
    ui->setupUi(this);
    ui->txtNViews->setValidator(new QIntValidator(0, 15, this));
}

DialogNewImg::~DialogNewImg()
{
    delete ui;
}

void DialogNewImg::on_btnUpload_clicked()
{
    QFileDialog dialog(this) ;
    QString _fileName = dialog.getOpenFileName();
    if (!_fileName.isEmpty())
    {
        ui->txtPath->setText(_fileName);
        path=_fileName;
    }

}

void DialogNewImg::setPeerLabel(QString x)
{
    ui->lblPeer->setText(x);
}
void DialogNewImg::setPeerName(QString a)
{
    QByteArray l = a.toLocal8Bit();
    x.setUsername(l.data());
}
void DialogNewImg::setPeerID(QString a)
{
    QByteArray l = a.toLocal8Bit();
    x.setId(l.data());
}
void DialogNewImg::setPeerIP(QString a)
{
    QByteArray l = a.toLocal8Bit();
    x.setPeerAddress(l.data());
}
void DialogNewImg::setPeerPort(QString a)
{
    x.setPortNumber(a.toInt());
}

int DialogNewImg::getViews()
{
    return views;
}

QString DialogNewImg::getImgPath()
{
    return path;
}
void DialogNewImg::setPeer(QString peer)
{
    ui->lblPeer->setText(peer);
}

QString DialogNewImg::getImagePath() {
    return ui->txtPath->text();
}

int DialogNewImg::getViewCount() {
    return ui->txtNViews->text().toInt();
}
void DialogNewImg::on_btnOk_clicked()
{
    //check the path file is an image and a number of
    bool ok;
    views= ui->txtNViews->text().toInt(&ok,10);
    setResult(QDialog::Accepted);
    this->accept();
}

void DialogNewImg::on_btnCancel_clicked()
{
    views=-1;
    path="";
    setResult(QDialog::Rejected);
    this->reject();
}

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
//#include "imagehandler.h"

//include the files with containing the code
using namespace std;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(1);
    ui->txtPwd->setEchoMode(QLineEdit::Password);
    ui->txtConfirmPwd->setEchoMode(QLineEdit::Password);
    ui->txtUsrPwd->setEchoMode(QLineEdit::Password);
    ui->btnEdit->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::signUp(string _name, string _pwd, string _confirmPwd, string _dOB, string _profile )
{
    return true;
}

bool MainWindow::saveImg(vector<permission> _Permissions)
{
    return true;
}

void MainWindow::logIn(string _usrName, string _pwd, string *&_imageList, int& _numOfImg)
{
    //set it to the number of images or -1 if authentication failed
     _numOfImg =3;
    _imageList=new string[_numOfImg];
    for (int i =0; i <_numOfImg; i++)
    {
        _imageList[i]= "Image";
    }
}

void MainWindow::on_txtConfirmPwd_editingFinished()
{
    string _pwd =ui->txtPwd->text().toStdString();
    string _confirmPwd =ui->txtConfirmPwd->text().toStdString();

    if (_pwd != _confirmPwd && !_pwd.empty())
    {
        QMessageBox::warning(this, tr("Application"),
                                         tr("Your passwords do not match up!"));
        ui->txtConfirmPwd->clear();

    }
}

void MainWindow::on_btnLogIn_clicked()
{
    //Getting the username & password & validating them
    string _usrname =ui->txtUsrName->text().toStdString();
    string _pwd =ui->txtUsrPwd->text().toStdString();
    if (_usrname.empty() || _pwd.empty())
        QMessageBox::warning(this, tr("Application"),
                                     tr("Entr a Username and a Password!"));
    else
    {
        string * _imageList;
        int _numOfImg=0;
        logIn(_usrname, _pwd, _imageList,_numOfImg);
        if(_numOfImg !=-1)
        {
            ui->stackedWidget->setCurrentIndex(2);
            for (int i=0; i <_numOfImg; i++)
            {
                QString x=QString::fromStdString(_imageList[i]);
                ui->listWidget->addItem(new QListWidgetItem(x));
            }
        }
        else
        {
            QMessageBox::warning(this, tr("Application"),
                                         tr("Wrong Username or Password!"));
            ui->txtUsrName->clear();
            ui->txtUsrPwd->clear();
        }
    }
}

void MainWindow::on_btnGotoSignUp_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);

}

void MainWindow::on_btnGotoLogIn_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_btnSignUp_clicked()
{
    //Read data and Store data in Database
    string _name = ui->txtName->text().toStdString();
    string _pwd =ui->txtPwd->text().toStdString();
    string _confirmPwd =ui->txtConfirmPwd->text().toStdString();
    string _dOB =ui->drpDOB->text().toStdString();
    string _profile =ui->txtProfile->toPlainText().toStdString();
    if (_name.empty()||_pwd.empty()|| _confirmPwd.empty()|| _dOB.empty()|| _profile.empty())
    {
        QMessageBox::warning(this, tr("Application"),
                                         tr("You have to fill all the fields!"));
    }
    else
    {
        if (signUp(_name,_pwd,_confirmPwd,_dOB,_profile))
            ui->stackedWidget->setCurrentIndex(4);
        else
        {
            QMessageBox::warning(this, tr("Application"),
                                             tr("Error Creating Your Account!"));
            ui->txtPwd->clear();
            ui->txtConfirmPwd->clear();
        }
    }
}

void MainWindow::on_btnGotoUpload_clicked()
{
    QStringList x;
    x << "Name" << "Number of views";
    ui->stackedWidget->setCurrentIndex(4);
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(10);
    ui->tableWidget->setHorizontalHeaderLabels(x);
}

void MainWindow::on_btnUpload_clicked()
{
    QFileDialog dialog(this) ;
    QString _fileName = dialog.getOpenFileName();
    if (!_fileName.isEmpty())
        ui->txtImgPath->setText(_fileName);
} 


void MainWindow::on_btnSvae_clicked()
{
    QString _fileName= ui->txtImgPath->toPlainText();
    QFile _file(_fileName);

     if (!_file.open(QFile::ReadOnly) || !_fileName.endsWith(".jpg", Qt::CaseSensitive) )
     {
         QMessageBox::warning(this, tr("Application"),
                              tr("Cannot read file %1:\n%2.")
                              .arg(_fileName)
                              .arg(_file.errorString()));
         ui->txtImgPath->clear();
         return;
     }

     vector<permission> _Permissions;
     for (int i=0; i<ui->tableWidget->rowCount() ;i++)
     {

         if (ui->tableWidget->item(i,0)&&!ui->tableWidget->item(i,0)->text().isEmpty())
        {
             bool _ok=true;
             permission x;
             x.name = ui->tableWidget->item(i,0)->text().toStdString();
             if (!ui->tableWidget->item(i,1)|| ui->tableWidget->item(i,1)->text().isEmpty())
                 x.views=1;
             else
                 x.views = ui->tableWidget->item(i,0)->text().toInt(&_ok,10);
             if (_ok)
                 _Permissions.push_back(x);
         }


     }


     //Get the data and send them to the server

     if (saveImg(_Permissions))
     {
         QMessageBox::information(this, tr("Application"), tr("Saving Finished"));
         ui->stackedWidget->setCurrentIndex(2);

     }
     else
         QMessageBox::warning(this, tr("Application"),
                                          tr("Error Connecting to Server!"));


}

void MainWindow::on_btnLogOut1_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->btnEdit->setVisible(false);
}

void MainWindow::on_btnLogOut2_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->btnEdit->setVisible(false);
}

void MainWindow::on_btnBackList_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_btnLogOut3_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->btnEdit->setVisible(false);
}

void MainWindow::on_btnBackList2_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    ui->stackedWidget->setCurrentIndex(3);
    QString x= item->text() ;
    QImage image("C:/Users/Sarah Soliman/Pictures/backgrounds/3d_balloons_in_the_sky-wide.jpg");
    //check for permission
    //if no permission
    //if there is a permission
    //get the image from the server & display it & increment the number of views

     //if the owner
     ui->btnEdit->setVisible(true);

}

void MainWindow::on_btnEdit_clicked()
{
    ui->stackedWidget->setCurrentIndex(4);
}

void MainWindow::on_btnLogOut4_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->btnEdit->setVisible(false);
}

void MainWindow::on_btnBackImg_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);

}

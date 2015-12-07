#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "vector"
#include "string.h"
#include "permission.h"
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    bool signUp(string _name, string _pwd, string _confirmPwd, string _dOB, string _profile );

    void logIn (string _usrName, string _pwd, string *&_imageList, int & _numOfImg);

    bool saveImg(vector<permission> _Permissions);

    void on_txtConfirmPwd_editingFinished();

    void on_btnLogIn_clicked();

    void on_btnGotoSignUp_clicked();

    void on_btnGotoLogIn_clicked();

    void on_btnSignUp_clicked();

    void on_btnUpload_clicked();

    void on_btnGotoUpload_clicked();

    void on_btnSvae_clicked();

    void on_btnLogOut1_clicked();

    void on_btnLogOut2_clicked();

    void on_btnBackList_clicked();

    void on_btnLogOut3_clicked();

    void on_btnBackList2_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_btnEdit_clicked();

    void on_btnLogOut4_clicked();

    void on_btnBackImg_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

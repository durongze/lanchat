/********************************************************************************
* File Name:	login.h
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>

namespace Ui {
    class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();
private slots:
    void on_btnRegister_clicked();
    void on_btnLogin_clicked();
    void on_toolButton_clicked();

private:
    Ui::Login *ui;
};

#endif // LOGIN_H

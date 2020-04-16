/********************************************************************************
* File Name:	register.h
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>

namespace Ui {
    class Register;
}

class Register : public QDialog
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = 0);
    ~Register();
private:
    Ui::Register *ui;

private slots:
    void on_btnQuit_clicked();
    void on_btnOk_clicked();
};

#endif // REGISTER_H

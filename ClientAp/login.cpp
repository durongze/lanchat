/********************************************************************************
* File Name:	login.cpp
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#include "login.h"
#include "ui_login.h"
#include <QMessageBox>
#include <QtSql>
#include <QSqlQuery>
#include "database.h"
#include "widget.h"
#include "register.h"
#include "dataconfig.h"
Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
     ui->setupUi(this);
}
Login::~Login()
{
    delete ui;
}

void Login::on_btnLogin_clicked()
{   
	QString msg;
	if (!database::Check(ui->leUsername->text(), ui->lePasswd->text(), msg))
	{
		QMessageBox::about(this, tr("error"), msg);
		return;
	}
    Widget *w = new Widget;
    w->SetName(ui->leUsername->text());
    w->show();
    this->close();
}
/* 打开注册界面 */
void Login::on_btnRegister_clicked()
{
    Register *reg = new Register;
    reg->show();
}

void Login::on_toolButton_clicked()
{
    DataConfig  *cfg=new DataConfig;
    cfg->show();
}

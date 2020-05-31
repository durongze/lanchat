/********************************************************************************
* File Name:	register.cpp
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#include "register.h"
#include "ui_register.h"
//添加的头文件
#include <QMessageBox>
#include <QtSql>
#include <qsqlquery.h>
#include "database.h"

Register::Register(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
}

Register::~Register()
{
    delete ui;
}

void Register::on_btnOk_clicked()
{
    QString username, passwd, confirmPasswd, sex, address, phoneNum,NiName;
    if(!database::Open()){
        QMessageBox::about(0,tr("error"),tr("open database fail\n%1"));
        return ;
    }
    if(!ui->leUsername->text().isEmpty())
        username = ui->leUsername->text();
    else{
        QMessageBox::about(0,tr("prompt"),tr("username is null"));
        database::Close();
        return ;
    }
    if(!ui->lePasswd->text().isEmpty())
        passwd = ui->lePasswd->text();
    else{
        QMessageBox::about(0,tr("prompt"),tr("password is null"));
        database::Close();
        return ;
    }
    if(!ui->leNiName->text().isEmpty())
        NiName = ui->leNiName->text();
    if(!ui->leConfimPasswd->text().isEmpty())
        confirmPasswd = ui->leConfimPasswd->text();
    else{
        QMessageBox::about(0,tr("prompt"),tr("password is null"));
        database::Close();
        return ;
    }
    if(!ui->cbxSex->currentText().isEmpty())
        sex = ui->cbxSex->currentText();
    if(!ui->leAddress->text().isEmpty())
        address=ui->leAddress->text();
    if(!ui->lePhoneNum->text().isEmpty())
        phoneNum = ui->lePhoneNum->text();
    if(passwd.compare(confirmPasswd) != 0){
        QMessageBox::about(0,tr("prompt"),tr("password is error"));
        database::Close();
        return ;
    }
    QSqlQuery query;
    int id;
    query.exec("select max(ID) from user");
    if(!query.next())
        return;
    id = query.value(0).toInt()+1;
    //sql语句
    QString insertstr="insert into user(ID,userName,passwd,niName, sex, phone, address) values('"+QString::number(id)+"','"+username+"','"+passwd+"',"
                      "'"+NiName+"','"+sex+"','"+phoneNum+"','"+address+"')";
    //执行sql语句
    query.exec(insertstr);
    //sql语句是否成功执行
    if(!query.isActive()){
        QMessageBox::about(0,tr("结果"),tr("注册失败\n%1").arg(query.lastError().text()));
        database::Close();
        return ;
    }
    QMessageBox::about(0,tr("结果"),tr("注册成功"));
    database::Close();
    this->close();
}

void Register::on_btnQuit_clicked()
{
    this->close();
}

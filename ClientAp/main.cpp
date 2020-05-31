/********************************************************************************
* File Name:	main.cpp
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#include <QApplication>
#include <QTextCodec>
#include "login.h"
#include "database.h"
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "trace.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    QCoreApplication::addLibraryPath("./plugins");
    qDebug() << QSqlDatabase::drivers();
    Login login;
    login.show();
    return a.exec();
}

/********************************************************************************
* File Name:	database.h
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#ifndef DATABASE_H
#define DATABASE_H
#include <QSqlDatabase>
#include <QString>
typedef enum tagDATABASE_STATE_STEP
{
    DATABASE_STATE_STEP_INIT,
    DATABASE_STATE_STEP_IP_ERR,
    DATABASE_STATE_STEP_USER_ERR,
    DATABASE_STATE_STEP_PASS_ERR,
    DATABASE_STATE_STEP_COMPLETE,
    DATABASE_STATE_STEP_OPEN,
    DATABASE_STATE_STEP_CLOSE
}DATABASE_STATE_STEP;
class database
{
    static QString m_dbIp;
    static QString m_dbUserName;
    static QString m_dbPassWord;
public:
    static int m_dbStateFlag;
    static QSqlDatabase m_db;
    database();
    static bool Config(QString  ip = NULL,  QString userName = NULL,   QString passWord = NULL);
    static bool Open();
    static void Close();
	static bool Check(QString& user, QString& pass, QString& msg);
};

#endif // DATABASE_H

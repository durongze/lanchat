/********************************************************************************
* File Name:	database.cpp
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#include <QMessageBox>
#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QtSql>

/* 静态成员变量必须初始化，否则没机会分配空间了 */
QSqlDatabase database::m_db = QSqlDatabase::addDatabase("QMYSQL");

int database:: m_dbStateFlag = DATABASE_STATE_STEP_INIT;
QString database:: m_dbIp = 0;
QString database:: m_dbUserName = 0;
QString database:: m_dbPassWord = 0;

database::database()
{

}
bool database::Config(QString  ip,  QString userName,  QString passWord)
{
    m_dbStateFlag = DATABASE_STATE_STEP_INIT;
    m_dbIp = ip;
    m_dbUserName = userName;
    m_dbPassWord = passWord;

    if (!m_dbIp.isEmpty())
    {
        qDebug(m_dbIp.toStdString().c_str());
        m_db.setHostName(QLatin1String(m_dbIp.toStdString().c_str()));
    }
    else
    {
        qDebug("ip is empty!");
        m_dbStateFlag = DATABASE_STATE_STEP_IP_ERR;
    }
    if (!m_dbUserName.isEmpty())
    {
        qDebug(m_dbUserName.toStdString().c_str());
        m_db.setUserName(m_dbUserName.toStdString().c_str());
    }
    else
    {
        qDebug("userName is empty!");
        m_dbStateFlag = DATABASE_STATE_STEP_USER_ERR;
    }
    if (!m_dbPassWord.isEmpty())
    {
        qDebug(m_dbPassWord.toStdString().c_str());
        m_db.setPassword(m_dbPassWord.toStdString().c_str());
    }
    else
    {
        qDebug("password is empty!");
        m_dbStateFlag = DATABASE_STATE_STEP_PASS_ERR;
    }
    m_db.setDatabaseName("qqdb");

    if (DATABASE_STATE_STEP_INIT  == m_dbStateFlag )
    {
       m_dbStateFlag = DATABASE_STATE_STEP_COMPLETE;
    }
    if (DATABASE_STATE_STEP_COMPLETE != m_dbStateFlag )
    {
        return false;
    }
    else
    {
        qDebug(__FUNCTION__);
        return true;
    }
}

bool database::Open()
{
    if (DATABASE_STATE_STEP_COMPLETE != m_dbStateFlag)
    {
        return false;
    }
    if(!m_db.open())
    {
        qDebug() << database::m_db.lastError().text();
        return false;
    }
    else
    {
        m_dbStateFlag = DATABASE_STATE_STEP_OPEN;
    }
    //m_db.exec("SET NAMES 'GBK'");
    qDebug(__FUNCTION__);
    return true;
}

void database::Close()
{
    if (DATABASE_STATE_STEP_OPEN != m_dbStateFlag)
    {
        return ;
    }
    m_db.close();
    m_dbStateFlag = DATABASE_STATE_STEP_COMPLETE;
}

bool database::Check(QString& user, QString& pass, QString& msg)
{
	return true;
	if (DATABASE_STATE_STEP_COMPLETE != database::m_dbStateFlag)
	{
		msg = QString("database config is error\n");
		return false;
	}
	if (!database::Open()) {
		msg = QString("open database failed \n") + database::m_db.lastError().text();
		return false;
	}
	QSqlQuery query;
	query.exec("select * from user where userName='" + user + "' AND passwd='" + pass + "'");
	if (!query.isActive()) {
		msg = QString("log in error\n") + (query.lastError().text());
		database::Close();
		return false;
	}
	if (query.size() == 0)
	{
		msg = QString("user no exist\n") + (query.lastError().text());
		database::Close();
		return false;
	}
	database::Close();
	return true;
}
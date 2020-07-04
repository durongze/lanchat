/********************************************************************************
* File Name:	widget.h
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtNetwork>
#include <QtGui>
#include "chatwidgit.h"
#include "filesender.h"
#include <qmap.h>
#include <QSystemTrayIcon>
#include <QPushButton>
#include <QMessageBox>
namespace Ui {
    class Widget;
}
//枚举变量标志信息的类型，分别为消息，新用户加入，和用户退出
enum MessageType
{
    Message,
    NewParticipant,ParticipantLeft,
    FileName,FileRefuse,
    AskStartChat,StartChat,RefuseChat,
    CameraMessage,CameraRefuse,
    SpeekerMessage,SpeekerRefuse,
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    QString getUserName();
    void SetName(QString);
protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *);
    void sendMessage(MessageType type,QString serverAddress="",QString message = "");
    void newParticipant(QString localHostName,QString ipAddress,QString sex);
    void participantLeft(QString localHostName,QString time);  
    void hasPendingFile(QString localhostname,QString serverAddress,QString clientAddress,QString fileName);
    void hasPendingCamera(QString localHostName, QString serverAddress,QString clientAddress,QString CameraName);
    void hasPendingSpeeker(QString localHostName, QString serverAddress,QString clientAddress,QString speekerName);
private:
    QString Name;
    Ui::Widget *ui;
    QUdpSocket *udpSocket;
    QMap<QString,ChatWidgit *> list;
    qint16 port;
    FileSender * Sender;
    QString getIP();
private slots:
    void on_btnAll_clicked();
    void on_btnCancel_clicked();
    void on_tableWidget_doubleClicked(QModelIndex index);

    void closeFromChat(QString);     
    void sendMessagesToMain(QString,QString);
    void processPendingDatagramsStartChat(QDataStream& in);
    void processPendingDatagramsAskStartChat(QDataStream& in);
    void processPendingDatagramsMessage(QDataStream& in);
    void processPendingDatagrams();
    void sentFileName(QString,QString);
    void hasFileToSend(QString,QString);

signals:
    void sendMessagesZ(QString address,QString first,QString second);
 /*托盘*/
private:
    void showMessage(const char *msg);
    void createActions();
    void createTrayIcon();

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    QPushButton* button;
private slots:
    void showMessage();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void on_tableWidget_clicked(const QModelIndex &index);
    void contextMenuEvent( QContextMenuEvent * event );
};

#endif // WIDGET_H

/********************************************************************************
* File Name:	filesender.h
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#ifndef FILESENDER_H
#define FILESENDER_H

#include <QDialog>
#include <QTcpServer>
#include <QFile>
#include <QTime>

namespace Ui {
    class FileSender;
}

class FileSender : public QDialog
{
    Q_OBJECT

public:
    explicit FileSender(QWidget *parent = 0);
    ~FileSender();
    void refusedFile();
    void initSender();
    void SenderStart(QString,QString);
protected:
    void changeEvent(QEvent *e);
private:
    Ui::FileSender *ui;
    qint16 tcpPort;
    QTcpServer *fileSender;
    QString fileName;
    QString theFileName;
    QFile *localFile;

    qint64 TotalBytes;
    qint64 bytesWritten;
    qint64 bytesToWrite;
    qint64 loadSize;
    QByteArray outBlock;//缓存一次发送的数据
    QTcpSocket*clientConnection;
    QTime time;//计时器

private slots:
    void on_btnQuit_clicked();
    void sendMessage();
    void updateClientProgress(qint64 numBytes);
signals:
    void sendFileName(QString fileName,QString addr);
};

#endif // FILESENDER_H

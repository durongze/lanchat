/********************************************************************************
* File Name:	filereceiver.h
* Description:	��15��ʵ��ѵ��
* Reference book:��Linux������Qt4ͼ�ν�����MySQL��̡�����е��ҵ������.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#ifndef FILERECEIVER_H
#define FILERECEIVER_H

#include <QDialog>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>
#include <QTime>
namespace Ui {
    class fileReceiver;
}

class fileReceiver : public QDialog
{
    Q_OBJECT

public:
    explicit fileReceiver(QWidget *parent = 0);
    ~fileReceiver();
    void setHostAddress(QHostAddress address);
    void setFileName(QString fileName){localFile = new QFile(fileName);}
    void newConnect();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::fileReceiver *ui;
    QTcpSocket *FileReceiver;
    quint16 blockSize;
    QHostAddress hostAddress;
    qint16 tcpPort;

    qint64 TotalBytes;
    qint64 bytesReceived;
    qint64 bytesToReceive;
    qint64 fileNameSize;
    QString fileName;
    QFile *localFile;
    QByteArray inBlock;

    QTime time;

private slots:
    void on_btnClose_clicked();
    void on_btnCancel_clicked();
    void readMessage();
    void displayError(QAbstractSocket::SocketError);
};


#endif // FILERECEIVER_H

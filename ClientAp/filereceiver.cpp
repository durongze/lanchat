/********************************************************************************
* File Name:	filereceiver.cpp
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#include "filereceiver.h"
#include "ui_filereceiver.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

//接收方 连接信号槽  有数据读入 触发readmessage
fileReceiver::fileReceiver(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::fileReceiver)
{
    ui->setupUi(this);
    this->setFixedSize(350,180);

    TotalBytes = 0;
    bytesReceived = 0;
    fileNameSize = 0;

    FileReceiver = new QTcpSocket(this);
    tcpPort = 6666;
    connect(FileReceiver,SIGNAL(readyRead()),this,SLOT(readMessage()));
    connect(FileReceiver,SIGNAL(error(QAbstractSocket::SocketError)),this,
            SLOT(displayError(QAbstractSocket::SocketError)));

}

fileReceiver::~fileReceiver()
{
    delete ui;
}

void fileReceiver::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//设置服务器地址并连接服务器
void fileReceiver::setHostAddress(QHostAddress address)
{
    hostAddress = address;
    newConnect();
}
/* 连接服务器 */
void fileReceiver::newConnect()
{
    blockSize = 0;
    FileReceiver->abort();
    FileReceiver->connectToHost(hostAddress,tcpPort);
    time.start();
}

/* 槽 读取收到的文件 */
void fileReceiver::readMessage()
{
    QDataStream in(FileReceiver);
    in.setVersion(QDataStream::Qt_4_6);

    float useTime = time.elapsed();
    if(bytesReceived <= sizeof(qint64)*2){
        if((FileReceiver->bytesAvailable() >= sizeof(qint64)*2) && (fileNameSize == 0)){
            in>>TotalBytes>>fileNameSize;
            bytesReceived += sizeof(qint64)*2;
        }
        if((FileReceiver->bytesAvailable() >= fileNameSize) && (fileNameSize != 0)){
            in>>fileName;
            bytesReceived +=fileNameSize;

            if(!localFile->open(QFile::WriteOnly)){
                QMessageBox::warning(this,tr("应用程序"),tr("无法读取文件 %1:\n%2.").arg(fileName).arg(localFile->errorString()));
                return;
            }
        }
        else
            return;
    }
    if(bytesReceived < TotalBytes){
        bytesReceived += FileReceiver->bytesAvailable();
        inBlock = FileReceiver->readAll();
        localFile->write(inBlock);
        inBlock.resize(0);
    }
    ui->progressBar->setMaximum(TotalBytes);
    ui->progressBar->setValue(bytesReceived);
    qDebug()<<bytesReceived<<"received"<<TotalBytes;

    double speed = bytesReceived / useTime;
    ui->FileReceiverStatusLabel->setText(tr("received %1MB\n").arg(bytesReceived / (1024*1024)));
    ui->FileReceiverStatusLabel->setText(tr("(%1MB/s) \n").arg(speed*1000/(1024*1024),0,'f',2));
    ui->FileReceiverStatusLabel->setText(tr("cnt %1MB\n").arg(TotalBytes / (1024 * 1024)));
    ui->FileReceiverStatusLabel->setText(tr("used time:%1s\n").arg(useTime/1000,0,'f',0));
    ui->FileReceiverStatusLabel->setText(tr(" time:%1s \n").arg(TotalBytes/speed/1000 - useTime/1000,0,'f',0));

    if(bytesReceived == TotalBytes)
    {
        FileReceiver->close();
        ui->FileReceiverStatusLabel->setText(tr("接收文件 %1 完毕").arg(fileName));
        localFile->close();   //接收完文件后，一定要关闭，不然可能出问题
    }
}
void fileReceiver::displayError(QAbstractSocket::SocketError socketError) //错误处理
{
    switch(socketError)
    {
        case QAbstractSocket::RemoteHostClosedError : break;
        default : qDebug() << FileReceiver->errorString();
    }
}
//中途取消的操作
void fileReceiver::on_btnCancel_clicked()
{
    FileReceiver->abort();
}

void fileReceiver::on_btnClose_clicked()
{
    FileReceiver->abort();
    this->close();
}

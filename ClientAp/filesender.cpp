/********************************************************************************
* File Name:	filesender.cpp
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#include "filesender.h"
#include "ui_filesender.h"
#include <QTcpSocket>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidget>

/* 初始化 新连接 信号槽 */
FileSender::FileSender(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileSender)
{
    ui->setupUi(this);
    this->setFixedSize(350,180);
    tcpPort = 6666;
    fileSender = new QTcpServer();
    connect(fileSender,SIGNAL(newConnection()),this,SLOT(sendMessage()));
}

FileSender::~FileSender()
{
    delete ui;
}

void FileSender::changeEvent(QEvent *e)
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

/* 当有新连接时触发 新连接在接受方同意时 发送此信号 */
void FileSender::sendMessage()
{
    clientConnection = fileSender->nextPendingConnection();
    connect(clientConnection,SIGNAL(bytesWritten(qint64)),this,SLOT(updateClientProgress(qint64)));

    ui->serverStatusLabel->setText(tr("start send %1 .").arg(theFileName));

    localFile = new QFile(fileName);
    if(!localFile->open((QFile::ReadOnly))) {
        QMessageBox::warning(this,tr("SendMsg"),tr("Read file %1:\n%2").arg(fileName).arg(localFile->errorString()));
        return;
    }
    TotalBytes = localFile->size();
    QDataStream sendOut(&outBlock,QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_6);
    time.start();
    QString currentFile = fileName.right(fileName.size() - fileName.lastIndexOf('/')-1);
    sendOut<<qint64(0)<<qint64(0)<<currentFile;
    TotalBytes += outBlock.size();
    sendOut.device()->seek(0);
    sendOut<<TotalBytes<<qint64((outBlock.size()-sizeof(qint64)*2));
    bytesToWrite = TotalBytes - clientConnection->write(outBlock);
    qDebug()<<currentFile<<TotalBytes;
    outBlock.resize(0);
}

/* 根据发送情况实时更新进度条 */
void FileSender::updateClientProgress(qint64 numBytes)
{
    bytesWritten += (int)numBytes;
    if(bytesToWrite > 0){
        outBlock = localFile->read(qMin(bytesToWrite,loadSize));
        bytesToWrite -= (int)clientConnection->write(outBlock);
        outBlock.resize(0);
    }
    else{
        localFile->close();
    }
    ui->progressBar->setMaximum(TotalBytes);
    ui->progressBar->setValue(bytesWritten);

   float useTime = time.elapsed();
   double speed = bytesWritten / useTime;
   ui->serverStatusLabel->setText(tr("send %1MB").arg(bytesWritten / (1024*1024)));
   ui->serverStatusLabel->setText(tr("(%1MB/s)").arg(speed*1000/(1024*1024),0,'f',2));
   ui->serverStatusLabel->setText(tr("cnt %1MB").arg(TotalBytes / (1024 * 1024)));
   ui->serverStatusLabel->setText(tr("used time:%1s\n").arg(useTime/1000,0,'f',0));
   ui->serverStatusLabel->setText(tr("time:%1s").arg(TotalBytes/speed/1000 - useTime/1000,0,'f',0));
    if(bytesWritten == TotalBytes)
        ui->serverStatusLabel->setText(tr("传送文件 %1 成功").arg(theFileName));
}

/* 被拒绝时触发    主窗体调用 */
void FileSender::refusedFile()
{
    fileSender->close();
    ui->serverStatusLabel->setText(tr("refuse by user."));
}

/* 当需要发送文件时  主窗体调用初始化 */
void FileSender::initSender()
{
    loadSize = 4*1024;
    TotalBytes = 0;
    bytesWritten = 0;
    bytesToWrite = 0;
    ui->progressBar->reset();

    fileSender->close();
}

/* 选择完文件 初始化完成后 自动调用 等待  触发主窗口通知接收方 */
void FileSender::SenderStart(QString file, QString addr)
{
    this->fileName = file;
    this->theFileName = fileName.right(fileName.size() - fileName.lastIndexOf('/')-1);

    if(!fileSender->listen(QHostAddress::Any,tcpPort))
    {
        qDebug() << fileSender->errorString();
        close();
        return;
    }
    ui->serverStatusLabel->setText(tr("waiting receive"));
    emit sendFileName(theFileName,addr);
   // this->close();
}

/* 发送完成后点击退出 */
void FileSender::on_btnQuit_clicked()
{
    if(fileSender->isListening())
        fileSender->close();
    this->close();
}

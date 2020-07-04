/********************************************************************************
* File Name:	widget.cpp
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#include "widget.h"
#include "trace.h"
#include "ui_widget.h"
#include "chatwidgit.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QSystemTrayIcon>
#include <QPushButton>
#include <QMessageBox>
#include <QMenu>

/* 初始化函数  文件传输只有这里一个变量 所以直接连接信号与槽 */
Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->resize(300, 650);
    this->move(1000, 50);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    Sender = new FileSender(this);
    udpSocket = new QUdpSocket(this);
    port = 45454;
    udpSocket->bind(port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagrams()));
    connect(Sender,SIGNAL(sendFileName(QString,QString)),this,SLOT(sentFileName(QString,QString)));
	/* 建立系统托盘图标 */
    setMinimumSize(200,100);
    QIcon icon;
	icon.addFile(QString(":/new/prefix1/image/tray.svg"), QSize(), QIcon::Normal, QIcon::On);
    setWindowIcon(icon);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("a trayicon example");
    createActions();
    createTrayIcon();
    trayIcon->show();
    setWindowTitle(tr("Systray"));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    // connect(button, SIGNAL(clicked()), this, SLOT(showMessage()));
}

/* 公有函数  在登录成功后调用  设置昵称  并广播加入网络 */
void Widget::SetName(QString name)
{
    Name = name;
    sendMessage(NewParticipant);
}

void Widget::processPendingDatagramsStartChat(QDataStream& in)
{
    QString localHostName, ipAddress, secretAddress;
    in  >> localHostName >> ipAddress >> secretAddress;
    if(secretAddress == getIP())
    {
        ChatWidgit *chatView = new ChatWidgit();
        list.insert(ipAddress,chatView);
        connect(chatView,SIGNAL(closeChat(QString)),this,SLOT(closeFromChat(QString)));
        connect(this,SIGNAL(sendMessagesZ(QString,QString,QString)),chatView,SLOT(setMessage(QString,QString,QString)));
        connect(chatView,SIGNAL(sendMessagesFromChat(QString,QString)),this,SLOT(sendMessagesToMain(QString,QString)));
        connect(chatView,SIGNAL(sendFile(QString,QString)),this,SLOT(hasFileToSend(QString,QString)));
        chatView->setSecAddr(ipAddress,Name);
        chatView->show();
    }
}

void Widget::processPendingDatagramsAskStartChat(QDataStream& in)
{
    int btn;
    QString prompt;
    QString localHostName, ipAddress, secretAddress;
    in  >> localHostName >> ipAddress >> secretAddress;
    if (secretAddress == getIP()) {
        prompt = tr("from %1 %2 ,receive (Yes/No)?").arg(ipAddress).arg(localHostName);
        btn = QMessageBox::information(this,tr("agree chat"), prompt, QMessageBox::Yes,QMessageBox::No);
        if (btn == QMessageBox::Yes) {
            ChatWidgit *chatView = new ChatWidgit();
            list.insert(ipAddress,chatView);
            connect(chatView,SIGNAL(closeChat(QString)),this,SLOT(closeFromChat(QString)));
            connect(this,SIGNAL(sendMessagesZ(QString,QString,QString)),chatView,SLOT(setMessage(QString,QString,QString)));
            connect(chatView,SIGNAL(sendMessagesFromChat(QString,QString)),this,SLOT(sendMessagesToMain(QString,QString)));
            connect(chatView,SIGNAL(sendFile(QString,QString)),this,SLOT(hasFileToSend(QString,QString)));
            chatView->setSecAddr(ipAddress,Name);
            chatView->show();
            sendMessage(StartChat,ipAddress);
        } else if(btn == QMessageBox::No) {
            sendMessage(RefuseChat,ipAddress);
        }
    }
}

/* 开始传送消息 查看窗口是否打开  并传送消息，  之前是否请求？
        未打开就打开  打开聊天窗口会连接一些 信号与槽
        分别是 发送消息 接收消息 发送文件 关闭窗口 */
void Widget::processPendingDatagramsMessage(QDataStream& in)
{
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString localHostName, ipAddress, message, secretAddress, sex;
    in  >> localHostName >> ipAddress >> secretAddress >> message;
    // QMessageBox::information(this,tr("0"),tr("%1sec\n%2").arg(secretAddress).arg(ipAddress));
    if(secretAddress == getIP() )
    {
        // QMessageBox::information(this,tr("0"),tr("%1sec").arg(secretAddress));
        if(list.contains(ipAddress)){
            list[ipAddress]->show();
            emit sendMessagesZ(ipAddress, tr("[ %1 ] %2").arg(localHostName).arg(time), message);
        }else{
            ChatWidgit *chatView = new ChatWidgit();
            list.insert(ipAddress,chatView);
            connect(chatView,SIGNAL(closeChat(QString)),this,SLOT(closeFromChat(QString)));
            connect(this,SIGNAL(sendMessagesZ(QString,QString,QString)),chatView,SLOT(setMessage(QString,QString,QString)));
            connect(chatView,SIGNAL(sendMessagesFromChat(QString,QString)),this,SLOT(sendMessagesToMain(QString,QString)));
            connect(chatView,SIGNAL(sendFile(QString,QString)),this,SLOT(hasFileToSend(QString,QString)));
            chatView->setSecAddr(ipAddress,Name);
            chatView->show();
            emit sendMessagesZ(ipAddress,tr("[ %1 ] %2").arg(localHostName).arg(time),message);
        }
    }else if(secretAddress == "NULL") {
        if(list.contains("NULL")){
            list["NULL"]->show();
            emit sendMessagesZ(secretAddress,tr("[ %1 ] %2").arg(localHostName).arg(time),message);
        }else{
            ChatWidgit *chatView = new ChatWidgit();
            list.insert(secretAddress,chatView);
            connect(chatView,SIGNAL(closeChat(QString)),this,SLOT(closeFromChat(QString)));
            connect(this,SIGNAL(sendMessagesZ(QString,QString,QString)),chatView,SLOT(setMessage(QString,QString,QString)));
            connect(chatView,SIGNAL(sendMessagesFromChat(QString,QString)),this,SLOT(sendMessagesToMain(QString,QString)));
            connect(chatView,SIGNAL(sendFile(QString,QString)),this,SLOT(hasFileToSend(QString,QString)));
            chatView->setSecAddr(secretAddress,Name);
            chatView->show();
            emit sendMessagesZ(secretAddress,tr("[ %1 ] %2").arg(localHostName).arg(time),message);
        }
    }
}
/* 接收数据  并根据type 分类 */
void Widget::processPendingDatagrams()
{
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(),datagram.size());
        QDataStream in(&datagram,QIODevice::ReadOnly);
        int messageType;
        in >> messageType;
        QString localHostName, ipAddress, message, secretAddress, sex;
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        switch(messageType)
        {
        case RefuseChat:
            {
                in  >>localHostName >>ipAddress >> secretAddress;
                if(secretAddress == getIP())
                    QMessageBox::information(this,tr("Refuse"),tr("from %1 %2  refuse chat.").arg(ipAddress).arg(localHostName));
                break;
            }
        case StartChat:
            processPendingDatagramsStartChat(in);
            break;
        case AskStartChat:
            processPendingDatagramsAskStartChat(in);
            break;
        case Message:
            processPendingDatagramsMessage(in);
            break;
        case NewParticipant:
            {
                in  >>localHostName >>ipAddress >>sex;
                newParticipant(localHostName,ipAddress,sex);
                break;
            }
        case ParticipantLeft:
            {
                in  >>localHostName;
                participantLeft(localHostName,time);
                break;
            }
        case FileName:
            {
                in  >>localHostName >> ipAddress;
                QString clientAddress,fileName;
                in >> clientAddress >> fileName;
                QMessageBox::information(this,tr(""),tr("%1ipaddr\n%2sev\n%3file").arg(ipAddress).arg(clientAddress).arg(message));
                hasPendingFile(localHostName,ipAddress,clientAddress,fileName);
                break;
            }
        case CameraMessage:
            {
                in  >>localHostName >> ipAddress;
                QString clientAddress,cameraName;
                in >> clientAddress;
                hasPendingCamera(localHostName,ipAddress,clientAddress,cameraName);
                break;
            }
        case SpeekerMessage:
            {
                in  >>localHostName >> ipAddress;
                QString clientAddress,speekerName;
                in >> clientAddress;
                hasPendingSpeeker(localHostName,ipAddress,clientAddress,speekerName);
                break;
            }
        case FileRefuse:
            {
                in  >> localHostName;
                QString serverAddress;
                in >> ipAddress >> serverAddress;

                if(getIP() == serverAddress)
                {
                    this->Sender->refusedFile();
                }
                break;
            }
        }
    }
}


/* 接收方有文件需要接收，选择同意后拒绝  并打开接收窗口 连接发送方 */
void Widget::hasPendingCamera(QString localHostName, QString serverAddress, QString clientAddress,QString cameraName)
{
    int btn;
    QString prompt;
    QString ipAddress = getIP();
    if (ipAddress == clientAddress) {
        prompt = tr("from %1 %2:%3, receive(Yes/No)").arg(localHostName).arg(serverAddress).arg(cameraName);
        btn = QMessageBox::information(this, tr("接受视频"), prompt, QMessageBox::Yes,QMessageBox::No);
        if(btn == QMessageBox::Yes)
        {
            TRACE(EN_CTRACE_INFO,"startReceiveCamera");
        }
        else{
            sendMessage(CameraRefuse,serverAddress);
        }
    }
}

void Widget::hasPendingSpeeker(QString localHostName, QString serverAddress,
                            QString clientAddress,QString speekerName)
{
    int btn;
	QString prompt;
    QString ipAddress = getIP();
    if(ipAddress == clientAddress)
    {
        prompt = tr("from %1 %2:%3,receive(Yes/No)?").arg(localHostName).arg(serverAddress).arg(speekerName);
        btn = QMessageBox::information(this,tr("receive"), prompt, QMessageBox::Yes,QMessageBox::No);
        if(btn == QMessageBox::Yes)
        {
            TRACE(EN_CTRACE_INFO,"startReceiveSpeeker");
        }
        else{
            sendMessage(SpeekerRefuse,serverAddress);
        }
    }
}

void Widget::hasPendingFile(QString localHostName, QString serverAddress,
                            QString clientAddress,QString fileName)
{
    int btn;
    QString prompt;
    QString ipAddress = getIP();
    if (ipAddress == clientAddress) {
        prompt = tr("from %1 %2 file:%3,receive(yes/no)?").arg(localHostName).arg(serverAddress).arg(fileName);
        btn = QMessageBox::information(this,tr("receive file"), prompt, QMessageBox::Yes,QMessageBox::No);
        if(btn == QMessageBox::Yes)
        {
            QString name = QFileDialog::getSaveFileName(0,tr("save file"),fileName);
            if(!name.isEmpty())
            {
                fileReceiver *receiver = new fileReceiver(this);
                receiver->setFileName(name);
                receiver->setHostAddress(QHostAddress(serverAddress));
                receiver->move(500,200);
                receiver->show();
            }

        }
        else{
            sendMessage(FileRefuse,serverAddress);
        }
    }
}

/* 双击用户 弹出对应聊天框 所有聊天框有qmap 来管理 并在关闭槽函数中 统一销毁 */
void Widget::on_tableWidget_doubleClicked(QModelIndex index)
{
    QString secretAddress = ui->tableWidget->item(index.row(),1)->text();
    if (secretAddress == getIP()) {
        QMessageBox::information(this,tr("warning"),tr("automatic speaking"));
        return;
    }
    if(list.contains(secretAddress))
    {
        list[secretAddress]->raise();
        list[secretAddress]->activateWindow();
        list[secretAddress]->show();
        return;
    }
    sendMessage(AskStartChat,secretAddress);
}

/* 群聊窗口  同上 */
void Widget::on_btnAll_clicked()
{
    if(list.contains("NULL"))
    {
        list["NULL"]->raise();
        list["NULL"]->activateWindow();
        list["NULL"]->show();
        //QMessageBox::information(this,tr("a"),tr("cunzai"));
    }else {
        ChatWidgit *chatView = new ChatWidgit();
        list.insert("NULL",chatView);
        connect(chatView,SIGNAL(closeChat(QString)),this,SLOT(closeFromChat(QString)));
        connect(this,SIGNAL(sendMessagesZ(QString,QString,QString)),chatView,SLOT(setMessage(QString,QString,QString)));
        connect(chatView,SIGNAL(sendMessagesFromChat(QString,QString)),this,SLOT(sendMessagesToMain(QString,QString)));
        chatView->setSecAddr("NULL",Name);
        chatView->show();
    }
}

/* 发送消息 和接收消息相对  根据type 区分 */
void Widget::sendMessage(MessageType type, QString serverAddress,QString message)
{
    if(Name == "")
        return;

    QByteArray data;
    QHostAddress addr;
    QDataStream out(&data,QIODevice::WriteOnly);
    QString localHostName = Name;
    QString address = getIP();
    QString sex = "boy";
    out << type<< localHostName;
    switch(type)
    {
    case ParticipantLeft:
        {
            break;
        }
    case NewParticipant:
        {
            out << address <<sex;
            break;
        }
    case RefuseChat:
        {
            out<<serverAddress;
            break;
        }
    case StartChat:
        {
            out<<address<<serverAddress;
            break;
        }
    case AskStartChat:
        {
            out<<address<<serverAddress;
            break;
        }
    case Message :
        {
            out<<address<<serverAddress<<message;
            break;
        }
    case CameraMessage :
        {
            out<<address<<serverAddress<<message;
            break;
        }
    case SpeekerMessage :
        {
            out<<address<<serverAddress<<message;
            break;
        }
    case FileName:
        {
            out << address << serverAddress << message;
            break;
        }
    case FileRefuse:
        {
            out << address <<serverAddress;
            break;
        }
    }
    udpSocket->writeDatagram(data,data.length(),QHostAddress::Broadcast, port);
}

/* 聊天窗口发送文件信号  这里处理 槽 */
void Widget::sentFileName(QString fileName,QString Addr)
{
    sendMessage(FileName,Addr,fileName);
}


/* 主窗口关闭槽函数 引发所有子窗口关闭 */
void Widget::closeEvent(QCloseEvent *)
{
    QMap<QString,ChatWidgit *>::Iterator it;
    while(list.size() > 0)
    {
        it=list.begin();
        list[it.key()]->close();
    }
    sendMessage(ParticipantLeft);
}
/* 没什么用的 */
void Widget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
/* 处理新用户加入 广播 */
void Widget::newParticipant(QString localHostName,QString ipAddress,QString ssex)
{
    bool bb = ui->tableWidget->findItems(localHostName,Qt::MatchExactly).isEmpty();
    if(bb)
    {
        QTableWidgetItem *host = new QTableWidgetItem(localHostName);
        QTableWidgetItem *ip = new QTableWidgetItem(ipAddress);
        QTableWidgetItem *sex = new QTableWidgetItem(ssex);
        /**/
        ui->tableWidget->insertRow(0);
        ui->tableWidget->setItem(0,0,host);
        ui->tableWidget->setItem(0,1,ip);
        ui->tableWidget->setItem(0,2,sex);
        ui->lbOnline->setText(tr("在线人数：%1").arg(ui->tableWidget->rowCount()));
        sendMessage(NewParticipant);
    }
}
/* 处理用户离开 广播 */
void Widget::participantLeft(QString localHostName,QString time)
{
    int rowNum = ui->tableWidget->findItems(localHostName,Qt::MatchExactly).first()->row();
    ui->tableWidget->removeRow(rowNum);
    ui->lbOnline->setText(tr("在线人数：%1").arg(ui->tableWidget->rowCount()));
}
/* 对方确认接收文件后 sender 触发的发送文件槽函数 并显示进度 */
void Widget::hasFileToSend(QString fileName, QString SecretAddress)
{
    this->Sender->initSender();
    this->Sender->SenderStart(fileName,SecretAddress);
    this->Sender->move(500,200);
    this->Sender->show();
}
/* 槽函数 聊天窗口发来的信号 这里处理 */
void Widget::sendMessagesToMain(QString message,QString secretAddr)
{
    sendMessage(Message,secretAddr,message);
}

/* 聊天窗口关闭信号触发的槽 */
void Widget::closeFromChat(QString addr)
{
    list.remove(addr);
}
/* 退出按钮  触发主窗口 关闭槽函数 */
void Widget::on_btnCancel_clicked()
{
    this->close();
}

Widget::~Widget()
{
    delete ui;
}

/* 获取ip地址  貌似在新用户的时候用到 */
QString Widget::getIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
       if(address.protocol() == QAbstractSocket::IPv4Protocol)
        {
           if(address.toString() != tr("127.0.0.1"))
                return address.toString();
        }
    }
    return tr("");
}

/* 托盘 */
void  Widget::showMessage(const char *msg)
{
    QString titlec=tr(msg);
    QString textc=QString::fromLocal8Bit("test mouse");
    trayIcon->showMessage(titlec, textc, QSystemTrayIcon::Information, 5000);
}
void  Widget::createActions()
{
    minimizeAction = new QAction(tr("最小化 (&I)"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
    maximizeAction = new QAction(tr("最大化 (&X)"), this);
    connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));
    restoreAction = new QAction(tr("还原 (&R)"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    quitAction = new QAction(tr("退出 (&Q)"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}
void  Widget::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);
}

void  Widget::showMessage()
{
    QString titlec=tr("slot and signal no matched");
    QString textc=QString::fromLocal8Bit("test mouse.");
    trayIcon->showMessage(titlec, textc, QSystemTrayIcon::Information, 5000);
}

/* 触发后台图标执行相应事件 */
void  Widget::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        showMessage("mouse Trigger");
        break;
    case QSystemTrayIcon::DoubleClick:
        showMessage("mouse DoubleClick");
        break;
    case QSystemTrayIcon::MiddleClick:
        showMessage("mouse MiddleClick");
        break;
    default:
        break;
    }
}

void Widget::on_tableWidget_clicked(const QModelIndex &index)
{
    (void)index;
}

void Widget::contextMenuEvent( QContextMenuEvent * event )
{
    (void)event;
}



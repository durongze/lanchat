/********************************************************************************
* File Name:	chatwidgit.cpp
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#include "chatwidgit.h"
#include "ui_chatwidgit.h"
#include "camera.h"
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QScrollBar>
#include "camera.h"
//此函数是网上的  将html格式的qstring 转换
QString simplifyRichText( QString f_richText )
{
    QRegExp rxBody("<body[^>]*>(.*)</body>");
    if(rxBody.indexIn(f_richText))
    {
        f_richText = rxBody.cap(1); // throw away anything not in the body, and the body tags as well
        f_richText.replace(QRegExp("^\\s+"),""); // throw away any leading whitespace (be it new lines or spaces)
    }
    QRegExp rxBody1("<span[^>]*>(.*)</span>");
    if(f_richText.indexOf(rxBody1) != -1){
        if(rxBody1.indexIn(f_richText))
        {
            f_richText = rxBody1.cap(1); // throw away anything not in the body, and the body tags as well
            f_richText.replace(QRegExp("^\\s+"),""); // throw away any leading whitespace (be it new lines or spaces)
        }
    }
    f_richText.replace(QRegExp("<p style=\"[^\"]*\">"),"<p>"); // throw away paragraph styles


    if(f_richText == "<p></p>") // thats what we get for an empty QTextEdit
        return QString();
    f_richText.replace("<p>","");
    f_richText.replace("</p>","");

    return f_richText;
}



/* 初始化    连接编辑框 和字体装换的信号与槽  设置位置 */
ChatWidgit::ChatWidgit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ChatWidgit)
{
    cameraDisplayer = NULL;
    speekerPlayer = NULL;
    ui->setupUi(this);
    ui->tbHistory->hide();
    ui->teEdit->setFocusPolicy(Qt::StrongFocus);
    ui->teEdit->setFocus();
    ui->teEdit->installEventFilter(this);

    this->resize(500,400);

    connect(ui->teEdit,SIGNAL(currentCharFormatChanged(QTextCharFormat)),this,SLOT(currentFormatChanged(const QTextCharFormat)));
}

//析构
ChatWidgit::~ChatWidgit()
{
    delete ui;
    if (NULL != cameraDisplayer)
    {
        delete cameraDisplayer;
        cameraDisplayer = NULL;
    }
    if (NULL != speekerPlayer)
    {
        delete speekerPlayer;
        speekerPlayer = NULL;
    }

}


//收到主窗体发来的聊天消息  对比是否是此聊天框接收  并将聊天记录储存 显示
void ChatWidgit::setMessage(QString address, QString first, QString second)
{
    if(address == secretAddress ){
        ui->tbShow->setTextColor(Qt::blue);
        ui->tbShow->setCurrentFont(QFont("Times New Roman",12));
        ui->tbShow->append(first);
        ui->tbShow->append(second);

        //        QMessageBox::information(this,tr("a"),tr("%1\n%2").arg(first).arg(second));

        if(!saveFile.open(QFile::WriteOnly | QFile::Text | QFile::Append))

        {
            QMessageBox::warning(this,tr("保存文件"),
                                 tr("无法保存文件 :\n %1").arg(saveFile.errorString()));
            return;
        }
        QTextStream out(&saveFile);
        out << first <<endl << simplifyRichText(second) <<endl;

        //        out << first.trimmed() << endl << second.trimmed()<<endl;

        saveFile.close();
    }
}

//获取本机ip
QString ChatWidgit::getIP()  //获取ip地址
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
       if(address.protocol() == QAbstractSocket::IPv4Protocol) //我们使用IPv4地址
        {
           if(address.toString()!="127.0.0.1")
                return address.toString();
        }
    }
    return 0;
}

//获取此聊天框对应的聊天人ip 用不到？
QString ChatWidgit::getSecAddr()
{
    return this->secretAddress;
}

//公有函数  在创建之前设这聊天人ip 并创建聊天记录文件 存在saveFile  相对路径
void ChatWidgit::setSecAddr(QString addr,QString niname)
{
    this->secretAddress = addr;
    if(secretAddress == "NULL"){
        ui->lbConn->setText(tr("对方ip: All"));
        ui->textSave->hide();
    }
    else
        ui->lbConn->setText(tr("对方ip：%1").arg(secretAddress));
    QDir dir;
    dir.mkpath("history");

    saveFile.setFileName(tr("history\\%1%2.chat").arg(addr).arg(niname));
    this->setWindowTitle(tr("聊天--%1").arg(niname));
}

/* 发送消息给聊天人 触发主窗口的槽函数  并显示储存消息 */
void ChatWidgit::on_btnSend_clicked()
{
    if(ui->teEdit->toPlainText() == "")
    {
        QMessageBox::warning(0,tr("warning"),tr("msg is null"),QMessageBox::Ok);
        return;
    }
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString message = ui->teEdit->toHtml();
    if(secretAddress != "NULL"){
        ui->tbShow->setTextColor(Qt::blue);
        ui->tbShow->setCurrentFont(QFont("Times New Roman",12));
        ui->tbShow->append(tr("[ MM ]  %1").arg(time));
        ui->tbShow->append(message);

        if(!saveFile.open(QFile::WriteOnly | QFile::Text | QFile::Append))

        {
            QMessageBox::warning(this,tr("保存文件"),
                                 tr("无法保存文件 :\n %1").arg(saveFile.errorString()));
            return;
        }
        QTextStream out(&saveFile);
        out <<tr("[ 你说 ]  %1").arg(time)<<endl << simplifyRichText(message)<<endl;

        //        out <<tr("[ 你说 ]  %1").arg(time).trimmed()<<endl << message.trimmed()<<endl;
        saveFile.close();
    }

    ui->teEdit->clear();
    ui->teEdit->setFocus();

    emit sendMessagesFromChat(message,secretAddress);
}
//加粗
void ChatWidgit::on_textbold_clicked(bool checked)
{
    if(checked)
        ui->teEdit->setFontWeight(QFont::Bold);
    else
        ui->teEdit->setFontWeight(QFont::Normal);
    ui->teEdit->setFocus();
}

/* 倾斜 */
void ChatWidgit::on_textitalic_clicked(bool checked)
{
    ui->teEdit->setFontItalic(checked);
    ui->teEdit->setFocus();
}

/* 下滑线 */
void ChatWidgit::on_textunderline_clicked(bool checked)
{
    ui->teEdit->setFontUnderline(checked);
    ui->teEdit->setFocus();
}

/* 颜色 */
void ChatWidgit::on_textcolor_clicked()
{
    QColor color = QColorDialog::getColor(color,this);
    if(color.isValid())
    {
        ui->teEdit->setTextColor(color);
        ui->teEdit->setFocus();
    }
}
/* 事件处理  在输入时按下ctrl + enter 发送 */
bool ChatWidgit::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->teEdit)
    {
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *k = static_cast<QKeyEvent *>(event);
            //if (k->key() == Qt::Key_Return)
            if (k->key() == Qt::Key_Return && (k->modifiers() & Qt::ControlModifier))
            {
                on_btnSend_clicked();
                return true;
            }
        }
    }
    return QWidget::eventFilter(target,event);
}

/* 聊天窗口关闭槽函数  发送信号 让主窗体删除 map 表项 */
void ChatWidgit::closeEvent(QCloseEvent *)
{
    emit closeChat(secretAddress);
    // this->destroyed(0);
}

/* 关闭 触发槽 */
void ChatWidgit::on_btnClose_clicked()
{
    this->hide();
}

/* 清空聊天框内容 */
void ChatWidgit::on_textBin_clicked()
{
    ui->tbShow->clear();
}

/* 发送文件 触发主窗口的槽  让接收方确认  群发时此按钮hide */
void ChatWidgit::on_textSave_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if(fileName.isEmpty())
    {
        QMessageBox::information(this,tr("warning"),tr("select file"));
        return;
    }

    emit sendFile(fileName,secretAddress);
}

/* 具体用处没发现 ，当编辑器的字体格式改变时，我们让部件状态也随之改变 */
void ChatWidgit::currentFormatChanged(const QTextCharFormat &format)
{
    ui->fontComboBox->setCurrentFont(format.font());

    if(format.fontPointSize() < 9)
        ui->fontComboBox->setCurrentIndex(3);
    else ui->comboBox->setCurrentIndex(
            ui->comboBox->findText(QString::number(format.fontPointSize())));

    ui->textbold->setChecked(format.font().bold());
    ui->textitalic->setChecked(format.font().italic());
    ui->textunderline->setChecked(format.font().underline());
    color = format.foreground().color();
}

/* 字体大小 */
void ChatWidgit::on_comboBox_currentIndexChanged(QString size)
{
    ui->teEdit->setFontPointSize(size.toDouble());
    ui->teEdit->setFocus();
}

/* 显示聊天记录的按键 */
void ChatWidgit::on_textHistory_clicked()
{
    if(ui->tbHistory->isHidden())
    {
        if(!saveFile.open(QFile::ReadOnly))
        {
            QMessageBox::warning(this,tr("save file"),
                                 tr("save file :\n %1").arg(saveFile.errorString()));
            return ;
        }

        QTextStream t(&saveFile);

        ui->tbHistory->setText(t.readAll());
        ui->tbHistory->verticalScrollBar()->setValue(ui->tbHistory->verticalScrollBar()->maximum());
        saveFile.close();
        ui->tbHistory->show();
        this->resize(800,400);
    }
    else{
        ui->tbHistory->hide();
        this->resize(500,400);
    }
}

void ChatWidgit::on_btnCamera_clicked()
{
    cameraDisplayer = Camera::getCamera();
    if (NULL != cameraDisplayer)  cameraDisplayer->show();
}

void ChatWidgit::on_btnSpeeker_clicked()
{
    speekerPlayer = AudioRecorder::getAudioRecorder();
    if (NULL != speekerPlayer)  speekerPlayer->show();
}

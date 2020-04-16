/********************************************************************************
* File Name:	chatwidgit.h
* Description:	第15章实例训练
* Reference book:《Linux环境下Qt4图形界面与MySQL编程》，机械工业出版社.2012.1
* E_mail: openlinux2011@gmail.com
*
********************************************************************************/
#ifndef CHATWIDGIT_H
#define CHATWIDGIT_H
#include "camera.h"
#include "audiorecorder.h"
#include <QWidget>
#include <QtNetwork>
#include <QtGui>
#include "filesender.h"
#include "filereceiver.h"
#include "ui_dataconfig.h"
#include <QString>
namespace Ui {
    class ChatWidgit;
}

class ChatWidgit : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidgit(QWidget *parent = 0);
    ~ChatWidgit();
    QString getSecAddr();
    void setSecAddr(QString,QString);

private:
    Camera* cameraDisplayer;
    AudioRecorder* speekerPlayer;
    QFile saveFile;
    QString secretAddress;
    Ui::ChatWidgit *ui;
    QString getIP();
    void closeEvent(QCloseEvent *);

    QColor color;

    bool eventFilter(QObject *target, QEvent *event);//事件过滤器
private slots:

    void on_textHistory_clicked();
    void on_textunderline_clicked(bool checked);
    void on_textSave_clicked();
    void on_textBin_clicked();
    void on_btnClose_clicked();
    void on_btnSend_clicked();
    void setMessage(QString address,QString first,QString second);

    void on_textcolor_clicked();
    void on_textitalic_clicked(bool checked);
    void on_textbold_clicked(bool checked);
    void on_comboBox_currentIndexChanged(QString );
    void currentFormatChanged(const QTextCharFormat &format);

    void on_btnCamera_clicked(); 
    void on_btnSpeeker_clicked();

signals:
    void sendMessagesFromChat(QString ,QString);
    void closeChat(QString);
    void sendFile(QString,QString);
    void sendCamera(Camera&,QString);
    void sendSpeeker(AudioRecorder&,QString);
};

#endif // CHATWIDGIT_H

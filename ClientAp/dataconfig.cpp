#include <QMessageBox>
#include "dataconfig.h"
#include "ui_dataconfig.h"
#include "database.h"
#include <iostream>
DataConfig::DataConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataConfig)
{
    ui->setupUi(this);
}

DataConfig::~DataConfig()
{
    delete ui;
}

void DataConfig::on_buttonBox_clicked(QAbstractButton *button)
{
    if ((QAbstractButton *)ui->buttonBox->button(QDialogButtonBox::Ok) == button)
    {
        if ( !database::Config(ui->leDatabase->text(),  ui->leUsername->text(),  ui->lePassword->text()) )
        {
            QMessageBox::about(0,tr("错误"),tr("配置数据库失败\n"));
        }
        else
        {

        }
    }
    else if ( (QAbstractButton *)ui->buttonBox->button((QDialogButtonBox::Cancel)) == button)
    {

    }
    /*这里关闭会将IP 帐号 密码 释放，database里如果使用会导致段错误，所以需要database里保存这三个信息*/
    this->close();
    return;
}

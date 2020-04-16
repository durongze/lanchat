#ifndef CONFIG_H
#define CONFIG_H

#include <QDialog>
#include <QAbstractButton>
namespace Ui {
class DataConfig;
}

class DataConfig : public QDialog
{
    Q_OBJECT

public:
    explicit DataConfig(QWidget *parent = 0);
    ~DataConfig();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::DataConfig *ui;
};

#endif // CONFIG_
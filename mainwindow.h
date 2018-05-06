#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QIODevice>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>
#include <QLineEdit>
#include <QIntValidator>
#include <QRegExp>
#include <QDateTime>
#include <QTimer>
#include <QTime>
#include "ControlCAN.h"
#include <QDir>
#include <QFileDialog>
#include <QLCDNumber>
#define DEBUG 1
namespace Ui {
        class MainWindow;
    }

class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();


    private slots:
        void on_Button_open_can_clicked();

        void on_Button__closecan_clicked();

        void on_Button_send_clicked();

        void on_Button_read_clicked();

        void on_Button_sample_clicked();

        void on_Button_sample_end_clicked();

        void on_lineEdit_DATA_textChanged(const QString &arg1);

        void on_lineEdit_DLC_textChanged(const QString &arg1);

        void on_Button_sample_test_clicked();

         void sample_timer_update();
         void on_action_open_device_triggered();

         void on_action_close_device_triggered();

    private:
        unsigned char USB_CAN_status;
        Ui::MainWindow *ui;
        int CAN_GetBaudRateNum(unsigned int BaudRate);
        int CAN_GetError_info(int error_state);
        int Open_device(DWORD DeviceType,DWORD DeviceInd,DWORD CANInd,int baud);//输入参数:设备类型,设备序号,设备通道号,波特率
        int Close_devive(DWORD DeviceType,DWORD DeviceInd,DWORD CANInd);
        qint64 start_time;//记录当前启动时间
        unsigned char start_time_flag;//启动时间记录标志,当按下采样按钮时,首先读取数据,如果有数据,这将标志位置一
        QTimer *sample_timer;//定时器
        QString file_path;//文件路径缓存
        int sample_counter;
        QString file_name;//文件名
        QFile  firmwareFile;
         VCI_CAN_OBJ  can_read_msg[2500];
    };

#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "can_driver.h"
#define sample_time 200 //单位为毫秒
MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent),ui(new Ui::MainWindow)
{
    int ret;
    sample_counter= 0;
    USB_CAN_status = 0;
    ui->setupUi(this);
    VCI_BOARD_INFO1 vci;
    ui->comboBox_channel->setCurrentIndex(0);
    ui->baudRateComboBox->setCurrentIndex(4);
    //检测是否有CAN连接
    ret = VCI_FindUsbDevice(&vci);
    if(ret <= 0)
    {
        USB_CAN_status = 1;
        ui->Button_open_can->setText(tr("无设备"));
    }
    else
    {
        ui->Button_open_can->setText(tr("连接"));
        ui->comboBox_device->setMaxCount(ret);
    }
    ui->Button__closecan->setEnabled(false);
    ui->Button_open_can->setEnabled(true);
    ui->Button_send->setEnabled(false);
    ui->Button_read->setEnabled(false);
    ui->Button_sample->setEnabled(false);
    ui->Button_sample_end->setEnabled(false);
    ui->action_open_device->setEnabled(true);
    ui->action_close_device->setEnabled(false);
    //设置发送数据长度的限制
    QIntValidator *pIntValidator = new QIntValidator(this);
    pIntValidator->setRange(0,8);
    ui->lineEdit_DLC->setValidator(pIntValidator);
    //设置发送数据的内容的限制及其输入数据长度
    // 字符和数字,限制输入的字符串包含0-9,A-F,a-f和空格;
    QRegExp lineedit_data_reg("[a-fA-F0-9 ]{24}");
    QRegExpValidator *pValidator = new QRegExpValidator(this);
    pValidator->setRegExp(lineedit_data_reg);
    ui->lineEdit_DATA->setValidator(pValidator);
    ui->lineEdit_DATA->setMaxLength((ui->lineEdit_DLC->text().toInt(NULL,10)*3-1));
    ui->lineEdit_FrameID->setMaxLength(8);
    QRegExp FrameID_reg("[a-fA-F0-9]{8}");
    pValidator->setRegExp(FrameID_reg);
    ui->lineEdit_FrameID->setValidator(pValidator);
    ui->lcd_sample_counter->display(sample_counter);
    //连接槽函数
    sample_timer = new QTimer(this);
    connect(sample_timer,&QTimer::timeout,this,&this->sample_timer_update,Qt::AutoConnection);
   //connect(this,&MainWindow)
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    (void)event;
    if(USB_CAN_status == 4)
    {
         on_Button__closecan_clicked();
    }
}
int MainWindow::CAN_GetBaudRateNum(unsigned int BaudRate)
{

    for(int i=0;i<27;i++)
    {
        if(BaudRate == CANBus_Baudrate_table[i].BaudRate)
        {
            return i;
        }
    }
    return 0;
}
int MainWindow::CAN_GetError_info(int error_state)
{
        for(int i=0;i<27;i++)
        {
            if(error_state == ERROR_INFO_table[i].error_ind)
            {
                return i;
            }
        }
        return 0;
}
int MainWindow::Open_device(DWORD DeviceType,DWORD DeviceInd,DWORD CANInd,int baud)//输入参数:设备类型,设备序号,设备通道号
{
        bool state;
        state = VCI_OpenDevice(DeviceType,DeviceInd,0);
        if(!state)
        {
            return CAN_ERR_OPEN;
        }
        state = 0;
        VCI_INIT_CONFIG VCI_init;
        VCI_init.Mode        = 0x00;
        VCI_init.Filter      = 0x01;
        VCI_init.AccCode     = 0x00000000;
        VCI_init.AccMask     = 0xFFFFFFFF;
        VCI_init.Reserved    = 0x00;
        VCI_init.Timing0  =  CANBus_Baudrate_table[CAN_GetBaudRateNum(baud)].Timing0;//波特率的配置
        VCI_init.Timing1  =  CANBus_Baudrate_table[CAN_GetBaudRateNum(baud)].Timing1;//波特率的配置
        state = VCI_InitCAN(DeviceType,DeviceInd,CANInd,&VCI_init);
        if(state == 0)
            {
                return CAN_ERR_Init;
            }
        state = 0;
        state = VCI_StartCAN(DeviceType,DeviceInd,CANInd);
        if(state!=1)
        {
            return CAN_ERR_Start;
        }
        VCI_ClearBuffer(DeviceType,DeviceInd,CANInd);
        return CAN_SUCCESS;
}
int MainWindow::Close_devive(DWORD DeviceType,DWORD DeviceInd,DWORD CANInd)
{
        int ret;
        ret = VCI_ResetCAN(DeviceType,DeviceInd,CANInd);
        if(ret != 1)
            {
                return CAN_ERR_Reset;
            }
        ret = VCI_CloseDevice(DeviceType,DeviceInd);
        if(ret != 1)
            {
                return CAN_ERR_Close;
            }
     return CAN_SUCCESS;
}
void MainWindow::on_Button_open_can_clicked()
{

    int state;
    ERROR_INFO error_info;
    QString str          = ui->baudRateComboBox->currentText();
    str.resize(str.length()-4);
    int baud = str.toInt(NULL,10)*1000;
    state = Open_device(4,ui->comboBox_device->currentIndex(),ui->comboBox_channel->currentIndex(),baud);
    error_info.error_ind = ERROR_INFO_table[CAN_GetError_info(state)].error_ind;
    error_info.error_str = ERROR_INFO_table[CAN_GetError_info(state)].error_str;
    if(error_info.error_ind != CAN_SUCCESS)
        {
            if(error_info.error_ind == CAN_ERR_OPEN)
            {
                QMessageBox::warning(this,
                                     QStringLiteral("警告"),
                                     error_info.error_str
                                     );
                ui->Button_open_can->setText(tr("无设备"));
                USB_CAN_status = 1;
                return;
            }
            if(error_info.error_ind == CAN_ERR_Init)
                {
                    QMessageBox::warning(this,
                                         QStringLiteral("警告"),
                                         error_info.error_str
                                         );
                    USB_CAN_status = 3;
                    return;
                }
            if(error_info.error_ind == CAN_ERR_Start)
            {
                QMessageBox::warning(this,
                                     QStringLiteral("警告"),
                                      error_info.error_str
                                     );
                USB_CAN_status = 3;
                return;
            }
        }
    else
    {
        if((USB_CAN_status == 1)||(USB_CAN_status == 0))
        {
            USB_CAN_status = 0;
            ui->Button_open_can->setText(tr("连接"));
        }
        USB_CAN_status = 0x04;
        VCI_ClearBuffer(4,
                        ui->comboBox_device->currentIndex(),
                        ui->comboBox_channel->currentIndex()
                        );
        ui->Button_open_can->setEnabled(false);
        ui->baudRateComboBox->setEnabled(false);
        ui->Button__closecan->setEnabled(true);
        ui->comboBox_channel->setEnabled(false);
        ui->comboBox_device->setEnabled(false);
        ui->Button_send->setEnabled(true);
        ui->Button_read->setEnabled(true);
        ui->Button_sample->setEnabled(true);
        ui->Button_sample_end->setEnabled(false);
        ui->action_open_device->setEnabled(false);
        ui->action_close_device->setEnabled(true);
    }
}
void MainWindow::on_Button__closecan_clicked()
{
        int state;
        VCI_ClearBuffer(4,
                        ui->comboBox_device->currentIndex(),
                        ui->comboBox_channel->currentIndex()
                        );
        state = Close_devive(4,ui->comboBox_device->currentIndex(),ui->comboBox_channel->currentIndex());
        ERROR_INFO error_info;
        error_info.error_ind = ERROR_INFO_table[CAN_GetError_info(state)].error_ind;
        error_info.error_str = ERROR_INFO_table[CAN_GetError_info(state)].error_str;
        if(error_info.error_ind != CAN_SUCCESS)
            {
                QMessageBox::warning(this,
                                     QStringLiteral("警告"),
                                     error_info.error_str
                                     );
            }
        else
            {
                 USB_CAN_status = 0;
            }
        ui->Button_open_can->setEnabled(true);
        ui->baudRateComboBox->setEnabled(true);
        ui->Button__closecan->setEnabled(false);
        ui->comboBox_channel->setEnabled(true);
        ui->comboBox_device->setEnabled(true);
        ui->Button_send->setEnabled(false);
        ui->Button_read->setEnabled(false);
        ui->Button_sample->setEnabled(false);
        ui->Button_sample_end->setEnabled(false);
        ui->action_close_device->setEnabled(false);
        ui->action_open_device->setEnabled(true);
}
void MainWindow::on_Button_send_clicked()
{
     VCI_CAN_OBJ can_send_msg;
     unsigned long int frame_id = ui->lineEdit_FrameID->text().toInt(NULL,16);
    if(ui->comboBox_Frametype->currentIndex() == 0)
        {
            can_send_msg.ExternFlag = CAN_ID_STD;//标准帧
        }
    else if(ui->comboBox_Frametype->currentIndex() == 1)
        {
            can_send_msg.ExternFlag = CAN_ID_EXT;//扩展帧
        }

     can_send_msg.ID         = frame_id;
     if(ui->comboBox_Frameformat->currentIndex() == 0)
         {
             can_send_msg.RemoteFlag = CAN_RTR_Data;//数据帧
         }
     else if(ui->comboBox_Frameformat->currentIndex() == 1)
         {
            can_send_msg.RemoteFlag = CAN_RTR_Remote;//远程帧,
         }
     can_send_msg.SendType =1 ;
     can_send_msg.TimeFlag = 1;

    if(ui->lineEdit_DATA->text() != "")
        {
            QStringList list1 = ui->lineEdit_DATA->text().split(' ', QString::SkipEmptyParts);//split(',', QString::SkipEmptyParts);
            qDebug()<<"list1.size() = "<<list1.size();
            can_send_msg.DataLen = list1.size();
            QString data_string;
            for(int i = 0;i < list1.size();i++)
                {
                    if(list1[i] != ' ')
                        {
                            data_string = list1[i];
                            can_send_msg.Data[i] = data_string.toInt(NULL,16)&0xFF;
#if DEBUG
                            qDebug()<<"can_send_msg.Data["<<i<<"] = "<<can_send_msg.Data[i];
#endif
                        }
                }
            VCI_Transmit(4,
                         ui->comboBox_device->currentIndex(),
                         ui->comboBox_channel->currentIndex(),
                         &can_send_msg,
                         1
                         );
        }
    else
        {
#if DEBUG
            qDebug()<<"发送数据为空";
#endif
        }
}
void MainWindow::on_Button_read_clicked()
{

}
void MainWindow::on_Button_sample_clicked()
{
    ui->Button_sample->setEnabled(false);
    ui->Button_sample_end->setEnabled(true);
    ui->Button_sample->setText("数据采集中");
    ui->file_path_textBrowser->clear();
    ui->lcd_sample_counter->display(0);
    //设置通道数据保存路径
    //将通道0和通道1分开处理
    if(ui->comboBox_channel->currentIndex() == 0)
    {
         file_path  = QCoreApplication::applicationDirPath()+"/数据采集"+"/CAN0数据";
    }
    else
    {
         file_path  = QCoreApplication::applicationDirPath()+"/数据采集"+"/CAN1数据";
    }

    //此处判断当前文件文件夹是否存在,如果不存在就创建一个文件夹
     QDir file_path_dir(file_path);
     if(file_path_dir.exists() == false)
         {
             file_path_dir.mkdir(file_path);
         }
    QString line_data = NULL;
    short int read_num = 0;
    QDateTime current_time = QDateTime::currentDateTime();
    //第一步.确定文件名
    file_name = current_time.toString("yyyy-MM-dd_hh_mm_ss")+".asc";
#if DEBUG
    qDebug()<<"file_name_temp =  "<<file_name;
#endif
    //第二步.确定文件路径
    file_path = file_path +"/"+file_name;
#if DEBUG
    qDebug()<<"file_path =  "<<file_path;
#endif
    firmwareFile.setFileName(file_path);
    if(firmwareFile.open(QFile::ReadWrite))
        {
#if DEBUG
             qDebug()<<"打开文件成功";
#endif
        }
    else
        {
#if DEBUG
             qDebug()<<"打开文件失败";
#endif
        }
    if(firmwareFile.openMode() !=QIODevice::NotOpen )
        {
            #if DEBUG
              qDebug()<<"文件正在操作"<<"firmwareFile.openMode() = "<<firmwareFile.openMode()<<endl;
            #endif
        }

/* 文件头格式
2017-10-21 11:05:30
base hex timestamps absolute
internal events logged
0.000000 start of measurement
*/
    //第一步 写入当时的时间
     ULONG read_len = 0;
    line_data = current_time.toString("yyyy-MM-dd hh:mm:ss");
    QTextStream in(&firmwareFile);
    in<<line_data<<endl;
    in<<"base hex timestamps absolute"<<endl;
    in<<"internal events logged"<<endl;
    in<<"0.000000 start of measurement"<<endl;
    read_num = VCI_GetReceiveNum(4,ui->comboBox_device->currentIndex(),ui->comboBox_channel->currentIndex());
    switch (read_num)
        {
               case 0:
               case -1:
                       sample_timer->start(sample_time);
                       return;
                       break;
               default:
                 read_len =   VCI_Receive(4,
                               ui->comboBox_device->currentIndex(),
                               ui->comboBox_channel->currentIndex(),
                               &can_read_msg[0],
                               2500,
                               0
                               );
                   start_time  = can_read_msg[0].TimeStamp;
                    VCI_ClearBuffer(4,
                                    ui->comboBox_device->currentIndex(),
                                    ui->comboBox_channel->currentIndex()
                                    );
                       break;
        }
#if DEBUG
    qDebug()<<"read_num = "<<read_num<<"start_time = "<<can_read_msg[0].TimeStamp;
#endif
    float time_temp;
    QString data_line_temp;
    if(read_len >0)
        {
            for(ULONG i = 0;i<read_len;i++)
                {
                    sample_counter++;
                    //1 换算时间
                    time_temp = (can_read_msg[i].TimeStamp-start_time)/10000.0;
                    data_line_temp.sprintf("%.8f",time_temp);
                     //2 添加通道
                     data_line_temp = data_line_temp+" 1 ";
                     line_data = data_line_temp;
                     //3 换算CAN的ID和数据长度
                     data_line_temp.sprintf("%08Xx Rx d %d ",
                                            can_read_msg[i].ID,
                                            can_read_msg[i].DataLen
                                            );
                      line_data = line_data+data_line_temp;
                      //4转换数据
                      for(int j = 0;j <can_read_msg[i].DataLen;j++)
                          {
                              data_line_temp.sprintf("0x%02X",can_read_msg[i].Data[j]);
                              if(j!=can_read_msg[i].DataLen-1)
                                  {
                                     data_line_temp = data_line_temp+" ";
                                  }
                              line_data = line_data+data_line_temp;

                          }
                      //将时间记录进去用于测试使用,发布时删除

#if DEBUG
qDebug()<<"line_data = "<<line_data<<"line_data time = "<<can_read_msg[i].TimeStamp<<"can_read_msg[i].TimeFlag ="<<can_read_msg[i].TimeFlag;
#endif
                        ui->lcd_sample_counter->display(sample_counter);
                      in<<line_data<<endl;
                }
        }
    sample_timer->start(sample_time);
    for(int i= 0;i< 2500;i++)
    {
        can_read_msg[i].TimeStamp = 0;
        for(int j = 0;j<8;j++)
        {
             can_read_msg[i].Data[j] = 0;
        }

        can_read_msg[i].DataLen = 0;
        can_read_msg[i].ID = 0;
    }
}

void MainWindow::sample_timer_update()
{
    QTextStream in(&firmwareFile);
    short int read_num = 0;
    ULONG read_len = 0;
    float time_temp = 0;
    QString data_line_temp = NULL;
     QString line_data     = NULL;
    read_num = VCI_GetReceiveNum(4,ui->comboBox_device->currentIndex(),ui->comboBox_channel->currentIndex());
    switch (read_num)
        {
               case 0:
               case -1:
                       return;
                       break;
               default:
                   read_len =  VCI_Receive(4,
                                           ui->comboBox_device->currentIndex(),
                                           ui->comboBox_channel->currentIndex(),
                                           &can_read_msg[0],
                                           2500,
                                           0
                                           );
                    VCI_ClearBuffer(4,ui->comboBox_device->currentIndex(),ui->comboBox_channel->currentIndex());
                       break;
        }
    if(read_len >0)
            {
                for(ULONG i = 0;i<read_len;i++)
                    {
                        //1 换算时间
                        time_temp = (can_read_msg[i].TimeStamp-start_time)/10000.0;
                        data_line_temp.sprintf("%.8f",time_temp);
                         //2 添加通道
                         data_line_temp = data_line_temp+" 1 ";
                         line_data = data_line_temp;
                         //3 换算CAN的ID和数据长度
                         data_line_temp.sprintf("%08Xx Rx d %d ",
                                                can_read_msg[i].ID,
                                                can_read_msg[i].DataLen
                                                );
                          line_data = line_data+data_line_temp;
                          //4转换数据
                          for(int j = 0;j <can_read_msg[i].DataLen;j++)
                              {
                                  if(j!=can_read_msg[i].DataLen-1)
                                      {
                                        data_line_temp.sprintf("0x%02X ",can_read_msg[i].Data[j]);
                                      }
                                  else
                                      {
                                        data_line_temp.sprintf("0x%02X",can_read_msg[i].Data[j]);
                                      }
                                  line_data = line_data+data_line_temp;

                              }
    #if DEBUG
qDebug()<<"line_data = "<<line_data<<"line_data time = "<<can_read_msg[i].TimeStamp<<"can_read_msg[i].TimeFlag ="<<can_read_msg[i].TimeFlag;
    #endif
                          sample_counter++;
                          ui->lcd_sample_counter->display(sample_counter);
                          in<<line_data<<endl;
                    }
            }
    for(int k = 0;k< 2500;k++)
    {
        can_read_msg[k].TimeStamp = 0;
        for(int j = 0;j<8;j++)
        {
             can_read_msg[k].Data[j] = 0;
        }

        can_read_msg[k].DataLen = 0;
        can_read_msg[k].ID = 0;
    }
}
void MainWindow::on_Button_sample_end_clicked()
{
    sample_counter = 0;
     ui->Button_sample->setText("采集数据");
     ui->Button_sample->setEnabled(true);
     ui->Button_sample_end->setEnabled(false);
#if DEBUG
     qDebug()<<"firmwareFile.size() = "<<firmwareFile.size();
#endif
     if(firmwareFile.size() < 105)
         {
             firmwareFile.remove();
         }
     else
         {
              ui->file_path_textBrowser->setText(file_name);
         }

     ui->file_path_textBrowser->show();
     firmwareFile.close();
     start_time = 0;
     sample_timer->stop();
}
void MainWindow::on_lineEdit_DATA_textChanged(const QString &arg1)
{
    ui->lineEdit_DATA->setMaxLength((ui->lineEdit_DLC->text().toInt(NULL,10)*3-1));
    qDebug()<<""<<arg1;
    qDebug()<<"ui->lineEdit_DATA->text().length() = "<<ui->lineEdit_DATA->text().length();
    if(ui->lineEdit_DATA->text() != "")
        {
            QStringList list1 = ui->lineEdit_DATA->text().split(' ', QString::SkipEmptyParts);//split(',', QString::KeepEmptyParts);
            qDebug()<<"list1.size() = "<<list1.size();
            qDebug()<<"list1.size() = "<<list1[0];
        }
    else
        {
            qDebug()<<"输入为空";
        }

}
void MainWindow::on_lineEdit_DLC_textChanged(const QString &arg1)
{
   qDebug()<<"arg1 = "<<arg1;
   ui->lineEdit_DATA->setMaxLength((ui->lineEdit_DLC->text().toInt(NULL,10)*3-1));
}
void MainWindow::on_Button_sample_test_clicked()
{
        QString fileName;
        fileName = QFileDialog::getSaveFileName(this,
                                                tr("Open Config"),
                                                "",
                                                tr("Hex Files (*.hex);;Binary Files (*.bin);;All Files (*.*);;文本文档(*.txt)")
                                                );
        qDebug()<<""<<fileName;

        if (!fileName.isNull())
        {
                            //fileName是文件名
        }
        else
        {

        }
}

void MainWindow::on_action_open_device_triggered()
{
    on_Button_open_can_clicked();
    ui->action_open_device->setEnabled(false);
    ui->action_close_device->setEnabled(true);
}

void MainWindow::on_action_close_device_triggered()
{
    on_Button__closecan_clicked();
    ui->action_open_device->setEnabled(true);
    ui->action_close_device->setEnabled(false);
}

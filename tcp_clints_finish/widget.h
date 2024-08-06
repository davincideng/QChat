#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QString>
#include <QByteArray>
#include <QDateTime>
#include <QMessageBox>
#include <QSound>
#include <QIcon>
#include <QFileDialog>
#include <QStringList>
#include <QFile>
#include <QListView>
#include <QStringListModel>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    QString pro_pict = ":/new/prefix1/image/qq.png";//点击头像  获取用来存放头像的路径
    QString pro_pict_name = "qq.png";
    QString Name;//存放昵称
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    QSound *messageSound;
    QSound *connectSound;
private slots:
    void on_connectBtn_clicked();   //连接按钮

    void on_sendBtn_clicked();      //发送按钮

    void readMessage();         //接收信息

    void disconnectSlot();          //断开连接槽函数

    void on_image_btn_clicked();

    void sendTextMessage(QTcpSocket *socket, const QString &message);

    void sendFile(QTcpSocket *socket, const QString &filePath, QString fileName);

    void downloadFile(const QModelIndex &index);


    //void receiveImage(const QString& savePath, QTcpSocket* socket);

    void on_pushButton_clicked();

    void selectFolder();

private:
    Ui::Widget *ui;
    QTcpSocket *socket;
    bool connectState;  //客户端连接状态
    //显示文件目录
    QListView *view;
    QStringListModel *model;
    QStringList fileNames;
    QString clickedFile;//点击下载的文件名
    QString downloadPath;//下载路径

};

#endif // WIDGET_H

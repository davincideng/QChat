#include "widget.h"
#include "ui_widget.h"

#define SERVER_IP "127.0.0.1" // 替换为服务器的实际IP地址
#define SERVER_PORT 8081
#include <QHostAddress>
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->connectBtn->setStyleSheet("background-color: rgb(6,163,220)");
    ui->sendBtn->setStyleSheet("background-color: rgb(6,163,220)");
    ui->leport->setStyleSheet("color:blue");
    ui->leipAddress->setStyleSheet("color:blue");

    ui->listWidget->setStyleSheet("border:2px solid blue");

    socket = new QTcpSocket(this);
    connectState = false;     //未连接状态

    messageSound = new QSound(":/new/prefix1/sounds/iphone.wav", this);
    connectSound = new QSound(":/new/prefix1/sounds/keke.wav", this);
    //设置初始头像
    QString styleSheet = QString(
                "QPushButton {"
                "    border-image: url(%1);"
                "}"
                ).arg(pro_pict);
    ui->image_btn->setStyleSheet(styleSheet);

    this->setWindowIcon(QIcon(":/new/prefix1/image/qq.png"));
    //文件列表
    model = new QStringListModel(this);
    ui->view->setModel(model);
    //双击文件名，下载文件
    connect(ui->pushButton_2, &QPushButton::clicked, this, &Widget::selectFolder);//选择路径
    connect(ui->view, &QListView::doubleClicked, this, &Widget::downloadFile);
    connect(socket, &QTcpSocket::readyRead, this, &Widget::readMessage);    //接收信息
    connect(socket, &QTcpSocket::disconnected, this, &Widget::disconnectSlot);   //打印断开连接信息   

}

Widget::~Widget()
{
    delete ui;
}


void Widget::readMessage() {
    QByteArray data = socket->readAll();
    qDebug()<<"-------------";
    qDebug()<<data.size();
    if (!data.isEmpty()){
        // 判断是否是图片数据
        if (data.startsWith("GETFILE ")) {
            //读取到的内容为GETFILE +"文件名"
            //QByteArray data = socket->readAll();
            QString arr(data);
            QStringList parts = arr.split(' ');
            QString file_name = parts.at(1);
            fileNames.append(file_name);
            model->setStringList(fileNames);

        } else if (data.startsWith("TEXTMSG ")) {
            // 处理文本消息  TEXTMSG 昵称 消息 头像
            //设置接收到的消息全部左对齐
            QString arr(data);
            QTextCursor cursor(ui->textReceive->textCursor());
            cursor.movePosition(QTextCursor::End); // 移动到文档末
            QTextBlockFormat format;
            format.setAlignment(Qt::AlignLeft); // 设置右对齐
            cursor.insertBlock(format); // 插入一个新的块，并应用格式
            messageSound->play();
            //QString arr = socket->readAll();
            QStringList parts = arr.split(' ');
            QString str;
            if(parts.size()>1)
                str = parts.at(1) + QDateTime::currentDateTime().toString("hh:mm:ss");
            cursor.insertText(str); // 插入昵称+日期
            cursor.insertText("\n"); // 换行
            //cursor.insertText(parts.at(3));//插入头像
            QString pict = ":/new/prefix1/image/"+parts.at(3);
            qDebug()<<pict;

            QTextImageFormat imageFormat;
            imageFormat.setName(pict);  // 这里的路径是图片的路径
            imageFormat.setWidth(30);  // 设置宽度为 100 像素
            imageFormat.setHeight(30); // 设置高度为 100 像素
            cursor.insertImage(imageFormat);

            cursor.insertText(parts.at(2)); // 插入消息

        }else{
            //处理接收文件信息
            downloadPath = ui->lineEdit->text();
            QString file = downloadPath+"//"+clickedFile;
            QFile file1(file);
            if (file1.open(QIODevice::WriteOnly)) {
                file1.write(data);
                file1.close();
                qDebug() << "File saved successfully to" << file;
            } else {
                qDebug() << "Failed to save file to" << file;
            }
        }
    }
}



void Widget::disconnectSlot()    //打印连接断开信息
{
    ui->listWidget->addItem("clint disconnected");
}


void Widget::on_connectBtn_clicked()      //与客户端连接或者断开
{
    QString ipStr = ui->leipAddress->text();    //界面显示的地址
    quint16 currentPort = ui->leport->text().toInt();   //界面显示的当前端口
    if(!connectState)    //客户端还未连接服务端
    {
        socket->connectToHost(ipStr, currentPort);   //连接服务端
        if(socket->waitForConnected())   //等待连接成功
        {
            ui->listWidget->addItem("连接成功");
            ui->connectBtn->setText("关闭连接");
            connectSound->play();
            connectState = true;
        }

        else     //连接失败
        {
            QMessageBox::warning(this, "连接失败", socket->errorString());   //连接错误信息提醒
        }
    }

    else   //客户端已经连接
    {
        socket->close();   //关闭套接字，此时会发送disconnected信号
        connectSound->play();
        ui->connectBtn->setText("连接");
    }
}


void Widget::on_sendBtn_clicked()    //给服务端发送信息
{    
    if(!ui->textSend->toPlainText().isEmpty()){
        //设置接收到的消息全部右对齐
        QTextCursor cursor1(ui->textReceive->textCursor());
        cursor1.movePosition(QTextCursor::End); // 移动到文档末
        QTextBlockFormat format;
        format.setAlignment(Qt::AlignRight); // 设置右对齐
        cursor1.insertBlock(format); // 插入一个新的块，并应用格式

        //点击发送 立即获取输入框里的昵称
        Name = ui->nameEd->text();
        QString str = ui->textSend->toPlainText();
        QString showStr = QDateTime::currentDateTime().toString("hh:mm:ss");
        showStr+=Name;
        showStr+="\n";
        cursor1.insertText(showStr);
        cursor1.insertText(str);

        //显示头像
        QTextCursor cursor(ui->textReceive->textCursor());
        // 插入文本
        cursor.movePosition(QTextCursor::End);
        // 创建 QTextImageFormat 对象
        QTextImageFormat imageFormat;
        //qDebug()<<pro_pict;
        imageFormat.setName(pro_pict);  // 这里的路径是图片的路径
        imageFormat.setWidth(30);  // 设置宽度为 100 像素
        imageFormat.setHeight(30); // 设置高度为 100 像素
        // 插入图片
        cursor.insertImage(imageFormat);
        //把编辑框的内容清空
        ui->textSend->clear();
        QString send_msg = Name+" ";
        send_msg+=str;
        if(socket->isOpen() && socket->isValid())
        {
            sendTextMessage(socket,send_msg);
            //socket->write(send_msg.toUtf8());    //给服务端发送信息
            ui->textSend->clear();
        }
    }    
}


void Widget::on_image_btn_clicked()
{
    // 设置资源文件夹路径
    QString resourcePath = ":/new/prefix1/image";

    // 打开文件选择对话框
    pro_pict = QFileDialog::getOpenFileName(this, tr("Open Image"), resourcePath, tr("Image Files (*.png *.jpg *.bmp)"));
    QFileInfo fileInfo(pro_pict);
    pro_pict_name = fileInfo.fileName();
    qDebug()<<pro_pict_name;
    // 检查用户是否选择了文件
    if (!pro_pict.isEmpty()) {
        // 在这里处理选择的文件路径
        QString styleSheet = QString(
                    "QPushButton {"
                    "    border-image: url(%1);"
                    "}"
                    ).arg(pro_pict);
        ui->image_btn->setStyleSheet(styleSheet);
    }
}

void Widget::sendTextMessage(QTcpSocket *socket, const QString &message)
{
    //头像信息
//    QFile file(pro_pict);
//    if (!file.open(QIODevice::ReadOnly)) {
//        qDebug() << "Failed to open file" << pro_pict;
//        return;
//    }
//    QByteArray fileData = file.readAll();
//    file.close();
    //QByteArray textData = message.toUtf8();
    // 构建消息头部（前8字节表示文字消息，后8字节表示消息长度）
    QByteArray header;
    header.append("TEXTMSG ");  // 消息类型
    header.append(message);
    //header.append("\n");
    header.append(" ");
    header.append(pro_pict_name);//发送头像名称
    socket->write(header);  // 发送头部
    //socket->write(textData);  // 发送文字数据
    socket->flush();
    qDebug() << "Text message sent successfully";
}

void Widget::sendFile(QTcpSocket *socket, const QString &filePath, QString fileName)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file" << filePath;
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    // 构建消息头部（前8字节表示图片消息，后8字节表示消息长度）
    QByteArray header;
    fileName+='$';
    header.append(fileName);  // 消息类型
    //header.append(QByteArray::number(fileData.size()).rightJustified(8, '0'));  // 消息长度
    //发送文件
    QByteArray merge = header+fileData;
    socket->write(merge);  // 发送头部
    qDebug()<<merge;
//    socket->write(fileData);  // 发送数据
//    socket->flush();
    qDebug() << "File sent successfully";
}

void Widget::on_pushButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*)"));
    QString fileName1 = QFileInfo(filePath).fileName();
    //qDebug()<<fileName1;
    if (!filePath.isEmpty()) {
        sendFile(socket, filePath, fileName1);
    }
}
void Widget::downloadFile(const QModelIndex &index){
    // 获取被点击项的 QString
    clickedFile = model->data(index, Qt::DisplayRole).toString();
    QByteArray header;
    QString head = "GETFILE ";
    header.append(head);
    header.append(clickedFile);
    header.append('$');
    socket->write(header);
    qDebug() << "clicked File:" << header;
}

void Widget::selectFolder() {
    downloadPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!downloadPath.isEmpty()) {
        // Replace single slashes with double slashes
        downloadPath.replace("/", "//");
    }
    ui->lineEdit->setText(downloadPath);
    qDebug()<<downloadPath;
}




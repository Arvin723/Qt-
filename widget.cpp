#include "widget.h"
#include "ui_widget.h"
#include "Board.h"
#include "GoGame.h"
#include "Player.h"
#include "handstap.h"
#include "gameendcal.h"
#include <QMouseEvent>
#include <QDebug>
#include <QMessageBox>
#include <QAction>
#include <QMenu>
#include <QTime>
#include <cstdlib>
#include <QFile>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    udpsock = new QUdpSocket;
    QFile ipFile("../serverip.txt");
    if (ipFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray ipData = ipFile.readLine();
        serverIp = ipData;
        qDebug()<<"Srever ip: "<<serverIp;
        ipFile.close();
    }
    else
    {
        serverIp = "127.0.0.1";
        qDebug()<<"Srever default ip: "<<serverIp;
    }
    port = 6723;
    connect(udpsock, SIGNAL(readyRead()), this, SLOT(on_udpmsg_recved()));

    /*对局请求对话框*/
    gamedialog = new QDialog;
    gamedialog->setWindowTitle(tr("对局设置"));
    gamedialog->setFixedSize(300, 150);
    gamedialog->setWindowFlags(Qt::WindowTitleHint);
    dlgGrdLayout = new QGridLayout;

    dlgOkBtn = new QPushButton;
    dlgOkBtn->setText(tr("确认"));
    dlgOkBtn->setFixedSize(100,30);

    dlgCancelBtn = new QPushButton;
    dlgCancelBtn->setText(tr("取消"));
    dlgCancelBtn->setFixedSize(100,30);

    dlgSelectLabel = new QLabel;
    dlgSelectLabel->setText(tr("让子数"));
    dlgSelectLabel->setAlignment(Qt::AlignCenter);

    dlgNegativeLabel = new QLabel;
    dlgNegativeLabel->setText(tr("贴子"));
    dlgNegativeLabel->setAlignment(Qt::AlignCenter);

    dlgNegativeCmb = new QComboBox;
    dlgNegativeCmb->addItem(tr("0"));
    dlgNegativeCmb->addItem(tr("0.75"));
    dlgNegativeCmb->addItem(tr("1.75"));
    dlgNegativeCmb->addItem(tr("3.75"));
    dlgNegativeCmb->addItem(tr("5.75"));
    dlgNegativeCmb->setCurrentIndex(3);

    dlgPieceCmb = new QComboBox;
    for (int i = 0; i <= 9; i++)
    {
        if (i > 1)
            dlgPieceCmb->addItem(QString::number(i));
        else if (i == 1)
            dlgPieceCmb->addItem(tr("让先"));
        else if (i == 0)
        {
            dlgPieceCmb->addItem(tr("分先"));
            dlgPieceCmb->addItem(tr("定先"));
        }
    }

    dlgcolorLabel = new QLabel;
    dlgcolorLabel->setText(tr("己方执"));
    dlgcolorLabel->setAlignment(Qt::AlignCenter);

    dlgColorCmb = new QComboBox;
    dlgColorCmb->addItem(tr("黑"));
    dlgColorCmb->addItem(tr("白"));
    dlgColorCmb->setEnabled(false);

    dlgGrdLayout->addWidget(dlgSelectLabel, 0, 0);
    dlgGrdLayout->addWidget(dlgPieceCmb, 0, 1);
    dlgGrdLayout->addWidget(dlgNegativeLabel, 1, 0);
    dlgGrdLayout->addWidget(dlgNegativeCmb, 1, 1);
    dlgGrdLayout->addWidget(dlgcolorLabel, 2, 0);
    dlgGrdLayout->addWidget(dlgColorCmb, 2, 1);
    dlgGrdLayout->addWidget(dlgOkBtn, 3, 0);
    dlgGrdLayout->addWidget(dlgCancelBtn, 3, 1);
    gamedialog->setLayout(dlgGrdLayout);

    connect(dlgOkBtn, SIGNAL(clicked()), this, SLOT(on_dlgOkBtn_clicked()));
    connect(dlgCancelBtn, SIGNAL(clicked()), this, SLOT(on_dlgCancelBtn_clicked()));
    connect(dlgPieceCmb, SIGNAL(currentTextChanged(QString)), this, SLOT(on_piececmbxchange(QString)));

    /*登录注册对话框*/
    logindialog = new QDialog;
    loginDlgGridLayout= new QGridLayout;

    loginBtn = new QPushButton;
    loginBtn->setText(tr("登录"));
    loginBtn->setFixedWidth(90);

    registerBtn = new QPushButton;
    registerBtn->setText(tr("注册"));
    registerBtn->setFixedWidth(90);

    usernameLabel = new QLabel;
    usernameLabel->setText(tr("用户名"));
    usernameLabel->setAlignment(Qt::AlignHCenter);

    passwordLabel = new QLabel;
    passwordLabel->setText(tr("密码"));
    passwordLabel->setAlignment(Qt::AlignHCenter);

    usernameEdit = new QLineEdit;
    passwordEdit = new QLineEdit;

    loginDlgGridLayout->addWidget(usernameLabel, 0, 0);
    loginDlgGridLayout->addWidget(usernameEdit, 0, 1, 1, 2);
    loginDlgGridLayout->addWidget(passwordLabel, 1, 0);
    loginDlgGridLayout->addWidget(passwordEdit, 1, 1, 1, 2);
    loginDlgGridLayout->addWidget(loginBtn, 2, 0);
    loginDlgGridLayout->addWidget(registerBtn, 2, 2);
    logindialog->setLayout(loginDlgGridLayout);
    logindialog->setFixedSize(320, 180);

    connect(loginBtn, SIGNAL(clicked()), this, SLOT(on_loginBtn_clicked()));
    connect(registerBtn, SIGNAL(clicked()), this, SLOT(on_registerBtn_clicked()));

    this->setWindowTitle(tr("天疯GO"));
    this->setWindowIcon(QIcon(":/icon/ico.jpeg"));

    /*设置菜单属性*/
    ui->playerList->setContextMenuPolicy(Qt::CustomContextMenu);

    /*初始化画图尺寸为默认值*/
    margin = D_MARGIN ;
    blockLen = D_BLOCKLEN;
    pieceSize = D_PIECESIZE;
    starSize = D_STARSIZE;
}

Widget::~Widget()
{
    delete ui;
}

void Widget::paintEvent(QPaintEvent *event)
{
    event->setAccepted(true);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);

    /*底色*/
    QRect rect = QRect(margin/2, margin/2,
               margin + blockLen * (MAXLINE - 1), margin + blockLen * (MAXLINE - 1));
    brush.setColor(QColor(227, 168, 105));
    painter.setBrush(brush);
    painter.drawRect(rect);

    /*棋盘绘制*/
    QPen pen;
    pen.setColor(Qt::black);
    painter.setPen(pen);
    for (int i = 0; i < MAXLINE; i++)
    {
        /*画横线*/
        painter.drawLine(margin, margin + i*blockLen,
                         margin + blockLen * (MAXLINE - 1), margin + i*blockLen);
        /*画竖线*/
        painter.drawLine(margin + i*blockLen, margin,
                         margin + i*blockLen, margin + blockLen * (MAXLINE - 1));
    }

    /*棋子，星位绘制*/
    for (int i = 0; i < MAXLINE; i++)
    {
        for (int j = 0; j < MAXLINE; j++)
        {
            /*星位*/
            if ((i == 3 || i == 9 || i == 15) &&
                (j == 3 || j == 9 || j == 15))
            {
                brush.setColor(Qt::black);
                painter.setBrush(brush);
                painter.drawEllipse(margin + i*blockLen - starSize/2, margin + j*blockLen - starSize/2, starSize, starSize);
            }
            /*棋子*/
            if (nGame->getPointSta(i, j) == BLACK)
            {
                brush.setColor(Qt::black);
                painter.setBrush(brush);
                painter.drawEllipse(margin + i*blockLen - pieceSize/2, margin + j*blockLen - pieceSize/2, pieceSize, pieceSize);
            }
            else if (nGame->getPointSta(i, j) == WHITE)
            {
                brush.setColor(Qt::white);
                painter.setBrush(brush);
                painter.drawEllipse(margin + i*blockLen - pieceSize/2, margin + j*blockLen - pieceSize/2, pieceSize, pieceSize);
            }
            else if (nGame->getPointSta(i, j) == BLACKDIE)
            {
                brush.setColor(Qt::black);
                painter.setBrush(brush);
                painter.drawEllipse(margin + i*blockLen - pieceSize/2, margin + j*blockLen - pieceSize/2, pieceSize, pieceSize);
                brush.setColor(Qt::white);
                painter.setBrush(brush);
                painter.drawEllipse(margin + i*blockLen - pieceSize/4, margin + j*blockLen - pieceSize/4, pieceSize/2, pieceSize/2);
            }
            else if (nGame->getPointSta(i, j) == WHITEDIE)
            {
                brush.setColor(Qt::white);
                painter.setBrush(brush);
                painter.drawEllipse(margin + i*blockLen - pieceSize/2, margin + j*blockLen - pieceSize/2, pieceSize, pieceSize);
                brush.setColor(Qt::black);
                painter.setBrush(brush);
                painter.drawEllipse(margin + i*blockLen - pieceSize/4, margin + j*blockLen - pieceSize/4, pieceSize/2, pieceSize/2);
            }
            else if (nGame->getPointSta(i, j) == BLACKEMPTY)
            {
                brush.setColor(Qt::black);
                painter.setBrush(brush);
                painter.drawEllipse(margin + i*blockLen - pieceSize/4, margin + j*blockLen - pieceSize/4, pieceSize/2, pieceSize/2);
            }
            else if (nGame->getPointSta(i, j) == WHITEEMPTY)
            {
                brush.setColor(Qt::white);
                painter.setBrush(brush);
                painter.drawEllipse(margin + i*blockLen - pieceSize/4, margin + j*blockLen - pieceSize/4, pieceSize/2, pieceSize/2);
            }
            else if (nGame->getPointSta(i, j) == SINGLEYOSE)
            {
                brush.setColor(Qt::red);
                painter.setBrush(brush);
                painter.drawEllipse(margin + i*blockLen - pieceSize/4, margin + j*blockLen - pieceSize/4, pieceSize/2, pieceSize/2);
            }
        }
    }


}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    clickPosRow = -1;
    clickPosCol = -1;

    gameReqData reqData;

    /*得到棋盘坐标*/
    if (event->x() >= margin - blockLen/2 && event->x() <= margin + blockLen * (blockLen - 1) + blockLen/2 &&
        event->y() >= margin - blockLen/2 && event->y() <= margin + blockLen * (blockLen - 1) + blockLen/2)
    {
        clickPosRow = (event->x() - margin + blockLen/2)/blockLen;
        clickPosCol = (event->y() -margin + blockLen/2)/ blockLen;
        reqData.x = clickPosRow;
        reqData.y = clickPosCol;
    }

    /*棋盘区域，本地打谱模式，右键*/
    if (clickPosCol >=0 && clickPosRow >=0
            &&  clickPosCol < 19 && clickPosRow < 19
            && nGame->mode == LOCALGAME
            && nGame->status != ADDBLACKPIECE && nGame->status != ADDWHITEPIECE && nGame->status !=REMOVEPIECE
            && event->button() == Qt::RightButton)
    {
        /*退一步,且清空前进栈*/
        nGame->backOneStep();
        nGame->clearFrwdStack();
    }
    /*棋盘区域，左键*/
    else if (clickPosCol >=0 && clickPosRow >=0  //无子，且在棋盘内，可落子
           &&  clickPosCol < 19 && clickPosRow < 19
           && event->button() == Qt::LeftButton)
    {
       if ((nGame->status == WAITEPLAYERACT_1 || nGame->status == WAITEPLAYERACT_2)  //黑/白方落子
           && nGame->getPointSta(clickPosRow, clickPosCol) == NOPIECE)
       {
            int color = NOPIECE;
            if (nGame->status == WAITEPLAYERACT_1)
                color = BLACK;
            else if (nGame->status == WAITEPLAYERACT_2)
                color = WHITE;

            if (nGame->mode == NETGAME)
            {
                /*不该自己落子，则点击无效，返回*/
                if ((nGame->curPlayer->color == BLACK && nGame->status == WAITEPLAYERACT_2)
                        || (nGame->curPlayer->color == WHITE && nGame->status == WAITEPLAYERACT_1))
                {
                    return ;
                }
            }

            int ret = nGame->gameReqRecver(0, color, ADDPIECE, reqData);

            /*若是网络对局，且落子成功，将落点发送给对方*/
            if (ret == 0 && nGame->mode == NETGAME)
            {
                char sendmsg[120] = {0};
                sendmsg[0] = PLACEONEPIECE;
                sendmsg[1] = 0x61;
                sendmsg[52] = 0x62;
                sendmsg[103] = 0x19;
                if (nGame->curPlayer == nGame->player_1)
                    strncpy(sendmsg+53, nGame->player_2->username.toUtf8().data(), 49);
                else if (nGame->curPlayer == nGame->player_2)
                    strncpy(sendmsg+53, nGame->player_1->username.toUtf8().data(), 49);
                strncpy(sendmsg+2, nGame->curUsername.toUtf8().data(), 49);
                sendmsg[104] = (char)(clickPosRow);
                sendmsg[105] = (char)(clickPosCol);
                udpsock->writeDatagram(sendmsg, sizeof(sendmsg), QHostAddress(serverIp), port);
            }

            /*任何情况下，调用落子，即可将打谱前进栈清空*/
            if (ret == 0)
                nGame->clearFrwdStack();
       }
       else if ((nGame->status == ADDBLACKPIECE || nGame->status == ADDWHITEPIECE)  //直接添加棋子
                && nGame->mode == LOCALGAME)
       {
           if (nGame->getPointSta(clickPosRow, clickPosCol) == NOPIECE)
           nGame->gameReqRecver(0, -1, ADDPIECEDRRECT, reqData);
           else if (nGame->getPointSta(clickPosRow, clickPosCol) == BLACK
                   || nGame->getPointSta(clickPosRow, clickPosCol) == WHITE)
               nGame->setPointSta(clickPosRow, clickPosCol, NOPIECE);
       }
       else if (nGame->status == REMOVEPIECE && nGame->mode == LOCALGAME)  //移除棋子
       {
           nGame->gameReqRecver(0, -1, RMPIECE, reqData);
       }
       else if ((nGame->status == FLAGDIEPIECE_1
                 || nGame->status == FLAGDIEPIECE_2
                 || nGame->status == FLAGDIEPIECE)
                && nGame->mode == LOCALGAME)
       {
           nGame->endCal->clickDiePiece(clickPosRow, clickPosCol,
                                nGame->getPointSta(clickPosRow, clickPosCol));
       }
    }

   if (nGame->status == WAITEPLAYERACT_1)
       ui->blackOrWhite->setText(tr("黑方行棋"));
   else if (nGame->status == WAITEPLAYERACT_2)
       ui->blackOrWhite->setText(tr("白方行棋"));
   update();
   //repaint();
}

void Widget::resizeEvent(QResizeEvent * event)
{
    event->setAccepted(true);

    if (geometry().height() <= (geometry().width()-250))
    {
        margin = (int)(D_MARGIN * ((double)geometry().height()/(double)D_WIDGETHEIGHT));
        blockLen = (int)(D_BLOCKLEN * ((double)geometry().height()/(double)D_WIDGETHEIGHT));
        pieceSize = (int)(D_PIECESIZE * ((double)geometry().height()/(double)D_WIDGETHEIGHT));
        starSize = (int)(D_STARSIZE * ((double)geometry().height()/(double)D_WIDGETHEIGHT));
    }
    else
    {
        margin = (int)(D_MARGIN * ((double)geometry().width()/(double)D_WIDGETWIDTH));
        blockLen = (int)(D_BLOCKLEN * ((double)geometry().width()/(double)D_WIDGETWIDTH));
        pieceSize = (int)(D_PIECESIZE * ((double)geometry().width()/(double)D_WIDGETWIDTH));
        starSize = (int)(D_STARSIZE * ((double)geometry().width()/(double)D_WIDGETWIDTH));
    }

    update();
    //repaint();
}

/*滚轮控制前进后退--本地打谱模式使用*/
void Widget::wheelEvent(QWheelEvent *event)
{
    if (nGame->mode == LOCALGAME
        && nGame->status != FLAGDIEPIECE_1
        && nGame->status != FLAGDIEPIECE_2
        && nGame->status != ADDBLACKPIECE
        && nGame->status != ADDWHITEPIECE
        && nGame->status != REMOVEPIECE)
    {
        if (event->delta() > 0)
            nGame->frwdOneStep();
        else
            nGame->backOneStep();

        if (nGame->status == WAITEPLAYERACT_1)
            ui->blackOrWhite->setText(tr("黑方行棋"));
        else if (nGame->status == WAITEPLAYERACT_2)
            ui->blackOrWhite->setText(tr("白方行棋"));

        update();
    }
}

void Widget::closeEvent(QCloseEvent *event)
{
    event->setAccepted(true);
    if (nGame->curUsername.isEmpty())
        this->close();


    /*若正在进行网络对局,发送认输*/
    if (nGame->mode == NETGAME)
    {
        char sendmsg[120] = {0};
        sendmsg[0] = GIVEUP;
        sendmsg[1] = 0x61;
        sendmsg[52] = 0x62;

        if (nGame->curPlayer == nGame->player_1)
            strncpy(sendmsg+53, nGame->player_2->username.toUtf8().data(), 49);
        else if (nGame->curPlayer == nGame->player_2)
            strncpy(sendmsg+53, nGame->player_1->username.toUtf8().data(), 49);
        strncpy(sendmsg+2, nGame->curUsername.toUtf8().data(), 49);

        udpsock->writeDatagram(sendmsg, sizeof(sendmsg), QHostAddress(serverIp), port);
    }

    /*若已登录，先登出再退出*/
    char sendbuf[60] = {0};
    sendbuf[0] = 0x05;
    sendbuf[1] = 0x11;
    strcpy(sendbuf+2, nGame->curUsername.toUtf8().data());
    udpsock->writeDatagram(sendbuf, sizeof(sendbuf),QHostAddress(serverIp), port);

    this->close();
}

void Widget::on_addBlackButton_clicked()
{
    if (nGame->status != GAMEEND && nGame->mode == LOCALGAME)
    {
        if (nGame->status != ADDBLACKPIECE)
        {
            nGame->status = ADDBLACKPIECE;
            ui->addBlackButton->setText(tr("关闭添加黑子"));
            ui->addWhiteButton->setText(tr("添加白子"));
            ui->rmPieceButton->setText("移除棋子");
        }
        else
        {
            nGame->status = WAITEPLAYERACT_2;
            ui->blackOrWhite->setText(tr("白方行棋"));
            ui->addBlackButton->setText(tr("添加黑子"));
        }
    }
}

void Widget::on_addWhiteButton_clicked()
{
    if (nGame->status != GAMEEND && nGame->mode == LOCALGAME)
    {
        if (nGame->status != ADDWHITEPIECE)
        {
            nGame->status = ADDWHITEPIECE;
            ui->addWhiteButton->setText(tr("关闭添加白子"));
            ui->addBlackButton->setText(tr("添加黑子"));
            ui->rmPieceButton->setText("移除棋子");
        }
        else
        {
            nGame->status = WAITEPLAYERACT_1;
            ui->blackOrWhite->setText(tr("黑方行棋"));
            ui->addWhiteButton->setText(tr("添加白子"));
        }
    }
}

void Widget::on_rmPieceButton_clicked()
{
    if (nGame->status != GAMEEND && nGame->mode == LOCALGAME)
    {
        if (nGame->status != REMOVEPIECE)
        {
            nGame->status = REMOVEPIECE;
            ui->rmPieceButton->setText("关闭移除棋子");
            ui->addBlackButton->setText(tr("添加黑子"));
            ui->addWhiteButton->setText(tr("添加白子"));
        }
        else
        {
            nGame->status = WAITEPLAYERACT_1;
            ui->blackOrWhite->setText(tr("黑方行棋"));
            ui->rmPieceButton->setText("移除棋子");
        }
    }
}

void Widget::on_changeColorButton_clicked()
{
    if (nGame->mode == LOCALGAME)
    {
        if (nGame->status == WAITEPLAYERACT_1)
        {
            nGame->status = WAITEPLAYERACT_2;
            ui->blackOrWhite->setText(tr("白方行棋"));
        }
        else if (nGame->status == WAITEPLAYERACT_2)
        {
            nGame->status = WAITEPLAYERACT_1;
            ui->blackOrWhite->setText(tr("黑方行棋"));
        }
    }
}

void Widget::on_endButton_clicked()
{
    if (nGame->mode == NETGAME)
    {
        char sendmsg[120] = {0};
        sendmsg[0] = GIVEUP;
        sendmsg[1] = 0x61;
        sendmsg[52] = 0x62;

        if (nGame->curPlayer == nGame->player_1)
            strncpy(sendmsg+53, nGame->player_2->username.toUtf8().data(), 49);
        else if (nGame->curPlayer == nGame->player_2)
            strncpy(sendmsg+53, nGame->player_1->username.toUtf8().data(), 49);
        strncpy(sendmsg+2, nGame->curUsername.toUtf8().data(), 49);

        udpsock->writeDatagram(sendmsg, sizeof(sendmsg), QHostAddress(serverIp), port);
    }
    nGame->status = GAMEEND;
    ui->beginButton->setEnabled(true);
    ui->addBlackButton->setEnabled(true);
    ui->addWhiteButton->setEnabled(true);
    ui->rmPieceButton->setEnabled(true);
    ui->changeColorButton->setEnabled(true);
    ui->endButton->setEnabled(true);
    nGame->player_1->username.clear();
    nGame->player_2->username.clear();
}

void Widget::on_beginButton_clicked()
{
    /*正在进行网络对局，不允许新开本地对局*/
    if (nGame->mode == NETGAME && nGame->status != GAMEEND)
        return;

    /*凡是通过‘新开对局’按钮启动的游戏，都是本地对局*/
    nGame->mode = LOCALGAME;
    ui->endButton->setText(tr("对局结束"));
    gamedialog->show();
}

void Widget::on_dlgOkBtn_clicked()
{
    int pieceNum = 0;
    bool ok;
    int textRet = dlgPieceCmb->currentText().toInt(&ok, 10);
    if (dlgPieceCmb->currentText() == "分先" || dlgPieceCmb->currentText() == "定先")
    {
        pieceNum = 0;
        if (dlgPieceCmb->currentText() == "定先")
        {
            dlgColorCmb->setEnabled(true);
        }
    }
    else if (dlgPieceCmb->currentText() == "让先")
    {
        pieceNum = 0;
        dlgNegativeCmb->setCurrentIndex(0);
    }
    else if (ok)//让子棋
    {
        pieceNum = textRet;
        dlgNegativeCmb->setCurrentIndex(0);
    }

    nGame->negativeNum = dlgNegativeCmb->currentText().toDouble(&ok);

    ui->rmPieceButton->setText("移除棋子");
    ui->addBlackButton->setText(tr("添加黑子"));
    ui->addWhiteButton->setText(tr("添加白子"));

    if (nGame->mode == LOCALGAME)
    {
        nGame->gameStart(MAXLINE, pieceNum);
        gamedialog->setVisible(false);
        ui->blackOrWhite->setText(tr("黑方行棋"));
        update();
    }

}

void Widget::on_dlgCancelBtn_clicked()
{
    gamedialog->setVisible(false);
    nGame->mode = LOCALGAME;
    ui->endButton->setText(tr("对局结束"));
}

void Widget::on_piececmbxchange(QString text)
{
    bool ok;
    text.toInt(&ok, 10);
    if (text == "分先" || text == "定先")
    {
        dlgNegativeCmb->setCurrentIndex(3);
        dlgColorCmb->setEnabled(false);
        if (text == "定先")
        {
            dlgColorCmb->setEnabled(true);
            dlgColorCmb->setCurrentIndex(0);
        }
    }
    else if (text == "让先")
    {
        dlgNegativeCmb->setCurrentIndex(0);
        dlgColorCmb->setEnabled(false);
    }
    else if (ok)
    {
        dlgNegativeCmb->setCurrentIndex(0);
        dlgColorCmb->setEnabled(false);
    }
}

void Widget::on_loginOrRegisterBtn_clicked()
{
    logindialog->show();
}

void Widget::on_loginBtn_clicked()
{
    if (!nGame->curUsername.isEmpty())
    {
        QMessageBox::information(this, tr("提示"), tr("已登录,请先退出再重新登录"));
        this->logindialog->setVisible(false);
        return ;
    }

    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if (username == "")
    {
        QMessageBox::information(this->logindialog, tr("提示"), "请填写用户名");
    }
    else if (password == "")
    {
        QMessageBox::information(this->logindialog, tr("提示"), "请填写密码");
    }
    else
    {
        if (username.length() >= 35)
        {
            QMessageBox::information(this->logindialog, tr("提示"), "用户名过长");
        }
        else if (password.length() >= 35)
        {
            QMessageBox::information(this->logindialog, tr("提示"), "密码过长");
        }
        else
        {
            qDebug()<<"on_loginBtn_clicked";
            qDebug()<<"username: "<<username;
            qDebug()<<"password: "<<password;
            char sendbuf[120] = {0};
            sendbuf[0] = 0x02;
            sendbuf[1] = 0x11;
            sendbuf[52] = 0x12;
            strcpy(sendbuf+2, username.toUtf8().data());
            strcpy(sendbuf+53, password.toUtf8().data());
            if (udpsock->writeDatagram(sendbuf, sizeof(sendbuf), QHostAddress(serverIp), port)
                    == sizeof(sendbuf))
            {
                qDebug()<<"发送成功，等待服务器回应...";
            }
        }
    }
}

void Widget::on_registerBtn_clicked()
{
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if (username == "")
    {
        QMessageBox::information(this->logindialog, tr("提示"), "请填写用户名");
    }
    else if (password == "")
    {
        QMessageBox::information(this->logindialog, tr("提示"), "请填写密码");
    }
    else
    {
        if (username.length() >= 35)
        {
            QMessageBox::information(this->logindialog, tr("提示"), "用户名过长");
        }
        else if (password.length() >= 35)
        {
            QMessageBox::information(this->logindialog, tr("提示"), "密码过长");
        }
        else
        {
            qDebug()<<"on_registerBtn_clicked";
            qDebug()<<"username: "<<username;
            qDebug()<<"password: "<<password;
            char sendbuf[120] = {0};
            sendbuf[0] = 0x01;
            sendbuf[1] = 0x11;
            sendbuf[52] = 0x12;
            strcpy(sendbuf+2, username.toUtf8().data());
            strcpy(sendbuf+53, password.toUtf8().data());
            if (udpsock->writeDatagram(sendbuf, sizeof(sendbuf), QHostAddress(serverIp), port)
                    == sizeof(sendbuf))
            {
                qDebug()<<"发送成功，等待服务器回应...";
            }
        }
    }
}

/*处理UDP信息，用于网络对局*/
void Widget::on_udpmsg_recved()
{
    while (udpsock->hasPendingDatagrams())
    {
        QByteArray dataGram;
        dataGram.resize(udpsock->pendingDatagramSize());
        udpsock->readDatagram(dataGram.data(), dataGram.size());

        /*for debuge*/
        qDebug()<<"response msg from server: ";
        QString tmpmsg;
        for (int i = 0; i < dataGram.size(); i++)
        {
            char tmp[5] = {0};
            sprintf(tmp, "%02x ", dataGram.data()[i]);
            tmpmsg += tmp;
        }
        qDebug()<<tmpmsg;

        switch (dataGram.data()[0]) {
        case REGISTER:
        {
            if (dataGram.data()[1] == 0x21)
            {
                if (dataGram.data()[2] == 0x01)//注册成功
                {
                    QMessageBox::information(this->logindialog, tr("注册成功"), tr("恭喜您注册成功！"));
                }
                else if (dataGram.data()[2] == 0x02)//注册失败
                {
                    QString regFailReson = "不明原因";
                    if (dataGram.data()[4] == 0x03)
                        regFailReson = "该用户名已被注册";
                    QMessageBox::information(this->logindialog, tr("注册失败"), regFailReson);
                }
            }
        }
            break;
        case LOGIN:
        {
            if (dataGram.data()[1] == 0x21)
            {
                if (dataGram.data()[2] == 0x01)//登录成功
                {
                    ui->showUsernameLabel->setText(usernameEdit->text());
                    if(ui->playerList->findItems(usernameEdit->text(), Qt::MatchExactly).isEmpty())
                        ui->playerList->insertItem(0, usernameEdit->text());
                    logindialog->setVisible(false);
                    nGame->curUsername = usernameEdit->text();
                }
                else if (dataGram.data()[2] == 0x02 && dataGram.data()[3] == 0x22)//登录失败
                {
                    QString logFailReson = "不明原因";
                    if (dataGram.data()[4] == 0x01)
                        logFailReson = "该用户已登录";
                    else if (dataGram.data()[4] == 0x02)
                        logFailReson = "用户名或密码错误";
                    QMessageBox::information(this->logindialog, tr("登录失败"), logFailReson);
                }
            }
        }
            break;
        case LOGINOTHER://其他用户登录消息
        case LOGINGETOTHER://获取其他用户登录信息
        {
            if (dataGram.data()[1] == 0x11)
            {
                char nameStr[50] = {0};
                strcpy(nameStr, dataGram.data()+2);
                QString usrname = nameStr;

                if(ui->playerList->findItems(usrname, Qt::MatchExactly).isEmpty())
                    ui->playerList->insertItem(1, usrname);
                else
                    qDebug()<<"user "<<usrname<<" is already in playerlist";
            }
        }
            break;
        case LOGINOUTSUC://获取用户登出信息
        {
            if (dataGram.data()[1] == 0x11)
            {
                char namestr[50] = {0};
                strcpy(namestr, dataGram.data()+2);
                QString curLoginoutPlayer = namestr;
                if (curLoginoutPlayer == nGame->curUsername)
                {
                    nGame->curUsername.clear();
                    nGame->player_1->username.clear();
                    nGame->player_2->username.clear();
                    ui->playerColorLabel_2->setText(tr("player_1"));
                    ui->playerColorLabel_3->setText(tr("player_2"));
                    ui->showUsernameLabel->setText(tr("user"));
                    ui->playerList->clear();
                }
                else
                {
                    for (int i = 0; i < ui->playerList->count(); i++)
                    {
                        if (ui->playerList->item(i)->text() == curLoginoutPlayer)
                        {
                            ui->playerList->takeItem(i);
                        }
                    }
                }
            }
        }
            break;

        /*对局申请*/
        case PLAYGAMEREQ:
        {
            if (dataGram.length() < 120)
                return;
            char sendbuf[120] = {0};
            memcpy(sendbuf, dataGram.data(), 120);
            sendbuf[0] = 0x70;



            /*对方用户名*/
            char tmpname[50] = {0};
            if (dataGram.data()[1] == 0x61)
                strncpy(tmpname, dataGram.data()+2, 49);
            QString opName = tmpname;

            /*获取更详细的对局信息，让子，贴子等*/
            int pieceNum = 0;
            int negativeIndex = 0;

            if (dataGram.data()[103] == 0x63)
                pieceNum = (int)dataGram.data()[104];
            if (dataGram.data()[105] == 0x64)
                negativeIndex = (int)dataGram.data()[106];

            if (negativeIndex == 0)
                nGame->negativeNum = 0.0;
            else if (negativeIndex == 1)
                nGame->negativeNum = 0.75;
            else if (negativeIndex == 2)
                nGame->negativeNum = 1.75;
            else if (negativeIndex == 3)
                nGame->negativeNum = 3.75;
            else if (negativeIndex == 4)
                nGame->negativeNum = 5.75;

            /*获取棋子颜色(猜先结果)，并分配对局者name*/
            if (dataGram.data()[107] == 0x65)
            {
                if (dataGram.data()[108] == 0x01) //0x01--申请方黑子，己方白子
                {
                    nGame->curPlayer = nGame->player_2;
                    nGame->player_2->username = nGame->curUsername;
                    nGame->player_1->username = opName;
                }
                else if (dataGram.data()[108] == 0x02) //0x02--申请方白子，己方黑子
                {
                    nGame->curPlayer = nGame->player_1;
                    nGame->player_1->username = nGame->curUsername;
                    nGame->player_2->username = opName;
                }
                else
                    return ; //信息错误

                ui->playerColorLabel_2->setText(nGame->player_1->username);
                ui->playerColorLabel_3->setText(nGame->player_2->username);
            }

            /*选择接受或拒绝对局请求*/
            if (QMessageBox::question(this, tr("对局申请"), opName + tr("向您提出对局申请"),
                                      QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
            {
                sendbuf[110] = 0x01;
                nGame->mode = NETGAME;
                nGame->gameStart(MAXLINE, pieceNum);
                ui->blackOrWhite->setText(tr("黑方行棋"));
                ui->endButton->setText(tr("认输"));
                ui->beginButton->setEnabled(false);
                ui->addBlackButton->setEnabled(false);
                ui->addWhiteButton->setEnabled(false);
                ui->rmPieceButton->setEnabled(false);
                ui->changeColorButton->setEnabled(false);
                update();

            }
            else
            {
                nGame->mode = LOCALGAME;
                sendbuf[110] = 0x00;
            }
            udpsock->writeDatagram(sendbuf, sizeof(sendbuf), QHostAddress(serverIp), port);
        }
            break;

        /*对局申请回应*/
        case PLAYGAMERSP:
        {
            if (dataGram.length() < 115)
                return;

            if (dataGram.data()[109] == 0x66)
            {
                if (dataGram.data()[110] == 0x00) //对方拒绝对局申请
                {
                    nGame->mode = LOCALGAME;
                    QMessageBox::information(this, tr("请求结果"), tr("对方拒绝对局请求"));
                    return;
                }
                else if (dataGram.data()[110] != 0x01)
                {
                    nGame->mode = LOCALGAME;
                    return ; //信息错误
                }
            }
            else
            {
                nGame->mode = LOCALGAME;
                return ; //信息错误
            }

            /*对方用户名*/
            char tmpname[50] = {0};
            if (dataGram.data()[52] == 0x62)
                strncpy(tmpname, dataGram.data()+53, 49);
            QString opName = tmpname;

            /*获取更详细的对局信息，让子，贴子等*/
            int pieceNum = 0;
            int negativeIndex = 0;

            if (dataGram.data()[103] == 0x63)
                pieceNum = (int)dataGram.data()[104];
            if (dataGram.data()[105] == 0x64)
                negativeIndex = (int)dataGram.data()[106];

            if (negativeIndex == 0)
                nGame->negativeNum = 0.0;
            else if (negativeIndex == 1)
                nGame->negativeNum = 0.75;
            else if (negativeIndex == 2)
                nGame->negativeNum = 1.75;
            else if (negativeIndex == 3)
                nGame->negativeNum = 3.75;
            else if (negativeIndex == 4)
                nGame->negativeNum = 5.75;

            /*获取棋子颜色*/
            if (dataGram.data()[107] == 0x65)
            {
                if (dataGram.data()[108] == 0x01) //0x01--申请方为己方，黑子
                {
                    nGame->curPlayer = nGame->player_1;
                    nGame->player_1->username = nGame->curUsername;
                    nGame->player_2->username = opName;
                }
                else if (dataGram.data()[108] == 0x02) //0x02--申请方为己方，白子
                {
                    nGame->curPlayer = nGame->player_2;
                    nGame->player_2->username = nGame->curUsername;
                    nGame->player_1->username = opName;
                }
                else
                    return ; //信息错误
                ui->playerColorLabel_2->setText(nGame->player_1->username);
                ui->playerColorLabel_3->setText(nGame->player_2->username);
            }

            nGame->mode = NETGAME;
            nGame->gameStart(MAXLINE, pieceNum);
            ui->blackOrWhite->setText(tr("黑方行棋"));
            ui->endButton->setText(tr("认输"));
            ui->beginButton->setEnabled(false);
            ui->addBlackButton->setEnabled(false);
            ui->addWhiteButton->setEnabled(false);
            ui->rmPieceButton->setEnabled(false);
            ui->changeColorButton->setEnabled(false);
            update();
        }
            break;

        /*网络对局时对方落子信息*/
        case PLACEONEPIECE:
        {
            char playerName[50] = {0}; //落子方
            char acceptPlayerName[50] = {0}; //接受方， 此处自己是接受方
            QString playerNameStr;
            QString acceptPlayerNameStr;
            int pos_x = -1;
            int pos_y = -1;
            if (dataGram.data()[1] == 0x61)
            {
                strncpy(playerName, dataGram.data()+2, 49);
                playerNameStr = playerName;
            }
            else
                return ;
            if (dataGram.data()[52] == 0x62)
            {
                strncpy(acceptPlayerName, dataGram.data()+53, 49);
                acceptPlayerNameStr = acceptPlayerName;
            }
            else
                return ;


            if (dataGram.data()[103] == 0x19)
            {
                pos_x = int(dataGram.data()[104]);
                pos_y = int(dataGram.data()[105]);
                if (pos_x >= 19 || pos_y >= 19 || pos_x < 0 || pos_y < 0)
                    return ;
            }
            else
                return ;

            gameReqData reqData;
            reqData.x = pos_x;
            reqData.y = pos_y;
            int color = 0;
            if (nGame->curPlayer->color == BLACK)
                color = WHITE;
            else if (nGame->curPlayer->color == WHITE)
                color = BLACK;
            if (nGame->gameReqRecver(0, color, ADDPIECE, reqData) == 0)
            {
                nGame->clearFrwdStack();
            }
            if (nGame->status == WAITEPLAYERACT_1)
                ui->blackOrWhite->setText(tr("黑方行棋"));
            else if (nGame->status == WAITEPLAYERACT_2)
                ui->blackOrWhite->setText(tr("白方行棋"));
            update();

        }
            break;

        case GIVEUP:
        {
            nGame->status = GAMEEND;
            nGame->player_1->username.clear();
            nGame->player_2->username.clear();
            QMessageBox::information(this, tr("对方认输"), tr("恭喜您取得了胜利！"));
        }
            break;

        default:
            break;
        }
    }
}

void Widget::on_loginoutBtn_clicked()
{
    /*若正在进行网络对局,发送认输*/
    if (nGame->mode == NETGAME)
    {
        char sendmsg[120] = {0};
        sendmsg[0] = GIVEUP;
        sendmsg[1] = 0x61;
        sendmsg[52] = 0x62;

        if (nGame->curPlayer == nGame->player_1)
            strncpy(sendmsg+53, nGame->player_2->username.toUtf8().data(), 49);
        else if (nGame->curPlayer == nGame->player_2)
            strncpy(sendmsg+53, nGame->player_1->username.toUtf8().data(), 49);
        strncpy(sendmsg+2, nGame->curUsername.toUtf8().data(), 49);

        udpsock->writeDatagram(sendmsg, sizeof(sendmsg), QHostAddress(serverIp), port);
    }

    nGame->mode = LOCALGAME;
    if (nGame->curUsername.isEmpty())
        return ;

    char sendbuf[60] = {0};
    sendbuf[0] = 0x05;
    sendbuf[1] = 0x11;
    strcpy(sendbuf+2, nGame->curUsername.toUtf8().data());
    if (udpsock->writeDatagram(sendbuf, sizeof(sendbuf), QHostAddress(serverIp), port)
            == sizeof(sendbuf))
    {
        qDebug()<<"发送成功";
    }
    else
    {
        QMessageBox::information(this, tr("提示"),tr("登出失败"));
    }
    nGame->curUsername.clear();
    ui->showUsernameLabel->setText(tr("user"));
}

//右键申请对局菜单
void Widget::on_playerList_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem *curItem = ui->playerList->itemAt(pos);
    if (curItem == nullptr)
        return;

    ui->playerList->setCurrentItem(curItem);

    /*不可向自己申请*/
    if (curItem->text() == nGame->curUsername)
        return;

    QMenu *popMenu = new QMenu(this);
    QAction *playReqAction = new QAction(tr("申请对局"), this);
    popMenu->addAction(playReqAction);
    connect(playReqAction, SIGNAL(triggered()), this, SLOT(on_playreq_action()));
    popMenu->exec(QCursor::pos());
    delete popMenu;
    delete playReqAction;
}

/*申请网络对局相关设置,并申请网络对局*/
void Widget::on_playreq_action()
{
    char sendbuf[120] = {0};
    sendbuf[0] = 0x69;
    sendbuf[1] = 0x61;
    strncpy(sendbuf+2, nGame->curUsername.toUtf8().data(), 49);
    sendbuf[52] = 0x62;
    strncpy(sendbuf+53, ui->playerList->currentItem()->text().toUtf8().data(), 49);
    sendbuf[103] = 0x63;
    sendbuf[104] = 0x00;
    sendbuf[105] = 0x64;
    sendbuf[106] = 0x03;
    sendbuf[107] = 0x65;
    sendbuf[108] = 0x01;
    sendbuf[109] = 0x66;
    sendbuf[110] = 0x01;

    /*猜先，0x01--black, 0x02--white*/
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    int color = qrand() % 2;
    if (color == 1)
        sendbuf[108] = 0x01;
    else
        sendbuf[108] = 0x02;

    udpsock->writeDatagram(sendbuf, sizeof(sendbuf), QHostAddress(serverIp), port);
}

void Widget::on_clickPieceBtn_clicked()
{

    if (nGame->status == WAITEPLAYERACT_1
            || nGame->status == WAITEPLAYERACT_2
            || nGame->status == GAMEEND)
    {
        if (nGame->status == WAITEPLAYERACT_1)
            nGame->status = FLAGDIEPIECE_1;
        if (nGame->status == WAITEPLAYERACT_2)
            nGame->status = FLAGDIEPIECE_2;
        if (nGame->status == GAMEEND)
            nGame->status = FLAGDIEPIECE;

        ui->clickPieceBtn->setText(tr("点子结束"));
        ui->beginButton->setEnabled(false);
        ui->addBlackButton->setEnabled(false);
        ui->addWhiteButton->setEnabled(false);
        ui->rmPieceButton->setEnabled(false);
        ui->changeColorButton->setEnabled(false);
        ui->endButton->setEnabled(false);
    }
    else if (nGame->status == FLAGDIEPIECE_1
             || nGame->status == FLAGDIEPIECE_2
             || nGame->status == FLAGDIEPIECE)
    {
        if (nGame->status == FLAGDIEPIECE_1)
            nGame->status = WAITEPLAYERACT_1;
        else if (nGame->status == FLAGDIEPIECE_2)
            nGame->status = WAITEPLAYERACT_2;
        else if (nGame->status == FLAGDIEPIECE)
            nGame->status = GAMEEND;

        ui->clickPieceBtn->setText(tr("标记死子"));

        /*计算对局结果*/
        nGame->endCal->countBlack();//计算
        update();
        double bNum = nGame->endCal->getBlackCount();//获取黑子数
        double wNum = nGame->endCal->getWhiteCount();//获取白子数
        QString resultStr = "黑子:" + QString::number(bNum)
                + " -" + QString::number(nGame->negativeNum) +"\n"
                + "白子:" + QString::number(wNum) + " +" + QString::number(nGame->negativeNum);
        double gameResult = bNum - nGame->negativeNum - 180.5;
        if (gameResult > 0)
            resultStr += "\n" + tr("黑胜 ") + QString::number(gameResult)+ "子";
        else if (gameResult < 0)
        {
            gameResult *= -1.0;
            resultStr += "\n" + tr("白胜 ") + QString::number(gameResult) + "子";
        }
        else if (gameResult == 0)
            resultStr += "\n" + tr("和棋");

        if (QMessageBox::question(this, tr("数子结果"), resultStr,
                         QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
        {
            if (bNum - 180.5 - nGame->negativeNum > 0)
                QMessageBox::information(this, tr("对局结果"), tr("黑胜"));
            else if (bNum - 180.5 - nGame->negativeNum < 0)
                QMessageBox::information(this, tr("对局结果"), tr("白胜"));
            else
                QMessageBox::information(this, tr("对局结果"), tr("和棋"));
            if (nGame->mode == LOCALGAME)
                nGame->status = GAMEEND;
        }
        else
        {
            QMessageBox::information(this, tr("点子结束"), tr("继续对局"));
        }

        ui->beginButton->setEnabled(true);
        ui->addBlackButton->setEnabled(true);
        ui->addWhiteButton->setEnabled(true);
        ui->rmPieceButton->setEnabled(true);
        ui->changeColorButton->setEnabled(true);
        ui->endButton->setEnabled(true);

        nGame->endCal->refresh();//计算后刷新棋盘状态
        update();
    }
}

void Widget::on_refreshPlayerListBtn_clicked()
{
    if (nGame->curUsername.isEmpty())
        return ;

    char sendmsg[60] = {0};
    sendmsg[0] = REFRESHPLAYERLIST;
    sendmsg[1] = 0x61;
    strncpy(sendmsg+2, nGame->curUsername.toUtf8().data(), 49);
    udpsock->writeDatagram(sendmsg, sizeof(sendmsg), QHostAddress(serverIp), port);
}

#pragma once

#include <QWidget>
#include <QPainter>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QUdpSocket>
#include <QListWidget>
#include <QString>


#define D_BLOCKLEN 36
#define D_MARGIN 30
#define D_PIECESIZE 30
#define D_STARSIZE 10
#define D_WIDGETHEIGHT 720
#define D_WIDGETWIDTH 1000

enum RespType
{
    REGISTER = 0x01,
    LOGIN  = 0x02,
    LOGINOTHER = 0x03,
    LOGINGETOTHER = 0x04,
    LOGINOUTSUC = 0x05,
    REFRESHPLAYERLIST = 0x06,
    PLAYGAMEREQ = 0x69,
    PLAYGAMERSP = 0x70,
    PLACEONEPIECE = 0x71,

    GIVEUP = 0x72
};


class GoGame;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;

    //请求对局对话框
    QDialog *gamedialog;
    QGridLayout * dlgGrdLayout;
    QPushButton * dlgOkBtn;
    QPushButton * dlgCancelBtn;
    QLabel *dlgSelectLabel;//让子标签
    QComboBox *dlgPieceCmb;
    QLabel *dlgNegativeLabel;//贴子标签
    QComboBox *dlgNegativeCmb;
    QLabel *dlgcolorLabel;//选择黑白标签
    QComboBox *dlgColorCmb;

    //登录注册对话框
    QDialog *logindialog;
    QGridLayout *loginDlgGridLayout;
    QPushButton *loginBtn;
    QPushButton *registerBtn;
    QLabel *usernameLabel;
    QLabel *passwordLabel;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;

protected:
    void paintEvent(QPaintEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent * event);
    void wheelEvent(QWheelEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    int clickPosRow;
    int clickPosCol;

public:
    QString serverIp;
    int port;
    QUdpSocket *udpsock;
    GoGame * nGame;
    int margin;
    int blockLen;
    int pieceSize;
    int starSize;

private slots:
    void on_addBlackButton_clicked();
    void on_addWhiteButton_clicked();
    void on_rmPieceButton_clicked();
    void on_changeColorButton_clicked();
    void on_endButton_clicked();
    void on_beginButton_clicked();
    void on_dlgOkBtn_clicked();
    void on_dlgCancelBtn_clicked();
    void on_piececmbxchange(QString);
    void on_loginOrRegisterBtn_clicked();
    void on_loginBtn_clicked();
    void on_registerBtn_clicked();
    void on_udpmsg_recved();
    void on_loginoutBtn_clicked();
    void on_playerList_customContextMenuRequested(const QPoint &pos);
    void on_playreq_action();
    void on_clickPieceBtn_clicked();
    void on_refreshPlayerListBtn_clicked();
};

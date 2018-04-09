#pragma once

#include <QString>

class Board;
class Player;
class HandStap;
class GameEndCal;
struct StapNode;


enum GameStat
{
    GAMESTART = 0, //游戏开始(实际不会用到)
    WAITEPLAYERACT_1, //该黑落子
    WAITEPLAYERACT_2, //该白落子
    ADDBLACKPIECE,  //直接添加黑子
    ADDWHITEPIECE, //直接添加白子
    REMOVEPIECE, //移除棋子

    FLAGDIEPIECE,  //标记死子-终局时
    FLAGDIEPIECE_1, //标记死子-与WAITEPLAYERACT_1切换
    FLAGDIEPIECE_2, //标记死子-与WAITEPLAYERACT_2切换
    GAMEEND //终局
};

enum GameMode
{
    LOCALGAME = 0,
    NETGAME
};

enum reqType
{
    ADDPIECE = 1, //正常落子
    ADDPIECEDRRECT, //直接添加棋子
    RMPIECE //移除棋子
};

struct gameReqData
{
    int x;
    int y;
};

class GoGame
{
public:
	GoGame();
	~GoGame();
	int gameStart();
    int gameStart(int size, int pieces); //按照棋盘尺寸，让子数开始游戏，目前仅支持19*19规格
    int getPointSta(int x, int y);
    void setPointSta(int x, int y, int color);

    /****************************************************
     * gameReqRecver:请求接收器，根据player的请求，对游戏棋盘进行操作
     *reqNum:请求编号，请求的序列号
     *color:请求方的颜色
     *reqTyp：请求类型，包括落子，认输等
     *data，请求的具体细节内容，包括落子位置，聊天字符串等
     ****************************************************/
	int gameReqRecver(int reqNum, int color, int reqType, gameReqData data);

    void backOneStep();//退一步
    void frwdOneStep();//进一步
    GameStat status;//游戏状态
    GameMode mode;//游戏模式    
    Player * curPlayer;
    Player * player_1;
    Player * player_2;
    GameEndCal *endCal;
    QString curUsername;
    double negativeNum;


    int getPersentHandNum();//获取当前手数
    void clearFrwdStack();

    /* 获取前一手(当前已成功落下的最后一手)的部分信息
     * 死子数 *c,若*c为1，则(*x,*y)为该死子的坐标
     * 用于打劫判断,使游戏不能连续提劫*/
    void getPreHandInfo(int *pre_x, int *pre_y, int *preDead_x, int *preDead_y, int *c) const;

private:
    Board * pGoBoard;
    HandStap *handstap;
	
};


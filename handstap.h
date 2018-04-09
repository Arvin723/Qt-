#pragma once

#include <QQueue>
#include <QStack>

class GoGame;

struct BordPoint
{
  int x;
  int y;
};

/*记录一步棋的信息，包括手数、位置、提子*/
struct StapNode
{
    int handNum;//手数
    int color;
    BordPoint stapPoint;//位置
    QQueue<BordPoint> deadPiece;//提子
};

/*对局当前状态记录*/
class HandStap
{
public:
    HandStap();
    void clearFrwdStack();
    void initHandStap();
    GoGame * game;

private:
    int handNum; //当前手数
    QStack<StapNode> handstack; //状态记录栈
    QStack<StapNode> frwdstack; //状态记录栈

    void addOneHand(StapNode newStap); //添加一步棋到栈中
    void backOneStep(); //退一步
    void frwdOneStep();

    /*若上一手棋提起一子，则获取被提子的坐标，否则该坐标为 -1，-1*/
    void preHandInfo(int *pre_x, int *pre_y, int *preDead_x, int *preDead_y, int *c) const;

    friend class GoGame;
};

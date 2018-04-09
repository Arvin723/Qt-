#pragma once

enum endPieceType
{
    PENDING = 80,//待定
    BLACKDIE,//黑死子
    WHITEDIE,//白死子
    BLACKEMPTY,//黑空
    WHITEEMPTY,//白空
    SINGLEYOSE,//单官，包括共活状态下的公气
};

class GoGame;


class GameEndCal
{

public:
    GameEndCal();

    GoGame *game;
    double getBlackCount() const;
    double getWhiteCount() const;

    int clickDiePiece(int x, int y, int color);//确定死子
    int emptyJudge(int x, int y);
    void refresh();//计算后刷新棋盘状态
    double countBlack();

private:
    int confirmEmpty(int x, int y, char *colorflag);//确定无子点的性质,color必须带入0x00
    int resetCount();
    double blackNum;
    double whiteNum;
};

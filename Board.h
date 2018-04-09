#pragma once

#define BLACK 1
#define WHITE -1
#define INDETEMINATE 4 //提子判断时的中间状态
#define DEADWHITE_EDN 5 //终局时标记的死子白
#define DEADBLACK_END 6 //终局时标记的死子黑
#define NOPIECE 0 //无子
#define MAXLINE 19 //十九道

class GoGame;
struct StapNode;


class Board
{
public:
	int boardGo[MAXLINE][MAXLINE];
    GoGame * game;
	void initBoard();
    void initBoard(int size, int pieces);
	Board();
	~Board();

    /* 	由color色棋子落子，
	*	返回值：0--落子成功；1--落子失败 */
	int addOnePiece(int x, int y, int color);

private:
    int getSta(int x, int y, int color); //该棋子是否可探测到气：有-0，无-1
	int tryToKillOnePieceBlock(int x, int y, int color);
	int refresh(int newStat);
    int refresh(int newStat, int refreshType); //refreshType == 0 ---kill

};


#include "Board.h"
#include "handstap.h"
#include "GoGame.h"


/*临时储存当前一步的信息*/
StapNode temStap;

Board::Board()
{
    this->initBoard();
}


Board::~Board()
{
}


void Board::initBoard()
{
	for (int i = 0; i < MAXLINE; i++)
	{
		for (int j = 0; j < MAXLINE; j++)
			boardGo[i][j] = NOPIECE;
    }
}

/*根据让子数初始化棋盘*/
void Board::initBoard(int size, int pieces)
{
    if (size == MAXLINE)
    {
        for (int i = 0; i < MAXLINE; i++)
        {
            for (int j = 0; j < MAXLINE; j++)
            {
                boardGo[i][j] = NOPIECE;
                //让子数
                if (pieces == 9 && ((i == 3 || i == 9 || i == 15)
                     && (j == 3 || j == 9 || j == 15)))
                {
                    boardGo[i][j] = BLACK;
                }
                else if (pieces == 8 && ((i == 3 || i == 9 || i == 15)
                         && (j == 3 || j == 9 || j == 15))
                         && !(i == j && i == 9))
                {
                    boardGo[i][j] = BLACK;
                }
                else if (pieces == 7 && ((i == 3 || i == 9 || i == 15)
                         && (j == 3 || j == 9 || j == 15))
                         && !(j == 9 && (i == 3 || i == 15)))
                {
                    boardGo[i][j] = BLACK;
                }
                else if (pieces == 6 && ((i == 3 || i == 9 || i == 15)
                         && (j == 3 || j == 15)))
                {
                    boardGo[i][j] = BLACK;
                }
                else if (pieces == 5 && (((i == 3 || i == 15)
                         && (j == 3 || j == 15))
                         || (i == 9 && j == 9)))
                {
                    boardGo[i][j] = BLACK;
                }
                else if (pieces == 4 && ((i == 3 || i == 15)
                         && (j == 3 || j == 15)))
                {
                    boardGo[i][j] = BLACK;
                }
                else if (pieces == 3 && ((i == 3 || i == 15)
                         && (j == 3 || j == 15))
                         && !(i == j && j == 15))
                {
                    boardGo[i][j] = BLACK;
                }
                else if (pieces == 2 && ((i == 3 && j == 15)
                         || (i == 15 && j == 3)))
                {
                    boardGo[i][j] = BLACK;
                }
            }
        }
    }
}

/*在棋盘(x,y)位置上落子,落子方为color,落子成功返回0，否则返回1*/
int Board::addOnePiece(int x, int y, int color)
{
    /*有子，落子失败*/
	if (boardGo[x][y] != NOPIECE)
		return 1;

    /*对方棋子颜色*/
    int opColor = NOPIECE;
    if (color == BLACK)
    {
        opColor = WHITE;
    }
    else if (color == WHITE)
    {
        opColor = BLACK;
    }

    /*先尝试落子*/
    boardGo[x][y] = color;

    /*提劫判断,连续提劫则不允许落子*/
    int pre_x = -1;
    int pre_y = -1;
    int preDead_x = -1;
    int preDead_y = -1;
    int cnt = 0;
    game->getPreHandInfo(&pre_x, &pre_y, &preDead_x, &preDead_y, &cnt);
    if (   /*上一手棋在此处提一子*/
        cnt == 1 && preDead_x == x && preDead_y == y

        /*此处上下左右均无气*/
        && (x == 0 || boardGo[x-1][y] == opColor)
        && (x == 18 || boardGo[x+1][y] == opColor)
        && (y == 0 || boardGo[x][y-1] == opColor)
        && (y == 18 || boardGo[x][y+1] == opColor)

        /*此手将会提掉上一手的子(仅一颗)*/
        && (pre_x == 0 || boardGo[pre_x-1][pre_y] == color)
        && (pre_x == 18 || boardGo[pre_x+1][pre_y] == color)
        && (pre_y == 0 || boardGo[pre_x][pre_y-1] == color)
        && (pre_y == 18 || boardGo[pre_x][pre_y+1] == color)
            )
    {
        /*打劫，禁止落子*/
        boardGo[x][y] = NOPIECE;
        return 1;
    }

    /* 判断是否需要将周围的异色棋子提起，若可提子，则直接落子成功*/
	int killret = 1;
	if (x - 1 >= 0 && x - 1 <= 18 && y >= 0 && y <= 18 && boardGo[x - 1][y] == opColor)
	{
		if (tryToKillOnePieceBlock(x - 1, y, opColor) == 0)
		{
			killret = 0;
		}
	}
	if (x + 1 >= 0 && x + 1 <= 18 && y >= 0 && y <= 18 && boardGo[x + 1][y] == opColor)
	{
		if (tryToKillOnePieceBlock(x + 1, y, opColor) == 0)
		{
			killret = 0;
		}
	}
	if (x >= 0 && x <= 18 && y - 1 >= 0 && y - 1 <= 18 && boardGo[x][y - 1] == opColor)
	{
		if (tryToKillOnePieceBlock(x, y - 1, opColor) == 0)
		{
			killret = 0;
		}
	}
	if (x >= 0 && x <= 18 && y + 1 >= 0 && y + 1 <= 18 && boardGo[x][y + 1] == opColor)
	{
		if (tryToKillOnePieceBlock(x, y + 1, opColor) == 0)
		{
			killret = 0;
		}
	}
	if (killret == 0)
	{
        /*将落子加入临时节点中*/
        temStap.color = color;
        temStap.stapPoint.x = x;
        temStap.stapPoint.y = y;

		return killret;
    }/* 判断是否需要将周围的异色棋子提起，若可提子，则直接落子成功 end */

	/*若无法提子，则查看自己是否有气*/
	if (getSta(x, y, color) == 0) //若有气，落子成功
	{
        /*将落子加入临时节点中*/
        temStap.color = color;
        temStap.stapPoint.x = x;
        temStap.stapPoint.y = y;

		refresh(color);
		return 0;
	}
	else if (getSta(x, y, color) == 1)//若无气，落子失败,该点改为无子状态，返回1
	{
		refresh(color);
		boardGo[x][y] = NOPIECE;
		return 1;
	}

    return -1;
}


int Board::tryToKillOnePieceBlock(int x, int y, int color)
{
	if (getSta(x, y, color) == 0) //若有气,未杀死，返回1
	{
		refresh(color);
		return 1;
	}
	else if (getSta(x, y, color) == 1)//无气，杀死，返回0
	{
        //提子，则在提子的同时将死子加入临时栈中
        refresh(NOPIECE, 0);
		return 0;
	}

    return -1;
}

int Board::getSta(int x, int y, int color)
{
	if (x >= 0 && x <= 18 && y >= 0 && y <= 18)//该点是棋盘上的点
	{
		if (boardGo[x][y] == color)//同色棋子--递归调用
		{
			boardGo[x][y] = INDETEMINATE; //已递归调用过的棋子--设为待定
			if (getSta(x - 1, y, color) == 0)//上
			{
				return 0;
			}
			if (getSta(x, y - 1, color) == 0)//左
			{
				return 0;
			}
			if (getSta(x + 1, y, color) == 0)//下
			{
				return 0;
			}
			if (getSta(x, y + 1, color) == 0)//右
			{
				return 0;
			}
			return 1;
		}
        else if (boardGo[x][y] == INDETEMINATE ||			//已探索过的同色棋子
            (boardGo[x][y] == WHITE && color == BLACK) ||	//异色棋子
			(boardGo[x][y] == BLACK && color == WHITE)
            )//待定的棋子--已探索到边界，返回1
		{
			return 1;
		}
		else //无棋子--该处有气，返回0
		{
			return 0;
		}
	}
	else
		return 1;
}

/*落子动作结束后，刷新棋盘状态*/
int Board::refresh(int newStat)
{
	for (int i = 0; i < MAXLINE; i++)
	{
		for (int j = 0; j < MAXLINE; j++)
		{
			if (boardGo[i][j] == INDETEMINATE)
			{
				boardGo[i][j] = newStat;
			}
		}
	}
    return 0;
}

/*落子动作结束后,刷新棋盘状态为newStat,refreshType为0,表示需要提子*/
int Board::refresh(int newStat, int refreshType)
{
    for (int i = 0; i < MAXLINE; i++)
    {
        for (int j = 0; j < MAXLINE; j++)
        {
            if (refreshType == 0 && boardGo[i][j] == INDETEMINATE)
            {
                /*将死子加入临时节点中*/
                BordPoint tmpPoint;
                tmpPoint.x = i;
                tmpPoint.y = j;
                temStap.deadPiece.append(tmpPoint);
            }
            if ( boardGo[i][j] == INDETEMINATE)
            {
                boardGo[i][j] = newStat;
            }
        }
    }
    return 0;
}


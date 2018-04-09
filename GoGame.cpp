#include "GoGame.h"
#include "Board.h"
#include "Player.h"
#include "handstap.h"
#include "gameendcal.h"
#include <QDebug>

extern StapNode temStap;

GoGame::GoGame()
{
    pGoBoard = new Board();
	player_1 = new Player();
	player_2 = new Player();
    curPlayer = nullptr;
    handstap = new HandStap();
    endCal = new GameEndCal();
    pGoBoard->game = this;
	player_1->board_p = pGoBoard;
	player_2->board_p = pGoBoard;
    endCal->game = this;
    this->handstap->game = this;
}


GoGame::~GoGame()
{
    player_1->board_p = nullptr;
    player_2->board_p = nullptr;
	delete pGoBoard;
	delete player_1;
	delete player_2;
    delete handstap;
    delete endCal;
}

int GoGame::gameStart()
{
	player_1->color = BLACK;
	player_2->color = WHITE;
    handstap->initHandStap();
	this->status = WAITEPLAYERACT_1;
    this->mode = LOCALGAME;
    return 0;
}

int GoGame::gameStart(int size, int pieces)
{
    pGoBoard->initBoard(size, pieces);
    player_1->color = BLACK;
    player_2->color = WHITE;
    handstap->initHandStap();

    if (pieces > 1 && pieces <=9)//2-9子是让子棋，白棋先下
    {
        this->status = WAITEPLAYERACT_2;
    }
    else if (pieces == 1)//让先，仅表现为不贴子，且指定黑白，此处暂不处理
    {
        this->status = WAITEPLAYERACT_1;
    }
    else if (pieces == 0)//正常对局
    {
        this->status = WAITEPLAYERACT_1;
    }
    return 0;
}

int GoGame::getPointSta(int x, int y)
{
    if (x < 0 || y < 0 || x >= MAXLINE || y >= MAXLINE)
        return NOPIECE;
    return pGoBoard->boardGo[x][y];
}

void GoGame::setPointSta(int x, int y, int color)
{
    this->pGoBoard->boardGo[x][y] = color;
}

/*请求接收器，接收请求并执行*/
int GoGame::gameReqRecver(__attribute__((unused))int reqNum, int color, int reqType, gameReqData data)
{
    /*当前活动的玩家*/
    Player * curPlay = nullptr;
	if (color == BLACK)
		curPlay = player_1;
	else
		curPlay = player_2;

	switch (reqType)
	{
        case ADDPIECE: //落子，return 0--成功，1--失败
		{
			int addSuc = 1;
			if ((this->status == WAITEPLAYERACT_1 && curPlay->color == BLACK) || //黑棋落子请求
				(this->status == WAITEPLAYERACT_2 && curPlay->color == WHITE)	//白棋落子请求
				)
			{
                //addSuc = curPlay->addPiece(data.x, data.y);
                addSuc = pGoBoard->addOnePiece(data.x, data.y, color);
				if (addSuc == 0 && this->status == WAITEPLAYERACT_1)
					this->status = WAITEPLAYERACT_2;
				else if (addSuc == 0 && this->status == WAITEPLAYERACT_2)
					this->status = WAITEPLAYERACT_1;

			}
            if (addSuc == 0)//落子成功后,在悔棋栈中添加该手棋的信息
            {
                handstap->addOneHand(temStap);
                temStap.color = NOPIECE;
                temStap.stapPoint.x = -1;
                temStap.stapPoint.y = -1;
                temStap.handNum = 0;
                temStap.deadPiece.clear();
            }
			return addSuc;
		}
			break;
        case ADDPIECEDRRECT:
        {
            int color = NOPIECE;
            if (this->status == ADDBLACKPIECE)
                color = BLACK;
            else if (this->status == ADDWHITEPIECE)
                color = WHITE;
            this->setPointSta(data.x, data.y, color);
        }
            break;
        case RMPIECE:
        {
            this->setPointSta(data.x, data.y, NOPIECE);
        }
            break;
		default:
			break;
	}
    return -1;
}

/*退一步*/
void GoGame::backOneStep()
{
    this->handstap->backOneStep();
}

/*进一步*/
void GoGame::frwdOneStep()
{
    this->handstap->frwdOneStep();
}

/*获取当前手数*/
int GoGame::getPersentHandNum()
{
    return handstap->handNum;
}

/*清空前进栈*/
void GoGame::clearFrwdStack()
{
    this->handstap->clearFrwdStack();
}

/*获取前一手(当前已成功落下的最后一手)的部分信息*/
void GoGame::getPreHandInfo(int *pre_x, int *pre_y, int *preDead_x, int *preDead_y, int *c) const
{
    this->handstap->preHandInfo(pre_x, pre_y, preDead_x, preDead_y, c);
}

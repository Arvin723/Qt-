#include "handstap.h"
#include "GoGame.h"
#include "Board.h"
#include <QDebug>


HandStap::HandStap()
{
    handNum = 0;
}

void HandStap::clearFrwdStack()
{
    frwdstack.clear();
}

void HandStap::initHandStap()
{
    handstack.clear();
    frwdstack.clear();
    handNum = 0;
}

void HandStap::addOneHand(StapNode newStap)
{
    handNum++;
    newStap.handNum = handNum;
    //this->handstack.append(newStap);
    this->handstack.push(newStap);
}

/*退一步*/
void HandStap::backOneStep()
{
    StapNode tmpStep;
    if (!handstack.isEmpty())
    {
        int opColor = NOPIECE;
        tmpStep.color = handstack.top().color;
        tmpStep.stapPoint.x = handstack.top().stapPoint.x;
        tmpStep.stapPoint.y = handstack.top().stapPoint.y;
        tmpStep.deadPiece.swap(handstack.top().deadPiece);

        game->setPointSta(tmpStep.stapPoint.x, tmpStep.stapPoint.y, NOPIECE);

        if (tmpStep.color == BLACK)
            opColor = WHITE;
        else if (tmpStep.color == WHITE)
            opColor = BLACK;

        //死子队列非空，将死子重新加入
        if (!tmpStep.deadPiece.isEmpty())
        {
            QQueue<BordPoint>::iterator it;
            for (it = tmpStep.deadPiece.begin(); it != tmpStep.deadPiece.end(); it++)
            {
                game->setPointSta(it->x, it->y, opColor);
                qDebug()<<"恢复"<<it->x<<","<<it->y;
            }
        }
        if (opColor == BLACK && game->status != GAMEEND)
            game->status = WAITEPLAYERACT_2;
        else if (opColor == WHITE && game->status != GAMEEND)
            game->status = WAITEPLAYERACT_1;

    }
    else
        return;

    handNum--;
    frwdstack.push(tmpStep);
    handstack.pop();
}

/*进一步*/
void HandStap::frwdOneStep()
{
    StapNode tmpStep;
    if (!frwdstack.isEmpty())
    {
        int opColor = NOPIECE;
        tmpStep.color = frwdstack.top().color;
        tmpStep.stapPoint.x = frwdstack.top().stapPoint.x;
        tmpStep.stapPoint.y = frwdstack.top().stapPoint.y;
        tmpStep.deadPiece.swap(frwdstack.top().deadPiece);

        game->setPointSta(tmpStep.stapPoint.x, tmpStep.stapPoint.y, tmpStep.color);

        if (tmpStep.color == BLACK)
            opColor = WHITE;
        else if (tmpStep.color == WHITE)
            opColor = BLACK;

        //死子队列非空，将死子重新提起
        if (!tmpStep.deadPiece.isEmpty())
        {
            QQueue<BordPoint>::iterator it;
            for (it = tmpStep.deadPiece.begin(); it != tmpStep.deadPiece.end(); it++)
            {
                game->setPointSta(it->x, it->y, NOPIECE);
                //qDebug()<<"恢复"<<it->x<<","<<it->y;
            }
        }
        if (opColor == BLACK && game->status != GAMEEND)
            game->status = WAITEPLAYERACT_1;
        else if (opColor == WHITE && game->status != GAMEEND)
            game->status = WAITEPLAYERACT_2;
    }
    else
        return;

    handNum++;
    handstack.push(tmpStep);
    frwdstack.pop();
}

void HandStap::preHandInfo(int *pre_x, int *pre_y, int *preDead_x, int *preDead_y, int *c) const
{
    if (!handstack.isEmpty())
    {
        *pre_x = handstack.top().stapPoint.x;
        *pre_y = handstack.top().stapPoint.y;
        *c = handstack.top().deadPiece.count();
        if (*c == 1)
        {
            *preDead_x = handstack.top().deadPiece.at(0).x;
            *preDead_y = handstack.top().deadPiece.at(0).y;
        }
        else
        {
            *preDead_x = -1;
            *preDead_y = -1;
        }
    }
}

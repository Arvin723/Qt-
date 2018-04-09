#include "gameendcal.h"
#include "Board.h"
#include "GoGame.h"
#include <QDebug>


/*
 * 终局计算器
 * 包括标记死子,数子等功能
*/
GameEndCal::GameEndCal()
{
    this->blackNum = 0;
    this->whiteNum = 0;
}

double GameEndCal::getBlackCount() const
{
    return blackNum;
}

double GameEndCal::getWhiteCount() const
{
    return whiteNum;
}

int GameEndCal::clickDiePiece(int x, int y, int color)
{
    if (x < 0 || y < 0 || x >= MAXLINE || y >= MAXLINE)
        return 1;
    if (game->getPointSta(x, y) != color)
        return 1;

    int thisColor = NOPIECE;
    int tarColor = NOPIECE;
    if (color == BLACK)
    {
        thisColor = color;
        tarColor = BLACKDIE;
    }
    else if (color == WHITE)
    {
        thisColor = color;
        tarColor = WHITEDIE;
    }
    else if (color == BLACKDIE)
    {
        thisColor = color;
        tarColor = BLACK;
    }
    else if (color == WHITEDIE)
    {
        thisColor = color;
        tarColor = WHITE;
    }
    else
        return 1;

    this->game->setPointSta(x, y, tarColor);

    if (x >= 0 && x < MAXLINE && y >=0 && y < MAXLINE)
    {
        clickDiePiece(x-1, y, thisColor);
        clickDiePiece(x, y-1, thisColor);
        clickDiePiece(x+1, y, thisColor);
        clickDiePiece(x, y+1, thisColor);
    }
    return 0;
}

int GameEndCal::emptyJudge(int x, int y)
{
    if (game->getPointSta(x, y) != NOPIECE)
        return 1;

    char flag = 0x00;
    confirmEmpty(x, y, &flag);
    int tarColor = NOPIECE;
    if (flag == 0x01)
        tarColor = BLACKEMPTY;
    else if (flag == 0x02)
        tarColor = WHITEEMPTY;
    else if (flag == 0x03)
        tarColor = SINGLEYOSE;
    for (int i = 0; i < MAXLINE; i++)
    {
        for (int j = 0; j < MAXLINE; j++)
        {
            if (game->getPointSta(i, j) == PENDING)
            {
                game->setPointSta(i, j, tarColor);
            }
        }
    }
    return 0;
}

int GameEndCal::confirmEmpty(int x, int y, char *colorflag)
{
    if (x < 0 || y < 0 || x >= MAXLINE || y >= MAXLINE)
        return 1;
    if (game->getPointSta(x, y) == NOPIECE)
    {
        this->game->setPointSta(x, y, PENDING);
        confirmEmpty(x-1, y, colorflag);
        confirmEmpty(x, y-1, colorflag);
        confirmEmpty(x+1, y, colorflag);
        confirmEmpty(x, y+1, colorflag);
    }
    else if (game->getPointSta(x, y) == BLACK
             || game->getPointSta(x, y) == WHITEDIE)
    {
        *colorflag |= 0x01;
    }
    else if (game->getPointSta(x, y) == WHITE
             || game->getPointSta(x, y) == BLACKDIE)
    {
        *colorflag |= 0x02;
    }
    return 0;
}

int GameEndCal::resetCount()
{
    this->blackNum = 0.0;
    this->whiteNum = 0.0;
    return 0;
}

void GameEndCal::refresh()
{
    for (int i = 0; i < MAXLINE; i++)
    {
        for (int j = 0; j < MAXLINE; j++)
        {
            if (game->getPointSta(i, j) == BLACKDIE)
                game->setPointSta(i, j, BLACK);
            else if (game->getPointSta(i, j) == WHITEDIE)
                game->setPointSta(i, j, WHITE);
            else if (game->getPointSta(i, j) == BLACKEMPTY ||
                     game->getPointSta(i, j) == WHITEEMPTY ||
                     game->getPointSta(i, j) == SINGLEYOSE)
            {
                game->setPointSta(i, j, NOPIECE);
            }
        }
    }
}

double GameEndCal::countBlack()
{
    resetCount();
    for (int i = 0; i < MAXLINE; i++)
    {
        for (int j = 0; j < MAXLINE; j++)
        {
            emptyJudge(i, j);
            if (game->getPointSta(i, j) == BLACK
               || game->getPointSta(i, j) == WHITEDIE
               || game->getPointSta(i, j) == BLACKEMPTY)
            {
                blackNum++;
            }
            else if (game->getPointSta(i, j) == WHITE
                     || game->getPointSta(i, j) == BLACKDIE
                     || game->getPointSta(i, j) == WHITEEMPTY)
            {
                whiteNum++;
                qDebug()<<"WHITE"<<game->getPointSta(i, j);
            }
            else if (game->getPointSta(i, j) == SINGLEYOSE)
            {
                blackNum += 0.5;
                whiteNum += 0.5;
                qDebug()<<"SINGLEYOSE"<<game->getPointSta(i, j);
            }
        }
    }
    qDebug()<<"blackNum:"<<blackNum;
    qDebug()<<"whiteNum:"<<whiteNum;
    return blackNum;
}

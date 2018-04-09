#pragma once

#include <QString>

class Board;

class Player
{
public:
	Player();
	~Player();

	int color;
	Board * board_p;
    QString username;
};


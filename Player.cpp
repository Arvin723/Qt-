#include "Player.h"
#include "Board.h"

Player::Player()
{
    this->board_p = nullptr;
    this->color = NOPIECE;
    this->username.clear();
}


Player::~Player()
{
}

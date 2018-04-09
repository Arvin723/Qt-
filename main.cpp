#include "widget.h"
#include "GoGame.h"
#include <QApplication>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    w.nGame = new GoGame();
    w.nGame->gameStart();

    return a.exec();
}

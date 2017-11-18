#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    enum ChessStatus{Empty,Black,White};
    void initChess();
    void changeRole();
    void showScore();
    judgeRule(int x, int y, void *chess, ChessStatus currentRole, bool eatChess = true, int gridNum = 8);
    void machinePlay();
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void on_toolButtonEnd_clicked();

private:
    Ui::Widget *ui;
    QPoint p;

    //起点坐标,终点坐标
    QPoint start,end;

    //小格子的宽度和高度
    int gridW,gridH;

    //定义一个二维数组保存黑白子的状态
    int chess[8][8];

    ChessStatus role; //角色

    QTimer mTimer; //定时器

    //倒计时
    int timeNum;
    QTimer leftTimer;

};

#endif // WIDGET_H

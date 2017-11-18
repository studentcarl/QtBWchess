#include "widget.h"
#include "ui_widget.h"
#include <QPaintEvent>
#include <QPainter>//画家
#include <QMouseEvent>
#include <QDebug>
#define cout qDebug() //后面没有分号

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    //去边框
    this->setWindowFlags(Qt::FramelessWindowHint);

    start = QPoint(252,72);//起点坐标
    end = QPoint(745,480);//终点坐标
    gridW = (end.x() - start.x() )/8;
    gridH = (end.y() - start.y() )/8;

    initChess();

    connect(&mTimer,&QTimer::timeout,this,&Widget::machinePlay);
    //倒计时

    connect(&leftTimer,&QTimer::timeout,
            [=]()mutable
    {
        timeNum--;
        ui->lcdNumberTime->display(timeNum);
        if(timeNum == 0)
        {
            this->changeRole();
        }
    }

    );
}

Widget::~Widget()
{
    delete ui;
}

void Widget::paintEvent(QPaintEvent *event)//绘图事件,画棋盘,画棋子
{
    QPainter p(this);//创建一个画家,指定窗口画布

    p.drawPixmap(this->rect(),QPixmap("://image/chessboard.jpg"));

    //根据二维数组来画黑白子

    for(int i = 0 ;i < 8; ++i)
    {
        for(int j = 0;j < 8; ++j)
        {
            if(chess[i][j] == Black)
            {
                p.drawPixmap(start.x() + i*gridW, start.y()+j*gridH, gridW-2, gridH-2, QPixmap("://image/black.png"));
            }
            else if(chess[i][j] == White)
            {
                p.drawPixmap(start.x() + i*gridW, start.y()+j*gridH, gridW-2, gridH-2, QPixmap("://image/white.png"));
            }
        }
    }
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
       p = event->globalPos() - this->frameGeometry().topLeft();
    }

    int x = event->x();
    int y = event->y();
    int i = 0;
    int j = 0;

    if(x >= start.x() && x <= (start.x() + 8*gridW)
    && y >= start.y() && y <= (start.y() + 8*gridH))
    {
        i = ( event->x() - start.x() ) / gridW;
        j = ( event->y() - start.y() ) / gridH;

        cout << "i  = " << i << ", j = " << j;

        //chess[i][j] = role;
        if(judgeRule(i, j, chess, role) > 0)
        {
            this->changeRole(); //改变角色

            //更新绘图
            update();

        }


    }

}

void Widget::mouseMoveEvent(QMouseEvent *event)
{

    if(event->buttons() & Qt::LeftButton)
    {
        move(event->globalPos() - p);
    }
}

void Widget::on_toolButtonEnd_clicked()
{
    this->close();
}

void Widget::initChess()
{
    for(int i = 0 ;i < 8; ++i)
    {
        for(int j = 0;j < 8; ++j)
        {
            chess[i][j] = Empty;
        }
    }
    //中间四子
    chess[3][3] = Black;
    chess[4][3] = White;
    chess[3][4] = White;
    chess[4][4] = Black;

    role = Black;

    ui->labelBlack->show();
    ui->labelWhite->hide();

    ui->lcdNumberBlack->display(2);
    ui->lcdNumberWhite->display(2);

    timeNum = 15;

    ui->lcdNumberTime->display(timeNum);
    if(leftTimer.isActive() == false)
    {
        this->leftTimer.start(1000);
    }


}

//切换角色
void Widget::changeRole()
{
    timeNum = 15;
    ui->lcdNumberTime->display(timeNum);
    if(role == Black)
    {
        role = White;
        ui->labelBlack->hide();
        ui->labelWhite->show();
    }
    else
    {
        role = Black;
        ui->labelBlack->show();
        ui->labelWhite->hide();
    }

    if(role == White) //机器下子
    {

        //1秒后，在调用machinePlay();
        this->mTimer.start(1000); //毫秒为单位
    }

    //统计个数
    showScore();

}



//统计个数
void Widget::showScore()
{
    int b = 0;
    int w = 0;
    for(int i =0; i < 8; ++i)
    {
        for(int j =0; j < 8; ++j)
        {
            if(chess[i][j] == Black)
            {
                b++;
            }
            else if(chess[i][j] == White)
            {
                w++;
            }
        }
    }

    ui->lcdNumberBlack->display(b);
    ui->lcdNumberWhite->display(w);
    //输赢判断
    //黑子、白子都不能吃子，说明游戏结束
    for(int i =0; i < 8; ++i)
    {
        for(int j =0; j < 8; ++j)
        {
            if(judgeRule(i, j, chess, Black, false) > 0 ||
                judgeRule(i, j, chess, White, false) > 0
            )
            {
                return;
            }
        }
    }

    //指向到这步，说明游戏结束
    if(w > b)
    {
        cout << "白子赢";
    }
    else if(w < b)
    {
        cout << "黑子赢";
    }
    else
    {
        cout << "平局";
    }

}

// 吃子规则的参数：棋盘数组坐标位置(x y)、棋子状态数组(chess)、棋子的当前角色(注意其变量的写法)
// eatChess默认为true, 横着或竖着的格数(gridNum)默认值为8
int Widget::judgeRule(int x, int y, void *chess, ChessStatus currentRole, bool eatChess, int gridNum)
{
    // 棋盘的八个方向
    int dir[8][2]={{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1},{0,1},{1,1}};
    int tmpX = x, tmpY = y;             // 临时保存棋盘数组坐标位置
    int i = 0, eatNum = 0;                 // 初始化数据
    typedef int (*P)[gridNum];             // 自定义类型
    P chessBoard = P(chess);               // 类型转换

    if(chessBoard[tmpX][tmpY] != Empty)  // 如果此方格内已有棋子，返回;
        return 0;

    // 棋盘的8个方向
    for(i = 0 ; i <8; i++)
    {
        tmpX += dir[i][0]; tmpY += dir[i][1];	// 准备判断相邻棋子
        // 如果没有出界，且相邻棋子是对方棋子，才有吃子的可能．
        if((tmpX < gridNum && tmpX >=0 && tmpY < gridNum && tmpY >= 0)
            && (chessBoard[tmpX][tmpY] != currentRole) && (chessBoard[tmpX][tmpY] != Empty) )
        {
            tmpX += dir[i][0]; tmpY += dir[i][1];	            // 继续判断下一个，向前走一步
            while(tmpX < gridNum && tmpX >=0 && tmpY < gridNum && tmpY >= 0)
            {
                if(chessBoard[tmpX][tmpY] == Empty) // 遇到空位跳出
                        break;
                if(chessBoard[tmpX][tmpY] == currentRole)       // 找到自己的棋子，代表可以吃子
                {
                    if(eatChess == true)                        // 确定吃子
                    {

                        chessBoard[x][y] = currentRole;      // 开始点标志为自己的棋子
                        tmpX -= dir[i][0]; tmpY -= dir[i][1];// 后退一步
                        while((tmpX != x )||(tmpY != y))	 // 只要没有回到开始的位置就执行
                        {
                            chessBoard[tmpX][tmpY] = currentRole;   // 标志为自己的棋子
                            tmpX -= dir[i][0]; tmpY -= dir[i][1];   // 继续后退一步
                            eatNum++;  // 累计
                        }
                    }
                    else    // 不吃子，只是判断这个位置能不能吃子
                    {
                        tmpX -= dir[i][0]; tmpY -= dir[i][1];   // 后退一步
                        while((tmpX != x )||(tmpY != y))        // 只计算可以吃子的个数
                        {
                            tmpX -= dir[i][0]; tmpY -= dir[i][1];// 继续后退一步
                            eatNum++;
                        }
                    }
                    break;  // 跳出循环
                }// 没有找到自己的棋子，就向前走一步
                tmpX += dir[i][0]; tmpY += dir[i][1];   // 向前走一步
            }
        }// 如果这个方向不能吃子，就换一个方向
        tmpX = x; tmpY = y;
    }

    return eatNum;              // 返回能吃子的个数
}

//机器下子, 白子
void Widget::machinePlay()
{
    //时间停止
    this->mTimer.stop();

    int max = 0;
    int num = 0;
    int px = 0;
    int py = 0;
    for(int i = 0; i < 8; ++i)
    {
        for(int j = 0; j < 8; ++j)
        {
            //判断能吃子的位置
            if( ( num = judgeRule(i, j, chess, White, false) ) > 0 )
            {
                if(num > max)
                {
                    max = num;
                    px = i;
                    py = j;
                }
            }
        }
    }

    if(max == 0) //没有地方能吃子
    {
        this->changeRole();
        return;
    }

    cout << "max = " << max;
    //真正的吃子
    if( judgeRule(px, py, chess, White) > 0)
    {
        this->changeRole();
        this->update();
    }


}



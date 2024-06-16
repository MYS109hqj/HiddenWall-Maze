#ifndef WIDGET_H
#define WIDGET_H

#include<rulewindow.h>

#include <QWidget>
#include <QPainter>
#include <QVector>
#include <QMessageBox>
#include <QKeyEvent>
#include <list>
//动画类
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>
#include <QAnimationGroup>
//时间
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    //初始化函数
    void initUi();
    void initData();

    void paintEvent(QPaintEvent *);
    void Draw_rect(int index,QColor color);
    void myDrawLine(int n,int m);
    void myDrawLine(int n,int m,QColor color);//另加一种传参便于调试

    QVector<int> GetNeighbors(int n);//返回所有的邻居
    int Get_Random(QVector<int> &vector);
    int getward(int index,int index2);//获取第二个位置相对第一个位置的方向
    int Get_index(int index,int ward);
    int GetNeighbor(int n);

    void init_Hash(int* Hash);//清空哈希表

    //生成检查
    bool Check_in_row(int n,int max);
    bool Check_in_column(int n,int max);

    void keyPressEvent(QKeyEvent *event);

    int bfs(int cur, int end,bool through_finish);
    void Getdist();
    //迷宫生成
    void CreateMaze_Blocking_Dfs();
    void Blocking_Dfs(int n,int *path);
    bool Check_Blocking_Down(int *path);
    void BreakWall_Up(int index, int *path);
    void Check_Blocking_Up2();
    bool Check_BLocking_Center();
    void Check_Blocking_Up();
    bool Check_Distance();
    //后期检查函数
    void Cut();
    void Check_bfs();
    //设置、终局展示
    void showMaze();

    //撞墙动画
    void CrushAnimation(int n,int m,int flag);
    //窗口
    void showResultWindow(int index);

private slots:
    void on_rst_button_clicked();
    //透明度动画结束触发的槽函数
    void resetButtonOpacity(int flag);

    void on_modeCB_currentIndexChanged(int index);


    void on_giveup_button_clicked();

    void on_timeCB_currentIndexChanged(int index);

    void on_pushButton_clicked();

private:
    Ui::Widget *ui;
    QPixmap* pPixmap;
    QPainter* pPainter;

    QVector<QVector<int>> wall;
    //用于记录实时下标
    int green_index;
    int red_index;
    //用于阻止棋子乱移动
    int green_mutex;
    int red_mutex;
    int* hash;
    int roadlength1;
    int roadlength2;
    int crush;//是否开启撞墙回家
    int mode;//设置移动模式
    //轮次移动模式的两个变量
    int turn;
    int step;
    //自由移动模式的两个变量
    int greenstep;
    int redstep;
    //设置罚时的变量
    int penalty;
    //测试变量，默认不开启，自己调构造函数
    int test;
    //撞墙动画效果
    QGraphicsOpacityEffect*m_pushbuttonOpacity_R;
    QGraphicsOpacityEffect*m_pushbuttonOpacity_G;
    QSequentialAnimationGroup* SequenAnimation;
};
#endif // WIDGET_H

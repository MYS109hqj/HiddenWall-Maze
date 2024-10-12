#include "widget.h"
#include "ui_widget.h"

#define BEGINX 40
#define LINEHEIGHT 40
#define LINEWIDTH 40
#define ENDX (BEGINX+8*LINEHEIGHT)
#define ROWNUM 8

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , green_index(56)
    , red_index(63)
    , green_mutex(0)
    , red_mutex(0)
    , crush(1)
    , mode(0)
    , turn(0)
    , step(0)
    , greenstep(0)
    , redstep(0)
    , penalty(0)
    , test(0)
    , m_pushbuttonOpacity_R(NULL)
{
    ui->setupUi(this);

    // 移动模块 先前准备：让键盘焦点不被掠夺
    setFocus();
    ui->rst_button->setFocusPolicy(Qt::NoFocus);
    ui->giveup_button->setFocusPolicy(Qt::NoFocus);
    ui->btnGreen->setFocusPolicy(Qt::TabFocus);
    ui->btnRed->setFocusPolicy(Qt::TabFocus);
    ui->modeCB->setFocusPolicy(Qt::TabFocus);
    ui->timeCB->setFocusPolicy(Qt::TabFocus);

    ui->stackedWidget->setCurrentIndex(0);

    m_pushbuttonOpacity_R = new QGraphicsOpacityEffect(ui->btnRed);//关联控件对象
    m_pushbuttonOpacity_R -> setOpacity(1);// 设置透明度为1
    ui->btnRed->setGraphicsEffect(m_pushbuttonOpacity_R);//控件增加绘图效果
    m_pushbuttonOpacity_G = new QGraphicsOpacityEffect(ui->btnGreen);//关联控件对象
    m_pushbuttonOpacity_G -> setOpacity(1);// 设置透明度为1
    ui->btnGreen->setGraphicsEffect(m_pushbuttonOpacity_G);//控件增加绘图效果

    //移动控件
    QPixmap pix1(":/images/keyboard1.jpg");
    ui->keyboard1_LB->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->keyboard1_LB->setPixmap(pix1.scaled(ui->label->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    QPixmap pix2(":/images/keyboard2.jpg");
    ui->keyboard2_LB->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->keyboard2_LB->setPixmap(pix2.scaled(ui->label->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation));

    pPixmap = new QPixmap(QWidget::size());
    pPainter = new QPainter;
    on_rst_button_clicked();
}

Widget::~Widget()
{
    delete ui;
}



// 第一部分 界面交互逻辑

void Widget::resetButtonOpacity(int flag) {
    QTimer* timer = new QTimer(this);

    //由于延时，移动操作要放在动画执行之后
    connect(timer,&QTimer::timeout,[=](){
        if(flag==0){
            ui->btnRed->move(ROWNUM * BEGINX+5,ROWNUM * BEGINX+5);
            red_index = 64 - 1;
            red_mutex = 0;
            m_pushbuttonOpacity_R->setOpacity(1); // 设置透明度为1
        }else if(flag==1){
            ui->btnGreen->move(BEGINX+5,ROWNUM * BEGINX+5);
            green_index = 64 - ROWNUM;
            green_mutex = 0;
            m_pushbuttonOpacity_G->setOpacity(1); // 设置透明度为1
        }
        timer->deleteLater();
    });

    timer->start(penalty);

}


void Widget::showResultWindow(int index){
    QMessageBox msgBox;
    QPushButton * reserve = msgBox.addButton(tr("查看结局版面"),QMessageBox::ActionRole);
    QPushButton * restart = msgBox.addButton(tr("重新开始游戏"),QMessageBox::ActionRole);
    msgBox.setDefaultButton(reserve);//设置默认按钮
    if(index==0)//红方胜利
    {
        msgBox.setText("红方到达终点，红方胜利");
    }else if(index == 1){//绿方胜利
        msgBox.setText("绿方到达终点，绿方胜利");
    }else if(index == 2){
        msgBox.setText("双方同时到达终点，双方平局");
    }
    msgBox.exec();
    if(msgBox.clickedButton()==reserve){
        if(test) qDebug()<<"reserve";
        showMaze();
        red_mutex = 1;
        green_mutex = 1;
    }else if(msgBox.clickedButton()==restart){
        if(test) qDebug()<<"restart";
        on_rst_button_clicked();
    }
}


void Widget::showMaze(){
    for(int i = 0; i < 64;i+=2){
        //if(test) qDebug()<<i;
        if(wall[i][0]==0) myDrawLine(i,i-ROWNUM);
        if(wall[i][1]==0) myDrawLine(i,i-1);
        if(wall[i][2]==0) myDrawLine(i,i+1);
        if(wall[i][3]==0) myDrawLine(i,i+ROWNUM);
        if(i%8 == 6) i++;
        else if(i%8 == 7) i--;
    }
    update();
}


void Widget::on_rst_button_clicked()
{
    QPixmap *clearPix = new QPixmap(size());
    clearPix->fill (Qt::white);
    pPixmap = clearPix;
    update();
    do{
        initUi();
        initData();
        red_mutex = 0;
        green_mutex = 0;
        redstep = 0;
        greenstep = 0;
        step = 0;
        turn = 0;
    }while(roadlength1==14||roadlength2==14||!Check_Distance());
}


void Widget::on_modeCB_currentIndexChanged(int index)
{
    if(index==0){
        mode = 0;ui->stackedWidget->setCurrentWidget(ui->page);
    }
    else if(index==1) {
        ui->roundLB->setText("现在是绿色方的回合");
        ui->stepLB->setText("当前的轮内步数为：0（初始）");
        mode = 1;ui->stackedWidget->setCurrentWidget(ui->page_2);
    }
}


void Widget::on_giveup_button_clicked()
{
    showMaze();
    red_mutex  = 1;
    green_mutex = 1;
}


void Widget::on_timeCB_currentIndexChanged(int index)
{
    if(index==0){
        penalty = 0;
    }else if(index==1){
        penalty = 2000;
    }else if(index==2){
        penalty = 5000;
    }
}


void Widget::on_pushButton_clicked()
{
    RuleWindow* rulewindow = new RuleWindow;
    rulewindow->show();
}




// 第二部分：界面部分

void Widget::initUi()
{
    //ui部分：1.迷宫
    pPixmap->fill(QColor(Qt::white));
    pPainter->begin(pPixmap);

    QPen pen1, pen2;
    pen1.setColor(QColor(Qt::black));
    pen1.setWidth(2);
    pen2.setColor(QColor("#808080").lighter(150));//灰色
    pen2.setWidth(1);


    QLineF line;
    int x1 = BEGINX, y1 = 0, x2 = ENDX, y2 = 0;
    for(int i = 0; i < 9;i++)
    {
        y1 += LINEHEIGHT;
        y2 = y1;
        if(i==0||i==8)pPainter->setPen(pen1);
        else pPainter->setPen(pen2);
        line.setLine(x1,y1,x2,y2);
        pPainter->drawLine(line);
        line.setLine(y1,x1,y2,x2);
        pPainter->drawLine(line);
    }
    pPainter->end();

    update();

    //ui部分：2.文字
}


void Widget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    painter.drawPixmap(QPoint(0,0),*pPixmap);
}


void Widget::Draw_rect(int index,QColor color)
{
    pPainter->begin(pPixmap);
    pPainter->setPen(Qt::black);
    //使用brush填充
    QBrush brush;
    brush.setColor(color);
    brush.setStyle(Qt::SolidPattern);
    pPainter->setBrush(brush);
    int y = index / ROWNUM + 1;
    int x = index % ROWNUM + 1;
    QRect rect(x*40+5,y*40+5,30,30);
    //test
    //if(test) qDebug()<<"rect:"<<x*40 + 5<<y*40 + 5<<20<<20;
    pPainter->drawRect(rect);
    QFont font("Arial",8);

    if(index==0||index==ROWNUM-1)
    {
        pPainter->setFont(font);
        pPainter->drawText(rect,Qt::AlignCenter,"Finish");
    }
    pPainter->end();
    //update();
}


//用白色画笔覆盖黑色的线，输入两个整数，为两个相邻的格子以及对应的标号
void Widget::myDrawLine(int n,int m)
{
    int x1 = n / ROWNUM;
    int y1 = n % ROWNUM;
    int x2 = m / ROWNUM;
    int y2 = m % ROWNUM;
    //只需要取相邻的量，相邻两数和必为奇数
    int x3 = (x1 + x2) / 2 + 1;
    int y3 = (y1 + y2) / 2 + 1;

    QPen pen;
    pen.setColor(QColor(Qt::white));
    pen.setWidth(3);
    QLineF line;
    pPainter->begin(pPixmap);
    pPainter->setPen(pen);
    if(x1 - x2 == 1 || x1 - x2 == -1){//传入的是相邻的格子
        line.setLine(y1 * LINEWIDTH + BEGINX,x3 * LINEHEIGHT + BEGINX,(y1 + 1) * LINEWIDTH + BEGINX,(x3 + 1) * LINEHEIGHT);
    }else{
        line.setLine(y3 * LINEWIDTH + BEGINX,x1 * LINEHEIGHT + BEGINX,(y3 + 1) * LINEWIDTH ,(x1 + 1) * LINEHEIGHT + BEGINX);
    }
    pPainter->drawLine(line);
    pPainter->end();
}


void Widget::myDrawLine(int n,int  m,QColor color)
{
    int x1 = n / ROWNUM;
    int y1 = n % ROWNUM;
    int x2 = m / ROWNUM;
    int y2 = m % ROWNUM;
    //只需要取相邻的量，相邻两数和必为奇数
    int x3 = (x1 + x2) / 2 + 1;
    int y3 = (y1 + y2) / 2 + 1;

    QPen pen;
    pen.setColor(color);
    pen.setWidth(3);
    QLineF line;
    pPainter->begin(pPixmap);
    pPainter->setPen(pen);
    if(x1 - x2 == 1 || x1 - x2 == -1){//传入的是相邻的格子
        line.setLine(y1 * LINEWIDTH + BEGINX,x3 * LINEHEIGHT + BEGINX,(y1 + 1) * LINEWIDTH + BEGINX,(x3 + 1) * LINEHEIGHT);
    }else{
        line.setLine(y3 * LINEWIDTH + BEGINX,x1 * LINEHEIGHT + BEGINX,(y3 + 1) * LINEWIDTH ,(x1 + 1) * LINEHEIGHT + BEGINX);
    }
    pPainter->drawLine(line);
    pPainter->end();
}


// 撞墙的动画显示
//从n走到m，失败时触发。flag标识按钮0红1绿
void Widget::CrushAnimation(int n,int m,int flag){
    int x1 = n / ROWNUM;
    int y1 = n % ROWNUM;
    int x2 = m / ROWNUM;
    int y2 = m % ROWNUM;

    QRect rect;
    rect.setRect(y1 * LINEWIDTH + BEGINX + 5,x1 * LINEHEIGHT + BEGINX + 5,30,30);
    QPropertyAnimation* singleAnimation = NULL;
    if(flag==0)
        singleAnimation = new QPropertyAnimation(ui->btnRed,"geometry");
    else
        singleAnimation = new QPropertyAnimation(ui->btnGreen,"geometry");

    SequenAnimation = new QSequentialAnimationGroup(this);

    singleAnimation->setDuration(300);
    singleAnimation->setStartValue(rect);
    if(y1 - y2 == 1){//m在n左边
        if(test) qDebug()<<"向左";
        rect.translate(-5,0);
    }else if(y1 - y2 == -1){//m在n右边
        if(test) qDebug()<<"向右";
        rect.translate(5,0);
    }else if(x1 - x2 == 1){//m在n上面
        if(test) qDebug()<<"向上";
        rect.translate(0,-5);
    }else if(x1 - x2 == -1){//m在n下面
        if(test) qDebug()<<"向下";
        rect.translate(0,5);
    }

    singleAnimation->setEndValue(rect);
    singleAnimation->setEasingCurve(QEasingCurve::Linear);
    SequenAnimation ->addAnimation(singleAnimation);

    singleAnimation->setStartValue(rect);
    if(y1 - y2 == 1){//m在n左边
        rect.translate(3,0);
    }else if(y1 - y2 == -1){//m在n右边
        rect.translate(-3,0);
    }else if(x1 - x2 == 1){//m在n上面
        rect.translate(0,3);
    }else if(x1 - x2 == -1){//m在n下面
        rect.translate(0,-3);
    }

    singleAnimation->setEndValue(rect);
    SequenAnimation ->addAnimation(singleAnimation);

    singleAnimation->setStartValue(rect);
    if(y1 - y2 == 1){//m在n左边
        rect.translate(3,0);
    }else if(y1 - y2 == -1){//m在n右边
        rect.translate(-3,0);
    }else if(x1 - x2 == 1){//m在n上面
        rect.translate(0,3);
    }else if(x1 - x2 == -1){//m在n下面
        rect.translate(0,-3);
    }
    singleAnimation->setEndValue(rect);
    singleAnimation->setLoopCount(2);
    SequenAnimation ->addAnimation(singleAnimation);

    QPropertyAnimation* OpacityAnimation = NULL;
    if(flag==0)
    {
        OpacityAnimation = new QPropertyAnimation(m_pushbuttonOpacity_R,"opacity",ui->btnRed);

    }
    else
    {
        OpacityAnimation = new QPropertyAnimation(m_pushbuttonOpacity_G,"opacity",ui->btnGreen);
    }

    OpacityAnimation->setDuration(300);//调这个？直接延时
    OpacityAnimation->setStartValue(1);
    OpacityAnimation->setEndValue(0);
    OpacityAnimation->setEasingCurve(QEasingCurve::Linear);
    SequenAnimation ->addAnimation(OpacityAnimation);

    connect(SequenAnimation, &QSequentialAnimationGroup::finished, this, std::bind(&Widget::resetButtonOpacity, this, flag));//连接槽函数
    SequenAnimation->start(QAbstractAnimation::DeleteWhenStopped);

}



// 第三部分 迷宫生成逻辑

// initData: 生成迷宫的主函数
void Widget::initData()
{

    //初始起点与终点标识
    green_index = 56;
    red_index = 63;
    ui->btnGreen->move(BEGINX+5,ROWNUM * BEGINX+5);
    ui->btnRed->move(ROWNUM * BEGINX+5,ROWNUM * BEGINX+5);
    ui->btnGreen->resize(30,30);
    ui->btnRed->resize(30,30);

    wall.resize(64);

    //调试：计算最短距离
    //创建哈希表注意内存泄露问题
    hash= new int[64];
    roadlength1 = 999;
    roadlength2 = 999;

    for(int i = 0; i < 64; i++){
        wall[i].resize(4);//上左右下；前者作为参照系
        wall[i].fill(1);
        //调试：计算最短距离
        hash[i] = 0;
    }

    //多种生成方法选一种
    CreateMaze_Blocking_Dfs();


    Cut();
    Check_bfs();
    Getdist();

    Draw_rect(0,Qt::red);
    Draw_rect(7,Qt::green);
    update();

    if(mode==1){
        turn = 0;
        step = 0;
        ui->roundLB->setText("现在轮到绿色方移动");
        ui->stepLB->setText("当前的轮内步数为：0");
    }
}


//计算两组对角的两个距离
void Widget::Getdist(){
    //printWall();
    //dfs(0,0,SQUARESNUM-1,1);
    roadlength1 = bfs(0,63,false);
    //dfs(0,ROWNUM-1,SQUARESNUM-ROWNUM,2);
    roadlength2 = bfs(ROWNUM-1,56,false);
}

//返回所有符合边界条件的邻居
QVector<int> Widget::GetNeighbors(int n)
{
    int x = n / ROWNUM;
    int y = n % ROWNUM;
    QVector<int> arr;
    if (y > 0) arr.push_back(n - 1);
    if (y < ROWNUM - 1) arr.push_back(n + 1);
    if (x > 0) arr.push_back((x - 1) * ROWNUM + y);
    if (x < ROWNUM - 1) arr.push_back((x + 1) * ROWNUM + y);
    return arr;
}

//随机获取vector里的一个元素并将其删除
int Widget::Get_Random(QVector<int> &vector){
    if(vector.size()==0)
    {
        //if(test) qDebug()<<"Warning in function Get_Random: vector size is 0!";
        return -1;
    }
    //随机抽取一个list中的结点
    int index = rand() % vector.size();
    //if(test) qDebug() <<"index:" <<index;
    auto it = vector.begin();
    std::advance(it,index);
    int n = *it;
    vector.erase(it);
    return n;
}



void Widget::init_Hash(int* Hash){
    for(int i = 0 ;i < 64;i++)
        Hash[i] = 0;
    return;
}
//分块dfs：将8*8的迷宫分为四个标准块，分别进行dfs；连通处从几堵墙里选一个
void Widget::CreateMaze_Blocking_Dfs(){
    //初始哈希表
    init_Hash(hash);
    //ward:朝向数组，1为横向，2为纵向，3为交叉点
    int * ward = new int[63];
    //默认：锁定分块
    //第一步：
    //将第五行hash值先填为1，隔断上下
    for(int i = 4 * ROWNUM;i<5*ROWNUM;i++){
        hash[i] = 1;
    }
    hash[56] = 1;
    int reset = 0;
    do{
        if(reset)
            for(int i = 40; i < 63; i++){
                wall[i].fill(1);//上左右下；前者作为参照系
                ward[i] = 0;
                hash[i] = 0;
                initUi();
            }
        Blocking_Dfs(56,ward);
        reset = 1;
        //if(test) qDebug()<<"完成一次dfs\n\n\n";
    }while(!Check_Blocking_Down(ward));
    //第二步：中间分块

    //定义边角数组，下标0-5是左上角，6-11是右上角
    int block_conner[] = {0,1,8,9,16,17,6,7,14,15,22,23};
    //将屏障还原
    for(int i = 4 * ROWNUM;i<5*ROWNUM;i++){
        hash[i] = 0;
    }

    //获取靠红色方下分块与中分块的接口
    int red_center_begin = 0;
    if(wall[45][0]==0) red_center_begin = 45-ROWNUM;
    else if(wall[46][0]==0) red_center_begin = 46-ROWNUM;
    else if(wall[47][0]==0) red_center_begin = 47-ROWNUM;

    if(red_center_begin==0) if(test) qDebug()<<"Error in Blocking_dfs center part: red_begin is 0!";
    reset = 0;
    do{

        for(int i = 0; i < 40; i++){
            hash[i] = 0;
            if(32<=i&&i<=39&&wall[i][3]==0) continue;
            else ward[i] = 0;
            wall[i].fill(1);//上左右下；前者作为参照系
        }
        //建立新的屏障，即不让其遍历左右上角
        for(int i = 0;i<12;i++){
            hash[block_conner[i]] = 1;
        }
        if(reset)
        {
            //initui()，但只初始化0-39
            pPainter->begin(pPixmap);
            QPen pen1, pen2;
            pen1.setColor(QColor(Qt::black));
            pen1.setWidth(2);
            pen2.setColor(QColor("#808080").lighter(150));//灰色
            pen2.setWidth(1);
            QLineF line;
            int x1 = BEGINX, y1 = 0, x2 = ENDX, y2 = 0;
            for(int i = 0; i < 9;i++)
            {
                y1 += LINEHEIGHT;
                y2 = y1;
                if(i==0||i==8)pPainter->setPen(pen1);
                else pPainter->setPen(pen2);
                if(i<5)//横向只用画上面五条
                {
                    line.setLine(x1,y1,x2,y2);
                    pPainter->drawLine(line);
                }
                //纵向只用划到第五个格子
                line.setLine(y1,x1,y2,x2-3*LINEHEIGHT);
                pPainter->drawLine(line);
            }
            pPainter->end();
        }
        hash[red_center_begin] = 1;
        Blocking_Dfs(red_center_begin,ward);
        reset++;
        if(test) qDebug()<<"完成一次center分块的 dfs\n\n\n";
    }while(!Check_BLocking_Center()&&reset<=10);

    //遍历左右上角
    for(int i = 0;i<12;i++){
        hash[block_conner[i]] = 0;
    }
    hash[0] = 1;
    Blocking_Dfs(0,ward);
    hash[7] = 1;
    Blocking_Dfs(7,ward);
    Check_Blocking_Up();

}

void Widget::Blocking_Dfs(int n, int *path){
    //获取邻居
    QVector<int> v = GetNeighbors(n);
    int tmp = 0;
    while(v.size()!=0){
        tmp = Get_Random(v);
        //未访问过
        if(hash[tmp]==0){
            //第一步：确定开拓的方向
            int original_ward = path[n];//记录操作前的ward值，用于回溯
            int tmp_ward = path[tmp];//同时也得记录tmp的ward值，也可以通过分析计算节省掉这个空间
            //获取并更新ward
            int ward = getward(n,tmp);

            //对于上分块的特殊检测：
            //情况一：1、9或6、14，若2、10连通或5、13连通，则打通外部通道
            if( ( ((n==1&&ward==3)||(n==9&&ward==0))&&wall[2][3]==0 ) || ( ((n==6&&ward==3)||(n==14&&ward==0))&&wall[5][3]==0 ) ){
                int flag = rand()%2;
                if(flag){
                    return;
                    //随机进入此分支，不到达下标为1的格子
                }
            }else if((n==8||n==9||n==21||n==22)&&ward==3){
                //要考虑到下面的墙
                wall[n+ROWNUM][3] = 0;
                wall[n][3] = 0;
                if(!Check_in_column(n,3)){
                    //条件不满足：回溯
                    wall[n][3] = 1;
                }
                else{
                    //条件满足
                    wall[n+ROWNUM][0] = 0;
                }
                //下面的墙实际上没被凿开，复原下面的墙
                wall[n+ROWNUM][3] = 1;
            }


            if(ward==1||ward==2) {
                //tmp一定未被访问过，调整后不一定了
                if(path[tmp]==0){
                    path[tmp] = 1;
                }else if(path[tmp]==2)
                    path[tmp] = 3;
            }
            else if(ward==0||ward==3) {
                if(path[tmp]==0)
                    path[tmp] = 2;
                else if(path[tmp]==1)
                    path[tmp] = 3;
            }
            if(path[n]==0) path[n]=path[tmp];//记录ward为0，即初始点无记录值
            else if(path[n]==1&&path[tmp]/2==1){//
                path[n] = 3;
            }else if(path[n]==2&&path[tmp]%2==1){//
                path[n] = 3;
            }

            //暂时拆除墙，进行检测
            bool flag = true;
            wall[n][ward] = 0;
            wall[tmp][3-ward] = 0;

            //test
            if(test) qDebug()<<"n:"<<n<<"path[n]:"<<path[n]<<"tmp:"<<tmp<<"path[tmp]:"<<path[tmp];
            //检测是否有连续ward
            if((24<=n&&n<=31&&path[n]%2==1)||(original_ward%2==1&&path[tmp]%2==1)){//是横向二次拓展
                flag = Check_in_row(n,3);
            }
            if((24<=n&&n<=31&&path[n]/2==1)||(original_ward/2==1&&(path[tmp]/2==1))){//是纵向二次拓展
                flag = Check_in_column(n,3);
            }

            //如果条件不满足，使flag为false，则回溯
            if(!flag){
                wall[n][ward] = 1;
                wall[tmp][3-ward] = 1;
                path[tmp] = tmp_ward;
                path[n] = original_ward;

            }else{//否则，确定将墙打通，继续dfs
                hash[tmp] = 1;//标记访问
                //测试输出
                if(test) qDebug()<<"In"<<n<<"Success between"<<n<<"and"<<tmp<<"Next to"<<tmp;
                Blocking_Dfs(tmp,path);
            }
        }
    }
}

//重载函数，检查一行是否有超过max个横向连续格子
bool Widget::Check_in_row(int n,int max){
    int tn = n;
    int count = 1;//从n延展的连续数
    //先向左检测
    while(tn%8!=0){
        if(test) qDebug()<<"In row index:"<<tn<<wall[tn][1];
        if(wall[tn][1]==0){
            count++;
            tn = tn-1;
        }
        else break;
    }
    //再向右检测
    tn = n;
    while(tn%8!=7){
        if(test) qDebug()<<"In row index:"<<tn<<wall[tn][2];
        if(wall[tn][2]==0){
            count++;
            tn = tn+1;
        }
        else break;
    }
    if(count>max){
        return false;
    }

    return true;
}
//重载函数，检查一列是否有超过max个纵向连续格子
bool Widget::Check_in_column(int n,int max){
    int tn = n;
    int count = 1;//从n延展的连续数
    //先向上检测
    while(tn/8!=0){
        if(test) qDebug()<<"In column test: in wardcheck"<<tn;
        if(wall[tn][0]==0){
            count++;
            tn = tn-ROWNUM;
        }
        else break;
    }
    //再向下检测
    tn = n;
    while(tn/8!=7){
        if(test) qDebug()<<"In column test: in wardcheck"<<tn;
        if(wall[tn][3]==0){
            count++;
            tn = tn+ROWNUM;
        }
        else break;
    }
    if(count>max){
        return false;
    }
    return true;
}

bool Widget::Check_Blocking_Down(int *path){
    std::list<int> l;
    int* dist = new int[24];
    for(int i = 0; i< 24; i++) dist[i] = 50;
    int count = 0;//记录遍历格子的总数
    //放入左下角
    l.push_back(56);
    int n = 56;
    int times = l.size();//单轮循环次数
    int next = 0;//记录下一轮需要循环的次数
    int distance = 0;//记录距离
    int* is_end = new int[6];


    while(l.size()!=0){
        next = 0;
        for(int i = 0; i< times;i++){
            if(test) qDebug()<<"In Check_Blocking_Down  Count:"<<count<<"l.size()"<<l.size()<<"l.front()"<<l.front()<<"times:"<<times<<"distance"<<distance;
            n=l.front();
            count++;


            //尝试走向周围未被访问过的格子
            int flag = 0;
            if(wall[n][0]==0&&dist[n-ROWNUM-40]>=50) {l.push_back(n-ROWNUM);next++;flag++;}
            if(wall[n][1]==0&&dist[n-1-40]>=50) {l.push_back(n-1);next++;flag++;}
            if(wall[n][2]==0&&dist[n+1-40]>=50) {l.push_back(n+1);next++;flag++;}
            if(wall[n][3]==0&&dist[n+ROWNUM-40]>=50) {l.push_back(n+ROWNUM);next++;flag++;}

            //若flag>=2，则是分支点起点，对分支进行标记
            if(flag>=2){
                dist[n-40] = 50;//有多分支，重置标记为50
                //将未访问分支标记为51
                if(wall[n][0]==0&&dist[n-ROWNUM-40]>=50) dist[n-ROWNUM-40] = 51;
                if(wall[n][1]==0&&dist[n-1-40]>=50) dist[n-1-40] = 51;
                if(wall[n][2]==0&&dist[n+1-40]>=50) dist[n+1-40] = 51;
                if(wall[n][3]==0&&dist[n+ROWNUM-40]>=50) dist[n+ROWNUM-40] = 51;
            }else if(flag==1&&dist[n-40]>50){//若dist[n-40]>50,则这是一个分支的延伸
                if(wall[n][0]==0&&dist[n-ROWNUM-40]>=50) dist[n-ROWNUM-40] = dist[n-40]+1;
                if(wall[n][1]==0&&dist[n-1-40]>=50) dist[n-1-40] = dist[n-40]+1;
                if(wall[n][2]==0&&dist[n+1-40]>=50) dist[n+1-40] = dist[n-40]+1;
                if(wall[n][3]==0&&dist[n+ROWNUM-40]>=50) dist[n+ROWNUM-40] = dist[n-40]+1;
            }

            //if(test) qDebug()<<"Count:"<<count<<"index is:"<<n<<"flag is"<<flag<<"dist[n-40] is:"<<dist[n-40];

            if(dist[n-40]>=52){
                if(flag == 0){//没有新的分支延伸，路在此终结
                    if(n>=40&&n<=42){
                        is_end[n-40] = 1;
                    }
                    else if(n>=45&&n<=47){
                        if(n==45){
                            is_end[n-42] = 1;
                        }
                    }
                    else if(n==56||n==63){}
                    else {
                        //测试
                        //Draw_rect(n,Qt::green);
                        return false;
                    }
                }
            }
            dist[n-40] = distance;
            l.pop_front();
        }
        times = next;
        distance++;
    }
    if(count!=24){
        if(test) qDebug()<<"Count is not enough! count:"<<count;
        return false;
    }

    //放入右下角，计算red_dist。由于已经检查过了，这轮只需要遍历
    n = 63;
    l.push_back(n);
    times = l.size();//单轮循环次数
    next = 0;//记录下一轮需要循环的次数
    distance = 0;//记录距离
    int* red_dist = new int[24];
    for(int i = 0; i< 24; i++) red_dist[i] = 50;
    while(l.size()!=0){
        next = 0;
        for(int i = 0; i< times;i++){
            if(test) qDebug()<<"In Check_Blocking_Down  (red_dist)"<<"l.size()"<<l.size()<<"l.front()"<<l.front()<<"times:"<<times<<"distance"<<distance;
            n=l.front();
            count++;
            //尝试走向周围未被访问过的格子
            if(wall[n][0]==0&&red_dist[n-ROWNUM-40]==50) {l.push_back(n-ROWNUM);next++;}
            if(wall[n][1]==0&&red_dist[n-1-40]==50) {l.push_back(n-1);next++;}
            if(wall[n][2]==0&&red_dist[n+1-40]==50) {l.push_back(n+1);next++;}
            if(wall[n][3]==0&&red_dist[n+ROWNUM-40]==50) {l.push_back(n+ROWNUM);next++;}
            red_dist[n-40] = distance;
            if(test) qDebug()<<red_dist[n-40]<<n<<n-40;
            l.pop_front();
        }
        times = next;
        distance++;
    }

    //接下来根据获取到的距离数据进行处理

    //不考虑分支，只考虑距离
    int green_index = 0;
    int red_index = 0;
    int absolute = 99;
    //检测通过，进行距离运算
    for(int i = 40;i< 48;i++){
        if(test) qDebug()<<i<<":"<<dist[i-40]<<red_dist[i-40];
    }
    for(int i = 40;i <= 42;i++){
        for(int j = 47; j >=45;j--){
            if(abs(dist[i-40]-red_dist[j-40])<=absolute){
                red_index = j;
                green_index = i;
                absolute = abs(dist[i-40]-red_dist[j-40]);
            }
        }
    }
    if(test) qDebug()<<"red:"<<red_index<<"green:"<<green_index<<"absolute"<<absolute<<"\n\n\n";

    BreakWall_Up(red_index,path);
    BreakWall_Up(green_index,path);

    if(green_index==42&&is_end[1]&&wall[42][1]){
        //给41开路
        BreakWall_Up(41,path);
    }else if(is_end[2]&&wall[42][1]){
        //给42开路
        BreakWall_Up(42,path);
    }
    if(red_index==45&&is_end[4]&&wall[45][2]){
        //给46开路
        BreakWall_Up(46,path);
    }else if(is_end[3]&&wall[45][2]){
        //给45开路
        BreakWall_Up(45,path);
    }

    return true;
}

void Widget::BreakWall_Up(int index,int* path){
    wall[index][0] = 0;
    wall[index - ROWNUM][3] = 0;
    path[index] = 2;
    if(path[index] == 1)path[index] = 3;
    path[index-ROWNUM] = 2;
}

bool Widget::Check_BLocking_Center(){
    std::list<int> l;
    int* Hash = new int[64];
    init_Hash(Hash);
    l.push_back(56);
    int n = 56;
    int count = 0;//记录遍历格子的总数
    while(l.size()!=0){
        n=l.front();
        if(Hash[n]==0)
        {count++;
            if(wall[n][0]==0&&!Hash[n-ROWNUM]) {l.push_back(n-ROWNUM);}
            if(wall[n][1]==0&&!Hash[n-1-40]) {l.push_back(n-1);}
            if(wall[n][2]==0&&!Hash[n+1-40]) {l.push_back(n+1);}
            if(wall[n][3]==0&&!Hash[n+ROWNUM-40]) {l.push_back(n+ROWNUM);}
            Hash[n] = 1;
        }
        l.pop_front();
    }

    if(count<=48){
        if(test) qDebug()<<"\n\n\nCheck_BLocking_Center failed!\n\n\n";
        return false;
    }else
        return true;
}

void Widget::Check_Blocking_Up(){
    int distance[10] = {0};
    int inside_dist[10] = {0};
    //下侧
    int down[] = {24,25,30,31};
    //左、右侧
    int side[] = {2,10,18,5,13,21};

    //设置四个标记，记录是否需要进行距离判断凿墙。
    bool red_flag1 = true;
    bool red_flag2 = true;
    bool green_flag1 = true;
    bool green_flag2 = true;

    if(wall[1][3]==1&&wall[2][3]==0){
        //2只有单开口时只能凿9|10
        if(!wall[2][3] || !wall[2][2] || bfs(7,2,true) < bfs(7,10,true)){
            wall[10][1] = 0;
            wall[9][2] = 0;
            //myDrawLine(9,10);
        }else{
            wall[2][1] = 0;
            wall[1][2] = 0;
           // myDrawLine(1,2);
        }
        red_flag1 = false;
        //distance[2] = 50;
        //distance[3] = 50;
        //distance[4] = 50;
    }
    if(wall[8][3]&&wall[9][3]){
        //24 25，取离左下角远的格子开凿
        //if(!wall[24][2]){
        if(bfs(56,8,true)>bfs(56,9,true)){//8远
            wall[8][3]=0;
            wall[16][0]=0;
            //myDrawLine(8,16);
        }else{
            wall[9][3]=0;
            wall[17][0]=0;
            //myDrawLine(9,17);
        }
        //}
        wall[17][2] = 0;
        wall[18][1] = 0;
        //myDrawLine(17,18);
        red_flag2 = false;
        distance[0] = 50;
        distance[1] = 50;
        distance[2] = 50;
    }

    if(wall[6][3]==1&&wall[5][3]==0){
        //25只有单开口时只能凿13|14
        if(!wall[5][3] || !wall[5][1] || bfs(0,5,true) < bfs(0,13,true)){
            wall[13][2] = 0;
            wall[14][1] = 0;
            //myDrawLine(13,14);
        }else{
            wall[5][2] = 0;
            wall[6][1] = 0;
            //myDrawLine(5,6);
        }
        green_flag1 = false;
        //distance[7] = 50;
        //distance[8] = 50;
        //distance[9] = 50;
    }
    if(wall[14][3]&&wall[15][3]){
        //24 25，取离左下角远的格子开凿
        //if(!wall[24][2]){
        if(bfs(63,15,true)>bfs(63,14,true)){//15远
            wall[15][3]=0;
            wall[23][0]=0;
            //myDrawLine(15,23);
        }else{
            wall[14][3]=0;
            wall[22][0]=0;
            //myDrawLine(14,22);
        }
        //}
        wall[22][1] = 0;
        wall[21][2] = 0;
        //myDrawLine(21,22);
        green_flag2 = false;
        distance[9] = 50;
        distance[5] = 50;
        distance[6] = 50;
    }

    //先对左上角,红色终点
    for(int i = 0;i < 2;i++){
        if(distance[i]==50) {distance[i] = 0;continue;}
        distance[i] = bfs(63,down[i],false);
        if(distance[i]>50) distance[i] = 0;
    }
    for(int i = 0;i < 3;i++){
        if(distance[i]==50) {distance[i] = 0;continue;}
        distance[i+2] = bfs(63,side[i],false);
        while(distance[i] > 50) {
            side[i] = side[i]+1;
            distance[i] = bfs(63,side[i],false);
        }
    }

    //再对右上角，绿色终点
    for(int i = 2;i < 4;i++){//i=2对应下标5
        if(distance[i]==50) {distance[i] = 0;continue;}
        distance[i+3] = bfs(56,down[i],false);
        if(distance[i]>50) distance[i] = 0;
    }
    for(int i = 3;i < 6;i++){//i=3对应下标7
        if(distance[i]==50) {distance[i] = 0;continue;}
        distance[i+4] = bfs(56,side[i],false);
        while(distance[i] > 50) {
            if(test) qDebug()<<"now is i"<<i<<"side[i] is"<<side[i]<<"distance[i+4] is:"<<distance[i+4];
            if(side[i]==3) {if(test) qDebug()<<"An error4!\n\n\n\n\n\n\n\n\n";break;}
            side[i] = side[i]-1;
            distance[i+4] = bfs(56,side[i],false);
        }
        if(side[i]==3) distance[i+4] = 0;
    }
    //计算中上区块边界离对应起点的距离distance
    for(int i = 0;i < 10;i++){
        //已经处理完毕的，此时distance为0，直接跳过
        if(distance[i]==0) {continue;}

        if(i<2){
            inside_dist[i] = bfs(down[i]-ROWNUM,0,false);
        }else if(i<5){
            inside_dist[i] = bfs(side[i-2]-1,0,false);
        }else if(i<7){
            inside_dist[i] = bfs(down[i-3]-ROWNUM,7,false);
        }else{
            inside_dist[i] = bfs(side[i-4]+1,7,false);
        }
        distance[i] = inside_dist[i] + distance[i];
        if(test) qDebug()<<"success! distance:"<<i<<distance[i];
    }

    //对distance四周进行“超三连”检测

    //先选择最接近的距离
    int red_index = 0;
    int green_index = 0;
    int absolute = 99;
    for(int i = 0;i < 5;i++){
        if(distance[i] ==0) continue;
        for(int j = 5; j < 10;j++){
            if(distance[j]== 0) continue;
            if(abs(distance[i]-distance[j])<=absolute){
                absolute = abs(distance[i]-distance[j]);
                red_index = i;
                green_index = j;
            }
        }
    }
    if(test) qDebug()<<"Distance-check 1:\n red_index"<<red_index<<distance[red_index]<<"green_index"<<green_index<<distance[green_index];

    if(red_index<2){
        int tmp = down[red_index];
        wall[tmp][0] = 0;
        wall[tmp-ROWNUM][3] = 0;
       // myDrawLine(tmp,tmp-ROWNUM);
        //if(red_index == 1)
        distance[4] = 0;
        distance[0] = 0;
        distance[1] = 0;
    }else{
        if(red_flag1) {
            int tmp = side[red_index-2];
            wall[tmp][1] = 0;
            wall[tmp-1][2] = 0;
            //myDrawLine(tmp,tmp-1);
            if(red_index==4) {
                distance[1] = 0;
            }
        }
        distance[2] = 0;
        distance[3] = 0;
        distance[4] = 0;

    }
    if(green_index<7){
        int tmp = down[green_index-3];
        wall[tmp][0] = 0;
        wall[tmp-ROWNUM][3] = 0;
        //myDrawLine(tmp,tmp-ROWNUM);
        //if(green_index == 6)
        distance[9] = 0;
        distance[5] = 0;
        distance[6] = 0;
    }else{
        if(green_flag1){
            int tmp = side[green_index-4];
            wall[tmp][2] = 0;
            wall[tmp+1][1] = 0;
            //myDrawLine(tmp,tmp+1);
            if(green_index==9)
                distance[5] = 0;
        }
        distance[7] = 0;
        distance[8] = 0;
        distance[9] = 0;
    }

    if(!red_flag2||!green_flag2){
        if(!red_flag2&&!green_flag2){
            return;
        }
        else if(!red_flag2){
            red_index = 10;
            absolute = 99;
            int sample = distance[green_index];
            for(int j = 5; j < 10;j++){
                if(distance[j]== 0) continue;
                if(abs(sample-distance[j])<=absolute){
                    absolute = abs(sample-distance[j]);
                    green_index = j;
                }
            }

        }else{
            green_index = 10;
            absolute = 99;
            int sample = distance[red_index];
            for(int j = 0; j < 5;j++){
                if(distance[j]== 0) continue;
                if(abs(sample-distance[j])<=absolute){
                    absolute = abs(sample-distance[j]);
                    red_index = j;
                }
            }
        }
    }else{
        absolute = 99;
        for(int i = 0;i < 5;i++){
            if(distance[i] ==0) continue;
            for(int j = 5; j < 10;j++){
                if(distance[j]== 0) continue;
                if(abs(distance[i]-distance[j])<=absolute){
                    absolute = abs(distance[i]-distance[j]);
                    red_index = i;
                    green_index = j;
                }
            }
        }
    }

    if(test) qDebug()<<"Distance-check 2:\n red_index"<<red_index<<distance[red_index]<<"green_index"<<green_index<<distance[green_index];
    if(red_index == 10){}
    else if(red_index<2){
        int tmp = down[red_index];
        wall[tmp][0] = 0;
        wall[tmp-ROWNUM][3] = 0;
        //myDrawLine(tmp,tmp-ROWNUM);
    }else{
        int tmp = side[red_index-2];
        wall[tmp][1] = 0;
        wall[tmp-1][2] = 0;
       //myDrawLine(tmp,tmp-1);
    }
    if(green_index==10){}
    else if(green_index<7){
        int tmp = down[green_index-3];
        wall[tmp][0] = 0;
        wall[tmp-ROWNUM][3] = 0;
        //myDrawLine(tmp,tmp-ROWNUM);
    }else{
        int tmp = side[green_index-4];
        wall[tmp][1] = 0;
        wall[tmp+1][2] = 0;
        //myDrawLine(tmp,tmp+1);
    }
}

bool Widget::Check_Distance(){
    int distance = 0;
    distance = bfs(63,56,false);
    if(test) qDebug()<<"\n\n\ndistance from 63 to 56 is:"<<distance;
    if(distance<=13) return false;
    distance = bfs(0,56,false);
    if(test) qDebug()<<"\n\n\ndistance from 0 to 56 is:"<<distance;
    if(distance>17) return false;
    distance = bfs(63,7,false);
    if(test) qDebug()<<"distance from 7 to 63 is:"<<distance;
    if(distance>17) return false;
    distance = bfs(0,7,false);
    if(test) qDebug()<<"distance from 0 to 7 is:"<<distance;
    if(distance>17) return false;
    if(test) qDebug()<<"distance check succeed!\n\n\n";
    return true;
}

int Widget::getward(int index,int index2){
    int ward = 0;
    if(index-index2==ROWNUM) ward = 0;//index2在index上面
    else if(index-index2==-ROWNUM) ward = 3;//下面
    else if(index-index2==1) ward = 1;//左边
    else ward = 2;//右边
    return ward;
}



//注意red和greenindex必须初始化赋值为对应下标
void Widget::keyPressEvent(QKeyEvent *event)
{
    //mode变量控制模式设置
    //mode=0同时移动
    //mode=1轮次移动

    //轮次移动的定义:变量step，轮次turn,turn=0绿方，turn=1绿方
    //step=3->重置并更换turn


    //获取按钮原位置
    QPoint point_R = ui->btnRed->pos();
    QPoint point_G = ui->btnGreen->pos();
    int tmp_R =red_index;
    int tmp_G =green_index;
    //若移动前后下标合法，让其移动
    if(red_mutex == 0){
        if(mode==1&&turn==0){
            //轮次且绿方轮次，不读取
        }
        else if (event->key() == Qt::Key_Left) {
            if (point_R.x() - LINEWIDTH > BEGINX) {
                point_R.setX(point_R.x() - LINEWIDTH);
                red_index -= 1;
            }
        } else if (event->key() == Qt::Key_Right) {
            if (point_R.x() + LINEWIDTH < ENDX) {
                point_R.setX(point_R.x() + LINEWIDTH);
                red_index += 1;
            }
        } else if (event->key() == Qt::Key_Up) {
            if (point_R.y() - LINEHEIGHT > BEGINX) {
                red_index -= ROWNUM;
                point_R.setY(point_R.y() - LINEHEIGHT);
            }
        } else if (event->key() == Qt::Key_Down) {
            if (point_R.y() + LINEHEIGHT < ENDX) {
                red_index += ROWNUM;
                point_R.setY(point_R.y() + LINEHEIGHT);
            }
        }
    }
    if(green_mutex==0){
        if(mode==1&&turn==1){
            //轮次且红方轮次，不读取
        }
        else if (event->key() == Qt::Key_A) {
            if (point_G.x() - LINEWIDTH > BEGINX) {
                point_G.setX(point_G.x() - LINEWIDTH);
                green_index -= 1;
            }
        } else if (event->key() == Qt::Key_D) {
            if (point_G.x() + LINEWIDTH < ENDX) {
                point_G.setX(point_G.x() + LINEWIDTH);
                green_index += 1;
            }
        } else if (event->key() == Qt::Key_W) {
            if (point_G.y() - LINEHEIGHT > BEGINX) {
                green_index -= ROWNUM;
                point_G.setY(point_G.y() - LINEHEIGHT);
            }
        } else if (event->key() == Qt::Key_S) {
            if (point_G.y() + LINEHEIGHT < ENDX) {
                green_index += ROWNUM;
                point_G.setY(point_G.y() + LINEHEIGHT);
            }
        }
    }

    //对比下标，确定移动方向（其实可以嵌入上面的判断里）
    int ward_R = 0;
    int ward_G = 0;

    if(tmp_R!=red_index){
    if(tmp_R-red_index==ROWNUM) ward_R = 0;//neighbor在randnum上面
    else if(tmp_R-red_index==-ROWNUM) ward_R = 3;//下面
    else if(tmp_R-red_index==1) ward_R = 1;//左边
    else ward_R = 2;//右边

    if(test) qDebug()<<"Checkpoint in movement function";
    if(wall[tmp_R][ward_R] == 0){
        ui->btnRed->move(point_R);
        //轮次移动：step加一，step到达3交给后面判断
        if(mode==1){
            step++;
        }else if(mode==0){
            redstep++;
        }
    }else if(crush&&red_index!=tmp_R){
        red_mutex = 1;
        CrushAnimation(tmp_R,red_index,0);
        //轮次移动：直接置step为3
        if(mode==1){
            step = 3;
        }else if(mode==0){
            redstep = 0;
        }
        if(test) qDebug()<<"Checkpoint in movement function 3";
    }else{
        red_index = tmp_R;
    }
    ui->redstepnumLB->setText(QString::number(redstep));
    }

    if(test) qDebug()<<"Checkpoint in movement function 5";

    if(tmp_G!=green_index){
    if(tmp_G-green_index==ROWNUM) ward_G = 0;//neighbor在randnum上面
    else if(tmp_G-green_index==-ROWNUM) ward_G = 3;//下面
    else if(tmp_G-green_index==1) ward_G = 1;//左边
    else ward_G = 2;//右边
    if(wall[tmp_G][ward_G] == 0){
        ui->btnGreen->move(point_G);
        if(mode==1){
            step++;
        }else if(mode==0){
            greenstep++;
        }
    }else if(crush&&green_index!=tmp_G){
        green_mutex = 1;
        CrushAnimation(tmp_G,green_index,1);
        if(mode==1){
            step = 3;
        }else if(mode==0){
            greenstep = 0;
        }
    }else{
        green_index = tmp_G;
    }
    ui->greenstepnumLB->setText(QString::number(greenstep));
    }
    if(test) qDebug()<<"Red_index:"<<red_index;
    if(test) qDebug()<<"Green_index:"<<green_index;

    //根据step值切换turn
    if(mode){
        if(step==3){
            turn = turn==0 ? 1 : 0;
            step = 0;
            if(test) qDebug()<<"now turn is:"<<turn;
        }
        ui->stepLB->setText("当前的轮内步数为"+QString::number(step));
        QString roundstr =  "现在是";
        //turn==0->绿色 1->红色
        if(!turn) roundstr+="绿色方的回合";
        else roundstr+="红色方的回合";
        ui->roundLB->setText(roundstr);
    }

    //结束界面
    if(red_index!=tmp_R && red_index==0){
        if(green_index!=tmp_G && green_index==ROWNUM-1)
            showResultWindow(2);
        else showResultWindow(0);
    }else if(green_index!=tmp_G && green_index==ROWNUM-1){
        showResultWindow(1);
    }
}

int Widget::bfs(int cur, int end,bool through_finish){
    std::list<int> list;
    int* hash_bfs = new int[64];
    init_Hash(hash_bfs);
    int* path = new int[64];
    int n = cur;
    int length = 0;
    hash_bfs[n] = 1;
    list.push_back(n);
    //240310 让计算长度时不经过两个终点 之后添加through_finish，为false时不经过终点
    if(!through_finish){
        if(end!=0&&end!=ROWNUM-1)
        {
            hash_bfs[0] = 1;
            hash_bfs[ROWNUM-1] = 1;
        }
    }
    while(hash_bfs[end]!=1)
    {
        if(n/ROWNUM != 0 && !hash_bfs[n-ROWNUM] && !wall[n][0])//不是第一行，有上面一格
        {
            list.push_back(n-ROWNUM);
            path[n-ROWNUM] = n;
            hash_bfs[n-ROWNUM] = 1;
        }
        if(n/ROWNUM != ROWNUM-1 && !hash_bfs[n+ROWNUM] && !wall[n][3])//不是最后一行，有下面一格
        {
            list.push_back(n+ROWNUM);
            path[n+ROWNUM] = n;
            hash_bfs[n+ROWNUM] = 1;
        }
        if(n%ROWNUM != 0 && !hash_bfs[n-1] && !wall[n][1]) //不是第一列,有左边一格
        {
            list.push_back(n-1);
            path[n-1] = n;
            hash_bfs[n-1] = 1;
        }
        if(n%ROWNUM != ROWNUM-1 && !hash_bfs[n+1] && !wall[n][2]) //不是最后一列，有右边一格
        {
            list.push_back(n+1);
            path[n+1] = n;
            hash_bfs[n+1] = 1;
        }

        if(list.size()==0){ //不连通检测：赋值999并退出
            if(test) qDebug()<<"error! in bfs: no road from"<<cur<<"to"<<end;
            length = 999;
            break;
        }

        n = list.front();
        list.pop_front();

    }
    int t = end;
    if(length != 999)
    {
        while(t!=cur){
            //if(test) qDebug()<<path[t];
            length++;
            t = path[t];
        }
    }
    return length;
}

//进行检查：保证第一行不会出现长段死路
void Widget::Cut(){
    if(test) qDebug()<<"Into cut()";
    int dist;
    //对靠红色终点的2、3与靠绿色终点的4、5朝终点方向检测，目的为破坏第一行的"死路"
    for(int i = 2;i<=3;i++){
        dist = bfs(i,i-1,true);
        if(dist>9){
            if(test) qDebug()<<"into: cut() first if";
            //显然，能进入这个分支，左墙一定存在，不然距离就是1
            //法一：检查可能结构，尝试给出改良的凿墙法
            //情况1： |  |
            //       |     |--
            //       -------
            //直接破左上，不用考虑......
            //情况2：  |
            //        |  |
            //        ----
            //要破下面一格的左墙
            if(wall[i][2]==0&&wall[i][3]==0&&wall[i+ROWNUM][1]==1&&wall[i+ROWNUM][2]==1){
                //wall[i+ROWNUM][1]==1不一定用
                wall[i+ROWNUM][1] = 0;
                wall[i-1+ROWNUM][2] = 0;
                //myDrawLine(i+ROWNUM,i-1+ROWNUM);
                continue;
            }else if(wall[i][1]==0&&wall[i][3]==0&&wall[i+ROWNUM][1]==1&&wall[i+ROWNUM][2]==1){
                //wall[i+ROWNUM][1]==1不一定用
                wall[i+ROWNUM][2] = 0;
                wall[i+1+ROWNUM][1] = 0;
                //myDrawLine(i+ROWNUM,i+1+ROWNUM);
                continue;
            }else if(wall[i][2]==0&&wall[i+1][2]==0&&wall[i][3]==1&&wall[i+1][3]==1){
                //情况3:  |
                //        -- -- --
                //打通该格下面的墙
                wall[i][3] = 0;
                wall[i+ROWNUM][0] = 0;
                //myDrawLine(i+ROWNUM,i);
                continue;
            }
            // i-1 | i
            wall[i][1] = 0;
            wall[i-1][2] = 0;
            //myDrawLine(i,i-1);
        }
    }
    for(int i = 4;i<=5;i++){
        dist = bfs(i,i+1,true);
        if(dist>9){
            if(test) qDebug()<<"into: cut() second if";
            //显然，能进入这个分支，右墙一定存在，不然距离就是1
            //同理对称
            //情况2：     |
            //        |  |
            //        ----
            //要破下面一格的右墙
            if(wall[i][1]==0&&wall[i][3]==0&&wall[i+ROWNUM][1]==1&&wall[i+ROWNUM][2]==1){
                //wall[i+ROWNUM][1]==1不一定用
                wall[i+ROWNUM][2] = 0;
                wall[i+1+ROWNUM][1] = 0;
                //myDrawLine(i+ROWNUM,i+1+ROWNUM);
                continue;
            }else if(wall[i][2]==0&&wall[i][3]==0&&wall[i+ROWNUM][1]==1&&wall[i+ROWNUM][2]==1){
                //wall[i+ROWNUM][1]==1不一定用
                wall[i+ROWNUM][1] = 0;
                wall[i-1+ROWNUM][2] = 0;
                //myDrawLine(i+ROWNUM,i-1+ROWNUM);
                continue;
            }else if(wall[i][1]==0&&wall[i-1][1]==0&&wall[i][3]==1&&wall[i-1][3]==1){
                //情况3:          |
                //        -- -- --
                //打通该格下面的墙
                wall[i][3] = 0;
                wall[i+ROWNUM][0] = 0;
                //myDrawLine(i+ROWNUM,i);
                continue;
            }
            // i | i+1
            wall[i][2] = 0;
            wall[i+1][1] = 0;
            //myDrawLine(i,i+1);
        }
    }
}

void Widget::Check_bfs(){
    if(test) qDebug()<<"\n\nInto Function Check_bfs";
    //建立哈希表
    int* Hash = new int[64];
    init_Hash(Hash);
    //建立标记链表
    std::list<int> flag_array;
    for(int i = 0 ; i < 64; i++){
        //对下标i进行处理
        int index = i;
        //若唯一连通只有两个开口，要接着检查
        while(index<=i){
            //哈希表，防止无效有害的重复检查
            if(Hash[index]) break;
            else Hash[index] = 1;
            if(index==0||index==7||index==56||index==63) break;
            int count = 0;//记录无墙的面数
            for(int j = 0;j<4;j++){
                if(wall[index][j]==0) count++;
            }
            if(test) qDebug()<<"now: index is:"<<index<<"wall count is"<<count;
            //记录无墙的方向，检测出只有一处开口的格子，否则直接结束本次循环
            int ward = 0;
            if(count == 1){
                for(int j = 0;j<4;j++){
                    if(wall[index][j]==0) {
                        ward=j;
                        break;
                    }
                }
            }else break;
            if(test) qDebug()<<"now: into check index is:"<<index<<"ward is"<<ward;
            //检查与其相联个格子的开口数
            int temp = Get_index(index,ward);
            if(temp==0||temp==7||temp==56||temp==63) break;
            count = 0;
            for(int j = 0;j<4;j++){
                if(wall[temp][j]==0) {
                    count++;
                }
            }
            if(count==3){
                //记录在flag_array里面
                if(flag_array.size()==0){
                    flag_array.push_back(temp);
                    break;
                }else{
                    auto it = std::find(flag_array.begin(),flag_array.end(),temp);
                    if(it == flag_array.end()){
                        flag_array.push_back(temp);
                        break;//若查不到，把其放入flag链表
                    }
                    else{//不然是多格组成的死路,直接进行填充
                        for(int j = 0;j<4;j++){
                            if(wall[temp][j]==0) {
                                ward=j;
                                int temp2 = Get_index(temp,ward);
                                count = 0;
                                for(int k = 0;k<4;k++){
                                    if(wall[ward][k]==0) {
                                        count++;
                                    }
                                }
                                if(count == 1||count == 2){
                                    wall[temp][ward]=1;
                                    wall[temp2][3-ward] = 1;
                                    //myDrawLine(temp,temp2,Qt::gray);
                                }
                            }
                        }
                        break;
                    }
                }
            }else if(count==2){//这个格子是死路的一部分
                //把墙填上
                wall[index][ward]=1;
                wall[temp][3-ward] = 1;
                //myDrawLine(index,temp,Qt::gray);
                //遍历新的单开口格子
                index = temp;
                Hash[index] = 0;
                continue;
            }
            if(test) qDebug()<<"Error! In Check_bfs: it is bound to not reach here";
            break;
        }
        update();
    }
}

int Widget::Get_index(int index,int ward){
    int offset = 0;
    switch(ward){
    case 0:offset = -ROWNUM;
        break;
    case 1:offset = -1;
        break;
    case 2:offset = 1;
        break;
    case 3:offset = ROWNUM;
        break;
    default:
        if(test) qDebug()<<"Error in Function Get_index! invalid ward! ward is"<<ward<<"now index is:"<<index;
        break;
    }
    return index + offset;
}




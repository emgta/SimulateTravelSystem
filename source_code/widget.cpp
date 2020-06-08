#include "widget.h"
#include "ui_widget.h"
#include "log.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent), currentTraveler(-1), ui(new Ui::Widget),
    addtravelertimes(0), startclickedtimes(0), throughcity(10, false)
{
    ui->setupUi(this);
    Initial_UI();//初始化界面
    Initial_Time_Thread();//初始化线程
    Initial_Signal_Slot();//初始化信号与槽
}

//初始化图形界面
void Widget::Initial_UI()
{
    qDebug() << "Successfully initialize UI";
    this->setFixedSize(1280, 820);//设置窗口大小
    this->setWindowTitle("旅行模拟");//设置窗口标题
    QIcon exe_ico(":/exe.ico");
    this->setWindowIcon(exe_ico);//设置窗口图标

    //初始化时间与选择项
    ui->StartDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    int deaDay = QDateTime::currentDateTime().date().day();
    deaDay += 1;
    QDateTime deadlineDateTime;
    deadlineDateTime.setDate(QDate(QDateTime::currentDateTime().date().year(), QDateTime::currentDateTime().date().month(), deaDay));
    deadlineDateTime.setTime(QTime(QDateTime::currentDateTime().time()));
    ui->DeadlineDateTimeEdit->setDateTime(deadlineDateTime);
    ui->StrategyChoice->setEnabled(false);
    ui->StartChoice->setEnabled(false);
    ui->DestinationChoice->setEnabled(false);
    ui->DestinationChoice->setCurrentIndex(1);
    ui->TravelerChoice->setEnabled(false);
    ui->ThroughCityChoice->setEnabled(false);

//    ui->StartButton->setFocus();
//    ui->StartButton->setShortcut(Qt::Key_Return);    

    //初始化城市选项
    ui->city_box_0->setEnabled(false);
    ui->city_box_1->setEnabled(false);
    ui->city_box_2->setEnabled(false);
    ui->city_box_3->setEnabled(false);
    ui->city_box_4->setEnabled(false);
    ui->city_box_5->setEnabled(false);
    ui->city_box_6->setEnabled(false);
    ui->city_box_7->setEnabled(false);
    ui->city_box_8->setEnabled(false);
    ui->city_box_9->setEnabled(false);

    ui->StartDateTimeEdit->setEnabled(false);
    ui->DeadlineDateTimeEdit->setEnabled(false);
    ui->StartButton->setEnabled(false);
    ui->DurationText->setEnabled(false);
    ui->FareEdit->setEnabled(false);
    ui->TotalTimeEdit->setEnabled(false);

}

//建立信号与槽
void Widget::Initial_Signal_Slot()
{

    connect(ui->AddTraveler, SIGNAL(clicked()), this, SLOT(AddTravelerClicked()));//添加旅客
    QObject::connect(ui->TravelerChoice, SIGNAL(currentIndexChanged(int)), this, SLOT(travelerChanged()));//选择旅客

    QObject::connect(ui->ThroughCityChoice, SIGNAL(toggled(bool)), this, SLOT(activeThroughCity()));//途经城市选择
    //城市
    QObject::connect(ui->city_box_0, SIGNAL(toggled(bool)), this, SLOT(Set_Through_City0()));
    QObject::connect(ui->city_box_1, SIGNAL(toggled(bool)), this, SLOT(Set_Through_City1()));
    QObject::connect(ui->city_box_2, SIGNAL(toggled(bool)), this, SLOT(Set_Through_City2()));
    QObject::connect(ui->city_box_3, SIGNAL(toggled(bool)), this, SLOT(Set_Through_City3()));
    QObject::connect(ui->city_box_4, SIGNAL(toggled(bool)), this, SLOT(Set_Through_City4()));
    QObject::connect(ui->city_box_5, SIGNAL(toggled(bool)), this, SLOT(Set_Through_City5()));
    QObject::connect(ui->city_box_6, SIGNAL(toggled(bool)), this, SLOT(Set_Through_City6()));
    QObject::connect(ui->city_box_7, SIGNAL(toggled(bool)), this, SLOT(Set_Through_City7()));
    QObject::connect(ui->city_box_8, SIGNAL(toggled(bool)), this, SLOT(Set_Through_City8()));
    QObject::connect(ui->city_box_9, SIGNAL(toggled(bool)), this, SLOT(Set_Through_City9()));

    QObject::connect(ui->StartButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));//开始按钮
    QObject::connect(ui->StrategyChoice, SIGNAL(currentIndexChanged(int)), this, SLOT(enOrDisAbleDeadline(int)));//出行选择

    //计时线程
    QObject::connect(mstimer, SIGNAL(timeout()), this, SLOT(Show_Spent_Time()));
    QObject::connect(ui->StartButton, SIGNAL(clicked()), this, SLOT(timeStart()));
    QObject::connect(this, SIGNAL(DoStartTimer()), mstimer, SLOT(start()));

}

//添加计时所用的线程和timer以确保时间计算的准确
void Widget::Initial_Time_Thread()
{
    //计时功能及所需的线程
    mstimer = new QTimer;
    mstimer->setInterval(1);

    timethread = new QThread();
    timethread->start();

    mstimer->moveToThread(timethread);
}

//退出图形界面
Widget::~Widget()
{
    qDebug() << "Successfully exit.";
    //安全退出时间线程
    timethread->terminate();
    timethread->wait();
    delete ui;
    delete timethread;
}

//单击“开始”按钮，获取用户输入信息
void Widget::startButtonClicked()
{
    QDateTime startDateTime;
    //对于当前旅客，初次点击开始按钮
    if (startclicked[ui->TravelerChoice->currentIndex()] == false)
    {
        qDebug() << "StartButton clicked 1st time for CurrentTraveler";
        strategy = Get_Strategy();
        start = Get_Depart();
        destination = Get_Destination();
        //始发地和目的地相同则弹框报错，不作操作
        if (start == destination)
        {
            qDebug() << "Start and Dedtination is the same one, wait for  another command";
            QMessageBox::information(this, "Error", QString::fromWCharArray(L"出发地和目的地相同"));
            return;
        }
        //（策略三的情况下）截止时间早于当前时间报错，不作操作
        if (!(ui->StartDateTimeEdit->dateTime() < ui->DeadlineDateTimeEdit->dateTime()))
        {
            qDebug() << "Deadline ahead of StratTime, wait for another command";
            QMessageBox::information(this, "Error", QString::fromWCharArray(L"截止时间早于当前时间"));
            return;
        }

        startDateTime = Get_Start_Time();

        travelers[ui->TravelerChoice->currentIndex()] = (Traveler(addtravelertimes-1, startDateTime,
                                                                    Get_Deadline(), strategy, start, destination, ui->ThroughCityChoice->isChecked(), throughcity));
        std::vector<Attribute> path = travelers[ui->TravelerChoice->currentIndex()].getPlan();
        if (path.size() == 0)
        {
            qDebug() << "No legal path";
            QMessageBox::information(this, "Error", QString::fromWCharArray(L"无有效路径"));
            startclicked[ui->TravelerChoice->currentIndex()] = false;
            return;
        }

        startclicked[ui->TravelerChoice->currentIndex()] = true;
        currentTraveler = ui->TravelerChoice->currentIndex();

        Show_Total_Time();
        Show_Fare(path);
        Show_Solution(path);

        qDebug() << "StartButton rename as ChangePlan for CurrentTraveler";
        ui->StartButton->setText(QString::fromWCharArray(L"更改"));
        ui->StartChoice->setEnabled(false);
        ui->StartDateTimeEdit->setEnabled(false);
        startclickedtimes += 1;
        startclicked[ui->TravelerChoice->currentIndex()] = true;
        return;
    }
    //对于当前旅客，执行更改计划操作
    if (startclicked[ui->TravelerChoice->currentIndex()] == true)
    {
        qDebug() << "StartButton clicked for CurrentTraveler";
        strategy = Get_Strategy();
        destination = Get_Destination();

        if (!(ui->StartDateTimeEdit->dateTime() < ui->DeadlineDateTimeEdit->dateTime()))
        {
            qDebug() << "Deadline ahead of StartTime, reset the DeadlineDateTimeEdit,wait for another command";
            QMessageBox::information(this, "Error", QString::fromWCharArray(L"截止时间早于当前时间"));

            int deaDay = ui->StartDateTimeEdit->dateTime().date().day();
            deaDay += 1;
            QDateTime deadlineDateTime;
            deadlineDateTime.setDate(QDate(ui->StartDateTimeEdit->dateTime().date().year(), ui->StartDateTimeEdit->dateTime().date().month(), deaDay));
            deadlineDateTime.setTime(QTime(ui->StartDateTimeEdit->dateTime().time()));
            ui->DeadlineDateTimeEdit->setDateTime(deadlineDateTime);

            return;
        }

        //获得新计划的始发地，即原计划的当前停留地/运行途中即将到达地
        int nextCity2Arrive = ui->LeftWidget->nextCity();
        if (nextCity2Arrive != -1)
        {
            std::vector<Attribute> path = travelers[ui->TravelerChoice->currentIndex()].changePlan(nextCity2Arrive, strategy, destination, Get_Deadline(),
                                                                                                     ui->ThroughCityChoice->isChecked(),throughcity);
            if (path.size() == 0)
            {
                qDebug() << "No legal path";
                QMessageBox::information(this, "Error", QString::fromWCharArray(L"无有效路径"));
                return;
            }
            qDebug() << "Change plan success.";
            currentTraveler = ui->TravelerChoice->currentIndex();
            Show_Total_Time();
            Show_Fare(path);
            Show_Solution(path);
        }
    }
}

//根据策略决定截止日期栏状态，只有策略三使得截止日期状态栏可编辑
void Widget::enOrDisAbleDeadline(int currentStrategy)
{
    qDebug() << "Strategy changed to Strategy" << currentStrategy;
    if (currentStrategy != 2)
        ui->DeadlineDateTimeEdit->setEnabled(false);
    else
        ui->DeadlineDateTimeEdit->setEnabled(true);
}

//单击“添加旅客”按钮，开始运行
void Widget::AddTravelerClicked()
{
    //添加旅客，初始化旅客信息
    qDebug() << "Add new traveler, reset all elements in widget";
    std::vector<bool> temp(10, false);
    throughcity = temp;
    qDebug() << "throughcity creat success.";
    travelers.push_back(Traveler(addtravelertimes-1, Get_Start_Time(), Get_Deadline(),
                                 Get_Strategy(), Get_Depart(), Get_Destination(),
                                 ui->ThroughCityChoice->isChecked(), throughcity));
    qDebug() << "travelers.pushback...";
    startclicked.push_back(false);
    addtravelertimes += 1;
    startclickedtimes = 0;

    //将界面右侧各栏初始化显示
    ui->TravelerChoice->addItem(QString::number(addtravelertimes));
    ui->TravelerChoice->setCurrentText(QString::number(addtravelertimes));

    ui->StartButton->setText(QString::fromWCharArray(L"开始"));
    ui->TravelerChoice->setEnabled(true);
    ui->StartChoice->setEnabled(true);
    ui->StrategyChoice->setEnabled(true);
    ui->DestinationChoice->setEnabled(true);
    ui->StrategyChoice->setCurrentIndex(0);
    ui->StartChoice->setCurrentIndex(0);
    ui->DestinationChoice->setCurrentIndex(1);
    ui->ThroughCityChoice->setEnabled(true);
    ui->DeadlineDateTimeEdit->setEnabled(false);
    ui->StartDateTimeEdit->setEnabled(true);
    ui->StartButton->setEnabled(true);
    ui->Solution->clear();

    ui->StartDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    int deaDay = QDateTime::currentDateTime().date().day();
    deaDay += 1;
    QDateTime deadlineDateTime;
    deadlineDateTime.setDate(QDate(QDateTime::currentDateTime().date().year(), QDateTime::currentDateTime().date().month(), deaDay));
    deadlineDateTime.setTime(QTime(QDateTime::currentDateTime().time()));
    ui->DeadlineDateTimeEdit->setDateTime(deadlineDateTime);
}

//旅客选择更改，显示更改
void Widget::travelerChanged()
{
    ui->StartButton->setEnabled(true);
    //当前旅客执行过构造计划，则将界面显示为该旅客的信息
    if (startclicked[ui->TravelerChoice->currentIndex()])
    {
        qDebug() << "Change traveler to who possesses a plan, display his/her info and plan";
        ui->StartDateTimeEdit->setDateTime(travelers[ui->TravelerChoice->currentIndex()].startTime);
        int deaDay = travelers[ui->TravelerChoice->currentIndex()].deadlineTime.date().day();
        QDateTime deadlineDateTime;
        deadlineDateTime.setDate(QDate(travelers[ui->TravelerChoice->currentIndex()].deadlineTime.date().year(), travelers[ui->TravelerChoice->currentIndex()].deadlineTime.date().month(), deaDay));
        deadlineDateTime.setTime(QTime(travelers[ui->TravelerChoice->currentIndex()].deadlineTime.time()));
        ui->DeadlineDateTimeEdit->setDateTime(deadlineDateTime);
        ui->StrategyChoice->setCurrentIndex(travelers[ui->TravelerChoice->currentIndex()].strategy);
        ui->StartChoice->setCurrentIndex(travelers[ui->TravelerChoice->currentIndex()].origin);
        ui->DestinationChoice->setCurrentIndex(travelers[ui->TravelerChoice->currentIndex()].destination);

        Show_Fare(travelers[ui->TravelerChoice->currentIndex()].getPlan());
        Show_Total_Time();
        Show_Solution(travelers[ui->TravelerChoice->currentIndex()].getPlan());
        Show_Spent_Time();

        ui->StartButton->setText(QString::fromWCharArray(L"更改"));
        ui->StartChoice->setEnabled(false);
        ui->StartDateTimeEdit->setEnabled(false);
        ui->ThroughCityChoice->setChecked(travelers[ui->TravelerChoice->currentIndex()].isChecked);
        throughcity = travelers[ui->TravelerChoice->currentIndex()].throughCity;
        activeThroughCity();

        currentTraveler = ui->TravelerChoice->currentIndex();
    }
    else
    {
        //当前旅客未执行过构造计划操作，则将界面初始化
        qDebug() << "Change traveler to who possesses no plan, reset the elements in widget";
        ui->StartButton->setText(QString::fromWCharArray(L"开始"));
        ui->StartChoice->setEnabled(true);
        ui->DestinationChoice->setEnabled(true);
        ui->StartDateTimeEdit->setEnabled(true);
        ui->DeadlineDateTimeEdit->setEnabled(true);
        ui->ThroughCityChoice->setChecked(false);
        throughcity = travelers[ui->TravelerChoice->currentIndex()].throughCity;
        activeThroughCity();

        ui->StartDateTimeEdit->setDateTime(QDateTime::currentDateTime());
        int deaDay = QDateTime::currentDateTime().date().day();
        deaDay += 1;
        QDateTime deadlineDateTime;
        deadlineDateTime.setDate(QDate(QDateTime::currentDateTime().date().year(), QDateTime::currentDateTime().date().month(), deaDay));
        deadlineDateTime.setTime(QTime(QDateTime::currentDateTime().time()));
        ui->DeadlineDateTimeEdit->setDateTime(deadlineDateTime);
        ui->StrategyChoice->setCurrentIndex(0);
        ui->StartChoice->setCurrentIndex(0);
        ui->DestinationChoice->setCurrentIndex(1);
        QVBoxLayout *listlayout = new QVBoxLayout;
        QWidget *containwidget = new QWidget(ui->Solution);
        containwidget->setLayout(listlayout);
//        ui->Solution->setWidget(containwidget);
        ui->FareEdit->clear();
        ui->TotalTimeEdit->clear();
        ui->DurationText->clear();

        currentTraveler = -1;
    }
}

//获取用户所选策略
int Widget::Get_Strategy()
{
    return ui->StrategyChoice->currentIndex();
}

//获取用户所选始发地
int Widget::Get_Depart()
{
    qDebug() << "Get Start City";
    return ui->StartChoice->currentIndex();
}

//获取用户所选目的地
int Widget::Get_Destination()
{
    qDebug() << "Det Destination";
    return ui->DestinationChoice->currentIndex();
}

//获取截止时间
QDateTime Widget::Get_Deadline()
{
    qDebug() << "Get Deadline";
    return ui->DeadlineDateTimeEdit->dateTime();
}

//获取开始时间
QDateTime Widget::Get_Start_Time()
{
    qDebug() << "Get StartTime";
    return ui->StartDateTimeEdit->dateTime();
}

//获取已用时间，根据纪录的旅客开始旅行时的系统时间和当前系统时间获得系统经过时间，按照10s = 1h的比率得到旅客经过时间
QDateTime Widget::Get_Spent_Time()
{    
    QDateTime usedDateTime = travelers[ui->TravelerChoice->currentIndex()].usedTime;

    int duration_day = usedDateTime.date().day();
    int duration_hour = usedDateTime.time().hour();
    int duration_min = usedDateTime.time().minute();
    int duration_sec = usedDateTime.time().second();
    int duration_ms = usedDateTime.time().msec();

    duration_ms += 360;

    duration_sec += duration_ms / 1000;
    duration_ms = duration_ms % 1000;
    duration_min += duration_sec / 60;
    duration_sec = duration_sec % 60;
    duration_hour += duration_min / 60;
    duration_min = duration_min % 60;
    duration_day += duration_hour /24;
    duration_hour = duration_hour % 24;
    duration_day = duration_day % 30;

   travelers[ui->TravelerChoice->currentIndex()].usedTime = QDateTime(QDate(1, 1, duration_day), QTime(duration_hour, duration_min, duration_sec, duration_ms));
   return travelers[ui->TravelerChoice->currentIndex()].usedTime;
}

//如果mstimer未激活，那么发出DoStartTimer信号
void Widget::timeStart()
{
    qDebug() << "mstimer timeout, emit SIGNAL DoStartTimer()";
    if (mstimer->isActive())
        return;
    emit DoStartTimer();
}

//显示开始出行到目前所用的时间
void Widget::Show_Spent_Time()
{
    //当前用户执行过构造计划
    if (startclicked[ui->TravelerChoice->currentIndex()])
    {
        QDateTime spentTime = Get_Spent_Time();
        //已用时间不超过计划用总时间
        if (travelers[ui->TravelerChoice->currentIndex()].totalTime >= spentTime)
        {
            ui->DurationText->setText(QString::number(spentTime.date().day()-1) + QString::fromWCharArray(L"天 ")
                    + QString::number(spentTime.time().hour()) + QString::fromWCharArray(L"小时 ")
                    + QString::number(spentTime.time().minute()) + QString::fromWCharArray(L"分钟"));
        }
        //已用时间超过计划用总时间，显示总时间
        else if (ui->StartButton->isEnabled())
        {
            qDebug() << "Time spent equals total time";
            ui->DurationText->setText(QString::number(travelers[ui->TravelerChoice->currentIndex()].totalTime.date().day()-1)
                    + QString::fromWCharArray(L"天 ") +
                    QString::number(travelers[ui->TravelerChoice->currentIndex()].totalTime.time().hour())
                    + QString::fromWCharArray(L"小时 ") +
                    QString::number(travelers[ui->TravelerChoice->currentIndex()].totalTime.time().minute())
                    + QString::fromWCharArray(L"分钟"));
            qDebug() << "SpentTime: " << QString::number(travelers[ui->TravelerChoice->currentIndex()].totalTime.date().day()-1)
                    + "Day " +
                    QString::number(travelers[ui->TravelerChoice->currentIndex()].totalTime.time().hour())
                    + "Hour " +
                    QString::number(travelers[ui->TravelerChoice->currentIndex()].totalTime.time().minute())
                    + "Minute";
            ui->StartButton->setEnabled(false);
        }
    }
    else
        ui->DurationText->clear();
}

//直接在TotalTimeEdit显示方案所需总时间
void Widget::Show_Total_Time()
{
    QDateTime totalTime = travelers[ui->TravelerChoice->currentIndex()].totalTime;
    qDebug() << QString::number(totalTime.date().day()-1) + QString::fromWCharArray(L"天 ") +
                QString::number(totalTime.time().hour()) + QString::fromWCharArray(L"小时 ") +
                QString::number(totalTime.time().minute()) + QString::fromWCharArray(L"分钟");
    ui->TotalTimeEdit->setText(QString::number(totalTime.date().day()-1) + QString::fromWCharArray(L"天 ") +
                               QString::number(totalTime.time().hour()) + QString::fromWCharArray(L"小时 ") +
                               QString::number(totalTime.time().minute()) + QString::fromWCharArray(L"分钟"));
}

//显示方案所需经费
void Widget::Show_Fare(std::vector<Attribute> path)
{
    int totalcost = 0;
    std::vector<Attribute>::size_type index = 0;
    while(index != path.size())
    {
        totalcost += path[index].cost;
        index ++;
    }
    qDebug() << QString::number(totalcost) + QString::fromWCharArray(L"元");
    ui->FareEdit->setText(QString::number(totalcost) + QString::fromWCharArray(L"元"));
}

//将方案中城市编号对应城市名称
QString Widget::City_Number(int index){
    QString city;
    switch (index)
    {
    case 0:
        city = QString::fromWCharArray(L"北京");
        break;
    case 1:
        city = QString::fromWCharArray(L"上海");
        break;
    case 2:
        city = QString::fromWCharArray(L"广州");
        break;
    case 3:
        city = QString::fromWCharArray(L"重庆");
        break;
    case 4:
        city = QString::fromWCharArray(L"青岛");
        break;
    case 5:
       city = QString::fromWCharArray(L"郑州");
        break;
    case 6:
        city = QString::fromWCharArray(L"兰州");
        break;
    case 7:
       city = QString::fromWCharArray(L"乌鲁木齐");
        break;
    case 8:
        city = QString::fromWCharArray(L"贵阳");
        break;
    case 9:
        city = QString::fromWCharArray(L"哈尔滨");
        break;

    default:
        QMessageBox::warning(this, "Error", QString::fromWCharArray(L"程序运行错误--请联系工作人员"));
        break;
    }
    return city;
}

//在ScrollArea显示路径
void Widget::Show_Solution(std::vector<Attribute> path)
{
    ui->Solution->clearContents();
    int rowIndex = 0;
    for(std::vector<Attribute>::size_type index = 0;
            index != path.size(); index++)
    {
        rowIndex+=3;
        ui->Solution->setRowCount(rowIndex);
        if (path[index].vehicle == 0)
            ui->Solution->setItem(index*3,0,new QTableWidgetItem(QString::fromWCharArray(L"(汽)") +path[index].num));
        else if (path[index].vehicle == 1)
            ui->Solution->setItem(index*3,0,new QTableWidgetItem(QString::fromWCharArray(L"(铁)") +path[index].num));
        else if (path[index].vehicle == 2)
            ui->Solution->setItem(index*3,0,new QTableWidgetItem(QString::fromWCharArray(L"(机)") +path[index].num));
        ui->Solution->setItem(index*3,1,new QTableWidgetItem(City_Number(path[index].from)));
        ui->Solution->setItem(index*3+1,1,new QTableWidgetItem("->"+City_Number(path[index].to)));
        ui->Solution->setItem(index*3,2,new QTableWidgetItem(QString::fromWCharArray(L"出发:") +path[index].begin.toString("hh:mm")));
        ui->Solution->setItem(index*3+1,2,new QTableWidgetItem(QString::fromWCharArray(L" 到站:") +path[index].end.toString("hh:mm")));
        ui->Solution->setItem(index*3+1,0,new QTableWidgetItem(QString::fromWCharArray(L"票价:") + QString::number(path[index].cost)));
    }
    ui->Solution->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->Solution->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->Solution->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
}

//激活gridwidget中的checkbox并初始化throughcity
void Widget::activeThroughCity()
{
    ui->city_box_0->setChecked(throughcity[0]);
    ui->city_box_1->setChecked(throughcity[1]);
    ui->city_box_2->setChecked(throughcity[2]);
    ui->city_box_3->setChecked(throughcity[3]);
    ui->city_box_4->setChecked(throughcity[4]);
    ui->city_box_5->setChecked(throughcity[5]);
    ui->city_box_6->setChecked(throughcity[6]);
    ui->city_box_7->setChecked(throughcity[7]);
    ui->city_box_8->setChecked(throughcity[8]);
    ui->city_box_9->setChecked(throughcity[9]);


    if (ui->ThroughCityChoice->isChecked())
    {
        ui->city_box_0->setEnabled(true);
        ui->city_box_1->setEnabled(true);
        ui->city_box_2->setEnabled(true);
        ui->city_box_3->setEnabled(true);
        ui->city_box_4->setEnabled(true);
        ui->city_box_5->setEnabled(true);
        ui->city_box_6->setEnabled(true);
        ui->city_box_7->setEnabled(true);
        ui->city_box_8->setEnabled(true);
        ui->city_box_9->setEnabled(true);
    }
    else
    {
        ui->city_box_0->setEnabled(false);
        ui->city_box_1->setEnabled(false);
        ui->city_box_2->setEnabled(false);
        ui->city_box_3->setEnabled(false);
        ui->city_box_4->setEnabled(false);
        ui->city_box_5->setEnabled(false);
        ui->city_box_6->setEnabled(false);
        ui->city_box_7->setEnabled(false);
        ui->city_box_8->setEnabled(false);
        ui->city_box_9->setEnabled(false);
    }
}

//设置10个城市哪些被指定途经
void Widget::Set_Through_City0()
{
   if (ui->city_box_0->isChecked())
       throughcity[0] = true;
   else
       throughcity[0] = false;
}
void Widget::Set_Through_City1()
{
    if (ui->city_box_1->isChecked())
        throughcity[1] = true;
    else
        throughcity[1] = false;
}
void Widget::Set_Through_City2()
{
    if (ui->city_box_2->isChecked())
        throughcity[2] = true;
    else
        throughcity[2] = false;
}
void Widget::Set_Through_City3()
{
    if (ui->city_box_3->isChecked())
        throughcity[3] = true;
    else
        throughcity[3] = false;
}
void Widget::Set_Through_City4()
{
   if (ui->city_box_4->isChecked())
       throughcity[4] = true;
   else
       throughcity[4]= false;
}
void Widget::Set_Through_City5()
{
    if (ui->city_box_5->isChecked())
        throughcity[5] = true;
    else
        throughcity[5] = false;
}
void Widget::Set_Through_City6()
{
    if (ui->city_box_6->isChecked())
        throughcity[6] = true;
    else
        throughcity[6] = false;
}
void Widget::Set_Through_City7()
{
    if (ui->city_box_7->isChecked())
        throughcity[7] = true;
    else
        throughcity[7] = false;
}
void Widget::Set_Through_City8()
{
    if (ui->city_box_8->isChecked())
        throughcity[8] = true;
    else
        throughcity[8] = false;
}
void Widget::Set_Through_City9()
{
    if (ui->city_box_9->isChecked())
        throughcity[9] = true;
    else
        throughcity[9] = false;
}

void Widget::on_pushButton_clicked()
{
    qDebug() << "Show the search dialog";
    search = new Search;
    search->show();
}

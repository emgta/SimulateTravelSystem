#ifndef WIDGET_H
#define WIDGET_H

#include "schedule.h"
#include "traveler.h"
#include "mapwidget.h"
#include "search.h"

#include <QWidget>
#include <QDebug>

#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QString>
#include <QMessageBox>

#include <QDate>
#include <QTime>
#include <QDateTimeEdit>
#include <QTimer>
#include <QThread>
#include <QKeyEvent>
#include <QEvent>

#include <QPixmap>
#include <QScrollArea>
#include <QPalette>
#include <QLayout>
#include <QPalette>

class Search;
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    Search *search;

    Schedule schedule;
    int currentTraveler;
    std::vector<Traveler> travelers;

    QDateTime Get_Start_Time();//获取开始时间
    QDateTime Get_Spent_Time();//获取已用时间
    QDateTime Get_Deadline();//获取截止时间

private:
    Ui::Widget *ui;

    //主算法需用变量
    int strategy;
    int start;
    int destination;
    int addtravelertimes;//添加旅客次数，即旅客编号最大值
    int startclickedtimes;//“开始”按钮点击次数，0次表示首次运行，1表示其他
    int priordestination;//保留目的地，如果运行过程中目的地改变，作为两个目的地比较的前者，与currentIndex比较

    //参与时间进程的变量
    QTimer *mstimer;
    QThread *timethread;

    std::vector<bool> startclicked;//“开始”按钮第一次按下
    std::vector<bool> throughcity;//途经城市是否选定的布尔型数组

    void Initial_UI();
    void Initial_Signal_Slot();
    void Initial_Time_Thread();

    int Get_Strategy();//获取用户所选策略
    int Get_Depart();//获取用户所选始发地
    int Get_Destination();//获取用户所选目的地

    void Show_Total_Time();//显示方案所需总时间
    void Show_Fare(std::vector<Attribute> path);//显示方案所需经费
    void Show_Solution(std::vector<Attribute> path);//在Solution窗口中显示路径

    QString City_Number(int index);//将城市编号转为城市名称

private slots:
    void startButtonClicked();//开始按钮按下，开始计算路径图形输出
    void AddTravelerClicked();//reset按钮按下，重置所有变量重新来过
    void travelerChanged();//切换旅客时更改界面显示
    void timeStart();//此函数用于，如果mstimer未激活，那么this发出DoStartTimer信号
    void activeThroughCity();//得到途经城市
    void Show_Spent_Time();//显示已经花费的时间
    void enOrDisAbleDeadline(int currentStrategy);//根据所选策略更改截止时间栏状态

    //设置各个城市是否被选为途经城市
    void Set_Through_City0();
    void Set_Through_City1();
    void Set_Through_City2();
    void Set_Through_City3();
    void Set_Through_City4();
    void Set_Through_City5();
    void Set_Through_City6();
    void Set_Through_City7();
    void Set_Through_City8();
    void Set_Through_City9();

    void on_pushButton_clicked();

signals:
    void DoStartTimer();//开始计时的信号
};

#endif // WIDGET_H

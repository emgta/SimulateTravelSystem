#include "schedule.h"
#include "log.h"
std::multimap<int, Attribute> Schedule::database;

Schedule::Schedule()
{
    QFile file(":/database.txt");
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Could not open the file by reading";
        return;
    }

    QTextStream in(&file);
    QString from, to, number, begin, end, cost, vechile; //分别为出发地、目的地、班次、开始时间、结束时间、费用、交通方式

    //将时刻表信息读入database数据结构
    while(!in.atEnd())
    {
        in >> from >> to >> number >> begin >> end >> cost >> vechile >> endl;
        if(from != "")
        {
            qDebug() << from << to << number << begin;
            Attribute line(CityToNum(from), CityToNum(to), number, QTime::fromString(begin),
                           QTime::fromString(end), cost.toInt(), vechile.toInt());
            database.insert(std::make_pair(CityToNum(from), line));
        }
    }
    qDebug() << "input success...";
}

//根据字符串转换为相应的序号
int Schedule::CityToNum(QString city)
{
    int num = -1;

    if(city == "北京")
        num = 0;
    else if(city == "上海")
        num = 1;
    else if(city == "广州")
        num = 2;
    else if(city == "重庆")
        num = 3;
    else if(city == "青岛")
        num = 4;
    else if(city == "郑州")
        num = 5;
    else if(city == "兰州")
        num = 6;
    else if(city == "乌鲁木齐")
        num = 7;
    else if(city == "贵阳")
        num = 8;
    else if(city == "哈尔滨")
        num = 9;

    return num;
}

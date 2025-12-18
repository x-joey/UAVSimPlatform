#include "cz_data.h"
#include <QDebug>

// 全局变量定义
LeiDaGongZuoZhuangTai U209_SouSuoLeiDaGongZuoZhuangTai;
LeiDaGongZuoZhuangTai U215_GenZongLeiDaGongZuoZhuangTai;
LeiDaCanShuSheZhi U212_GenZongLeiDaCanShuSheZhi;
LeiDaCanShuSheZhi U206_SouSuoLeiDaCanShuSheZhi;
RenGongGanYuXinXi U241_RenGongGanYuXinXi;
WeiBoXiTongGongZuoZhuangTai C99_WeiBoXiTongGongZuoZhuangTaiHuiGao;
UDP_242_BoShuDaoYinXinXi U242_BoShuDaoYinXinXi;
double PI = 3.1415926;

// xjw添加
// 模拟数据初始化（在构造函数中添加）
bool show_dianji = false;
LockedHash<Mubiao> rhkq;
LockedVector<QVector<QPointF>> dj_point(100);
QVector<JinSheQuYu> JSQY_list;
QVector<JinSheQuYu> FX_JS_list;
QVector<JinSheQuYu> QY_JS_list;
ZeRenShanQu BenDiZeRenShanQu;
bool Is_ZeRenShanQu_Use = false;
QPoint m_FirCliPos,m_LosCliPos;
// 雷达状态标志
bool SS_ganraoqu1 = false;
bool SS_ganraoqu2 = false;
bool SS_ganraoqu3 = false;
bool SS_jingmoqu1 = false;
bool SS_jingmoqu2 = false;
bool SS_jingmoqu3 = false;
double SS_ganraoqu1_qishi = 0;
double SS_ganraoqu1_jiesu = 0;
double SS_ganraoqu2_qishi = 0;
double SS_ganraoqu2_jiesu = 0;
double SS_ganraoqu3_qishi = 0;
double SS_ganraoqu3_jiesu = 0;
double SS_jingmoqu1_qishi = 0;
double SS_jingmoqu1_jiesu = 0;
double SS_jingmoqu2_qishi = 0;
double SS_jingmoqu2_jiesu = 0;
double SS_jingmoqu3_qishi = 0;
double SS_jingmoqu3_jiesu = 0;
// 火力线角度
double HuoLiXian_MaPan = 90;

// 中心方位
int zhongxinfw = 0;

// 点选禁射相关
bool is_dianxuan_jinshe = false;
double left_dianxuan_jinshe = 0;
double right_dianxuan_jinshe = 0;

// 点选责任扇区相关
bool is_dianxuan_zerenshanqu = false;

// 移动标志
bool m_move = false;

// 距离环相关
uint32_t huan_ju = 5000;
uint8_t huan_ju_wheel = 5;
int huanjianju = 0;

// PPI中心位置
int ppizhongxinx = 380;
int ppizhongxiny = 380;

// 半径
int m_radius = 340;

// 刷新时间
int ui_fresh_time = 50; // 20Hz

void UdpSend(int type, char* data, int size)
{
    qDebug() << "模拟UDP发送: 类型" << type << "数据大小" << size;
    // 在实际系统中这里会发送网络数据
    // 测试环境中只打印日志
}
// 生成测试目标数据


// 重点关注功能
void zhongdian_guanzhu_fun(int pihao)
{
    if (auto mb = rhkq.value(pihao))
    {
        rhkq.modify(pihao, [&](Mubiao& mb) {
            mb.zhongdian = !mb.zhongdian;
            qDebug() << "目标" << pihao << (mb.zhongdian ? "设置为重点关注" : "取消重点关注");
        });
    }
}

void zhongdian_guanzhu_fun(const QVector<int>& pihaoList)
{
    for (int pihao : pihaoList)
    {
        zhongdian_guanzhu_fun(pihao);
    }
}

// 拦截排序
void LanJiePaiXu()
{
    qDebug() << "执行拦截排序逻辑";
    // 在实际系统中这里会进行目标威胁评估和拦截排序
}

// 导引功能
void daoyin_fun(int pihao)
{
    qDebug() << "对目标" << pihao << "执行导引功能";
    // 在实际系统中这里会启动目标导引流程
}

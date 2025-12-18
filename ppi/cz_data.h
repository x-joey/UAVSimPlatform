#ifndef CZ_DATA_H
#define CZ_DATA_H

#include <QVector>
#include <QPointF>
#include "mubiao.h"
// 模拟UDP数据结构
struct UDP_242_MuBiao
{
    double FangWeiJiao = 0;
};

struct UDP_242_BoShuDaoYinXinXi
{
    QVector<UDP_242_MuBiao> MuBiao_List;
};

// 雷达工作状态结构
struct LeiDaGongZuoZhuangTai
{
    int GongZuoMoShiZhuangTaiHuiGao = 0;
    int GenZongLeiDaFaSheGongZuoZhuangTai = 0;
    int QuYuTanCeMoShiHuiGao = 0;
};

// 雷达参数设置结构
struct LeiDaCanShuSheZhi
{
    int MuBiaoPiHao = 0;
    int ZhiPaiSheZhi = 0;
};

// 人工干预信息结构
struct RenGongGanYuXinXi
{
    int LeiDaXiaoPiPiHao = 0;
};

// 微波系统工作状态结构
struct WeiBoXiTongGongZuoZhuangTai
{
    struct
    {
        int WeiBoFaSheZhuangTai = 0;
    } B0;
};

// 全局变量声明
extern LeiDaGongZuoZhuangTai U209_SouSuoLeiDaGongZuoZhuangTai;
extern LeiDaGongZuoZhuangTai U215_GenZongLeiDaGongZuoZhuangTai;
extern LeiDaCanShuSheZhi U212_GenZongLeiDaCanShuSheZhi;
extern LeiDaCanShuSheZhi U206_SouSuoLeiDaCanShuSheZhi;
extern RenGongGanYuXinXi U241_RenGongGanYuXinXi;
extern WeiBoXiTongGongZuoZhuangTai C99_WeiBoXiTongGongZuoZhuangTaiHuiGao;
extern UDP_242_BoShuDaoYinXinXi U242_BoShuDaoYinXinXi;
extern double PI;
// 工具函数
inline double mil_du(double angle) { return angle; }
inline double du_mil(double angle) { return angle; }

// xjw添加
// 模拟数据初始化（在构造函数中添加）
extern bool show_dianji ;
extern LockedHash<Mubiao> rhkq;
extern LockedVector<QVector<QPointF>> dj_point;
extern QVector<JinSheQuYu> JSQY_list;
extern QVector<JinSheQuYu> FX_JS_list;
extern QVector<JinSheQuYu> QY_JS_list;
extern ZeRenShanQu BenDiZeRenShanQu;
extern bool Is_ZeRenShanQu_Use ;
extern QPoint m_FirCliPos,m_LosCliPos;
// 雷达状态标志
extern bool SS_ganraoqu1 ;
extern bool SS_ganraoqu2 ;
extern bool SS_ganraoqu3 ;
extern bool SS_jingmoqu1 ;
extern bool SS_jingmoqu2 ;
extern bool SS_jingmoqu3 ;
extern double SS_ganraoqu1_qishi;
extern double SS_ganraoqu1_jiesu;
extern double SS_ganraoqu2_qishi;
extern double SS_ganraoqu2_jiesu;
extern double SS_ganraoqu3_qishi;
extern double SS_ganraoqu3_jiesu;
extern double SS_jingmoqu1_qishi;
extern double SS_jingmoqu1_jiesu;
extern double SS_jingmoqu2_qishi;
extern double SS_jingmoqu2_jiesu;
extern double SS_jingmoqu3_qishi;
extern double SS_jingmoqu3_jiesu;
// 火力线角度
extern double HuoLiXian_MaPan ;

// 中心方位
extern int zhongxinfw ;

// 点选禁射相关
extern bool is_dianxuan_jinshe ;
extern double left_dianxuan_jinshe ;
extern double right_dianxuan_jinshe ;

// 点选责任扇区相关
extern bool is_dianxuan_zerenshanqu;

// 移动标志
extern bool m_move;

// 距离环相关
extern uint32_t huan_ju;
extern uint8_t huan_ju_wheel;
extern int huanjianju ;

// PPI中心位置
extern int ppizhongxinx;
extern int ppizhongxiny;

// 半径
extern int m_radius;

// 刷新时间
extern int ui_fresh_time; // 20Hz
// UDP发送函数（模拟）
void UdpSend(int type, char* data, int size);
//void generateTestTargets();
void zhongdian_guanzhu_fun(int pihao);
void zhongdian_guanzhu_fun(const QVector<int>& pihaoList);
void LanJiePaiXu();
void daoyin_fun(int pihao);


#endif // CZ_DATA_H

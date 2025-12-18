#ifndef MUBIAO_XINGZHI_H
#define MUBIAO_XINGZHI_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class MuBiao_XingZhi : public QWidget
{
    Q_OBJECT

public:
    explicit MuBiao_XingZhi(QWidget *parent = nullptr) : QWidget(parent), m_pihao(0)
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);

        setStyleSheet("background-color: rgba(50, 50, 50, 220); color: white; border: 2px solid #00FFFF; border-radius: 5px;");

        QVBoxLayout *layout = new QVBoxLayout(this);

        m_titleLabel = new QLabel("目标性质", this);
        m_titleLabel->setStyleSheet("font-weight: bold; color: #00FFFF;");
        m_pihaoLabel = new QLabel("批号: -", this);
        m_infoLabel = new QLabel("类型: 未知目标", this);

        QPushButton *closeBtn = new QPushButton("关闭", this);
        closeBtn->setStyleSheet("QPushButton { background-color: #555555; color: white; border: 1px solid #777777; }"
                              "QPushButton:hover { background-color: #666666; }");

        connect(closeBtn, &QPushButton::clicked, this, &MuBiao_XingZhi::hide);

        layout->addWidget(m_titleLabel);
        layout->addWidget(m_pihaoLabel);
        layout->addWidget(m_infoLabel);
        layout->addWidget(closeBtn);

        resize(150, 120);
    }

    void refresh()
    {
        m_pihaoLabel->setText(QString("批号: %1").arg(m_pihao));
        m_infoLabel->setText(QString("类型: 目标%1").arg(m_pihao % 3 + 1));
    }

    int m_pihao;

private:
    QLabel *m_titleLabel;
    QLabel *m_pihaoLabel;
    QLabel *m_infoLabel;
};

#endif // MUBIAO_XINGZHI_H

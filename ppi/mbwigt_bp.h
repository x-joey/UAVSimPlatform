#ifndef MBWIGT_BP_H
#define MBWIGT_BP_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class MBWigt_BP : public QWidget
{
    Q_OBJECT

public:
    explicit MBWigt_BP(QWidget *parent = nullptr) : QWidget(parent)
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);

        // 设置样式
        setStyleSheet("background-color: rgba(50, 50, 50, 200); color: white; border: 1px solid white; border-radius: 3px;");

        QVBoxLayout *layout = new QVBoxLayout(this);
        m_label = new QLabel(this);
        m_label->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_label);

        resize(80, 40);
    }

    void set_params(int pihao)
    {
        m_label->setText(QString("目标 %1").arg(pihao));
    }

    void hide()
    {
        QWidget::hide();
    }

    void show()
    {
        QWidget::show();
    }

private:
    QLabel *m_label;
};

#endif // MBWIGT_BP_H

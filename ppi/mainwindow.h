#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class CirclePPIWidget;
class QLabel;
class QCheckBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onToggleKuangXuan(bool checked);
    void onShowDianJi(bool checked);
    void onCreateJinshe(double left, double right, double top, double bottom);

private:
    void setupUI();
    void setupConnections();

    CirclePPIWidget *m_ppiWidget;
    QLabel *m_statusLabel;
};

#endif // MAINWINDOW_H

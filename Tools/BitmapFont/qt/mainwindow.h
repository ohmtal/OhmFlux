#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColor>
#include <QFont>
#include <QWidget>
#include <QPainter>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PaintBoxWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PaintBoxWidget(QWidget *parent = nullptr);

    QColor backgroundColor = Qt::magenta;
    int charW = 16;
    int charH = 16;
    bool drawLines = true;
    bool antiAliasing = true;

protected:
    void paintEvent(QPaintEvent *event) override;

public:
    void paintFont(QPainter* myPainter);
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnFont_clicked();
    void on_btnBackGroundColor_clicked();
    void on_btnSaveAs_clicked();
    void on_chkDrawLines_stateChanged(int arg1);
    void on_chkAntiAlasing_stateChanged(int arg1);
    void on_edHeight_editingFinished();
    void on_edWidth_editingFinished();
    void onFontChanged();
    void onSizeChanged();

private:
    Ui::MainWindow *ui;
    PaintBoxWidget *paintBoxWidget;
};
#endif // MAINWINDOW_H

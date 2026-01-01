#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFontDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QPixmap>
#include <QFontMetrics>
#include <QPainterPath>
#include <algorithm>

// PaintBoxWidget implementation
PaintBoxWidget::PaintBoxWidget(QWidget *parent) : QWidget(parent)
{
    // Set a default font for the widget
    QFont defaultFont("Hack Nerd Font", 48);
    setFont(defaultFont);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void PaintBoxWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    paintFont(&painter);
}

void PaintBoxWidget::paintFont(QPainter* myPainter)
{
    myPainter->setRenderHint(QPainter::Antialiasing, antiAliasing);

    myPainter->fillRect(rect(), backgroundColor);

    QPen pen;
    pen.setColor(backgroundColor);
    myPainter->setPen(pen);

    myPainter->setFont(font());
    myPainter->setBrush(QBrush(Qt::white)); // Text color

    QFontMetrics fm(font());
    // Calculate charW and charH based on current font
    charW = 0;
    charH = 0;
    for (int i = 32; i <= 126; ++i)
    {
        charW = std::max(charW, fm.horizontalAdvance(QChar(i)));
        charH = std::max(charH, fm.height());
    }

    int x = 0;
    int y = 0;

    if (drawLines)
    {
        pen.setColor(Qt::white);
        myPainter->setPen(pen);
    }
    else {
        pen.setColor(backgroundColor);
        myPainter->setPen(pen);
    }

    for (int i = 32; i <= 126; ++i)
    {
        QString charStr = QChar(i);
        int w = fm.horizontalAdvance(charStr);
        int addX = std::max(0, (charW - w) / 2);

        myPainter->drawText(x + addX, y + fm.ascent(), charStr);

        if (drawLines)
        {
            myPainter->drawLine(x, y, x, y + charH);
            myPainter->drawLine(x, y, x + charW, y);
        }

        x += charW;

        if (x + charW > width())
        {
            x = 0;
            y += charH;
        }
    }
}

// MainWindow implementation
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    paintBoxWidget = new PaintBoxWidget(this);
    ui->scrollArea->setWidget(paintBoxWidget);
    ui->scrollArea->setWidgetResizable(true);

    // Initial setup from FormCreate
    paintBoxWidget->setFixedSize(512, 512);
    onFontChanged();
    paintBoxWidget->backgroundColor = Qt::magenta;
    ui->btnBackGroundColor->setStyleSheet(QString("background-color: %1").arg(paintBoxWidget->backgroundColor.name()));

    ui->chkDrawLines->setChecked(paintBoxWidget->drawLines);
    ui->chkAntiAlasing->setChecked(paintBoxWidget->antiAliasing);

    connect(ui->edHeight, &QLineEdit::editingFinished, this, &MainWindow::on_edHeight_editingFinished);
    connect(ui->edWidth, &QLineEdit::editingFinished, this, &MainWindow::on_edWidth_editingFinished);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnFont_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, paintBoxWidget->font(), this);
    if (ok) {
        paintBoxWidget->setFont(font);
        onFontChanged();
        paintBoxWidget->update();
    }
}

void MainWindow::on_btnBackGroundColor_clicked()
{
    QColor color = QColorDialog::getColor(paintBoxWidget->backgroundColor, this, "Select Background Color");
    if (color.isValid()) {
        paintBoxWidget->backgroundColor = color;
        ui->btnBackGroundColor->setStyleSheet(QString("background-color: %1").arg(paintBoxWidget->backgroundColor.name()));
        paintBoxWidget->update();
    }
}

void MainWindow::on_btnSaveAs_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Bitmap Font", "", "Bitmap (*.bmp);;PNG (*.png);;JPG (*.jpg)");
    if (!fileName.isEmpty()) {
        QPixmap pixmap(paintBoxWidget->size());
        pixmap.fill(paintBoxWidget->backgroundColor);
        QPainter painter(&pixmap);

        bool savDrawLines = paintBoxWidget->drawLines;
        bool savAntiAliasing = paintBoxWidget->antiAliasing;

        paintBoxWidget->drawLines = false; // Temporarily disable lines for saving
        paintBoxWidget->antiAliasing = false; // Temporarily disable anti-aliasing

        painter.setFont(paintBoxWidget->font());
        paintBoxWidget->paintFont(&painter);

        paintBoxWidget->drawLines = savDrawLines; // Restore lines setting
        paintBoxWidget->antiAliasing = savAntiAliasing; // Restore anti-aliasing
        paintBoxWidget->update();

        bool saved = pixmap.save(fileName);

        paintBoxWidget->drawLines = savDrawLines; // Restore lines setting
        paintBoxWidget->antiAliasing = ui->chkAntiAlasing->isChecked();
        paintBoxWidget->update();

        if (saved) {
            QMessageBox::information(this, "Save Image", "Image saved successfully to: " + fileName);
        } else {
            QMessageBox::warning(this, "Save Image", "Failed to save image to: " + fileName);
        }
    }
}

void MainWindow::on_chkDrawLines_stateChanged(int arg1)
{
    paintBoxWidget->drawLines = (arg1 == Qt::Checked);
    paintBoxWidget->update();
}

void MainWindow::on_chkAntiAlasing_stateChanged(int arg1)
{
    paintBoxWidget->antiAliasing = (arg1 == Qt::Checked);
    paintBoxWidget->update();
}

void MainWindow::on_edHeight_editingFinished()
{
    paintBoxWidget->charH = ui->edHeight->text().toInt();
    onSizeChanged();
}

void MainWindow::on_edWidth_editingFinished()
{
    paintBoxWidget->charW = ui->edWidth->text().toInt();
    onSizeChanged();
}

void MainWindow::onFontChanged()
{
    QFontMetrics fm(paintBoxWidget->font());
    paintBoxWidget->charW = 0;
    paintBoxWidget->charH = 0;
    for (int i = 32; i <= 126; ++i)
    {
        paintBoxWidget->charW = std::max(paintBoxWidget->charW, fm.horizontalAdvance(QChar(i)));
        paintBoxWidget->charH = std::max(paintBoxWidget->charH, fm.height());
    }

    paintBoxWidget->setFixedSize(paintBoxWidget->charW * 10, paintBoxWidget->charH * 10);

    ui->edWidth->setText(QString::number(paintBoxWidget->charW));
    ui->edHeight->setText(QString::number(paintBoxWidget->charH));

    ui->btnFont->setText(paintBoxWidget->font().family() + " " + QString::number(paintBoxWidget->font().pointSize()));

    paintBoxWidget->update();
}

void MainWindow::onSizeChanged()
{
    paintBoxWidget->setFixedSize(paintBoxWidget->charW * 10, paintBoxWidget->charH * 10);
    paintBoxWidget->update();
}

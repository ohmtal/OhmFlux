//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFontDialog>
#include <QFontDatabase>
#include <QColorDialog>
#include <QFileDialog>
#include <QPixmap>
#include <QFontMetrics>
#include <QPainterPath>
#include <algorithm>

static const int DEFAULT_NEW_FONT_SIZE = 36;
//------------------------------------------------------------------------------
PaintBoxWidget::PaintBoxWidget(QWidget *parent) : QWidget(parent)
{
    // Get the OS-specific default monospaced font
    QFont defaultFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    // Set the desired default size and smoothing strategy
    defaultFont.setPointSize(DEFAULT_NEW_FONT_SIZE);
    defaultFont.setStyleStrategy(QFont::PreferAntialias);

    setFont(defaultFont);

    drawLines = true;
    antiAliasing = true;
    backgroundColor = Qt::magenta;
}

//------------------------------------------------------------------------------
void PaintBoxWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    paintFont(&painter);
}
//------------------------------------------------------------------------------
void PaintBoxWidget::paintFont(QPainter* myPainter, bool fillBackground)
{
    myPainter->setRenderHints(/*QPainter::Antialiasing |*/ QPainter::TextAntialiasing, antiAliasing);
    if (fillBackground) {
        myPainter->fillRect(rect(), backgroundColor);
    }

    QFontMetrics fm(font());
    // charW and charH should already be calculated in onFontChanged()
    // or you can recalculate here if they might change dynamically.

    int x = 0;
    int y = 0;

    for (int i = 32; i <= 126; ++i)
    {
        QString charStr = QChar(i);
        int w = fm.horizontalAdvance(charStr); // equivalent to GetTextWidth

        // Center the character within the grid cell (charW)
        int addX = std::max(0, (charW - w) / 2);

        // Draw the Text
        myPainter->setPen(Qt::white);
        // Qt's drawText uses baseline, Delphi's TextOut uses top-left.
        // We use fm.ascent() to align the top of the text with 'y'.
        myPainter->drawText(x + addX, y + fm.ascent(), charStr);

        // Draw the Grid Lines
        if (drawLines)
        {
            myPainter->setPen(Qt::white);
            myPainter->drawLine(x, y, x, y + charH);         // Vertical line
            myPainter->drawLine(x, y, x + charW, y);         // Horizontal line
        }

        // Increment X and apply Wrapping Logic
        x += charW;

        // Check if the NEXT character would exceed the widget width
        if (x + charW > width())
        {
            x = 0;          // Reset to left margin
            y += charH;     // Move to next row
        }
    }
}
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
//------------------------------------------------------------------------------
void MainWindow::on_btnFont_clicked()
{
    bool ok;

    QFont initialFont = paintBoxWidget->font();
    // initialFont.setPointSize(DEFAULT_NEW_FONT_SIZE);
    initialFont.setStyleStrategy(QFont::PreferAntialias);

    QFont selectedFont = QFontDialog::getFont(&ok, initialFont, this);

    if (ok) {
        selectedFont.setStyleStrategy(QFont::PreferAntialias);

        paintBoxWidget->setFont(selectedFont);
        onFontChanged();
        paintBoxWidget->update();
    }
}
//------------------------------------------------------------------------------
void MainWindow::on_btnBackGroundColor_clicked()
{
    QColor color = QColorDialog::getColor(paintBoxWidget->backgroundColor, this, "Select Background Color");
    if (color.isValid()) {
        paintBoxWidget->backgroundColor = color;
        ui->btnBackGroundColor->setStyleSheet(QString("background-color: %1").arg(paintBoxWidget->backgroundColor.name()));
        paintBoxWidget->update();
    }
}
//------------------------------------------------------------------------------

void MainWindow::on_btnSaveAs_clicked() {
    QFileDialog dialog(this, "Save Bitmap Font");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilters({"PNG (*.png)", "Bitmap (*.bmp)", "JPG (*.jpg)"});
    dialog.setDefaultSuffix("png");

    if (dialog.exec()) {
        QString fileName = dialog.selectedFiles().first();

        // Ensure extension is appended (crucial for 2026 cross-platform support)
        if (QFileInfo(fileName).suffix().isEmpty()) {
            if (dialog.selectedNameFilter().contains("*.png")) fileName += ".png";
            else if (dialog.selectedNameFilter().contains("*.bmp")) fileName += ".bmp";
            else if (dialog.selectedNameFilter().contains("*.jpg")) fileName += ".jpg";
        }

        if (!fileName.isEmpty()) {
            QPixmap pixmap(paintBoxWidget->size());
            bool isPng = fileName.endsWith(".png", Qt::CaseInsensitive);

            if (isPng) {
                // Initialize pixmap with full transparency for PNGs
                pixmap.fill(Qt::transparent);
            } else {
                pixmap.fill(paintBoxWidget->backgroundColor);
            }
            {
                QPainter painter(&pixmap);
                painter.setFont(paintBoxWidget->font());
                painter.setPen(Qt::white);
                painter.setRenderHint(QPainter::TextAntialiasing, paintBoxWidget->antiAliasing);
                bool savDrawLines = paintBoxWidget->drawLines;
                paintBoxWidget->drawLines = false;
                paintBoxWidget->paintFont(&painter, !isPng);
                paintBoxWidget->drawLines = savDrawLines;
            }
            if (pixmap.save(fileName/*, isPng ? "PNG" : nullptr*/)) {
                // Success
            } else {
                QMessageBox::warning(this, "Save Error", "Could not save file.");
            }
        }
    }
}
/*
void MainWindow::on_btnSaveAs_clicked()
{
    QFileDialog dialog(this, "Save Bitmap Font");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilters({"Bitmap (*.bmp)", "PNG (*.png)"});
    dialog.setDefaultSuffix("bmp");

    if (dialog.exec()) {
        QString fileName = dialog.selectedFiles().first();

        // Handle automatic extension
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().isEmpty()) {
            QString selectedFilter = dialog.selectedNameFilter();
            if (selectedFilter.contains("*.png")) fileName += ".png";
            else if (selectedFilter.contains("*.bmp")) fileName += ".bmp";
        }

        if (!fileName.isEmpty()) {
            // Prepare the widget for a "clean" render
            bool savDrawLines = paintBoxWidget->drawLines;
            paintBoxWidget->drawLines = false;

            // Create the pixmap and render
            QPixmap pixmap(paintBoxWidget->size());
            bool isPng = fileName.endsWith(".png", Qt::CaseInsensitive);

            if (isPng) {
                // Initialize pixmap with full transparency for PNGs
                pixmap.fill(Qt::transparent);
            } else {
                pixmap.fill(paintBoxWidget->backgroundColor);
            }

            QPainter painter(&pixmap);
            paintBoxWidget->paintFont(&painter, !isPng);
            painter.end();

            // Since we aren't calling paintFont manually here,
            // render() will trigger the widget's paintEvent.
            // paintBoxWidget->render(&pixmap);

            if (pixmap.save(fileName)) {
                // Optional: success feedback
            } else {
                QMessageBox::warning(this, "Save Error", "Could not save file to: " + fileName);
            }

            // Restore the UI state
            paintBoxWidget->drawLines = savDrawLines;
            paintBoxWidget->update(); // Refresh screen to show lines again
        }
    }
}*/
//------------------------------------------------------------------------------
void MainWindow::on_chkDrawLines_stateChanged(int arg1)
{
    paintBoxWidget->drawLines = (arg1 == Qt::Checked);
    paintBoxWidget->update();
}
//------------------------------------------------------------------------------
void MainWindow::on_chkAntiAlasing_stateChanged(int arg1)
{
    paintBoxWidget->antiAliasing = (arg1 == Qt::Checked);
    paintBoxWidget->update();
}
//------------------------------------------------------------------------------
void MainWindow::on_edHeight_editingFinished()
{
    paintBoxWidget->charH = ui->edHeight->text().toInt();
    onSizeChanged();
}
//------------------------------------------------------------------------------
void MainWindow::on_edWidth_editingFinished()
{
    paintBoxWidget->charW = ui->edWidth->text().toInt();
    onSizeChanged();
}
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
void MainWindow::onSizeChanged()
{
    paintBoxWidget->setFixedSize(paintBoxWidget->charW * 10, paintBoxWidget->charH * 10);
    paintBoxWidget->update();
}
//------------------------------------------------------------------------------

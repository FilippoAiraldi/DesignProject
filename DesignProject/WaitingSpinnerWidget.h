#pragma once

#include <QWidget>
#include <QTimer>
#include <QColor>

class WaitingSpinnerWidget : public QWidget
{
    Q_OBJECT

public:
    WaitingSpinnerWidget(QWidget* parent = 0, bool centerOnParent = true, bool disableParentWhenSpinning = true);
    WaitingSpinnerWidget(Qt::WindowModality modality, QWidget* parent = 0, bool centerOnParent = true, bool disableParentWhenSpinning = true);

public slots:
    void start();
    void stop();

public:
    void setColor(QColor color);
    void setRoundness(qreal roundness);
    void setMinimumTrailOpacity(qreal minimumTrailOpacity);
    void setTrailFadePercentage(qreal trail);
    void setRevolutionsPerSecond(qreal revolutionsPerSecond);
    void setNumberOfLines(int lines);
    void setLineLength(int length);
    void setLineWidth(int width);
    void setInnerRadius(int radius);
    void setText(QString text);

    QColor color() { return _color; }
    qreal roundness() { return _roundness; }
    qreal minimumTrailOpacity() { return _minimumTrailOpacity; }
    qreal trailFadePercentage() { return _trailFadePercentage; }
    qreal revolutionsPersSecond() { return _revolutionsPerSecond; }
    int numberOfLines() { return _numberOfLines; }
    int lineLength() { return _lineLength; }
    int lineWidth() { return _lineWidth; }
    int innerRadius() { return _innerRadius; }
    bool isSpinning() const { return _isSpinning; }

private slots:
    void rotate();

protected:
    void paintEvent(QPaintEvent* paintEvent);

private:
    static int lineCountDistanceFromPrimary(int current, int primary, int totalNrOfLines);
    static QColor currentLineColor(int distance, int totalNrOfLines, qreal trailFadePerc, qreal minOpacity, QColor color);

    void initialize();
    void updateSize();
    void updateTimer();
    void updatePosition();

private:
    QColor  _color;
    qreal   _roundness; // 0..100
    qreal   _minimumTrailOpacity;
    qreal   _trailFadePercentage;
    qreal   _revolutionsPerSecond;
    int     _numberOfLines;
    int     _lineLength;
    int     _lineWidth;
    int     _innerRadius;

private:
    WaitingSpinnerWidget(const WaitingSpinnerWidget&);
    WaitingSpinnerWidget& operator=(const WaitingSpinnerWidget&);

    QTimer* _timer;
    bool    _centerOnParent;
    bool    _disableParentWhenSpinning;
    int     _currentCounter;
    bool    _isSpinning;
};
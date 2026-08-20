#pragma once
#include <QDoubleSpinBox>
#include <QLabel>
#include <QVariant>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QtMath>

class PlateGridPreview : public QLabel {
public:
    PlateGridPreview(QWidget* parent = nullptr) : QLabel(parent) {
        setMinimumSize(260, 260);
        setProperty("class", "bedPreview");
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setBedParams(double sx, double sy, double ox, double oy) {
        m_sizeX = sx; m_sizeY = sy;
        m_originX = ox; m_originY = oy;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // White background
        p.fillRect(rect(), Qt::white);

        if (m_sizeX <= 0 || m_sizeY <= 0) return;

        // Grid: each small cell = 10mm in real units
        const double cellMM = 10.0;

        // Scale: fit the bed rect inside the widget with padding
        double availW = width() - 60;
        double availH = height() - 60;
        double sc = qMin(availW / m_sizeX, availH / m_sizeY);

        double bw = m_sizeX * sc;  // bed pixel width
        double bh = m_sizeY * sc;  // bed pixel height
        double bx = (width() - bw) / 2.0;
        double by = (height() - bh) / 2.0;

        // Pixel size of a 10mm cell
        double cellPx = cellMM * sc;

        // ── Draw grid ──
        // Grid lines covering the bed area
        if (cellPx >= 4.0) {  // only draw grid if cells are visible
            p.setPen(QPen(QColor("#E0E0E0"), 1));
            for (double tx = bx; tx <= bx + bw + 0.5; tx += cellPx) {
                p.drawLine(QPointF(tx, by), QPointF(tx, by + bh));
            }
            for (double ty = by; ty <= by + bh + 0.5; ty += cellPx) {
                p.drawLine(QPointF(bx, ty), QPointF(bx + bw, ty));
            }
        }

        // ── Draw bed boundary ──
        p.setPen(QPen(QColor("#000000"), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRect(QRectF(bx, by, bw, bh));

        // ── Draw origin mark ──
        // origin (ox, oy) in mm → pixel position
        // X: right, Y: up from bottom-left corner of bed
        double opx = bx + m_originX * sc;
        double opy = by + bh - m_originY * sc;

        // Only draw if origin is within bed bounds (roughly)
        if (opx >= bx - 20 && opx <= bx + bw + 20 && opy >= by - 20 && opy <= by + bh + 20) {
            // Red dot at origin
            p.setPen(Qt::NoPen);
            p.setBrush(QColor("#000000"));
            p.drawEllipse(QPointF(opx, opy), 4, 4);

            // X arrow (right)
            p.setPen(QPen(QColor("#FF4444"), 2));
            p.setBrush(Qt::NoBrush);
            double arrowLen = cellPx * 3;
            p.drawLine(QPointF(opx, opy), QPointF(opx + arrowLen, opy));
            // arrowhead
            double ah = 5;
            p.drawLine(QPointF(opx + arrowLen, opy),
                QPointF(opx + arrowLen - ah, opy - ah));
            p.drawLine(QPointF(opx + arrowLen, opy),
                QPointF(opx + arrowLen - ah, opy + ah));

            // Y arrow (up, green)
            p.setPen(QPen(QColor("#00AE42"), 2));
            p.drawLine(QPointF(opx, opy), QPointF(opx, opy - arrowLen));
            p.drawLine(QPointF(opx, opy - arrowLen),
                QPointF(opx - ah, opy - arrowLen + ah));
            p.drawLine(QPointF(opx, opy - arrowLen),
                QPointF(opx + ah, opy - arrowLen + ah));

            // Axis labels
            QFont f = p.font();
            f.setPointSize(8);
            f.setBold(true);
            p.setFont(f);
            p.setPen(QColor("#FF4444"));
            p.drawText(QPointF(opx + arrowLen + 2, opy + 4), "X");
            p.setPen(QColor("#00AE42"));
            p.drawText(QPointF(opx - 6, opy - arrowLen - 4), "Y");
        }

        // ── Dimension label at bottom ──
        p.setPen(QColor("#6B6B6B"));
        QFont df = p.font();
        df.setPointSize(9);
        df.setBold(false);
        p.setFont(df);
        QString dimText = QString("%1 mm × %2 mm").arg(m_sizeX, 0, 'f', 0).arg(m_sizeY, 0, 'f', 0);
        p.drawText(QRectF(0, by + bh + 4, width(), 18), Qt::AlignHCenter, dimText);
    }

private:
    double m_sizeX = 500, m_sizeY = 500;
    double m_originX = 50, m_originY = 50;
};

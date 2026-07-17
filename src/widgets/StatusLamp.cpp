#include "widgets/StatusLamp.h"
#include <QPainter>
#include <QRadialGradient>
#include <QTimer>

//=============================================================================
// Public Methods
//=============================================================================

StatusLamp::StatusLamp(QWidget* parent)
	: QWidget(parent), m_timer(new QTimer(this)) {
	connect(m_timer, &QTimer::timeout,
        this, &StatusLamp::toggle);

	m_timer->start(blinkIntervalMs);
}

//=============================================================================
// Private Slots
//=============================================================================

void StatusLamp::toggle() {
	m_state = !m_state;
	update();
}

//=============================================================================
// Protected Methods
//=============================================================================

void StatusLamp::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    
    painter.setRenderHint(QPainter::Antialiasing);
    int size = qMin(width(), height()) - circleMargin;
    int x = (width() - size) / 2;
    int y = (height() - size) / 2;

    QRect rect(x, y, size, size);

    // グラデーション作成
    QRadialGradient gradient(rect.center(), size / 2);

    if (m_state)
    {
        // ON（青）
        gradient.setColorAt(0.0, QColor(255, 255, 255)); // 中心
        gradient.setColorAt(0.3, QColor(120, 200, 255));
        gradient.setColorAt(1.0, QColor(0, 80, 255));    // 外側
    }
    else
    {
        // OFF（グレー）
        gradient.setColorAt(0.0, QColor(240, 240, 240));
        gradient.setColorAt(0.3, QColor(180, 180, 180));
        gradient.setColorAt(1.0, QColor(100, 100, 100));
    }

    painter.setBrush(gradient);

    // 外枠
    painter.setPen(QPen(Qt::black, 1));

    painter.drawEllipse(rect);
}
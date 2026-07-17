#pragma once

#include <QWidget>
#include <QTimer>

class StatusLamp :public QWidget
{
	Q_OBJECT
public:
	explicit StatusLamp(QWidget* parent = nullptr);

protected:
	void paintEvent(QPaintEvent* event) override;

private slots:
	void toggle();

private:
	static constexpr int blinkIntervalMs = 500;
	static constexpr int circleMargin = 4;
	bool m_state = false;
	QTimer* m_timer = nullptr;
};
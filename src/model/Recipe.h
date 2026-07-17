#pragma once
#include <QString>
#include <QJsonObject>

class Recipe {
public:
	//Data
	QString id;
	QString recipeName;
	QString partNumber;
	QString waferType;
	int lineCount;
	double upperLimit;
	double lowerLimit;
	QString comment;

	//Methods
	bool isEmpty() const;
	QJsonObject toJson() const;
	static Recipe fromJson(const QJsonObject& object);
};

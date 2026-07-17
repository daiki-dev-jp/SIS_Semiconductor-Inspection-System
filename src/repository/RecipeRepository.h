#pragma once
#include "model/Recipe.h"

class RecipeRepository {
public:
	// CRUD
	QList<Recipe> loadAll();
	bool save(const Recipe& recipe);
	bool update(const Recipe& recipe);
	bool remove(const QString& recipeId);

private:
	// File
	QString recipeFilePath() const;

	// JSON
	QJsonArray loadJsonArray(const QString& filePath);
	bool saveJsonArray(const QString& filePath, const QJsonArray& array);

	// Search
	int findRecipeIndex(const QJsonArray& array, const QString& recipeId) const;
};
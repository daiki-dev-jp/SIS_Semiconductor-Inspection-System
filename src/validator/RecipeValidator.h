#pragma once

#include <QString>

#include "model/Recipe.h"

class RecipeValidator {
public:
	static bool validate(
		const Recipe& recipe,
		const QList<Recipe>& existingRecipes,
	    const Recipe& originalRecipe,
		QString& errorMessage
	);
};
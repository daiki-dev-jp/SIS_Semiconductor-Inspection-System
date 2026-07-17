#include "model/Recipe.h"

//=============================================================================
// Public Methods
//=============================================================================


//RecipeをJSONに変換する
QJsonObject Recipe::toJson() const {
	QJsonObject object;
    object["id"] = id;
    object["recipeName"] = recipeName;
    object["partNumber"] = partNumber;
    object["waferType"] = waferType;
    object["lineCount"] = lineCount;
    object["upperLimit"] = upperLimit;
    object["lowerLimit"] = lowerLimit;
    object["comment"] = comment;
    
    return object;
}

//JSONからRecipeを生成する
Recipe Recipe::fromJson(const QJsonObject& object)
{
    Recipe recipe;

    recipe.id = object["id"].toString();
    recipe.recipeName = object["recipeName"].toString();
    recipe.partNumber = object["partNumber"].toString();
    recipe.lineCount = object["lineCount"].toInt();
    recipe.upperLimit = object["upperLimit"].toDouble();
    recipe.lowerLimit = object["lowerLimit"].toDouble();
    recipe.comment = object["comment"].toString();
    recipe.waferType = object["waferType"].toString();

    return recipe;
}

//レシピが未登録かどうかを判定する
bool Recipe::isEmpty() const {
    return id.isEmpty();
}
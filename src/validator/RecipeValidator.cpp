#include <QString>
#include <QRegularExpression>

#include "validator/RecipeValidator.h"

bool RecipeValidator::validate(
    const Recipe& recipe,
    const QList<Recipe>& existingRecipes,
    const Recipe& originalRecipe,
    QString& errorMessage) {

    //必須チェック
    if (recipe.recipeName.trimmed().isEmpty()) {
        errorMessage = "レシピ名は必須項目です。";
        return false;
    }

    if (recipe.partNumber.trimmed().isEmpty()) {
        errorMessage = "品番は必須項目です。";
        return false;
    }

    //重複チェック
    for (const Recipe& recipeTemp : existingRecipes) {
        //名前が違うなら次へ
        if (recipeTemp.recipeName.trimmed() != recipe.recipeName.trimmed()) {
            continue;
        }

        //編集中の自分自身なら重複ではない
        if (!originalRecipe.isEmpty() &&
            recipeTemp.id == originalRecipe.id) {
            continue;
        }
        errorMessage = "同じレシピ名が既に存在します。";

        return false;
    }

    //上下限チェック
    if (recipe.upperLimit < recipe.lowerLimit) {
        errorMessage = "上限値は下限値以上にしてください。";
        return false;
    }

    return true;
}

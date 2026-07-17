#include "repository/RecipeRepository.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>

//=============================================================================
// Public Methods
//=============================================================================

//レシピ情報を読み込む
QList<Recipe> RecipeRepository::loadAll() {
	QString filePath = recipeFilePath();

	QJsonArray array = loadJsonArray(filePath);

	QList<Recipe> recipes;

	for (const QJsonValue& value : array) {
		if (value.isObject()) {
			recipes.append(Recipe::fromJson(value.toObject()));
		}
	}

	return recipes;
}

//レシピを追加する
bool RecipeRepository::save(const Recipe& recipe) {
	QString filePath = recipeFilePath();

	QJsonArray array = loadJsonArray(filePath);

	array.append(recipe.toJson());

	return saveJsonArray(filePath, array);
}

//レシピを更新する
bool RecipeRepository::update(const Recipe& recipe) {
	QString filePath = recipeFilePath();

	QJsonArray array = loadJsonArray(filePath);

	int index = findRecipeIndex(array, recipe.id);
	if (index != -1) {
		array[index] = recipe.toJson();
	}

	return saveJsonArray(filePath, array);
}

//レシピを削除する
bool RecipeRepository::remove(const QString& recipeId) {
	QString filePath = recipeFilePath();

	QJsonArray array = loadJsonArray(filePath);

	int index = findRecipeIndex(array, recipeId);
	if (index != -1) {
		array.removeAt(index);
	}

	return saveJsonArray(filePath, array);
}

//=============================================================================
// Private Methods
//=============================================================================

//レシピファイルのパスを取得する
QString RecipeRepository::recipeFilePath() const {
	QString dataPath = QCoreApplication::applicationDirPath() + "/data";

	QDir dir;
	if (!dir.exists(dataPath)) {
		dir.mkpath(dataPath);
	}

	return dataPath + "/recipes.json";
}

//JSONファイルを読み込む
QJsonArray RecipeRepository::loadJsonArray(const QString& filePath) {
	QJsonArray array;

	QFile readFile(filePath);

	if (readFile.exists()) {
		if (readFile.open(QIODevice::ReadOnly)) {
			QByteArray jsonData = readFile.readAll();
			QJsonDocument document = QJsonDocument::fromJson(jsonData);

			if (document.isArray()) {
				array = document.array();
			}
			readFile.close();
		}
	}
	return array;
}

//JSONファイルを書き込む
bool RecipeRepository::saveJsonArray(const QString& filePath,const QJsonArray& array) {
	QFile writeFile(filePath);

	if (!writeFile.open(QIODevice::WriteOnly)) {
		return false;
	}

	QJsonDocument document(array);
	
	writeFile.write(document.toJson(QJsonDocument::Indented));

	writeFile.close();

	return true;
}

//レシピIDからインデックスを検索する
int RecipeRepository::findRecipeIndex(const QJsonArray& array, const QString& recipeId) const {
	for (int i = 0; i < array.size(); ++i)
	{
		if (!array[i].isObject())
		{
			continue;
		}

		QJsonObject object = array[i].toObject();

		if (object["id"].toString() == recipeId) {
			return i;
		}
	}
	return -1;
}
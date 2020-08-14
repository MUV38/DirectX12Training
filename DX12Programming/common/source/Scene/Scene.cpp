#include "Scene/Scene.h"

Scene::Scene(Application* application)
	: mApplication(application)
{
}

Scene::~Scene()
{
}

// 初期化
bool Scene::initialize()
{
	return true;
}

// 更新
void Scene::update(float deletaTime)
{
}

// 描画
void Scene::draw()
{
}

// アプリケーション
Application* Scene::getApplication() const
{
	return mApplication;
}

// シーンコンテキスト
const SceneContext& Scene::getSceneContext() const
{
	return mSceneContext;
}

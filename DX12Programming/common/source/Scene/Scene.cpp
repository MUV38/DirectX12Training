#include "Scene/Scene.h"

Scene::Scene(Application* application)
	: mApplication(application)
{
}

Scene::~Scene()
{
}

// ������
bool Scene::initialize()
{
	return true;
}

// �X�V
void Scene::update(float deletaTime)
{
}

// �`��
void Scene::draw()
{
}

// �A�v���P�[�V����
Application* Scene::getApplication() const
{
	return mApplication;
}

// �V�[���R���e�L�X�g
const SceneContext& Scene::getSceneContext() const
{
	return mSceneContext;
}

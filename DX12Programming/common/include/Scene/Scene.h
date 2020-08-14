#pragma once

#include "Application/Application.h"
#include "Scene/SceneContext.h"

//! @brief �V�[��
class Scene
{
public:
	Scene(Application* application);
	virtual ~Scene();

protected:
	//! @brief ������
	virtual bool initialize();

	//! @brief �X�V
	virtual void update(float deletaTime);

	//! @brif �`��
	virtual void draw();

	//! @brief �A�v��
	Application* getApplication() const;

	//! @brief �V�[���R���e�L�X�g
	const SceneContext& getSceneContext() const;

private:
	Application* mApplication; //!< �A�v���P�[�V����
	SceneContext mSceneContext; //!< �V�[���R���e�L�X�g
};
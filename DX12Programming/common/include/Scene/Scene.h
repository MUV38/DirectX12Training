#pragma once

#include "Application/Application.h"
#include "Scene/SceneContext.h"

//! @brief シーン
class Scene
{
public:
	Scene(Application* application);
	virtual ~Scene();

protected:
	//! @brief 初期化
	virtual bool initialize();

	//! @brief 更新
	virtual void update(float deletaTime);

	//! @brif 描画
	virtual void draw();

	//! @brief アプリ
	Application* getApplication() const;

	//! @brief シーンコンテキスト
	const SceneContext& getSceneContext() const;

private:
	Application* mApplication; //!< アプリケーション
	SceneContext mSceneContext; //!< シーンコンテキスト
};
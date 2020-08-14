#pragma once
#include "Hazel.h"
namespace Hazel
{

	class CameraController : public ScriptableEntity
	{
	public:
		void OnCreate()
		{
		}
		void OnDestroy()
		{

		}
		void OnUpdate(Timestep ts)
		{
			//auto& transform = GetComponent<TransformComponent>().Transform;
			auto& transform = GetComponent<TransformComponent>().Transform;
			float speed = 5.0f;

			if (Input::IsKeyPressed(KeyCode::A))
				transform[3][0] -= speed * ts;
			if (Input::IsKeyPressed(KeyCode::D))
				transform[3][0] += speed * ts;
			if (Input::IsKeyPressed(KeyCode::W))
				transform[3][1] += speed * ts;
			if (Input::IsKeyPressed(KeyCode::S))
				transform[3][1] -= speed * ts;
		}
	};

}
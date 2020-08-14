#pragma once
#include "Entity.h"

namespace Hazel {

	class ScriptableEntity
	{
	public:
		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

		template<typename T>
		bool HasCompeonnt()
		{
			return m_Entity.HasComponent<T>();
		}
	private:
		Entity m_Entity;
		friend class Scene;
	};

}

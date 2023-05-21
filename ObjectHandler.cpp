#include "ObjectHandler.h"

void ObjectHandler::Instantiate(GameObject& gameObject)
{
	m_gameObjects.push_back(&gameObject);
}

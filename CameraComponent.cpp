#include "CameraComponent.h"
#include "GameObject.h"
#include "iRender.h"
#include "Editor.h"

CameraComponent::CameraComponent(GameObject* owner)
	: Component(owner)
{
}

void CameraComponent::Render()
{
#ifdef EDITOR
	if (Editor::GetInstance()->IsStarted() == false)
	{
		Render::DrawLine(m_owner->GetWorldMatrix().GetPosition(), m_owner->GetWorldMatrix().GetPosition() - m_owner->GetWorldMatrix().GetFront() * 2);
	}
#endif // Editor
}

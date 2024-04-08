#pragma once
#include "pch.h"

#ifdef EDITOR
#include "Editor.h"
#include <string>
#endif // EDITOR
static void LogMessage(std::string message)
{
#ifdef EDITOR
	Editor::GetInstance()->LogMessage(message);
#endif // EDITOR
}
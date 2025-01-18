#pragma once
#include "pch.h"

class Undo
{
	public:
		virtual ~Undo() = 0;
		virtual void Execute() = 0;
};
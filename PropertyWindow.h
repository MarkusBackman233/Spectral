#pragma once
#include <memory>
#include <functional>
class PropertyWindow
{
public:
	PropertyWindow();
	void Update();

protected:
	void CloseThisWindow();
	virtual void PopulateWindow();
private:
	bool m_windowOpen;

};


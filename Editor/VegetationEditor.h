#pragma once


class VegetationComponent;
class Mesh;

class VegetationEditor
{
public:
	VegetationEditor(VegetationComponent* vegetationComponent);


	bool Update(); // returns false if user wants to close the editor

private:


	VegetationComponent* m_vegetationComponent;
};


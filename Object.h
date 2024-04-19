#pragma once

#include "Calculator.h"


class Object {
public:
	Object() = default;

	Object(const Parser& parser);


	void Print(std::ofstream& output) const;

private:
	std::vector<MyData::DataObject> data;
};
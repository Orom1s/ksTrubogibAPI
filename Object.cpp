#include "stdafx.h"
#include "Object.h"



Object::Object(const Parser& parser) {
	CalcData calculator(parser);
	auto points = calculator.GetPoints();
	auto angles = calculator.GetAngles();
	auto lengths = calculator.GetLengths();
	auto radiuses = calculator.GetRadiuses();
	for (int i = 0; i < points.size(); ++i) {
		MyData::DataObject data;
		data.point_ = points[i];
		data.angle = angles[i];
		data.length = lengths[i];
		data.r = radiuses[i];
	}
}


void Object::Print(std::ofstream& output) const {
	double length = 0.0;
	for (int i = data.size(); i > 0; --i) {
		length += data[i].length;
		if (data[i].angle > 0) {
			output << "move" << length << "/n";
			output << "bending" << data[i].angle * 180 / PI << "/n";
			length = 0.0;
		}
	}
}

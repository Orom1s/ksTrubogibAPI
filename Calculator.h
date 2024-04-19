#pragma once

#include "common.h"
#include "Parser.h"



#define PI 3.14159265



class CalcData
{
public:
	using Radius = double;

	CalcData() = default;

	CalcData(const Parser& parser);

	void SetPoints(const std::vector<MyData::Point>& points);

	void SetPoint(MyData::Point point);

	void SetRadiusByPos(size_t pos, Radius r);

	void SetRadius(Radius r);

	void CalcAngles();

	void CalcLengths();

	double CalcArcLengthByVec(MyData::Point A, MyData::Point B, Radius r);

	double CalcArcLengthByAngle(double angle, Radius r);

	const std::vector<MyData::Point>& GetPoints() const;

	const std::vector<double>& GetAngles() const;

	const std::vector<double>& GetLengths() const;

	const std::vector<Radius>& GetRadiuses() const;

private:
	std::vector<MyData::Point> points_;
	std::vector<double> angles_;
	std::vector<double> lengths_;
	std::vector<Radius> r_{};

	double CalcAngle(MyData::Point p1, MyData::Point p2);

	MyData::Point CalcVector(MyData::Point A, MyData::Point B);

	double CalcVecLength(MyData::Point A, MyData::Point B);
};

#include "stdafx.h"
#include "Calculator.h"


CalcData::CalcData(const Parser& parser) {
	const auto& data = parser.GetData();
	for (auto [point, r] : data) {
		SetPoint(point);
		SetRadius(r);
	}
	CalcAngles();
	CalcLengths();

}

void CalcData::SetPoints(const std::vector<MyData::Point>& points) {
	points_ = points;
}

void CalcData::SetPoint(MyData::Point point) {
	points_.push_back(point);
}

void CalcData::SetRadiusByPos(size_t pos, Radius r) {
	r_[pos] = r;
}

void CalcData::SetRadius(Radius r) {
	r_.push_back(r);
}

//вычисляет углы для каждой точки, кроме первой и последней
void CalcData::CalcAngles() {
	if (points_.size() < 3) {
		throw std::invalid_argument("Low points");
	}
	angles_.clear();
	angles_.push_back(0);
	for (int i = 1; i < (points_.size() - 1); ++i) {
		MyData::Point vectorAB = CalcVector(points_[i - 1], points_[i]);
		MyData::Point vectorAC = CalcVector(points_[i], points_[i + 1]);
		auto anglA = CalcAngle(vectorAB, vectorAC);
		angles_.push_back(anglA);
	}
	angles_.push_back(0);
}

//вычисляет расстояние между всеми точками, так жепри наличии угла считает длину дуги
void CalcData::CalcLengths() {
	lengths_.push_back(0);
	for (int i = 0; i < (points_.size() - 1); ++i) {
		if (i == 0 || angles_[i] == angles_[i - 1]) {

			auto length = CalcVecLength(points_[i], points_[i + 1]);
			lengths_.push_back(length);
			continue;
		}
		if (angles_[i] > 0 && angles_[i+1] > 0 && angles_[i] != angles_[i+1]) {
			auto arc_length = CalcArcLengthByAngle(angles_[i], r_[i]);
			lengths_.push_back(arc_length);
		}
	}
}

//вычисляет длину дуги по двум точкам и радиусу
double CalcData::CalcArcLengthByVec(MyData::Point A, MyData::Point B, Radius r) {
	//расстояние между точками (основание дуги)
	double AB_length = CalcVecLength(A, B);
	//угол дуги в радианах
	double alpha = 2 * asin(AB_length / (2 * r));
	//длина дуги
	auto arc_length = alpha * r;
	return arc_length;
}

double CalcData::CalcArcLengthByAngle(double angle, Radius r) {
	//угол между радиусом и хордой равен вычитанию из угла между касательной и хордой угла в 90
	double rad_angle = (angle - 90.0) * (PI / 180);
	//вычисляется значение хорды на которой стоит дуга
	double horde = 2 * (r * acos(rad_angle));
	double arc_length = horde * r;
	return arc_length;
}

const std::vector<MyData::Point>& CalcData::GetPoints() const {
	return points_;
}

const std::vector<double>& CalcData::GetAngles() const {
	return angles_;
}

const std::vector<double>& CalcData::GetLengths() const {
	return lengths_;
}

const std::vector<CalcData::Radius>& CalcData::GetRadiuses() const {
	return r_;
}

//вычисляет угол в радинах между двумя векторами
double CalcData::CalcAngle(MyData::Point p1, MyData::Point p2)
{
	double numerator = p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
	double denominator1 = sqrt(std::pow(p1.x, 2) + std::pow(p1.y, 2) + std::pow(p1.z, 2));
	double denominator2 = sqrt(std::pow(p2.x, 2) + std::pow(p2.y, 2) + std::pow(p2.z, 2));
	double arg = numerator / (denominator1 * denominator2);
	return acos(arg);
}

//вычисляет вектор
MyData::Point CalcData::CalcVector(MyData::Point A, MyData::Point B) {
	return { B.x - A.x, B.y - A.y, B.z - A.z };
}

//вычисляет длину вектора
double CalcData::CalcVecLength(MyData::Point A, MyData::Point B) {
	MyData::Point vectorAB = CalcVector(A, B);
	auto length = sqrt(std::pow(vectorAB.x, 2) + std::pow(vectorAB.y, 2) + std::pow(vectorAB.z, 2));
	return length;
}



#pragma once

#include "common.h"

class Parser {
public:
	Parser() = default;

	Parser(std::istream& input) {
		std::string line;
		while (std::getline(input, line)) {
			auto data = SplitIntoWords(line);
			if (data[0] == "X") {
				FillPos(data);
				continue;
			}
			FillData(data);
		}
	}

	void FillData(std::vector<std::string> data);

	void FillPos(std::vector<std::string> data);

	const std::vector<std::pair<MyData::Point, double>>& GetData() const;
private:

	struct Pos {
		int x, y, z, r;
	};

	Pos pos{};
	std::vector<std::pair<MyData::Point, double>> data_;

	size_t FindPosWord(const std::vector<std::string>& data, const std::string& word);

	std::vector<std::string> SplitIntoWords(const std::string& text);
};
#include "stdafx.h"
#include "Parser.h"



void Parser::FillData(std::vector<std::string> data) {
	MyData::Point point{ std::stod(data[pos.x]), std::stod(data[pos.y]), std::stod(data[pos.z]) };
	double r = std::stod(data[pos.r]);
	data_.push_back(std::make_pair(point, r));
}

void Parser::FillPos(std::vector<std::string> data) {
	pos.x = FindPosWord(data, "X");
	pos.y = FindPosWord(data, "Y");
	pos.z = FindPosWord(data, "Z");
	pos.r = FindPosWord(data, "R");
}

const std::vector<std::pair<MyData::Point, double>>& Parser::GetData() const {
	return data_;
}

size_t Parser::FindPosWord(const std::vector<std::string>& data, const std::string& word) {
	auto it = std::find(data.begin(), data.end(), word);
	auto pos = std::distance(data.begin(), it);
	return pos;
}

std::vector<std::string> Parser::SplitIntoWords(const std::string& text) {
	std::vector<std::string> words;
	std::string word;
	for (const char c : text) {
		if (c == ' ') {
			if (!word.empty()) {
				words.push_back(word);
				word.clear();
			}
		}
		else {
			word += c;
		}
	}
	if (!word.empty()) {
		words.push_back(word);
	}
	return words;
}
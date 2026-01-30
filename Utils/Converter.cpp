#include "Converter.h"

std::string ConvertIntToString(int x) {
	return std::to_string(x);
}
const char* ConvertStringToStringC(std::string x) {
	const char* nb = x.c_str();
	return nb;
}

const char* ConvertIntToStringC(std::string x) {
	const char* nb = x.c_str();
	return nb;
}

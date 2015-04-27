#include "common.hpp"

VR_NAMESPACE_BEGIN

std::string indent (const std::string &string, int amount) {
    /* This could probably be done faster (it's not
       really speed-critical though) */
    std::istringstream iss(string);
    std::ostringstream oss;
    std::string spacer(amount, ' ');
    bool firstLine = true;
    for (std::string line; std::getline(iss, line); ) {
        if (!firstLine)
            oss << spacer;
        oss << line;
        if (!iss.eof())
            oss << std::endl;
        firstLine = false;
    }
    return oss.str();
}

unsigned int toUInt(const std::string &str) {
	char *end_ptr = nullptr;
	unsigned int result = (int)strtoul(str.c_str(), &end_ptr, 10);
	if (*end_ptr != '\0')
		throw VRException("Could not parse integer value \"%s\"", str);
	return result;
}

std::vector<std::string> tokenize(const std::string &string, const std::string &delim, bool includeEmpty) {
	std::string::size_type lastPos = 0, pos = string.find_first_of(delim, lastPos);
	std::vector<std::string> tokens;

	while (lastPos != std::string::npos) {
		if (pos != lastPos || includeEmpty)
			tokens.push_back(string.substr(lastPos, pos - lastPos));
		lastPos = pos;
		if (lastPos != std::string::npos) {
			lastPos += 1;
			pos = string.find_first_of(delim, lastPos);
		}
	}

	return tokens;
}

VR_NAMESPACE_END

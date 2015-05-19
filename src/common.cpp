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

Matrix4f scale (const Matrix4f &m, float s) {
	Matrix4f scale(m);
	scale += Matrix4f::Identity() * s;
	scale(3, 3) = 1.f;

	if (scale(0, 0) <= 10e-4)
		scale = Matrix4f::Identity() * 10e-4;

	return scale;
}

Matrix4f scale(const Matrix4f &m, float x, float y, float z) {
	Matrix4f scale(m);
	scale(0, 0) = x;
	scale(1, 1) = y;
	scale(2, 2) = z;
	scale(3, 3) = 1.f;

	return scale;
}

std::string gestureStateName (GESTURE_STATES state) {
	std::string name = "NOT DEFINED";
	switch (state) {
		case GESTURE_STATES::INVALID : {
			name = "INVALID";
			break;
		}

		case GESTURE_STATES::START : {
			name = "START";
			break;
		}

		case GESTURE_STATES::UPDATE : {
			name = "UPDATE";
			break;
		}

		case GESTURE_STATES::STOP : {
			name = "STOP";
			break;
		}
	}

	return name;
}

std::string handName (HANDS hand) {
	std::string name = "NOT DEFINED";
	switch (hand) {
		case HANDS::RIGHT : {
			name = "RIGHT";
			break;
		}

		case HANDS::LEFT : {
			name = "LEFT";
			break;
		}
	}
	return name;
}

extern float clamp(float x) {
	if (x > 1.f)
		return 1.f;
	else if (x < 0.f)
		return 0.f;
	else return x;
}

VR_NAMESPACE_END

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
	if (m(0, 0) <= std::numeric_limits<float>::epsilon() + 10e-6)
		s += 10e2 * std::numeric_limits<float>::epsilon();

	Matrix4f scale(m);
	scale *= s;
	scale(3, 3) = 1.f;

	return scale;
}

Matrix4f scale(const Matrix4f &m, float x, float y, float z) {
	Matrix4f scale(Matrix4f::Identity());
	scale(0, 0) = m(0, 0) * x;
	scale(1, 1) = m(1, 1) * y;
	scale(2, 2) = m(2, 2) * z;
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

float clamp(float x) {
	if (x > 1.f)
		return 1.f;
	else if (x < 0.f)
		return 0.f;
	else return x;
}

void ppv(Vector3f v) {
	cout << "[" << v.x() << ", " << v.y() << ", " << v.z() << "]" << endl;
}

bool fileExists(const std::string &name) {
	std::ifstream f(name);
	if (f.good()) {
		f.close();
		return true;
	} else {
		f.close();
		return false;
	}
}

Matrix4f stringToMatrix4f (std::string &s) {
	std::istringstream ss(s);
	float value;
	Matrix4f matrix;
	for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 4; c++) {
			ss >> value;
			matrix(r, c) = value;
		}
	}

	return matrix;
}

std::string matrix4fToString (Matrix4f &m) {
	std::string mat;
	for (int r = 0; r < m.rows(); r++)
		for (int c = 0; c < m.cols(); c++)
			mat += std::to_string(m(r, c)) + (c == m.cols() - 1 && r == m.rows() - 1 ? "" : " ");

	return mat;
}

VR_NAMESPACE_END

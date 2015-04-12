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

VR_NAMESPACE_END

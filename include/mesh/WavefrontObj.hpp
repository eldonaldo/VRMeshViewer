#pragma once

#include "common.hpp"
#include "mesh/Mesh.hpp"
#include <unordered_map>
#include <fstream>

VR_NAMESPACE_BEGIN

/**
 * \brief Loader for Wavefront OBJ triangle meshes
 */
class WavefrontOBJ : public Mesh {

public:

    WavefrontOBJ();
	WavefrontOBJ(const std::string &file);
	void loadFromString(std::istream &is);

protected:

    /// Vertex indices used by the OBJ format
    struct OBJVertex {
        uint32_t p = (uint32_t) -1;
        uint32_t n = (uint32_t) -1;
        uint32_t uv = (uint32_t) -1;

        inline OBJVertex() { }

        inline OBJVertex(const std::string &string) {
            std::vector<std::string> tokens = tokenize(string, "/", true);

            if (tokens.size() < 1 || tokens.size() > 3)
                throw std::runtime_error("Invalid vertex data:" + string);

            p = toUInt(tokens[0]);

            if (tokens.size() >= 2 && !tokens[1].empty())
                uv = toUInt(tokens[1]);

            if (tokens.size() >= 3 && !tokens[2].empty())
                n = toUInt(tokens[2]);
        }

        inline bool operator==(const OBJVertex &v) const {
            return v.p == p && v.n == n && v.uv == uv;
        }
    };

    /// Hash function for OBJVertex
    struct OBJVertexHash : std::unary_function<OBJVertex, size_t> {
        std::size_t operator()(const OBJVertex &v) const {
            size_t hash = std::hash<uint32_t>()(v.p);
            hash = hash * 37 + std::hash<uint32_t>()(v.uv);
            hash = hash * 37 + std::hash<uint32_t>()(v.n);
            return hash;
        }
    };

	std::vector<OBJVertex>  vertices;
};

VR_NAMESPACE_END

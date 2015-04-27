#include "mesh/WavefrontOBJ.hpp"

VR_NAMESPACE_BEGIN

WavefrontOBJ::WavefrontOBJ(const std::string &file) {
    typedef std::unordered_map<OBJVertex, uint32_t, OBJVertexHash> VertexMap;

	m_name = file;
	std::ifstream is(file);
    if (is.fail())
        throw VRException("Unable to open OBJ file \"%s\"!", file);
        
	std::vector<Vector3f>   positions;
    std::vector<Vector2f>   texcoords;
    std::vector<Vector3f>   normals;
    std::vector<uint32_t>   indices;
    std::vector<OBJVertex>  vertices;
    VertexMap vertexMap;

    std::string line_str;
    while (std::getline(is, line_str)) {
        std::istringstream line(line_str);

        std::string prefix;
        line >> prefix;

		//Transform trafo = propList.getTransform("toWorld", Transform());

        if (prefix == "v") {
            Point3f p;
            line >> p.x() >> p.y() >> p.z();
            //p = trafo * p;
			//m_bbox.expandBy(p);
            positions.push_back(p);
        } else if (prefix == "vt") {
            Point2f tc;
            line >> tc.x() >> tc.y();
            texcoords.push_back(tc);
        } else if (prefix == "vn") {
            Normal3f n;
            line >> n.x() >> n.y() >> n.z();
            //normals.push_back((trafo * n).normalized());
			normals.push_back(n.normalized());
        } else if (prefix == "f") {
            std::string v1, v2, v3, v4;
            line >> v1 >> v2 >> v3 >> v4;
            OBJVertex verts[6];
            int nVertices = 3;

            verts[0] = OBJVertex(v1);
            verts[1] = OBJVertex(v2);
            verts[2] = OBJVertex(v3);

            if (!v4.empty()) {
                /* This is a quad, split into two triangles */
                verts[3] = OBJVertex(v4);
                verts[4] = verts[0];
                verts[5] = verts[2];
                nVertices = 6;
            }
            /* Convert to an indexed vertex list */
            for (int i=0; i<nVertices; ++i) {
                const OBJVertex &v = verts[i];
                VertexMap::const_iterator it = vertexMap.find(v);
                if (it == vertexMap.end()) {
                    vertexMap[v] = (uint32_t) vertices.size();
                    indices.push_back((uint32_t) vertices.size());
                    vertices.push_back(v);
                } else {
                    indices.push_back(it->second);
                }
            }
        }
    }

    m_F.resize(3, indices.size()/3);
    memcpy(m_F.data(), indices.data(), sizeof(uint32_t)*indices.size());

    m_V.resize(3, vertices.size());
    for (uint32_t i=0; i<vertices.size(); ++i)
        m_V.col(i) = positions.at(vertices[i].p-1);

    if (!normals.empty()) {
        m_N.resize(3, vertices.size());
        for (uint32_t i=0; i<vertices.size(); ++i)
            m_N.col(i) = normals.at(vertices[i].n-1);
    }

    if (!texcoords.empty()) {
        m_UV.resize(2, vertices.size());
        for (uint32_t i=0; i<vertices.size(); ++i)
            m_UV.col(i) = texcoords.at(vertices[i].uv-1);
    }
}


VR_NAMESPACE_END
/*
 * Copyright 2016 Daniel Kirchner
 *
 * This file is part of assimp2vf.
 *
 * assimp2vf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * assimp2vf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with assimp2vf.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Node.h"
#include <map>
#include <vector>
#include <sstream>
#include "Scene.h"

Node::Node (Scene *scene_) : scene (scene_), type (Container), vf (vfAlloc ()) {
}

Node::~Node (void) {
    vfFree (vf);
}

struct Vertex {
    Vertex (const aiVector3D &position, const aiVector3D &normal, const aiVector3D &texcoord) {
        x = position.x;
        y = position.y;
        z = position.z;
        nx = normal.x;
        ny = normal.y;
        nz = normal.z;
        tx = texcoord.x;
        ty = texcoord.y;
    }
    bool operator< (const Vertex &rhs) const {
        if (x < rhs.x) return true;
        if (rhs.x < x) return false;
        if (y < rhs.y) return true;
        if (rhs.y < y) return false;
        if (z < rhs.z) return true;
        if (rhs.z < z) return false;
        if (nx < rhs.nx) return true;
        if (rhs.nx < nx) return false;
        if (ny < rhs.ny) return true;
        if (rhs.ny < ny) return false;
        if (nz < rhs.nz) return true;
        if (rhs.nz < nz) return false;
        if (tx < rhs.tx) return true;
        if (rhs.tx < tx) return false;
        return ty < rhs.ty;
    }
    float x;
    float y;
    float z;
    float nx;
    float ny;
    float nz;
    float tx;
    float ty;
};

void Node::Load (const aiNode *node) {
    name = std::string (node->mName.data, node->mName.length);
    for (auto &c : name) if (c == '.' || c == ' ' || c == '-') c = '_';
    if (node->mParent) {
        parent = std::string (node->mParent->mName.data, node->mParent->mName.length);
        for (auto &c : parent) if (c == '.' || c == ' ' || c == '-') c = '_';
    }
    node->mTransformation.Decompose (scaling, rotation, position);

    if (node->mNumMeshes > 0) {
        if (!name.compare (0, 12, "meloadCurve_")) {
            type = BezierCurve;
        } else if (!name.compare (0, 16, "meloadCurveEndP_")) {
            type = SplineCurve;
        } else {
            type = Mesh;
        }
    } else {
        type = Container;
    }

    if (type == Mesh) {
        std::map<Vertex, unsigned int> vertexmap;
        std::vector<Vertex> vertices;

        for (auto meshid = 0; meshid < node->mNumMeshes; meshid++) {
            std::vector<uint16_t> indices;
            const aiMesh *mesh = scene->GetScene ()->mMeshes[node->mMeshes[meshid]];
            materials.push_back (mesh->mMaterialIndex);
            for (auto faceid = 0; faceid < mesh->mNumFaces; faceid++) {
                const aiFace &face = mesh->mFaces[faceid];
                if (face.mNumIndices != 3) {
                    throw std::runtime_error ("not a triangle");
                }
                for (auto i = 0; i < 3; i++) {
                    unsigned int index = face.mIndices[i];
                    Vertex v (mesh->mVertices[index], mesh->mNormals[index], mesh->mTextureCoords[0][index]);
                    auto it = vertexmap.find (v);
                    if (it == vertexmap.end ()) {
                        index = vertices.size ();
                        vertexmap[v] = index;
                        vertices.push_back (v);
                    } else {
                        index = it->second;
                    }
                    if (index > 65535) throw std::runtime_error ("index too large");
                    indices.push_back (index);
                }
            }

            {
                std::stringstream stream;
                stream << "SUBMESH" << meshid;
                vfAddSet (vf, stream.str ().c_str (), 3, VF_UNSIGNED_SHORT, indices.size () / 3, indices.data (), 0);
            }
        }

        {
            std::vector<float> positions;
            positions.resize (vertices.size () * 3);
            for (auto i = 0; i < vertices.size (); i++) {
                positions[i*3+0] = vertices[i].x;
                positions[i*3+1] = vertices[i].y;
                positions[i*3+2] = vertices[i].z;
            }
            vfAddSet (vf, "POSITIONS", 3, VF_FLOAT, vertices.size (), positions.data (), 0);
        }
        {
            std::vector<float> normals;
            normals.resize (vertices.size () * 3);
            for (auto i = 0; i < vertices.size (); i++) {
                normals[i * 3 + 0] = vertices[i].nx;
                normals[i * 3 + 1] = vertices[i].ny;
                normals[i * 3 + 2] = vertices[i].nz;
            }
            vfAddSet (vf, "NORMALS", 3, VF_FLOAT, vertices.size (), normals.data (), 0);
        }
        {
            std::vector<float> texcoords;
            texcoords.resize (vertices.size () * 2);
            for (auto i = 0; i < vertices.size (); i++) {
                texcoords[i*2+0] = vertices[i].tx;
                texcoords[i*2+1] = vertices[i].ty;
            }
            vfAddSet (vf, "TEXCOORDS0", 2, VF_FLOAT, vertices.size (), texcoords.data (), 0);
        }
    } else if (type == SplineCurve || type == BezierCurve) {
        if (node->mNumMeshes != 1) throw std::runtime_error ("more than one mesh in curve");
        const aiMesh *mesh = scene->GetScene ()->mMeshes[node->mMeshes[0]];
        std::vector<float> positions;
        positions.resize (mesh->mNumVertices * 3);
        for (auto i = 0; i < mesh->mNumVertices; i++) {
            positions[i * 3 + 0] = mesh->mVertices[i].x;
            positions[i * 3 + 1] = mesh->mVertices[i].y;
            positions[i * 3 + 2] = mesh->mVertices[i].z;
        }
        vfAddSet (vf, "POSITIONS", 3, VF_FLOAT, mesh->mNumVertices, positions.data (), 0);
    }
}

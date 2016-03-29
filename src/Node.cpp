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
#include "Arguments.h"
#include <miniball/Seb.h>

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
        std::vector<unsigned int> submesh_order;
        for (auto meshid = 0; meshid < node->mNumMeshes; meshid++) {
            submesh_order.push_back (meshid);
        }

        std::sort (submesh_order.begin (), submesh_order.end (), [&] (unsigned int lhs, unsigned int rhs) -> bool {
            aiString _lhs_name;
            aiString _rhs_name;
            scene->GetScene ()->mMaterials[scene->GetScene ()->mMeshes[node->mMeshes[lhs]]->mMaterialIndex]->Get (AI_MATKEY_NAME, _lhs_name);
            scene->GetScene ()->mMaterials[scene->GetScene ()->mMeshes[node->mMeshes[rhs]]->mMaterialIndex]->Get (AI_MATKEY_NAME, _rhs_name);
            std::string lhs_name (_lhs_name.data, _lhs_name.length);
            std::string rhs_name (_rhs_name.data, _rhs_name.length);
            if (!lhs_name.compare (0, 9, "Material-"))
                lhs_name.erase (0, 9);
            if (!rhs_name.compare (0, 9, "Material-"))
                rhs_name.erase (0, 9);

            std::cout << "COMPARE: " << lhs_name << " < " << rhs_name << std::endl;

            for (auto i = 0; i < lhs_name.length (); i++) {
                if (rhs_name.length () <= i) return true;
                else if (std::toupper (lhs_name[i]) < std::toupper (rhs_name[i])) return true;
                else if (std::toupper (lhs_name[i]) > std::toupper (rhs_name[i])) return false;
            }
            return false;
        });

        std::map<Vertex, unsigned int> vertexmap;
        std::vector<Vertex> vertices;
        std::vector<float> bboxes;

        for (auto _meshid = 0; _meshid < node->mNumMeshes; _meshid++) {
            unsigned int meshid = submesh_order[_meshid];
            std::vector<uint16_t> indices;
            std::vector<Seb::Point<double>> sebpoints;
            const aiMesh *mesh = scene->GetScene ()->mMeshes[node->mMeshes[meshid]];
            materials.push_back (mesh->mMaterialIndex);
            {
                aiString name;
                scene->GetScene ()->mMaterials[mesh->mMaterialIndex]->Get (AI_MATKEY_NAME, name);
                std::cout << _meshid << " -> " << meshid << ": MAT: " << name.C_Str () << std::endl;
            }
            for (auto faceid = 0; faceid < mesh->mNumFaces; faceid++) {
                const aiFace &face = mesh->mFaces[faceid];
                if (face.mNumIndices != 3) {
                    throw std::runtime_error ("not a triangle");
                }
                for (auto i = 0; i < 3; i++) {
                    unsigned int index = face.mIndices[i];
                    Vertex v (mesh->mVertices[index], mesh->mNormals[index], mesh->mTextureCoords[0][index]);

                    const double coords[] = { v.x, v.y, v.z };
                    sebpoints.emplace_back (3, coords);

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

                Seb::Smallest_enclosing_ball<double> miniball (3, sebpoints);
                bboxes.push_back (Arguments::get ().scale () * *(miniball.center_begin () + 0));
                bboxes.push_back (Arguments::get ().scale () * *(miniball.center_begin () + 1));
                bboxes.push_back (Arguments::get ().scale () * *(miniball.center_begin () + 2));
                bboxes.push_back (Arguments::get ().scale () * miniball.radius ());
            }
        }

        {
            std::vector<float> positions;
            positions.resize (vertices.size () * 3);
            for (auto i = 0; i < vertices.size (); i++) {
                positions[i*3+0] = Arguments::get ().scale () * vertices[i].x;
                positions[i*3+1] = Arguments::get ().scale () * vertices[i].y;
                positions[i*3+2] = Arguments::get ().scale () * vertices[i].z;
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
        vfAddSet (vf, "BSPHERES", 4, VF_FLOAT, bboxes.size () / 4, bboxes.data (), 0);
    } else if (type == SplineCurve || type == BezierCurve) {
        if (node->mNumMeshes != 1) throw std::runtime_error ("more than one mesh in curve");
        const aiMesh *mesh = scene->GetScene ()->mMeshes[node->mMeshes[0]];
        std::vector<float> positions;
        positions.resize (mesh->mNumVertices * 3);
        for (auto i = 0; i < mesh->mNumVertices; i++) {
            positions[i * 3 + 0] = Arguments::get ().scale () * mesh->mVertices[i].x;
            positions[i * 3 + 1] = Arguments::get ().scale () * mesh->mVertices[i].y;
            positions[i * 3 + 2] = Arguments::get ().scale () * mesh->mVertices[i].z;
        }
        vfAddSet (vf, "POSITIONS", 3, VF_FLOAT, mesh->mNumVertices, positions.data (), 0);
    }
}

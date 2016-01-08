/*
 * Copyright 2015 Daniel Kirchner
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

#include "Scene.h"
#include "Node.h"
#include <queue>
#include <iostream>
#include <fstream>
#include <openvf/openvf.h>
#include <sstream>
#include <set>

Scene::Scene (void) {
}

Scene::~Scene (void) {
}

void Scene::Load (const aiScene *scene_) {
    scene = scene_;
    std::queue<aiNode*> nodequeue;

    nodequeue.push (scene->mRootNode);
    while (!nodequeue.empty ()) {
        aiNode *ainode = nodequeue.front ();
        nodequeue.pop ();

        nodelist.emplace_back (new Node (this));
        nodelist.back ()->Load (ainode);
        nodemap[ainode->mName.C_Str ()] = nodelist.back ().get ();

        for (auto i = 0; i < ainode->mNumChildren; i++) {
            nodequeue.push (ainode->mChildren[i]);
        }
    }
}

std::ostream &operator<< (std::ostream &os, const aiVector3D &v) {
    return (os << "{ " << v.x << ", " << v.y << ", " << v.z << " }");
}

std::ostream &operator<< (std::ostream &os, const aiQuaternion &q) {
    return (os << "{ " << q.x << ", " << q.y << ", " << q.z << ", " << q.w << " }");
}

void SaveNodeAnim (aiNodeAnim *anim, const std::string &filename) {
    struct VF {
        VF (void) : vf (vfAlloc ()) { }
        ~VF (void) { vfFree (vf); }
        operator vf_t* (void) { return vf; }
        vf_t *vf;
    };
    VF vf;

    {
        std::vector<float> positions;
        positions.resize (anim->mNumPositionKeys * 3);
        for (auto i = 0; i < anim->mNumPositionKeys; i++) {
            positions[i * 3 + 0] = anim->mPositionKeys[i].mValue.x;
            positions[i * 3 + 1] = anim->mPositionKeys[i].mValue.y;
            positions[i * 3 + 2] = anim->mPositionKeys[i].mValue.z;
        }
        vfAddSet (vf, "POSITIONS", 3, VF_FLOAT, anim->mNumPositionKeys, positions.data (), VF_FLAGS_LZ4);
    }
    {
        std::vector<float> scalings;
        scalings.resize (anim->mNumScalingKeys * 3);
        for (auto i = 0; i < anim->mNumScalingKeys; i++) {
            scalings[i * 3 + 0]= anim->mScalingKeys[i].mValue.x;
            scalings[i * 3 + 1]= anim->mScalingKeys[i].mValue.y;
            scalings[i * 3 + 2]= anim->mScalingKeys[i].mValue.z;
        }
        vfAddSet (vf, "SCALINGS", 3, VF_FLOAT, anim->mNumScalingKeys, scalings.data (), VF_FLAGS_LZ4);
    }
    {
        std::vector<float> rotations;
        rotations.resize (anim->mNumRotationKeys * 4);
        for (auto i = 0; i < anim->mNumRotationKeys; i++) {
            rotations[i * 4 + 0]= anim->mRotationKeys[i].mValue.x;
            rotations[i * 4 + 1]= anim->mRotationKeys[i].mValue.y;
            rotations[i * 4 + 2]= anim->mRotationKeys[i].mValue.z;
            rotations[i * 4 + 3]= anim->mRotationKeys[i].mValue.w;
        }
        vfAddSet (vf, "ROTATIONS", 4, VF_FLOAT, anim->mNumRotationKeys, rotations.data (), VF_FLAGS_LZ4);
    }

    vfSave (vf, filename.c_str ());
}

void Scene::Save (void) {
    std::set<std::string> materialnames;

    std::cout << "materials = {" << std::endl;
    for (auto i = 0; i < scene->mNumMaterials; i++) {
        aiString aimatname;
        scene->mMaterials[i]->Get (AI_MATKEY_NAME, aimatname);
        std::string matname (aimatname.data, aimatname.length);
        if (!matname.compare (0, 9, "Material-"))
            matname.erase (0, 9);
        std::cout << "  " << matname << " = Material {" << std::endl;
        std::cout << "  };" << std::endl;
        materialnames.insert (matname);
    }
    std::cout << "};" << std::endl << std::endl;

    for (auto &node : nodelist) {
        std::cout << "nodes." << node->GetName () << " = " << node->GetTypeName () <<" {" << std::endl;
        if (!node->GetParent ().empty ()) {
            std::cout << "  parent = nodes." << node->GetParent () << ";" << std::endl;
        }
        if (vfGetFirstSet (node->GetVF ()) != nullptr) {
            std::cout << "  filename = \"" << node->GetName () << ".vf\";" << std::endl;
            vfSave (node->GetVF (), (node->GetName () + ".vf").c_str ());
        }
        if (!node->GetMaterials ().empty ()) {
            std::cout << "  materials = {" << std::endl;
            for (auto &material : node->GetMaterials ()) {
                aiString aimatname;
                scene->mMaterials[material]->Get (AI_MATKEY_NAME, aimatname);
                std::string matname (aimatname.data, aimatname.length);
                if (!matname.compare (0, 9, "Material-"))
                    matname.erase (0, 9);
                std::cout << "    materials." << matname << std::endl;
                materialnames.insert (matname);
            }
            std::cout << "  };" << std::endl;
        }
        std::cout << "  position = " << node->GetPosition () << ";" << std::endl;
        std::cout << "  scaling = " << node->GetScaling () << ";" << std::endl;
        std::cout << "  rotation = " << node->GetRotation () << ";" << std::endl;
        std::cout << "};" << std::endl;
    }

    for (auto animid = 0; animid < scene->mNumAnimations; animid++) {
        std::cout << std::endl;
        aiAnimation *anim = scene->mAnimations[animid];
        std::string animname = std::string (anim->mName.data, anim->mName.length);
        if (animname.empty ()) {
            std::stringstream stream;
            stream << "anim";
            if (scene->mNumAnimations > 1) stream << animid;
            animname = stream.str ();
        }
        std::cout << "animations." << animname << " = AnimationData {" << std::endl;
        for (auto channel = 0; channel < anim->mNumChannels; channel++) {
            aiNodeAnim *nodeanim = anim->mChannels[channel];
            std::string nodename = std::string (nodeanim->mNodeName.data, nodeanim->mNodeName.length);
            std::string filename = nodename + "_" + animname + ".vf";
            std::cout << "  [nodes." << nodename << "] = \"" << filename << "\";" << std::endl;
            SaveNodeAnim (nodeanim, filename);
        }
        std::cout << "};" << std::endl;
    }
}

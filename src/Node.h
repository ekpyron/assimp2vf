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

#ifndef ASSIMP2VF_NODE_H
#define ASSIMP2VF_NODE_H

#include <assimp/scene.h>
#include <openvf/openvf.h>

class Scene;

class Node {
public:
    Node (Scene *scene);
    ~Node (void);
    void Load (const aiNode *node);
    const std::string &GetName (void) const {
        return name;
    }
    const std::string &GetParent (void) const {
        return parent;
    }
    const aiVector3D &GetPosition (void) const {
        return position;
    }
    const aiVector3D &GetScaling (void) const {
        return scaling;
    }
    const aiQuaternion &GetRotation (void) const {
        return rotation;
    }
    vf_t *GetVF (void) {
        return vf;
    }
    const vf_t *GetVF (void) const {
        return vf;
    }
    enum Type {
        Container,
        Mesh,
        SplineCurve,
        BezierCurve
    };
    Type GetType (void) const {
        return type;
    }
    std::string GetTypeName (void) const {
        switch (type) {
            case Mesh:
                return "Mesh";
            case SplineCurve:
                return "SplineCurve";
            case BezierCurve:
                return "BezierCurve";
            case Container:
                return "Container";
            default:
                return "Node";
        }
    }
    const std::vector<unsigned int> &GetMaterials (void) const {
        return materials;
    }
private:
    vf_t *vf;
    Type type;
    std::string name;
    std::string parent;
    aiVector3D position;
    aiVector3D scaling;
    aiQuaternion rotation;
    std::vector<unsigned int> materials;
    Scene *scene;
};

#endif /* !defined ASSIMP2VF_NODE_H */

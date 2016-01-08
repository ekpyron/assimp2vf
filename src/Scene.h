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

#ifndef ASSIMP2VF_SCENE_H
#define ASSIMP2VF_SCENE_H

#include <assimp/scene.h>
#include <map>
#include <vector>
#include <memory>

class Node;

class Scene {
public:
    Scene (void);
    ~Scene (void);
    void Load (const aiScene *scene);
    void Save (void);
    const aiScene *GetScene (void) const {
        return scene;
    }
private:
    std::map<std::string, Node*> nodemap;
    std::vector<std::unique_ptr<Node>> nodelist;
    const aiScene *scene;
};

#endif /* !defined ASSIMP2VF_SCENE_H */

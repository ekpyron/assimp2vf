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

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include "Scene.h"
#include "Arguments.h"

int main (int argc, char *argv[]) {
    try {
        Assimp::Importer importer;
#ifdef AI_CONFIG_IMPORT_COLLADA_USE_COLLADA_NAMES
        importer.SetPropertyBool(AI_CONFIG_IMPORT_COLLADA_USE_COLLADA_NAMES, true);
#else
# warning "AI_CONFIG_IMPORT_COLLADA_USE_COLLADA_NAMES not supported by assimp! Falling back to using id tags as names instead."
#endif
        Assimp::DefaultLogger::create ("", Assimp::Logger::VERBOSE);

        if (!arguments ().parse (argc, argv))
            return EXIT_SUCCESS;

        const aiScene *aiscene = importer.ReadFile (arguments ().inputfile (), aiProcess_GenSmoothNormals|aiProcess_CalcTangentSpace
                                                             |aiProcess_Triangulate|aiProcess_GenUVCoords|aiProcess_FlipUVs|aiProcess_OptimizeMeshes
                                                             |aiProcess_SortByPType|aiProcess_FindDegenerates|aiProcess_ImproveCacheLocality);
        if (!aiscene) {
            std::cerr << "Cannot load " << arguments ().inputfile () << ": " << importer.GetErrorString () << std::endl;
            return EXIT_FAILURE;
        }

        Scene scene;
        scene.Load (aiscene);

        switch (arguments().action ()) {
            case Arguments::CONVERT:
                scene.Save ();
                break;
            case Arguments::LIST_OUTPUTS:
                scene.ListOutputs ();
                break;
            case Arguments::LIST_MATERIALS:
                scene.ListMaterials ();
                break;
            case Arguments::LIST_NODES:
                scene.ListNodes ();
                break;
            case Arguments::LIST_ANIMATIONDATA:
                scene.ListAnimationData ();
                break;
            default:
                throw std::runtime_error ("invalid action");
        }

        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what () << std::endl;
        return EXIT_FAILURE;
    }
}

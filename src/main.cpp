#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include "Scene.h"

int main (int argc, char *argv[]) {
    try {
        Assimp::Importer importer;
        Assimp::DefaultLogger::create ("", Assimp::Logger::VERBOSE);

        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
            return EXIT_FAILURE;
        }

        const aiScene *aiscene = importer.ReadFile (argv[1], aiProcess_Triangulate);
        if (!aiscene) {
            std::cerr << "Cannot load " << argv[1] << ": " << importer.GetErrorString () << std::endl;
            return EXIT_FAILURE;
        }

        Scene scene;
        scene.Load (aiscene);

        scene.Save ();

        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what () << std::endl;
        return EXIT_FAILURE;
    }
}

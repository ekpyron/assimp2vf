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
#include "Arguments.h"

Arguments::Arguments (void) : action_ (CONVERT), scale_ (1.0f) {
}

Arguments::~Arguments (void) {
}

void Arguments::usage (const char *argv0) {
    std::cerr << "Usage: " << argv0 << " [-l|-m|-n|-a] [-s scale] inputfile" << std::endl << std::endl
    << "  -l    outputs a list of files that will be generated" << std::endl
    << "  -m    outputs a list of materials used by the nodes" << std::endl
    << "  -n    outputs the node hierarchy" << std::endl
    << "  -a    outputs the animation data" << std::endl
    << "  -s    scale factor" << std::endl;
}

bool Arguments::parse (int argc, char **argv) {
    for (auto i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] != 0 && argv[i][2] == 0) {
                switch (argv[i][1]) {
                    case 'l':
                        action_ = LIST_OUTPUTS;
                        break;
                    case 'm':
                        action_ = LIST_MATERIALS;
                        break;
                    case 'n':
                        action_ = LIST_NODES;
                        break;
                    case 'a':
                        action_ = LIST_ANIMATIONDATA;
                        break;
                    case 's':
                    {
                        i++;
                        if (i >= argc) {
                            throw std::runtime_error ("missing argument after -s");
                        }
                        scale_ = atof (argv[i]);
                        break;
                    }
                    default:
                        throw std::runtime_error (std::string ("invalid argument: \"") + argv[i] + "\"");
                        break;
                }
            } else {
                throw std::runtime_error (std::string ("invalid argument: \"") + argv[i] + "\"");
            }
        } else {
            args.emplace_back (argv[i]);
        }
    }
    if (args.size () != 1) {
        usage (argv[0]);
        return false;
    }
    return true;
}

Arguments& Arguments::get (void) {
    static Arguments arguments;
    return arguments;
}

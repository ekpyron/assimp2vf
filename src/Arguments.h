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

#ifndef ASSIMP2VF_ARGUMENTS_H
#define ASSIMP2VF_ARGUMENTS_H

#include <string>
#include <vector>

class Arguments {
public:
    Arguments (const Arguments&) = delete;
    ~Arguments (void);
    Arguments &operator= (const Arguments&) = delete;
    enum Action {
        CONVERT,
        LIST_OUTPUTS,
        LIST_MATERIALS,
        LIST_NODES,
        LIST_ANIMATIONDATA
    };
    static Arguments &get (void);
    void usage (const char *argv0);
    bool parse (int argc, char **argv);
    const std::string &inputfile (void) const {
        return args.front ();
    }

    Action action (void) const {
        return action_;
    }
    float scale (void) const {
        return scale_;
    }
private:
    Arguments (void);
    Action action_;
    float scale_;
    std::vector<std::string> args;
};

inline Arguments &arguments (void) {
    return Arguments::get ();
}


#endif /* !defined ASSIMP2VF_ARGUMENTS_H  */
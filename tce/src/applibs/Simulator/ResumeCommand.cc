/*
    Copyright 2002-2008 Tampere University of Technology.  All Rights
    Reserved.

    This file is part of TTA-Based Codesign Environment (TCE).

    TCE is free software; you can redistribute it and/or modify it under the
    terms of the GNU General Public License version 2 as published by the Free
    Software Foundation.

    TCE is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with TCE; if not, write to the Free Software Foundation, Inc., 51 Franklin
    St, Fifth Floor, Boston, MA  02110-1301  USA

    As a special exception, you may use this file as part of a free software
    library without restriction.  Specifically, if other files instantiate
    templates or use macros or inline functions from this file, or you compile
    this file and link it with other files to produce an executable, this
    file does not by itself cause the resulting executable to be covered by
    the GNU General Public License.  This exception does not however
    invalidate any other reasons why the executable file might be covered by
    the GNU General Public License.
*/
/**
 * @file ResumeCommand.cc
 *
 * Implementation of ResumeCommand class
 *
 * @author Pekka Jääskeläinen 2005 (pjaaskel-no.spam-cs.tut.fi)
 * @note rating: red
 */

#include "ResumeCommand.hh"
#include "Application.hh"
#include "SimulatorFrontend.hh"
#include "SimulatorInterpreterContext.hh"
#include "Exception.hh"
#include "SimulatorToolbox.hh"
#include "SimulatorTextGenerator.hh"

/**
 * Constructor.
 *
 * Sets the name of the command to the base class.
 */
ResumeCommand::ResumeCommand() : 
    SimControlLanguageCommand("resume") {
}

/**
 * Destructor.
 *
 * Does nothing.
 */
ResumeCommand::~ResumeCommand() {
}

/**
 * Executes the "resume" command.
 *
 * Resume simulation of the program until the simulation is finished or 
 * a breakpoint is reached. The optional argument gives the number of times 
 * the continue command is repeated, that is, the number of times breakpoints 
 * should be ignored.
 *
 * @param arguments (optional) count of times to repeat in case breakpoints are
 *                  reached.
 * @return Always true.
 * @exception NumberFormatException Is never thrown by this command.
 *
 */
bool 
ResumeCommand::execute(const std::vector<DataObject>& arguments)
    throw (NumberFormatException) {

    // how many times breakpoints should be ignored?
    int repeatCount = 0;
    if (!checkArgumentCount(arguments.size() - 1, 0, 1)) {
        return false;
    } else {
        if (arguments.size() > 1) {
            if  (!checkPositiveIntegerArgument(arguments[1])) {
                return false;
            } else {
                repeatCount = arguments[1].integerValue();
            }
        }
    }    

    if (!checkSimulationStopped()) {
        return false;
    }

    int repeats = 0;
    do {
        simulatorFrontend().run();
        ++repeats;
    } while(simulatorFrontend().isSimulationStopped() && 
            repeats < repeatCount);

    printStopInformation();

    return true;
}

/**
 * Returns the help text for this command.
 * 
 * Help text is searched from SimulatorTextGenerator.
 *
 * @return The help text.
 * @todo Use SimulatorTextGenerator to get the help text.
 */
std::string 
ResumeCommand::helpText() const {
    return SimulatorToolbox::textGenerator().text(
        Texts::TXT_INTERP_HELP_RESUME).str();
}


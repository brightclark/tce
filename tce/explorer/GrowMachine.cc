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
 * @file GrowMachine.cc
 *
 * Explorer plugin that adds resources until cycle count doesn't go down
 * anymore.
 *
 * @author Jari Mäntyneva 2007 (jari.mantyneva-no.spam-tut.fi)
 * @author Esa Määttä 2008 (esa.maatta-no.spam-tut.fi)
 * @note rating: red
 */

#include <vector>
#include <set>
#include <string>

#include "DesignSpaceExplorerPlugin.hh"

#include "DSDBManager.hh"
#include "Machine.hh"

#include "MachineResourceModifier.hh"
#include "DesignSpaceExplorer.hh"
#include "CostEstimates.hh"
#include "SimulatorConstants.hh"

using namespace TTAProgram;
using namespace TTAMachine;
using namespace HDB;

/**
 * Explorer plugin that adds resources until cycle count doesn't go down
 * anymore.
 *
 * Supported parameters:
 */
class GrowMachine : public DesignSpaceExplorerPlugin {
public:
    DESCRIPTION("Removes resources until the real time "
        "requirements of applications are not reached anymore.");
    
    /**
     * Optimizes the architecture in regards of the cycle count.
     *
     * Optimizes the architecture by growing it until cycle count doesn't go 
     * down anymore.
     *
     * @TODO: average cycle count lowering, or lowest/largest lowering
     *        percentage among apps now it's largest lowering among apps.
     * @TODO: parametrize machine growing, example how many buses to add
     *        each step.
     *
     * @return Configurations (including only adf) generated by CycleOptimizer.
     *         Best result is at the top of the list.
     */
    virtual std::vector<RowID>
    explore(const RowID& configurationID, const unsigned int&) {
        std::vector<RowID> result;

        readParameters();

        TTAMachine::Machine* adf = NULL;
        DSDBManager::MachineConfiguration configuration;
        DSDBManager& dsdb = db();

        // get the architecture from the database
        try {
            configuration = dsdb.configuration(configurationID);
            adf = dsdb.architecture(configuration.architectureID);
        } catch (const KeyNotFound& e) {
            debugLog(std::string("Fetching architecture from DSDB failed in "
                     "GrowMachine plugin. ")
                     + e.errorMessage() + std::string(" ")
                     + e.errorMessageStack());
            result.push_back(configurationID);
            return result;
        }

        // evaluate to get current cycle counts
        DesignSpaceExplorer explorer;
        explorer.setDSDB(dsdb);
        CostEstimates estimates;
        try {
            bool estimate = false;
            if (!explorer.evaluate(configuration, estimates, estimate)) {
                debugLog(std::string("Evaluate failed in GrowMachine plugin."));
                result.push_back(configurationID);
                return result;
            }
        } catch (const Exception& e) {
            debugLog(std::string("Error in GrowMachine plugin: ")
                    + e.errorMessage() + std::string(" ")
                    + e.errorMessageStack());
            result.push_back(configurationID);
            return result;
        }

        if (estimates.cycleCounts() < 1) {
            std::ostringstream msg(std::ostringstream::out);
            msg << "GrowMachine Plugin Error: Couldn't evaluate cycle "
                << "counts for applications, correct_simulation_output"
                << " probably missing." << std::endl;
            errorOuput(msg.str());
            result.push_back(configurationID);
            return result;
        }

        // all applications minimum cycle count
        ClockCycleCount currentMinCycles = estimates.cycleCount(0);
        for (int i = 1; i < estimates.cycleCounts(); i++) {
            if (estimates.cycleCount(i) < currentMinCycles) {
                currentMinCycles = estimates.cycleCount(i);
            }
        }

        ClockCycleCount prevMinCycles = 0;
        MachineResourceModifier modifier;
        std::map<ClockCycleCount, RowID> resultMap;
        do {
            prevMinCycles = currentMinCycles;
            try {
                // These parameters passed to the modifier can be changed.
                // They tell how many units of same type are added each time.
                modifier.addBusesByAmount(4, *adf);
                modifier.increaseAllRFsThatDiffersByAmount(1, *adf);
                modifier.increaseAllFUsThatDiffersByAmount(1, *adf);
                // @TODO immediate unit addition

                DSDBManager::MachineConfiguration newConfiguration;
                try {
                    newConfiguration.architectureID = dsdb.addArchitecture(*adf);
                } catch (const RelationalDBException& e) {
                    // Error occurred while adding adf to the dsdb, adf
                    // probably too big
                    break;
                }
                newConfiguration.hasImplementation = false;
                RowID confID = dsdb.addConfiguration(newConfiguration);
                CostEstimates newEstimates;

                // evaluate to get new cycle counts
                if (explorer.evaluate(newConfiguration, newEstimates, false)) {
                    // resets the currentMinCycles 
                    currentMinCycles = newEstimates.cycleCount(0);
                    for (int i = 1; i < newEstimates.cycleCounts(); i++) {
                        if (newEstimates.cycleCount(i) < currentMinCycles) {
                            currentMinCycles = newEstimates.cycleCount(i);
                        }
                    }
                    resultMap[currentMinCycles] = confID;
                } else {
                    // evaluating failed
                    break;
                }

            } catch (const Exception& e) {
                debugLog(std::string("Error in GrowMachine plugin: ")
                        + e.errorMessage() + std::string(" ")
                        + e.errorMessageStack());
                result.push_back(configurationID);
                return result;
            }
        } while ((currentMinCycles < prevMinCycles) &&
                (((static_cast<double>(superiority_) / 100) * prevMinCycles) <
                 (prevMinCycles - currentMinCycles)));

        std::map<ClockCycleCount, RowID>::const_iterator mapIter = 
            resultMap.begin();
        for (; mapIter != resultMap.end(); mapIter++) {
            result.push_back((*mapIter).second);
        }
        if (result.empty()) {
            // if now new configuration created return the old
            verboseLogC("GrowMachine could not generate new configs.", 2)
            result.push_back(configurationID);
        }
        return result;
    }

private:

    /// Percentage value of how much faster schedules are wanted until cycle
    /// count optimization is stopped.
    unsigned int superiority_;

    /**
     * Reads the parameters given to the plugin.
     */
    void readParameters() {
        const std::string superiority = "superiority";

        if (hasParameter(superiority)) {
            try {
                superiority_ = Conversion::toUnsignedInt(parameterValue(superiority));
            } catch (const Exception& e) {
                parameterError(superiority, "Unsigned integer");
                superiority_ = 2;
            }
        } else {
            // set default value to superiority
            superiority_ = 2;
        }
    }

    /**
     * Print error message of invalid parameter to plugin error stream.
     *
     * @param param Name of the parameter that has invalid value.
     * @param type Type of the parameter ought to be.
     */
    void parameterError(const std::string& param, const std::string& type) {
        std::ostringstream msg(std::ostringstream::out);
        msg << "Invalid parameter value '" << parameterValue(param)
            << "' on parameter '" << param << "'. " << type 
            << " value expected." << std::endl;
        errorOuput(msg.str());
    }
};

EXPORT_DESIGN_SPACE_EXPLORER_PLUGIN(GrowMachine)

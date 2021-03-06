//==========================================================================
//  RESULTFILEUTILS.CC - part of
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//  Author: Tamas Borbely, Andras Varga
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2017 Andras Varga
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "resultfileutils.h"

#include "omnetpp/simkerneldefs.h"

#include <cstring>
#include "common/stringutil.h"
#include "omnetpp/cconfigoption.h"
#include "omnetpp/csimulation.h"
#include "omnetpp/platdep/platmisc.h"
#include "envirbase.h"

using namespace omnetpp::common;

namespace omnetpp {
namespace envir {

std::string ResultFileUtils::getRunId()
{
    cConfigurationEx *cfg = getEnvir()->getConfigEx();
    return cfg->getVariable(CFGVAR_RUNID);
}

StringMap ResultFileUtils::getRunAttributes()
{
    cConfigurationEx *cfg = getEnvir()->getConfigEx();
    std::vector<const char *> keys = cfg->getPredefinedVariableNames();
    StringMap attrs;
    for (const char *key : keys)
        if (strcmp(key, CFGVAR_RUNID) != 0)  // skip runId
            attrs[key] = cfg->getVariable(key);
    return attrs;
}

StringMap ResultFileUtils::getIterationVariables()
{
    cConfigurationEx *cfg = getEnvir()->getConfigEx();
    StringMap itervars;
    std::vector<const char *> keys2 = cfg->getIterationVariableNames();
    for (const char *key : keys2)
        itervars[key] = cfg->getVariable(key);
    return itervars;
}


OrderedKeyValueList ResultFileUtils::getParamAssignments()
{
    cConfigurationEx *cfg = getEnvir()->getConfigEx();
    std::vector<const char *> params = cfg->getParameterKeyValuePairs();
    OrderedKeyValueList result;
    for (int i = 0; i < (int)params.size(); i += 2)
        result.push_back(std::make_pair(params[i], params[i+1]));
    return result;
}

OrderedKeyValueList ResultFileUtils::getConfigEntries()
{
    cConfigurationEx *cfg = getEnvir()->getConfigEx();
    std::vector<const char *> keys = cfg->getMatchingConfigKeys("*");
    OrderedKeyValueList result;
    for (const char *key : keys)
        result.push_back(std::make_pair(key, cfg->getConfigValue(key)));
    return result;
}

}  // namespace envir
}  // namespace omnetpp


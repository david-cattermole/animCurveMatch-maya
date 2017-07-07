/*
 *
 */


//
#include <animCurveMatchCmd.h>
#include <animCurveMatchUtils.h>

// STL
#include <cmath>

// Utils
#include <debugUtils.h>

animCurveMatchCmd::~animCurveMatchCmd() {}

void *animCurveMatchCmd::creator() {
    return new animCurveMatchCmd();
}


/*
 * Tell Maya we have a syntax function.
 */
bool animCurveMatchCmd::hasSyntax()
{
    return true;
}


/*
 * Add flags to the command syntax
 */
MSyntax animCurveMatchCmd::newSyntax() {
    MSyntax syntax;
    syntax.enableQuery(false);
    syntax.enableEdit(false);

    // Objects to work on.
    syntax.useSelectionAsDefault(false);
    syntax.setObjectType(MSyntax::kSelectionList);
    syntax.setMinObjects(2);
    syntax.setMaxObjects(2);

    // Flags
    syntax.addFlag(kNameFlag, kNameFlagLong, MSyntax::kString);
    syntax.addFlag(kIterationsFlag, kIterationsFlagLong, MSyntax::kUnsigned);
    syntax.addFlag(kAdjustValuesFlag, kAdjustValuesFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kAdjustTangentAnglesFlag, kAdjustTangentAnglesFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kAdjustTangentWeightsFlag, kAdjustTangentWeightsFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kAdjustTimesFlag, kAdjustTimesFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kScaleTimeKeysFlag, kScaleTimeKeysFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kForceWholeFramesFlag, kForceWholeFramesFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kAddKeysFlag, kAddKeysFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kNewCurveFlag, kNewCurveFlagLong, MSyntax::kBoolean);
    return syntax;
}

/*
 * Parse command line arguments
 */
MStatus animCurveMatchCmd::parseArgs(const MArgList &args) {
    MStatus status = MStatus::kSuccess;

    MArgDatabase argData(syntax(), args, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Get nodes
    MSelectionList selList;
    status = argData.getObjects(selList);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    int count = selList.length();
    if (count != 2)
    {
        ERR("2 animCurve objects must be given.");
        MGlobal::displayWarning("2 animCurve objects must be given.");
        return MStatus::kFailure;
    }
    selList.getDependNode(0, m_srcCurve);
    selList.getDependNode(1, m_dstCurve);

    MFnDependencyNode m_srcCurvePath(m_srcCurve);
    MFnDependencyNode m_dstCurvePath(m_dstCurve);
    INFO("srcCurve node name=" << m_srcCurvePath.name());
    INFO("dstCurve node name=" << m_dstCurvePath.name());

    // Get 'Name'
    m_name = m_srcCurvePath.name() + "_solved";
    if (argData.isFlagSet(kNameFlag)) {
        status = argData.getFlagArgument(kNameFlag, 0, m_name);
    }
    INFO("m_name=" << m_name);

    // Get 'Iterations'
    m_iterations = kIterationsDefaultValue;
    if (argData.isFlagSet(kIterationsFlag)) {
        status = argData.getFlagArgument(kIterationsFlag, 0, m_iterations);
    }
    INFO("m_iterations=" << m_iterations);

    // Get 'Adjust Values'
    m_adjustValues = kAdjustValuesDefaultValue;
    if (argData.isFlagSet(kAdjustValuesFlag)) {
        status = argData.getFlagArgument(kAdjustValuesFlag, 0, m_adjustValues);
    }
    INFO("m_adjustValues=" << m_adjustValues);

    // Get 'Adjust Times'
    m_adjustTimes = kAdjustTimesDefaultValue;
    if (argData.isFlagSet(kAdjustTimesFlag)) {
        status = argData.getFlagArgument(kAdjustTimesFlag, 0, m_adjustTimes);
    }
    INFO("m_adjustTimes=" << m_adjustTimes);

    // Get 'Adjust Tangent Angles'
    m_adjustTangentAngles = kAdjustTangentAnglesDefaultValue;
    if (argData.isFlagSet(kAdjustTangentAnglesFlag)) {
        status = argData.getFlagArgument(kAdjustTangentAnglesFlag, 0, m_adjustTangentAngles);
    }
    INFO("m_adjustTangentAngles=" << m_adjustTangentAngles);

    // Get 'Adjust Tangent Weights'
    m_adjustTangentWeights = kAdjustTangentWeightsDefaultValue;
    if (argData.isFlagSet(kAdjustTangentWeightsFlag)) {
        status = argData.getFlagArgument(kAdjustTangentWeightsFlag, 0, m_adjustTangentWeights);
    }
    INFO("m_adjustTangentWeights=" << m_adjustTangentWeights);

    // Get 'Force Whole Frames'
    m_scaleTimeKeys = kScaleTimeKeysDefaultValue;
    if (argData.isFlagSet(kScaleTimeKeysFlag)) {
        status = argData.getFlagArgument(kScaleTimeKeysFlag, 0, m_scaleTimeKeys);
    }
    INFO("m_scaleTimeKeys=" << m_scaleTimeKeys);

    // Get 'Force Whole Frames'
    m_forceWholeFrames = kForceWholeFramesDefaultValue;
    if (argData.isFlagSet(kForceWholeFramesFlag)) {
        status = argData.getFlagArgument(kForceWholeFramesFlag, 0, m_forceWholeFrames);
    }
    INFO("m_forceWholeFrames=" << m_forceWholeFrames);

    // Get 'Add Keys'
    m_addKeys = kAddKeysDefaultValue;
    if (argData.isFlagSet(kAddKeysFlag)) {
        status = argData.getFlagArgument(kAddKeysFlag, 0, m_addKeys);
    }
    INFO("m_addKeys=" << m_addKeys);

    // Get 'New Curve'
    m_createNewCurve = kNewCurveDefaultValue;
    if (argData.isFlagSet(kNewCurveFlag)) {
        status = argData.getFlagArgument(kNewCurveFlag, 0, m_createNewCurve);
    }
    INFO("m_createNewCurve=" << m_createNewCurve);

    return status;
}


MStatus animCurveMatchCmd::doIt(const MArgList &args) {
    MStatus status = MStatus::kSuccess;

    status = parseArgs(args);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnAnimCurve srcAnimCurveFn(m_srcCurve, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnAnimCurve dstAnimCurveFn(m_dstCurve, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Duplicate destination curve, so we modify it, rather than the destination curve.
    MObject newCurve = m_dstCurve;
    if (m_createNewCurve) {
        MString dstAnimCurveName = dstAnimCurveFn.name(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        MDGModifier dgMod;

//        MString cmd = MString("duplicate -name \"") + m_name + MString("\" -inputConnections \"") +
//                      dstAnimCurveName + MString("\";");

        MString name(m_name);

        // TODO: Ensure the name given is unique.
//        MFnDependencyNode dgNodeFn();
//        dgNodeFn.hasUniqueName()
//        MStatus uniqueNameStatus = MS::kSuccess;
//        MSelectionList selList;
//        bool hasUniqueName = true;
//        while (hasUniqueName) {
//            const MString tmpName(name);
//            uniqueNameStatus = selList.add(tmpName, false);
//            if (uniqueNameStatus != MS::kSuccess)
//            {
//                for (int i=0; i<10; ++i)
//                {
//                    MString c;
//                    c.set()
//                    if name.rindex()
//                    name = name;
//                }
//            }
//            MObject tmpObj;
//            uniqueNameStatus = selList.getDagPath(0, tmpObj);
//            if (uniqueNameStatus != MS::kSuccess)
//            {
//                return uniqueNameStatus;
//            }
//        }

        MString cmd;
        cmd += "duplicate -name \"";
        cmd += name; // NOTE: name must be unique, so we can get the output node by name.
        cmd += "\" -inputConnections \"";
        cmd += dstAnimCurveName;
        cmd += "\";";
        dgMod.commandToExecute(cmd);
        dgMod.doIt();

        // Convert name into MObject.
        MSelectionList selList;
        status = selList.add(name, false);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        status = selList.getDependNode(0, newCurve);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    int iterMax = m_iterations;
    double outError = -1.0;
    bool ret = solveCurveFit(iterMax,
                             m_srcCurve,
                             newCurve,
                             m_adjustValues,
                             m_adjustTimes,
                             m_adjustTangentAngles,
                             m_adjustTangentWeights,
                             m_scaleTimeKeys,
                             m_forceWholeFrames,
                             m_addKeys,
                             outError);
    if (ret == false)
    {
        WRN("Solver returned false!");
    }
    return status;
}

MStatus animCurveMatchCmd::undoIt() {
    return MPxCommand::undoIt();
}

MStatus animCurveMatchCmd::redoIt() {
    return MPxCommand::redoIt();
}


/*
 * Command for running animCurveMatch.
 *
 * Contains undo/redo functionality.
 */


//
#include <animCurveMatchCmd.h>
#include <animCurveMatchUtils.h>

// STL
#include <cmath>

// Utils
#include <utilities/debugUtils.h>

animCurveMatchCmd::~animCurveMatchCmd() {}

void *animCurveMatchCmd::creator() {
    return new animCurveMatchCmd();
}


/*
 * Tell Maya we have a syntax function.
 */
bool animCurveMatchCmd::hasSyntax() const {
    return true;
}

bool animCurveMatchCmd::isUndoable() const {
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
    if (count != 2) {
        ERR("2 animCurve objects must be given.");
        MGlobal::displayWarning("2 animCurve objects must be given.");
        return MStatus::kFailure;
    }

    MObject srcCurve;
    status = selList.getDependNode(0, srcCurve);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MFnDependencyNode srcNodeFn(srcCurve);
    m_srcCurveName = srcNodeFn.name(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MObject dstCurve;
    status = selList.getDependNode(1, dstCurve);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MFnDependencyNode dstNodeFn(dstCurve);
    m_dstCurveName = dstNodeFn.name(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    INFO("srcCurve node name=" << m_srcCurveName);
    INFO("dstCurve node name=" << m_dstCurveName);

    // Get 'Name'
    m_name = m_srcCurveName + "_solved";
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
//
//  Description:
//    implements the MEL animCurveMatch command.
//
//  Arguments:
//    argList - the argument list that was passes to the command from MEL
//
//  Return Value:
//    MS::kSuccess - command succeeded
//    MS::kFailure - command failed (returning this value will cause the
//                     MEL script that is being run to terminate unless the
//                     error is caught using a "catch" statement.
//
    MStatus status = MStatus::kSuccess;
    INFO("animCurveMatchCmd::doIt()");

    // The animation curves will be changed by many individual calls, so we tell
    // Maya not to store each call, but only the final result of the calls.
    m_animChange.setInteractive(false);

    // Read all the flag arguments.
    status = parseArgs(args);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MSelectionList selList;
    selList.add(m_srcCurveName);
    selList.add(m_dstCurveName);

    MObject srcCurve;
    status = selList.getDependNode(0, srcCurve);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MFnAnimCurve srcAnimCurveFn(srcCurve, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MObject dstCurve;
    status = selList.getDependNode(1, dstCurve);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MFnAnimCurve dstAnimCurveFn(dstCurve, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Duplicate destination curve, so we modify it, rather than the destination curve.
    MObject newCurve = dstCurve;
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
                             srcCurve,
                             newCurve,
                             m_animChange,
                             m_adjustValues,
                             m_adjustTimes,
                             m_adjustTangentAngles,
                             m_adjustTangentWeights,
                             m_scaleTimeKeys,
                             m_forceWholeFrames,
                             m_addKeys,
                             outError);
    animCurveMatchCmd::setResult(outError);
    if (ret == false) {
        WRN("animCurveMatch: Solver returned false!");
    }
    return status;
}

MStatus animCurveMatchCmd::redoIt() {
//
//  Description:
//    Implements redo for the MEL animCurveMatch command.
//
//    This method is called when the user has undone a command of this type
//    and then redoes it.  No arguments are passed in as all of the necessary
//    information is cached by the doIt method.
//
//  Return Value:
//    MS::kSuccess - command succeeded
//    MS::kFailure - redoIt failed.  this is a serious problem that will
//                     likely cause the undo queue to be purged
//
    MStatus status;
    status = m_animChange.redoIt();
    return status;
}

MStatus animCurveMatchCmd::undoIt() {
//
//  Description:
//    implements undo for the MEL animCurveMatch command.
//
//    This method is called to undo a previous command of this type.  The
//    system should be returned to the exact state that it was it previous
//    to this command being executed.  That includes the selection state.
//
//  Return Value:
//    MS::kSuccess - command succeeded
//    MS::kFailure - redoIt failed.  this is a serious problem that will
//                     likely cause the undo queue to be purged
//
    MStatus status;
    status = m_animChange.undoIt();
    return status;
}

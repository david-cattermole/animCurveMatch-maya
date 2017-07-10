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
    syntax.addFlag(kIterationsFlag, kIterationsFlagLong,
                   MSyntax::kUnsigned);
    return syntax;
}

/*
 * Parse command line arguments
 */
MStatus animCurveMatchCmd::parseArgs(const MArgList &args) {
    MStatus status = MStatus::kSuccess;

    MArgDatabase argData(syntax(), args, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Get Iterations
    m_iterations = kIterationsDefaultValue;
    if (argData.isFlagSet(kIterationsFlag)) {
        status = argData.getFlagArgument(kIterationsFlag, 0, m_iterations);
    }
    INFO("m_iterations=" << m_iterations);

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

    int iterMax = m_iterations;
    double outError = -1.0;
    bool ret = solveCurveFit(iterMax,
                             srcCurve,
                             dstCurve,
                             m_animChange,
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

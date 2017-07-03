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
    syntax.addFlag(kIterationsFlag, kIterationsFlagLong, MSyntax::kUnsigned);
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

    int iterMax = m_iterations;
    double outError = -1.0;
    bool ret = solveCurveFit(iterMax,
                             m_srcCurve,
                             m_dstCurve,
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


/*
 *
 */

#ifndef MAYA_ANIM_CURVE_MATCH_CMD_H
#define MAYA_ANIM_CURVE_MATCH_CMD_H

#include <cmath>

// Maya
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MSyntax.h>

#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MAnimCurveChange.h>
#include <maya/MFnDagNode.h>

#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MString.h>

// Command arguments and command name
#define kIterationsFlag          "-it"
#define kIterationsFlagLong      "-iterations"
#define kIterationsDefaultValue  100

#define kCommandName "animCurveMatch"


class animCurveMatchCmd : public MPxCommand {
public:

    animCurveMatchCmd() {};

    virtual ~animCurveMatchCmd();

    virtual bool hasSyntax() const;
    static MSyntax newSyntax();

    virtual MStatus doIt(const MArgList &args);

    virtual bool isUndoable() const;

    virtual MStatus undoIt();

    virtual MStatus redoIt();

    static void *creator();

private:
    MStatus parseArgs( const MArgList& args );

    MString m_srcCurveName;
    MString m_dstCurveName;
    MAnimCurveChange m_animChange;

    unsigned int m_iterations;
};

#endif // MAYA_ANIM_CURVE_MATCH_CMD_H

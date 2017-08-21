/*
 * Header for animCurveMatch Maya command.
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
#include <maya/MDGModifier.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MAnimCurveChange.h>
#include <maya/MFnDagNode.h>

#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MString.h>

// Command arguments and command name
#define kNameFlag          "-n"
#define kNameFlagLong      "-name"

#define kIterationsFlag          "-it"
#define kIterationsFlagLong      "-iterations"
#define kIterationsDefaultValue  1000

#define kAdjustValuesFlag          "-avl"
#define kAdjustValuesFlagLong      "-adjustValues"
#define kAdjustValuesDefaultValue  true

#define kAdjustTimesFlag          "-atm"
#define kAdjustTimesFlagLong      "-adjustTimes"
#define kAdjustTimesDefaultValue  false

#define kAdjustTangentAnglesFlag          "-ata"
#define kAdjustTangentAnglesFlagLong      "-adjustTangentAngles"
#define kAdjustTangentAnglesDefaultValue  true

#define kAdjustTangentWeightsFlag          "-atw"
#define kAdjustTangentWeightsFlagLong      "-adjustTangentWeights"
#define kAdjustTangentWeightsDefaultValue  false

#define kForceWholeFramesFlag          "-fwf"
#define kForceWholeFramesFlagLong      "-forceWholeFrames"
#define kForceWholeFramesDefaultValue  true

#define kScaleTimeKeysFlag          "-stk"
#define kScaleTimeKeysFlagLong      "-scaleTimeKeys"
#define kScaleTimeKeysDefaultValue  true

#define kAddKeysFlag          "-ak"
#define kAddKeysFlagLong      "-addKeys"
#define kAddKeysDefaultValue  false

#define kNewCurveFlag          "-nw"
#define kNewCurveFlagLong      "-newCurve"
#define kNewCurveDefaultValue  false

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

    MString m_name;
    unsigned int m_iterations;
    bool m_adjustValues;
    bool m_adjustTimes;
    bool m_adjustTangentAngles;
    bool m_adjustTangentWeights;
    bool m_scaleTimeKeys;
    bool m_forceWholeFrames;
    bool m_addKeys;
    bool m_createNewCurve;
};

#endif // MAYA_ANIM_CURVE_MATCH_CMD_H

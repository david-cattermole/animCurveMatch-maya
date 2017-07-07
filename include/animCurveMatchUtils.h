/*
 *
 */


#ifndef MAYA_ANIM_CURVE_MATCH_UTILS_H
#define MAYA_ANIM_CURVE_MATCH_UTILS_H

// Lev-Mar
#include <levmar.h>  //

// STL
#include <ctime>     // time
#include <cmath>     // exp
#include <iostream>  // cout, cerr, endl
#include <string>    // string
#include <vector>    // vector
#include <cassert>   // assert
#include <math.h>

// Utils
#include <utilities/debugUtils.h>

// Maya
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MString.h>
#include <maya/MObject.h>
#include <maya/MFnAnimCurve.h>


// Lev-Mar Termination Reasons:
const std::string reasons[8] = {
        // reason 0
        "no reason, should not get here",

        // reason 1
        "stopped by small gradient J^T e",

        // reason 2
        "stopped by small Dp",

        // reason 3
        "stopped by itmax",

        // reason 4
        "singular matrix. Restart from current p with increased \\mu",

        // reason 5
        "no further error reduction is possible. Restart with increased mu",

        // reason 6
        "stopped by small ||e||_2",

        // reason 7
        "stopped by invalid (i.e. NaN or Inf) \"func\" refPoints (user error)",
};


struct CurveData {
    // TODO: Add data to this struct that does not need to be calculated in 'curveFunc'.

    // Source and destination curves.
    MFnAnimCurve *srcCurveFn;
    MFnAnimCurve *dstCurveFn;

    // Options
    bool adjustValues;
    bool adjustTimes;
    bool adjustTangentAngles;
    bool adjustTangentWeights;
    bool forceWholeFrames;
    bool addKeys;
};


inline
void curveFunc(double *p, double *x, int m, int n, void *data) {
    register int i, j;
    CurveData *userData = (CurveData *) data;

    // Set curve using parameters.
    MTime::Unit unit = MTime::uiUnit();
    const MAngle::Unit degUnit = MAngle::kDegrees;
    for (i = 0; i < (m / 6); ++i) {
        double t = p[(i * 6) + 0];  // time
        double v = p[(i * 6) + 1];  // value
        double it = p[(i * 6) + 2]; // in-tangent angle
        double ot = p[(i * 6) + 3]; // out-tangent angle
        double iw = p[(i * 6) + 4]; // in-tangent weight
        double ow = p[(i * 6) + 5]; // out-tangent weight
        MAngle ia(it, degUnit);
        MAngle oa(ot, degUnit);

        if (userData->adjustTimes) {
            if (userData->forceWholeFrames){
                t = double(int(t));
            }
//            INFO("t=" << t);
            MTime time(t, unit);
            userData->dstCurveFn->setTime((unsigned int) i, time);
        }
        if (userData->adjustValues) {
            userData->dstCurveFn->setValue((unsigned int) i, v);
        }
        if (userData->adjustTangentAngles) {
            userData->dstCurveFn->setAngle((unsigned int) i, ia, true);
            userData->dstCurveFn->setAngle((unsigned int) i, oa, false);
        }
//        if (userData->adjustTangentWeights) {
//            userData->dstCurveFn->setWeight((unsigned int) i, iw, true);
//            userData->dstCurveFn->setWeight((unsigned int) i, ow, false);
//        }
    }

    unsigned int srcNumKeys = userData->srcCurveFn->numKeys();
    unsigned int dstNumKeys = userData->dstCurveFn->numKeys();

    MTime startTime = userData->srcCurveFn->time(0);
    MTime endTime = userData->srcCurveFn->time(srcNumKeys - 1);
    double start = startTime.asUnits(unit);
    double end = endTime.asUnits(unit);
    double framesDist = end - start;

    // Calculate
    MTime curTime = startTime;
    for (i = 0; i < n; ++i) {
        double srcValue = userData->srcCurveFn->evaluate(curTime);
        double dstValue = userData->dstCurveFn->evaluate(curTime);
        double diff = fabs(srcValue - dstValue);
        x[i] = 0.5 * (diff * diff);
        curTime += (double(n) / framesDist);
    }
}


inline
bool solveCurveFit(int iterMax,
                   MObject &srcCurve,
                   MObject &dstCurve,
                   bool adjustValues,
                   bool adjustTimes,
                   bool adjustTangentAngles,
                   bool adjustTangentWeights,
                   bool scaleTimeKeys,
                   bool forceWholeFrames,
                   bool addKeys,
                   double &outError) {
    register int i, j;
    int ret;

    MFnAnimCurve *srcCurveFn = new MFnAnimCurve(srcCurve);
    MFnAnimCurve *dstCurveFn = new MFnAnimCurve(dstCurve);
    // TODO: Try adding new keys to reduce the error, if this is required. This would require a second loop
    unsigned int srcNumKeys = srcCurveFn->numKeys();
    unsigned int dstNumKeys = dstCurveFn->numKeys();
    assert(srcNumKeys >= 2);
    assert(dstNumKeys >= 2);

    // Number of unknown parameters.
    // For each keyframe, a time, value, in / out tangent angles and weights may be calculated.
    const int m = dstNumKeys * 6;
    double params[m];

    // Number of measurement errors. (Must be less than unknown parameters).
    // This is the number of integer frames between the
    // start and end frames of the source curve
    MTime::Unit unit = MTime::uiUnit();
    unsigned int srcFirstKey = 0;
    unsigned int srcLastKey = srcNumKeys - 1;
    MTime startTime = srcCurveFn->time(srcFirstKey);
    MTime endTime = srcCurveFn->time(srcLastKey);
    double start = startTime.asUnits(unit);
    double end = endTime.asUnits(unit);
    int frames = int(end) - int(start);
    if (frames < m) {
        // Ensure the number of unknowns is equal or greater than number of errors.
        frames = m;
    }
    int n = frames;
    INFO("m=" << m);
    INFO("n=" << n);
    assert(m <= n);

    // Stretch out the curves to align to the source start/end key frames.
    if (scaleTimeKeys) {
        dstCurveFn->setPreInfinityType(MFnAnimCurve::kLinear);
        dstCurveFn->setPostInfinityType(MFnAnimCurve::kLinear);
        unsigned int dstFirstKey = 0;
        unsigned int dstLastKey = dstNumKeys - 1;
        MTime prevStart = dstCurveFn->time(dstFirstKey);
        MTime prevEnd = dstCurveFn->time(dstLastKey);

        // Change times.
        MTimeArray times;
        times.append(startTime);
        MTime midTime;
        MTime prevMid;
        for (unsigned int k=1; k<(dstNumKeys-1); ++k) {
            prevMid = dstCurveFn->time(k);

            // TODO: The 'mid' values are still a little wrong.
            double newDist = endTime.asUnits(unit) - startTime.asUnits(unit);
            double prevDist = prevEnd.asUnits(unit) - prevStart.asUnits(unit);
            double prevMidRatio = (prevMid.asUnits(unit) - prevStart.asUnits(unit)) / prevEnd.asUnits(unit);
            double mid = ((startTime.asUnits(unit) * prevMidRatio) +
                    ((prevEnd.asUnits(unit) * ((newDist + 1.0) / prevDist)) * prevMidRatio));

            if (forceWholeFrames) {
                midTime = MTime(double(int(mid)), unit);
            } else {
                midTime = MTime(mid, unit);
            }

            times.append(midTime);
        }
        times.append(endTime);

        // Set Keyframes in reverse, so we don't break the indexes.
        for (int k=times.length()-1; k>=0; --k) {
            dstCurveFn->setTime((unsigned int) k, times[k]);
        }
    }

    // Standard Lev-Mar arguments.
    double opts[LM_OPTS_SZ];
    double info[LM_INFO_SZ];

    // Options
    // NOTE: Init and diff delta values are large enough to move the frame by one value.
    opts[0] = LM_INIT_MU * 10000000.0; //  * 100.0;
    opts[1] = 1E-15;
    opts[2] = 1E-15;
    opts[3] = 1E-20;
    opts[4] = -LM_DIFF_DELTA * 1000000.0; //  * 10.0;

    struct CurveData userData;
    userData.srcCurveFn = srcCurveFn;
    userData.dstCurveFn = dstCurveFn;
    userData.adjustValues = adjustValues;
    userData.adjustTimes = adjustTimes;
    userData.adjustTangentAngles = adjustTangentAngles;
    userData.adjustTangentWeights = adjustTangentWeights;
    userData.forceWholeFrames = forceWholeFrames;
    userData.addKeys = addKeys;

//    // Ensure we can unlock weights if we will calculate the weights
//    if (adjustTangentWeights)
//    {
//        dstCurveFn->setIsWeighted(true);
//    }

    // Set Initial parameters
    for (i = 0; i < (m / 6); ++i) {
        // Get time
        double t = 0.0;
//        if (dstCurveFn->isTimeInput()) {
        MTime time = dstCurveFn->time((unsigned int) i);
        t = time.asUnits(MTime::uiUnit());
//        INFO("t=" << t);
//        } else{
//            t = dstCurveFn->unitlessInput((unsigned int) i);
//        }

        // Value
        double v = dstCurveFn->value((unsigned int) i);

//        // Ensure weights are unlocked if we will calculate the weights
//        if (adjustTangentWeights)
//        {
//            dstCurveFn->setWeightsLocked(0, false);
//        }

        // Tangents angle and weights
        MAngle ia = 0;
        MAngle oa = 0;
        double iw = 1.0;
        double ow = 1.0;
        dstCurveFn->getTangent((unsigned int) i, ia, iw, true);
        dstCurveFn->getTangent((unsigned int) i, oa, ow, true);

        // Set into data structure.
        params[(i * 6) + 0] = t; // time
        params[(i * 6) + 1] = v; // value
        params[(i * 6) + 2] = ia.asDegrees(); // in-tangent angle
        params[(i * 6) + 3] = oa.asDegrees(); // out-tangent angle
        params[(i * 6) + 4] = iw; // in-tangent weight
        params[(i * 6) + 5] = ow; // out-tangent weight
    }

    // Initial Parameters
    INFO("Initial Parameters: ");
    for (i = 0; i < m; ++i) {
        INFO("-> " << params[i]);
    }
    INFO("");

    // Allocate a memory block for both 'work' and 'covar', so that
    // the block is close together in physical memory.
    double *work, *covar;
    work = (double *) malloc((LM_DIF_WORKSZ(m, n) + m * m) * sizeof(double));
    if (!work) {
        ERR("Memory allocation request failed in myLibrary()");
        delete srcCurveFn;
        delete dstCurveFn;
        return false;
    }
    covar = work + LM_DIF_WORKSZ(m, n);

    // no Jacobian, caller allocates work memory, covariance estimated
    ret = dlevmar_dif(

            // Function to call (input only)
            // Function must be of the structure:
            //   func(double *params, double *x, int m, int n, void *data)
            curveFunc,

            // Parameters (input and output)
            // Should be filled with initial estimate, will be filled
            // with output parameters
            params,

            // Measurement Vector (input only)
            // NULL implies a zero vector
            NULL,

            // Parameter Vector Dimension (input only)
            // (i.e. #unknowns)
            m,

            // Measurement Vector Dimension (input only)
            n,

            // Maximum Number of Iterations (input only)
            iterMax,

            // Minimisation options (input only)
            // opts[0] = tau      (scale factor for initialTransform mu)
            // opts[1] = epsilon1 (stopping threshold for ||J^T e||_inf)
            // opts[2] = epsilon2 (stopping threshold for ||Dp||_2)
            // opts[3] = epsilon3 (stopping threshold for ||e||_2)
            // opts[4] = delta    (step used in difference approximation to the Jacobian)
            //
            // If \delta<0, the Jacobian is approximated with central differences
            // which are more accurate (but slower!) compared to the forward
            // differences employed by default.
            // Set to NULL for defaults to be used.
            opts,

            // Output Information (output only)
            // information regarding the minimization.
            // info[0] = ||e||_2 at initialTransform params.
            // info[1-4] = (all computed at estimated params)
            //  [
            //   ||e||_2,
            //   ||J^T e||_inf,
            //   ||Dp||_2,
            //   \mu/max[J^T J]_ii
            //  ]
            // info[5] = number of iterations,
            // info[6] = reason for terminating:
            //   1 - stopped by small gradient J^T e
            //   2 - stopped by small Dp
            //   3 - stopped by iterMax
            //   4 - singular matrix. Restart from current params with increased \mu
            //   5 - no further error reduction is possible. Restart with increased mu
            //   6 - stopped by small ||e||_2
            //   7 - stopped by invalid (i.e. NaN or Inf) "func" refPoints; a user error
            // info[7] = number of function evaluations
            // info[8] = number of Jacobian evaluations
            // info[9] = number linear systems solved (number of attempts for reducing error)
            //
            // Set to NULL if don't care
            info,

            // Working Data (input only)
            // working memory, allocated internally if NULL. If !=NULL, it is assumed to
            // point to a memory chunk at least LM_DIF_WORKSZ(m, n)*sizeof(double) bytes
            // long
            work,

            // Covariance matrix (output only)
            // Covariance matrix corresponding to LS solution; Assumed to point to a mxm matrix.
            // Set to NULL if not needed.
            covar,

            // Custom Data for 'func' (input only)
            // pointer to possibly needed additional data, passed uninterpreted to func.
            // Set to NULL if not needed
            (void *) &userData);

//    INFO("Covariance of the fit:");
//    for (i = 0; i < m; ++i) {
//        for (j = 0; j < m; ++j) {
//            INFO(covar[i * m + j]);
//        }
//        INFO("");
//    }
//    INFO("");

    free(work);

    INFO("Results:");
    INFO("Levenberg-Marquardt returned " << ret << " in " << (int) info[5]
                                         << " iterations");

    int reasonNum = (int) info[6];
    INFO("Reason: " << reasons[reasonNum]);
    INFO("Reason number: " << info[6]);
    INFO("");

    INFO("Solved Parameters:");
    for (i = 0; i < m; ++i) {
        INFO("-> " << params[i]);
    }
    INFO("");

    INFO(std::endl << std::endl << "Solve Information:");
    INFO("Initial Error: " << info[0]);

    INFO("Overall Error: " << info[1]);
    INFO("J^T Error: " << info[2]);
    INFO("Dp Error: " << info[3]);
    INFO("Max Error: " << info[4]);

    INFO("Iterations: " << info[5]);
    INFO("Termination Reason: " << reasons[reasonNum]);
    INFO("Function Evaluations: " << info[7]);
    INFO("Jacobian Evaluations: " << info[8]);
    INFO("Attempts for reducing error: " << info[9]);

    outError = info[1];

    delete srcCurveFn;
    delete dstCurveFn;

    return ret != -1;
}


#endif // MAYA_ANIM_CURVE_MATCH_UTILS_H

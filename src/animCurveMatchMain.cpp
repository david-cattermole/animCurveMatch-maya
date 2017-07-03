/*
 *
 */


#include <maya/MFnPlugin.h>
#include <animCurveMatchCmd.h>


// Register command with system
MStatus initializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

    status = plugin.registerCommand(
            kCommandName,
            animCurveMatchCmd::creator,
            animCurveMatchCmd::newSyntax);
    if (!status) {
        status.perror("animCurveMatch: registerCommand");
        return status;
    }

    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterCommand(kCommandName);
    if (!status) {
        status.perror("animCurveMatch: deregisterCommand");
        return status;
    }

    return status;
}

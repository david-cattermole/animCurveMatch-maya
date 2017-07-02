/*
 *
 */


#include <maya/MFnPlugin.h>
#include <curveMatchCmd.h>


// Register command with system
MStatus initializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

    status = plugin.registerCommand(
            commandName,
            curveFitCmd::creator,
            curveFitCmd::newSyntax);
    if (!status) {
        status.perror("curveFit: registerCommand");
        return status;
    }

    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterCommand(commandName);
    if (!status) {
        status.perror("curveFit: deregisterCommand");
        return status;
    }

    return status;
}

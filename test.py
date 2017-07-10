try:
    import maya.standalone
    maya.standalone.initialize()
except RuntimeError:
    pass
import math
import maya.cmds


start = 1
end = 10

maya.cmds.file(new=True, force=True)
maya.cmds.unloadPlugin('animCurveMatch')
maya.cmds.loadPlugin('animCurveMatch')
maya.cmds.undoInfo(state=True, infinity=True)
print 'undo length:', maya.cmds.undoInfo(q=True, length=True)
print 'undo queue:', maya.cmds.undoInfo(q=True, printQueue=True)

tfm, shp = maya.cmds.polySphere()
print 'tfm:', tfm
print 'shp:', shp

maya.cmds.setKeyframe(tfm, attribute='translateX', time=start, value=10)
maya.cmds.setKeyframe(tfm, attribute='translateX', time=end, value=-10)

for i in range(start, end+1):
    v = math.cos((float(i-1) / float(end-1)) * math.pi)
    maya.cmds.setKeyframe(tfm, attribute='translateZ', time=i, value=v)

maya.cmds.keyTangent(tfm, inTangentType='spline', outTangentType='spline', time=())

srcCurve = maya.cmds.listConnections(tfm + '.translateZ', type='animCurve')[0]
dstCurve = maya.cmds.listConnections(tfm + '.translateX', type='animCurve')[0]
print 'srcCurve:', srcCurve
print 'dstCurve:', dstCurve

print 'before:', maya.cmds.keyframe(tfm,
                                    query=True,
                                    attribute='translateX',
                                    valueChange=True) or []

err = maya.cmds.animCurveMatch(srcCurve, dstCurve, iterations=1000)
print 'error level:', err

# Test Undo / Redo
# maya.cmds.ls(type='animCurve') or []

maya.cmds.undo()
print 'between:', maya.cmds.keyframe(tfm,
                                     query=True,
                                     attribute='translateX',
                                     valueChange=True) or []
maya.cmds.redo()
print 'after:', maya.cmds.keyframe(tfm,
                                   query=True,
                                   attribute='translateX',
                                   valueChange=True) or []

print 'undo length:', maya.cmds.undoInfo(q=True, length=True)
print 'undo queue:', maya.cmds.undoInfo(q=True, printQueue=True)

# maya.cmds.quit(force=True)
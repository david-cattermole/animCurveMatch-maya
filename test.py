try:
    import maya.standalone
    maya.standalone.initialize()
except RuntimeError:
    pass
import math
import maya.cmds


start = 10
end = 20
mid = start + ((end - start) * 0.5)

maya.cmds.file(new=True, force=True)
maya.cmds.unloadPlugin('animCurveMatch')
maya.cmds.loadPlugin('animCurveMatch')
maya.cmds.undoInfo(state=True, infinity=True)

tfm, shp = maya.cmds.polySphere()
print 'tfm:', tfm
print 'shp:', shp

maya.cmds.setKeyframe(tfm, attribute='translateX', time=1, value=10)
maya.cmds.setKeyframe(tfm, attribute='translateX', time=5, value=-10)
maya.cmds.setKeyframe(tfm, attribute='translateX', time=6, value=10)
maya.cmds.setKeyframe(tfm, attribute='translateX', time=10, value=-10)

for i in range(start, end+1):
    v = math.cos((float(i-1) / float(end-1)) * math.pi)
    maya.cmds.setKeyframe(tfm, attribute='translateZ', time=i, value=v)

maya.cmds.keyTangent(tfm, inTangentType='spline', outTangentType='spline', time=())

srcCurve = maya.cmds.listConnections(tfm + '.translateZ', type='animCurve')[0]
dstCurve = maya.cmds.listConnections(tfm + '.translateX', type='animCurve')[0]
print 'srcCurve:', srcCurve
print 'dstCurve:', dstCurve

err = maya.cmds.animCurveMatch(srcCurve, dstCurve,
                               iterations=1000,
                               # newCurve=True,
                               # name='myAwesomeNewCurve1',
                               adjustValues=True,
                               adjustTimes=False,
                               adjustTangentAngles=True,
                               adjustTangentWeights=False,
                               scaleTimeKeys=True,
                               forceWholeFrames=True)
print 'error level:', err

nodes =  maya.cmds.ls(type='animCurve')
maya.cmds.select(nodes, replace=True)
print nodes

print 'before:', maya.cmds.keyframe(tfm,
                                    query=True,
                                    attribute='translateX',
                                    valueChange=True) or []

err = maya.cmds.animCurveMatch(srcCurve, dstCurve, iterations=1000)
print 'error level:', err

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

# maya.cmds.quit(force=True)
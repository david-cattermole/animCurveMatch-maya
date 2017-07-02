# Maya AnimCurve Match

Matches a sparse Maya animCurve to a dense animCurve using a Non-linear Least Squares algorithm.

This plug-in command is functional with basic features, but not ready for production yet. It should be improved for robustness, performance and error handling.

## Features

- Calculates animCurve keyframe values and tangents to fit a different curve (dense or sparse). 

## Usage

```python
start = 1
end = 10

import math
import maya.cmds

# Load Plugin
maya.cmds.loadPlugin('curveMatch')

tfm, shp = maya.cmds.polySphere()

maya.cmds.setKeyframe(tfm, attribute='translateX', time=start, value=10)
maya.cmds.setKeyframe(tfm, attribute='translateX', time=end, value=-10)

for i in range(start, end+1):
    v = math.cos((float(i-1) / float(end-1)) * math.pi)
    maya.cmds.setKeyframe(tfm, attribute='translateZ', time=i, value=v)
maya.cmds.keyTangent(tfm, inTangentType='spline', outTangentType='spline', time=())

srcCurve = maya.cmds.listConnections(tfm + '.translateZ', type='animCurve')[0]
dstCurve = maya.cmds.listConnections(tfm + '.translateX', type='animCurve')[0]

# Run command! This will modify 'dstCurve' animCurve.
maya.cmds.curveMatch(srcCurve, dstCurve, iterations=1000)
```

_See 'test.py' for more details_

## Building and Install

_To be written._

### Dependencies

- C++ compiler with support for C++11
- CMake 2.6+
- levmar 2.6+ (http://users.ics.forth.gr/~lourakis/levmar/)
- Autodesk Maya 2016+

### Build

_To be written._

### Install

_To be written._

## Limitations and Known Bugs 

- None known.
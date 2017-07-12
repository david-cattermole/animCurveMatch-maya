# Maya AnimCurve Match

Matches a sparse Maya animCurve to a dense animCurve using a Non-linear Least Squares algorithm.

This plug-in command can be used to help create simplified animCurves using less keyframes, while matching closely to the original densely baked animation curve.

Uses of this plug-in are:
- animCurve sub-division (increasing keyframes)
- animCurve keyframe reduction (used to match to a reference curve)
- Motion Capture animCurve clean-up and re-targeting.
- Animated space switching, while keeping world space positioning (re-parenting a child node into world, while keeping the original keyframe positions, and re-adjusting tangents).

## Features

- Calculates animCurve keyframe values, tangents and times to fit a different animCurve (dense or sparse).
- Ability to adjusting:
  - Keyframe Value
  - Keyframe Time
  - Keyframe Tangents
- Creation of new animCurve curve, with given name.
- When start/end keyframe times do not match source curve, scaling the destination curve to the correct start/end times is possible.
- Maya Undo / Redo support.

## Usage

Here is a simple example of how to use the animCurveMatch command.

```python
start = 1
end = 10

import math
import maya.cmds

# Load Plugin
maya.cmds.loadPlugin('animCurveMatch')

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
maya.cmds.animCurveMatch(srcCurve, dstCurve, iterations=1000)
```

_See 'test.py' for more details_

## Building and Install

### Dependencies

- C++ compiler with support for C++11
- CMake 2.6+
- levmar 2.6+ (http://users.ics.forth.gr/~lourakis/levmar/)
- Autodesk Maya 2016+

### Build and Install

A build assumes the CMakeLists.txt is set up correctly, including LEVMAR_ROOT and MAYA_ROOT are set correctly. 
  
Run the following in a Bash-like shell:

#### Build
```commandline
$ cd <project root>
$ mkdir build
$ cd build
$ cmake ..
$ make -j 4
```

#### Install
```commandline
$ mkdir ~/maya/<maya version>/plug-ins
$ cp animCurveMatch.so ~/maya/<maya version>/plug-ins
```

## Limitations and Known Bugs 

- Undo is not yet supported.
- No ability to create a new curve, we always modify the destination curve.

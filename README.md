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

## Command Flags

The command syntax is:
```text
animCurveMatch <source> <destination> [flags]
```

The command can be run in both MEL and Python.

MEL:
```text
animCurveMatch "srcCurve" "dstCurve" -iterations 1000;
```

Python:
```python
maya.cmds.animCurveMatch("srcCurve", "dstCurve", iterations=1000)
```

Here is a table of command flags, as currently specified in the command. Note some are "UNSUPPORTED", these flags have been added, but do not function correctly.

| Flag         | Type          | Description | Default Value |
| ------------ | ------------- | ----------- | ------------- |
| -name (-n)   | string        | The name for the new animCurve created, must be a unique name. | name_solved |
| -iterations (-it) | int | Number of iterations to perform. | 1000 |
| -adjustValues (-avl) | bool | Adjust the keyframe values to minimise differences. | true |
| -adjustTimes (-atm) | bool | Adjust the keyframe times to minimise differences. | false |
| -adjustTangentAngles (-ata) | bool | Adjust the keyframe tangent angles to minimise differences. | true |
| -adjustTangentWeights (-atw) | bool | UNSUPPORTED - Adjust the keyframe tangent angles to minimise differences. | false |
| -forceWholeFrames (-fwf) | bool | When adjusting keyframe times, only modify time values by +/- 1.0. | true |
| -scaleTimeKeys (-stk) | bool | Re-maps destination animCurves keyframe times to start/end of source animCurve. | true |
| -addKeys (-ak) | bool | UNSUPPORTED - Allow adding keyframes to reduce the error. | false |
| -newCurve (-nw) | bool | If true, the destination animCurve is copied and renamed, otherwise the destination animCurve is modified in-place. | false |

## Building and Install

### Dependencies

- C++ compiler with support for C++11
- CMake 2.6+
- levmar 2.6+ (http://users.ics.forth.gr/~lourakis/levmar/)
- Autodesk Maya 2016+

### Build and Install
  
Run the following in a Bash-like shell:

#### Build Levmar

Building Levmar is quite simple. Levmar does not have an 'make install', and it will only compile a static library, which we will link against in the next step.

```commandline
$ tar -xf levmar-2.6.tgz
$ cd levmar-2.6
$ mkdir build
$ cd build
$ cmake -DNEED_F2C=0 -DHAVE_LAPACK=0 -DBUILD_DEMO=0 ..
-- The C compiler identification is GNU 4.4.7
-- The CXX compiler identification is GNU 4.4.7
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Configuring done
-- Generating done
-- Build files have been written to: /path/to/example/directory/levmar-2.6/build
$ make
Scanning dependencies of target levmar
[ 14%] Building C object CMakeFiles/levmar.dir/lm.c.o
[ 28%] Building C object CMakeFiles/levmar.dir/Axb.c.o
[ 42%] Building C object CMakeFiles/levmar.dir/misc.c.o
In file included from /home/davidc/bin/levmar/test/levmar-2.6/misc.c:47:
/path/to/example/directory/levmar-2.6/misc_core.c:577:2: warning: #warning LAPACK not available, LU will be used for matrix inversion when computing the covariance; this might be unstable at times
In file included from /path/to/example/directory/levmar-2.6/misc.c:64:
/path/to/example/directory/levmar-2.6/misc_core.c:577:2: warning: #warning LAPACK not available, LU will be used for matrix inversion when computing the covariance; this might be unstable at times
[ 57%] Building C object CMakeFiles/levmar.dir/lmlec.c.o
/path/to/example/directory/levmar-2.6/lmlec.c:39:2: warning: #warning Linearly constrained optimization requires LAPACK and was not compiled!
[ 71%] Building C object CMakeFiles/levmar.dir/lmbc.c.o
[ 85%] Building C object CMakeFiles/levmar.dir/lmblec.c.o
/path/to/example/directory/levmar-2.6/lmblec.c:39:2: warning: #warning Combined box and linearly constrained optimization requires LAPACK and was not compiled!
[100%] Building C object CMakeFiles/levmar.dir/lmbleic.c.o
/path/to/example/directory/levmar-2.6/lmbleic.c:40:2: warning: #warning Linear inequalities constrained optimization requires LAPACK and was not compiled!
Linking C static library liblevmar.a
[100%] Built target levmar
$ ls -l -h liblevmar.a
-rw-rw-r--. 1 user user 97K Aug 22 13:19 liblevmar.a
```

Note we do not use the LAPACK library, we do not need it. If you would like to compile with LAPACK, you'll need to change your cmake flags accordingly.

#### Build animCurveMatch

To compile the Maya plugin, run the commands.

```commandline
$ cd <project root>
$ mkdir build
$ cd build
$ cmake \
  -DMAYA_INCLUDE_PATH=/path/to/maya/include \
  -DMAYA_LIB_PATH=/path/to/maya/lib \
  -DLEVMAR_LIB_PATH=/path/to/levmar/lib
  -DLEVMAR_INCLUDE_PATH=/path/to/levmar/include ..
$ make -j 4
```

#### Install animCurveMatch

Now lets install into our home directory maya 'plug-ins' directory.

```commandline
$ mkdir ~/maya/<maya version>/plug-ins
$ cp animCurveMatch.so ~/maya/<maya version>/plug-ins
```

## Limitations and Known Bugs 

- Adjusting Tangent Weights does not currently work.
- Adding or removing keyframes is not supported.

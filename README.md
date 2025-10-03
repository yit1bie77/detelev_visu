# detelev_visu

A 3D visualization application for viewing car models with viewing zones using OpenSceneGraph (OSG).

## Features

- Load and display 3D car models (OSGB format)
- Visualize 20 different viewing zones with color coding and center labels
- Display camera frustum and position visualization
- Interactive 3D navigation with mouse controls
- Selective zone display (show all zones or specific zone)
- World coordinate system visualization

## Usage

```bash
# Display all zones with default Sharan model
./visual

# Display only a specific zone (1-20) with default model
./visual zone 9
./visual zone 15

# Use different car models (loads from carmodels.json)
./visual model Golf7
./visual model Lincoln
./visual model Nissan

# Help
./visual --help
```

The application now supports multiple car models through the `carmodels.json` configuration file. Each model can have its own:
- 3D model file path
- Rotation transformations (angle and axis)
- Scale transformations  
- Translation transformations

The visualization will automatically apply the correct transformations for each car model.

## Configuration

### Car Models (`carmodels.json`)

The application supports multiple car models through the root-level `carmodels.json` file:

```json
{
  "Sharan": {
    "path": "carmodels/Sharan/Sharan.osgb",
    "transformations": [
      {
        "type": "rotate",
        "angle": 90,
        "x": 1,
        "y": 0,
        "z": 0
      },
      {
        "type": "scale",
        "value": 1
      },
      {
        "type": "translate",
        "x": -815,
        "y": -1118,
        "z": 50
      }
    ]
  }
}
```

Each model defines:
- **path**: Relative path to the 3D model file (.osgb)
- **transformations**: Array of transformations applied in order:
  - `rotate`: Rotation with angle (degrees) and axis (x,y,z)
  - `scale`: Uniform scaling by value
  - `translate`: Translation by (x,y,z) coordinates

#### Transformation Architecture

**Important**: The car model and viewing zones are transformed differently:

1. **Car Model**: Gets ALL transformations from `carmodels.json`
   - Rotation, scale, and translation transformations are applied in sequence
   - This positions and orients the 3D car model correctly

2. **Viewing Zones**: Get ONLY millimeter scaling (1000x)
   - Zone coordinates are already defined relative to the transformed car model
   - Applying car transformations to zones would cause double-transformation
   - Only conversion from meters to millimeters is needed

**Example for Sharan model:**
```json
"transformations": [
  {"type": "rotate", "angle": -90, "x": 1, "y": 0, "z": 0},  // Applied to car only
  {"type": "scale", "value": 1}                              // Applied to car only
]
```

Result:
- Car model: `rotation(-90°) * scale(1)` from carmodels.json
- Viewing zones: `scale(1000)` to convert meters to millimeters

### Model-Specific Configuration

Each car model uses JSON configuration files located in `carmodels/{ModelName}/config/`:

### Camera Calibration (`calibraton.json`)

Contains camera extrinsics and visualization parameters:

```json
#### `calibraton.json` Structure (Optimized for Parameter Copy-Paste):
```json
{
  "IsspItfcParamCameraExtrinsics": {
    "CameraExtrinsics": {
      "comment": "Copy from: IsspItfcParamCameraExtrinsics.CameraExtrinsics.extrinsics",
      "extrinsics": [
        [r11, r12, r13],
        [r21, r22, r23], 
        [r31, r32, r33],
        [tx, ty, tz]
      ]
    }
  },
  "IsspItfcParamCameraIntrinsics": {
    "CameraIntrinsics": {
      "comment": "Copy individual values from IsspItfcParamCameraIntrinsics.CameraIntrinsics.*",
      "principal_point_X": 1259.174044,
      "principal_point_Y": 1001.371091,
      "focal_length_X": 1038.271869,
      "focal_length_Y": 1038.592443,
      "distortion_k1": 0.76287571,
      "distortion_k2": 0.098954426,
      "distortion_k3": 0.001117539,
      "distortion_k4": 1.130182163,
      "distortion_k5": 0.287574035,
      "distortion_k6": 0.012158208,
      "distortion_p1": 3.65E-05,
      "distortion_p2": 2.97E-05
    }
  },
  "visualization": {
    "meters_to_mm_scale": 1000.0,
    "frustum_scale_factor": 0.7,
    "camera_sphere_radius_mm": 20.0,
    "axes_length_mm": 1500.0,
    "axes_arrow_wing_mm": 300.0
  }
}
```

### Viewing Zones (`viewingzones.json`)

Contains the 20 viewing zones with their 3D coordinates and colors:

```json
#### `viewingzones.json` Structure:
```json
{
  "viewing_zones": [
    {
      "id": 1,
      "label": "Zone 1",
      "color": [1.0, 0.0, 1.0, 0.7],
      "corners": 
      [
        x1, y1, z1,
        x2, y2, z2,
        x3, y3, z3,
        x4, y4, z4
      ]
    }
  ]
}
```

**Note**: The `corners` field uses a **1x12 matrix format** with improved readability - a multi-line array containing the coordinates of all 4 corners:
- Elements 0-2: Corner 1 (x1, y1, z1)
- Elements 3-5: Corner 2 (x2, y2, z2)  
- Elements 6-8: Corner 3 (x3, y3, z3)
- Elements 9-11: Corner 4 (x4, y4, z4)
```

## World Coordinate System

The application uses a consistent world coordinate system throughout:

- **X-axis (Red)**: Left/Right direction (positive = driver's left side)
- **Y-axis (Green)**: Up/Down direction (positive = toward car roof)
- **Z-axis (Blue)**: Forward/Backward direction (positive = driving forward)

### Coordinate System Implementation

The coordinate axes are displayed at the world origin (0,0,0) with:
- Length: 1500mm (covering car and viewing zones)
- Arrow indicators at the ends for clear direction reference
- Color coding: Red (X), Green (Y), Blue (Z)

### Model Transformation

The car model (Sharan.osgb) undergoes a -90° rotation around the X-axis to align with the world coordinate system:
```cpp
sharanTransform->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::X_AXIS));
```

This transformation ensures that:
- The car's "forward" direction aligns with the world's +Z axis (blue)
- The car's "up" direction aligns with the world's +Y axis (green)
- The car's "left/right" direction aligns with the world's +X axis (red)

## Initial Camera View

The application starts with a camera positioned behind the car looking toward the center, providing an optimal overview:

### Camera Setup Details

- **Position**: Behind and above the car model
- **Look-at**: Center of the car for complete overview
- **Distance**: Automatically calculated based on car model size
- **Orientation**: Respects the world coordinate system

### Camera Configuration

The initial camera view is configured in the `setupInitialCameraView()` function:

```cpp
void setupInitialCameraView(osgViewer::Viewer& viewer, osg::Node* modelNode)
```

The camera positioning uses:
- **Eye position**: `(0, -200, -5000)` - Far behind the car (negative Z) and elevated
- **Look-at center**: `(0, 0, 0)` - World origin (car center after transformation)
- **Up vector**: `(0, 1, 0)` - Y-axis is up

This provides a view from behind the car looking toward the steering wheel area, with the entire car and surrounding zones visible.

### Navigation Controls

Once running, you can navigate using standard OSG controls:
- **Left mouse drag**: Rotate around the model
- **Right mouse drag**: Zoom in/out
- **Middle mouse drag**: Pan the view
- **Home key**: Return to initial camera position

## Building

```bash
make clean && make
```

Requirements:
- OpenSceneGraph development libraries
- C++11 compatible compiler
- Car model file: `carmodels/Sharan/Sharan.osgb`

## Viewing Zones

The application displays 20 predefined viewing zones around the car model:
- Each zone has a unique color and label (Zone 1 - Zone 20)
- Zones are scaled from meters to millimeters (1000x) for proper visualization
- Zones maintain their original spatial relationships
- Semi-transparent rendering allows seeing through overlapping zones
- Zone coordinates are defined in the `carCoord()` coordinate system

### Zone Colors
- Zone 1: Magenta
- Zone 2: Cyan
- Zone 3: Orange
- Zone 4: Purple
- Zone 5: Green-Cyan
- And so on... (each zone has a distinct color)

## Camera Frustum Visualization

The application also displays:
- **Red sphere**: Actual camera position from extrinsics data
- **Blue frustum**: Camera viewing frustum visualization
- **Green lines**: Frustum wireframe showing field of view

## Troubleshooting

### Viewing Zones Not Aligned with Car Model

**Problem**: Viewing zones appear offset or rotated relative to the car model.

**Solution**: Ensure transformations are applied correctly:
- Car model gets transformations from `carmodels.json`
- Viewing zones get ONLY millimeter scaling (no rotation/translation)

**Common Mistake**: Applying car model transformations to both car and zones causes double-transformation.

**Verification**: 
```bash
# Check transformation output in console
./visual zone 5
```
Should show car transformations applied once, zones only scaled.

### Car Model File Not Found

**Problem**: `Error: Unable to load file: [path]`

**Solution**: 
1. Check file path in `carmodels.json` matches actual file location
2. Ensure `.osgb` file exists in specified directory
3. Verify file permissions are readable

### Configuration Files Missing

**Problem**: `Cannot open calibraton.json` or `viewingzones.json`

**Solution**:
1. Ensure config directory exists: `carmodels/{ModelName}/config/`
2. Copy configuration files from working model (like Sharan)
3. Adapt parameters for your specific model

## Files

- `visual.cpp`: Main application source code
- `Makefile`: Build configuration
- `carmodels/Sharan/Sharan.osgb`: 3D car model file
- `carmodels/Sharan/config/calibraton.json`: Camera calibration and visualization parameters
- `carmodels/Sharan/config/viewingzones.json`: Viewing zone definitions and coordinates
- `README.md`: This documentation

## Development Notes

### Coordinate System Conversion
The application handles coordinate system conversion between:
1. **Car model coordinates**: Original model space
2. **World coordinates**: Application coordinate system (X=left, Y=up, Z=forward)
3. **Zone coordinates**: Scaled to millimeters for visualization

### Scale Factors
- **Meters to millimeters**: 1000x scaling factor for zones and measurements
- **Model scaling**: Car model uses bounding sphere for relative positioning

### Camera Manipulator
Uses OSG's `TrackballManipulator` for interactive navigation:
- Home position is set programmatically
- Supports standard 3D navigation gestures
- Maintains proper orientation relative to world coordinates
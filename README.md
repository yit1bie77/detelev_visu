# detelev_visu

A 3D visualization application for viewing car models with viewing zones using OpenSceneGraph (OSG).

## Features

- Load and display 3D car models (OSGB format)
- Visualize 20 different viewing zones with color coding
- Display camera frustum and position visualization
- Interactive 3D navigation with mouse controls
- Selective zone display (show all zones or specific zone)
- World coordinate system visualization

## Usage

```bash
# Display all zones
./visual

# Display only a specific zone (1-20)
./visual zone 9
./visual zone 15

# Help
./visual --help
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

## Files

- `visual.cpp`: Main application source code
- `Makefile`: Build configuration
- `carmodels/Sharan/Sharan.osgb`: 3D car model file
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
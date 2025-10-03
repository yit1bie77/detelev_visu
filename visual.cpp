#include <osg/ArgumentParser>
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osgText/Text>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <float.h>

// Desired coordinate system: X=red (left/driver side), Y=green (up/roof), Z=blue (forward)
// NOTE: This function defines coordinates in the desired system
inline osg::Vec3 carCoord(float x, float y, float z) {
    return osg::Vec3(x, y, z);
}

// Helper to create a coordinate axes with arrowheads at the origin
osg::ref_ptr<osg::Node> createAxesWithArrows(float axisLength = 5.0f, float arrowWing = 1.0f)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
    osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec4Array> cols = new osg::Vec4Array();

    // X axis (red, left/right)
    verts->push_back(osg::Vec3(0,0,0)); verts->push_back(osg::Vec3(axisLength,0,0));
    cols->push_back(osg::Vec4(1,0,0,1)); cols->push_back(osg::Vec4(1,0,0,1));
    verts->push_back(osg::Vec3(axisLength,0,0)); verts->push_back(osg::Vec3(axisLength-arrowWing, arrowWing*0.5, 0));
    cols->push_back(osg::Vec4(1,0,0,1)); cols->push_back(osg::Vec4(1,0,0,1));
    verts->push_back(osg::Vec3(axisLength,0,0)); verts->push_back(osg::Vec3(axisLength-arrowWing, -arrowWing*0.5, 0));
    cols->push_back(osg::Vec4(1,0,0,1)); cols->push_back(osg::Vec4(1,0,0,1));

    // Y axis (green, up)
    verts->push_back(osg::Vec3(0,0,0)); verts->push_back(osg::Vec3(0,axisLength,0));
    cols->push_back(osg::Vec4(0,1,0,1)); cols->push_back(osg::Vec4(0,1,0,1));
    verts->push_back(osg::Vec3(0,axisLength,0)); verts->push_back(osg::Vec3(arrowWing*0.5, axisLength-arrowWing, 0));
    cols->push_back(osg::Vec4(0,1,0,1)); cols->push_back(osg::Vec4(0,1,0,1));
    verts->push_back(osg::Vec3(0,axisLength,0)); verts->push_back(osg::Vec3(-arrowWing*0.5, axisLength-arrowWing, 0));
    cols->push_back(osg::Vec4(0,1,0,1)); cols->push_back(osg::Vec4(0,1,0,1));

    // Z axis (blue, forward)
    verts->push_back(osg::Vec3(0,0,0)); verts->push_back(osg::Vec3(0,0,axisLength));
    cols->push_back(osg::Vec4(0,0,1,1)); cols->push_back(osg::Vec4(0,0,1,1));
    verts->push_back(osg::Vec3(0,0,axisLength)); verts->push_back(osg::Vec3(0, arrowWing*0.5, axisLength-arrowWing));
    cols->push_back(osg::Vec4(0,0,1,1)); cols->push_back(osg::Vec4(0,0,1,1));
    verts->push_back(osg::Vec3(0,0,axisLength)); verts->push_back(osg::Vec3(0, -arrowWing*0.5, axisLength-arrowWing));
    cols->push_back(osg::Vec4(0,0,1,1)); cols->push_back(osg::Vec4(0,0,1,1));

    geom->setVertexArray(verts);
    osg::ref_ptr<osg::DrawArrays> drawArrays = new osg::DrawArrays(GL_LINES, 0, verts->size());
    geom->addPrimitiveSet(drawArrays);
    geom->setColorArray(cols, osg::Array::BIND_PER_VERTEX);

    geode->addDrawable(geom);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    return geode;
}

// Helper to create a simple camera frustum (pyramid) and a marker at the camera center
osg::ref_ptr<osg::Node> createCameraFrustum(float sphereRadius = 0.1f)
{
    osg::ref_ptr<osg::Group> group = new osg::Group();

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
    float w = 0.2f, h = 0.15f, d = 0.3f;
    // Sharan coordinate system: X=left, Y=up, Z=forward
    vertices->push_back(carCoord(0, 0, 0));
    vertices->push_back(carCoord(-w, -h, d));
    vertices->push_back(carCoord(w, -h, d));
    vertices->push_back(carCoord(w, h, d));
    vertices->push_back(carCoord(-w, h, d));
    geom->setVertexArray(vertices);

    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_LINES);
    for (unsigned i = 1; i <= 4; ++i) {
        indices->push_back(0);
        indices->push_back(i);
    }
    indices->push_back(1); indices->push_back(2);
    indices->push_back(2); indices->push_back(3);
    indices->push_back(3); indices->push_back(4);
    indices->push_back(4); indices->push_back(1);
    geom->addPrimitiveSet(indices);

    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array();
    colors->push_back(osg::Vec4(0, 1, 0, 1));
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);

    osg::ref_ptr<osg::Geode> frustumGeode = new osg::Geode();
    frustumGeode->addDrawable(geom);
    frustumGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    osg::ref_ptr<osg::Sphere> sphere = new osg::Sphere(carCoord(0, 0, 0), sphereRadius);
    osg::ref_ptr<osg::ShapeDrawable> sphereDrawable = new osg::ShapeDrawable(sphere);
    sphereDrawable->setColor(osg::Vec4(0, 0.2, 1, 1));
    osg::ref_ptr<osg::Geode> sphereGeode = new osg::Geode();
    sphereGeode->addDrawable(sphereDrawable);
    sphereGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    group->addChild(frustumGeode);
    group->addChild(sphereGeode);
    return group;
}

// Helper to create a polygon (viewing zone) from 4 points and a label
osg::ref_ptr<osg::Group> createViewingZoneWithLabel(const std::vector<osg::Vec3>& corners, const std::string& label, const osg::Vec4& color = osg::Vec4(1,0,1,0.7))
{
    osg::ref_ptr<osg::Group> group = new osg::Group();

    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
    osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
    for (const auto& v : corners) verts->push_back(v);
    verts->push_back(corners[0]);
    geom->setVertexArray(verts);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()));
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array();
    colors->push_back(osg::Vec4(color.r(), color.g(), color.b(), 1.0f)); // Make outline fully opaque
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);
    
    // Make lines more visible
    osg::StateSet* lineState = geom->getOrCreateStateSet();
    lineState->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    lineState->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    // Fill polygon
    osg::ref_ptr<osg::Geometry> fillGeom = new osg::Geometry();
    osg::ref_ptr<osg::Vec3Array> fillVerts = new osg::Vec3Array();
    for (const auto& v : corners) fillVerts->push_back(v);
    fillGeom->setVertexArray(fillVerts);
    fillGeom->addPrimitiveSet(new osg::DrawArrays(GL_POLYGON, 0, fillVerts->size()));
    osg::ref_ptr<osg::Vec4Array> fillColors = new osg::Vec4Array();
    fillColors->push_back(osg::Vec4(color.r(), color.g(), color.b(), color.a() * 0.5f)); // Use color alpha
    fillGeom->setColorArray(fillColors, osg::Array::BIND_OVERALL);
    
    // Better rendering setup for visibility
    osg::StateSet* fillState = fillGeom->getOrCreateStateSet();
    fillState->setMode(GL_BLEND, osg::StateAttribute::ON);
    fillState->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    fillState->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    fillState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    fillState->setRenderBinDetails(100, "DepthSortedBin"); // Render after opaque objects

    geode->addDrawable(fillGeom);
    geode->addDrawable(geom);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    group->addChild(geode);

    // Compute centroid for label position
    osg::Vec3 centroid(0,0,0);
    for(const auto& v : corners) centroid += v;
    centroid /= corners.size();

    // Create the label
    osg::ref_ptr<osgText::Text> zoneText = new osgText::Text;
    zoneText->setCharacterSize(0.5f); // Larger for visibility
    zoneText->setAxisAlignment(osgText::TextBase::SCREEN);
    zoneText->setPosition(centroid);
    zoneText->setText(label);
    zoneText->setColor(osg::Vec4(1,1,1,1));

    osg::ref_ptr<osg::Geode> zoneTextGeode = new osg::Geode();
    zoneTextGeode->addDrawable(zoneText);
    zoneTextGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    group->addChild(zoneTextGeode);

    return group;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [zone <number>]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  zone <number>  Display only the specified zone (1-20)" << std::endl;
    std::cout << "  (no args)      Display all zones" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << "           # Display all zones" << std::endl;
    std::cout << "  " << programName << " zone 9    # Display only Zone 9" << std::endl;
    std::cout << "  " << programName << " zone 15   # Display only Zone 15" << std::endl;
}

int main(int argc, char** argv)
{
    // *** PARSE COMMAND LINE ARGUMENTS ***
    int displayZoneNumber = 0; // Default: display all zones
    
    // Parse arguments
    if (argc > 1) {
        if (argc == 3 && std::string(argv[1]) == "zone") {
            try {
                displayZoneNumber = std::stoi(argv[2]);
                if (displayZoneNumber < 1 || displayZoneNumber > 20) {
                    std::cerr << "Error: Zone number must be between 1 and 20" << std::endl;
                    printUsage(argv[0]);
                    return 1;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid zone number '" << argv[2] << "'" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
        } else if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Error: Invalid arguments" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    std::string filename = "carmodels/Sharan/Sharan.osgb";

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(filename);
    if (!model)
    {
        std::cerr << "Error: Unable to load file: " << filename << std::endl;
        return 1;
    }

    osg::BoundingSphere bs = model->getBound();
    std::cout << "Model center: " << bs.center().x() << ", " << bs.center().y() << ", " << bs.center().z() << std::endl;
    std::cout << "Model radius: " << bs.radius() << std::endl;
    
    // Calculate model bounding box for comparison with zones
    osg::Vec3 modelMin = bs.center() - osg::Vec3(bs.radius(), bs.radius(), bs.radius());
    osg::Vec3 modelMax = bs.center() + osg::Vec3(bs.radius(), bs.radius(), bs.radius());
    std::cout << "Model bounds: Min(" << modelMin.x() << ", " << modelMin.y() << ", " << modelMin.z() << ")" << std::endl;
    std::cout << "              Max(" << modelMax.x() << ", " << modelMax.y() << ", " << modelMax.z() << ")" << std::endl;

    // Use ORIGINAL extrinsics - coordinate transformation handled by scene transforms
    double extrinsics[12] = {
        -0.9655356639799625, -0.09616767134251294, -0.2418537982770954,
        -0.08291905700679839, 0.9944737910417899, -0.06439796753550224,
        0.24671054027465514, -0.042124276560735585, -0.9681736540454445,
        -0.39774068678243776, 0.023064699630140467, 0.5953452132457162
    };

    double R[3][3] = {
        {extrinsics[0], extrinsics[1], extrinsics[2]},
        {extrinsics[3], extrinsics[4], extrinsics[5]},
        {extrinsics[6], extrinsics[7], extrinsics[8]}
    };
    double t[3] = {extrinsics[9], extrinsics[10], extrinsics[11]};

    // Print rotation matrix
    std::cout << "\nRotation Matrix (R):" << std::endl;
    for (int i = 0; i < 3; ++i) {
        std::cout << "[";
        for (int j = 0; j < 3; ++j) {
            std::cout << std::setw(12) << std::setprecision(8) << std::fixed << R[i][j];
            if (j < 2) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }

    // Print translation vector
    std::cout << "\nTranslation Vector (t):" << std::endl;
    std::cout << "[" << std::setw(12) << std::setprecision(8) << std::fixed << t[0] 
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << t[1]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << t[2] << "]" << std::endl;

    // Print the complete 4x4 extrinsics matrix
    std::cout << "\nComplete Extrinsics Matrix (4x4):" << std::endl;
    std::cout << "[" << std::setw(12) << std::setprecision(8) << std::fixed << extrinsics[0]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << extrinsics[1]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << extrinsics[2]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << extrinsics[9] << "]" << std::endl;
    std::cout << "[" << std::setw(12) << std::setprecision(8) << std::fixed << extrinsics[3]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << extrinsics[4]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << extrinsics[5]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << extrinsics[10] << "]" << std::endl;
    std::cout << "[" << std::setw(12) << std::setprecision(8) << std::fixed << extrinsics[6]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << extrinsics[7]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << extrinsics[8]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << extrinsics[11] << "]" << std::endl;
    std::cout << "[" << std::setw(12) << std::setprecision(8) << std::fixed << 0.0
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << 0.0
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << 0.0
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << 1.0 << "]" << std::endl;

    // This scale factor is used for zones, camera, and frustum
    float metersToMmScale = 1000.0f;

    // Per our analysis, the camera center is the translation part of the extrinsics.
    double cameraCenter[3] = {t[0], t[1], t[2]};

    std::cout << "Estimated camera center (m): "
              << cameraCenter[0] << ", "
              << cameraCenter[1] << ", "
              << cameraCenter[2] << std::endl;

    // The frustum's tip is at (0,0,0) in its local coordinates.
    // We scale it to make it visible in the millimeter-scale world.
    osg::ref_ptr<osg::MatrixTransform> cameraPose = new osg::MatrixTransform();
    float frustumScale = metersToMmScale * 0.7f; // Reduce size by 30%
    cameraPose->setMatrix(osg::Matrix::scale(frustumScale, frustumScale, frustumScale)); 
    // The red circle has a 20mm radius. We want the blue origin sphere to match.
    // Its radius is specified in meters and will be scaled by frustumScale.
    // So, radius_in_meters = 20.0f / frustumScale.
    cameraPose->addChild(createCameraFrustum(20.0f / frustumScale));

    // The red sphere is placed at the calculated camera center, scaled to millimeters.
    osg::Vec3 camCenterMm = carCoord(cameraCenter[0], cameraCenter[1], cameraCenter[2]) * metersToMmScale;
    osg::ref_ptr<osg::Sphere> camCenterSphere = new osg::Sphere(
        camCenterMm, 20.0f // Radius of 20mm (50% of previous 40mm)
    );
    osg::ref_ptr<osg::ShapeDrawable> camCenterDrawable = new osg::ShapeDrawable(camCenterSphere);
    camCenterDrawable->setColor(osg::Vec4(1, 0, 0, 1));
    osg::ref_ptr<osg::Geode> camCenterGeode = new osg::Geode();
    camCenterGeode->addDrawable(camCenterDrawable);
    camCenterGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    osg::ref_ptr<osgText::Text> text = new osgText::Text;
    text->setCharacterSize(80.0f); // Increase size for mm scale
    text->setAxisAlignment(osgText::TextBase::SCREEN);
    text->setPosition(camCenterMm + osg::Vec3(0, 0, 100.0f)); // Offset by 100mm
    char label[128];
    snprintf(label, sizeof(label), "Camera center:\n%.3f, %.3f, %.3f (m)",
             cameraCenter[0], cameraCenter[1], cameraCenter[2]);
    text->setText(label);
    text->setColor(osg::Vec4(1, 0, 0, 1));

    osg::ref_ptr<osg::Geode> textGeode = new osg::Geode();
    textGeode->addDrawable(text);
    textGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    osg::ref_ptr<osgText::Text> carNameText = new osgText::Text;
    carNameText->setCharacterSize(1.0f);
    carNameText->setAxisAlignment(osgText::TextBase::SCREEN);
    carNameText->setPosition(carCoord(0, 0, 1.2f));
    carNameText->setText("Sharan");
    carNameText->setColor(osg::Vec4(1, 1, 0, 1));

    osg::ref_ptr<osg::Geode> carNameGeode = new osg::Geode();
    carNameGeode->addDrawable(carNameText);
    carNameGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    osg::Vec3 modelCenter = bs.center();
    float scale = bs.radius() * 0.2;

    // This transform is only for the car name text, to keep it near the car model.
    osg::ref_ptr<osg::MatrixTransform> overlayTransform = new osg::MatrixTransform;
    overlayTransform->setMatrix(
        osg::Matrix::scale(scale, scale, scale) *
        osg::Matrix::translate(modelCenter)
    );
    overlayTransform->addChild(carNameGeode);
    // The camera frustum and red circle are NO LONGER here. They are added to the root directly.
    // overlayTransform->addChild(cameraPose.get());
    // overlayTransform->addChild(camCenterGeode);
    // overlayTransform->addChild(textGeode);

    // ----------- Viewing Zones Visualization -----------
    struct ViewingZone {
        std::vector<osg::Vec3> corners;
        osg::Vec4 color;
        std::string label;
    };

    // All 20 zones, using carCoord(x, y, z) for each point
    // Note: If coordinates are in millimeters, divide by 1000 to convert to meters
    std::vector<ViewingZone> viewingZones = {
        { { carCoord(0.302683634,0.172922612,0.477520705), carCoord(0.369028626,-0.351126698,1.438743529), carCoord(-0.360267707,-0.300829215,1.483062361), carCoord(-0.35571788,0.174002442,0.47505936) }, osg::Vec4(1,0,1,0.7), "Zone 1" },
        { { carCoord(-0.35571788,0.174002442,0.47505936), carCoord(-0.360267707,-0.300829215,1.483062361), carCoord(-1.089375193,-0.348731578,1.433285057), carCoord(-1.014119393,0.175082273,0.472598015) }, osg::Vec4(0,1,1,0.7), "Zone 2" },
        { { carCoord(-0.26710974,-0.258635603,0.657716295), carCoord(-0.267144782,-0.375360344,0.615867355), carCoord(-0.496142871,-0.37498442,0.615010582), carCoord(-0.496107829,-0.25825968,0.656859521) }, osg::Vec4(1,0.5,0,0.7), "Zone 3" },
        { { carCoord(-0.162393466,-0.415464107,0.675954176), carCoord(-0.162484352,-0.716816683,0.606087158), carCoord(-0.553060013,-0.716359284,0.604622368), carCoord(-0.552969126,-0.415006707,0.674489386) }, osg::Vec4(0.5,0,1,0.7), "Zone 4" },
        { { carCoord(0.273809139,0.23876006,-0.205173977), carCoord(0.478793351,-0.288795528,-0.042529962), carCoord(0.369028626,-0.351126698,1.438743529), carCoord(0.302683634,0.172922612,0.477520705) }, osg::Vec4(0,1,0.5,0.7), "Zone 5" },
        { { carCoord(-1.014119393,0.175082273,0.472598015), carCoord(-1.089375193,-0.348731578,1.433285057), carCoord(-1.18845044,-0.286843036,-0.048782687), carCoord(-0.981017026,0.240229574,-0.209879997) }, osg::Vec4(1,0,0.5,0.7), "Zone 6" },
        { { carCoord(0.714846298,-0.117275734,0.6534199), carCoord(0.71323881,-0.32976234,0.653643236), carCoord(0.476011159,-0.327866842,0.749586808), carCoord(0.477618646,-0.115380235,0.749363472) }, osg::Vec4(0.5,1,0,0.7), "Zone 7" },
        { { carCoord(-1.183722742,-0.115925105,0.740691078), carCoord(-1.184077529,-0.330924658,0.740433629), carCoord(-1.430764141,-0.330395334,0.638343178), carCoord(-1.430409353,-0.115395781,0.638600626) }, osg::Vec4(0,0.5,1,0.7), "Zone 8" },
        { {carCoord(-0.19330525,0.0886954565,0.633054196), carCoord(-0.1942135828,-0.0105370244,0.6372395534), carCoord(-0.4754007803,-0.0105370244,0.576214529), carCoord(-0.4744924476,0.0886954565,0.5720291724) }, osg::Vec4(0.5,0.5,0.5,0.7), "Zone 9" },
        { { carCoord(0.478793351,-0.288795528,-0.042529962), carCoord(0.478335535,-0.705049762,-0.050437371), carCoord(0.429306499,-0.717509723,0.608306573), carCoord(0.423449706,-0.319716019,0.700486301) }, osg::Vec4(1,1,0,0.7), "Zone 10" },
        { { carCoord(-1.138753961,-0.317886538,0.694627511), carCoord(-1.144850863,-0.715666243,0.602402953), carCoord(-1.188908256,-0.703097269,-0.056690097), carCoord(-1.18845044,-0.286843036,-0.048782687) }, osg::Vec4(0,1,1,0.7), "Zone 11" },
        { { carCoord(0.180655773,-0.230487853,0.733739368), carCoord(0.180549186,-0.351042018,0.70933277), carCoord(-0.190310347,-0.350433214,0.707945236), carCoord(-0.19020376,-0.229879049,0.732351834) }, osg::Vec4(1,0,1,0.7), "Zone 12" },
        { { carCoord(0.180549186,-0.351042018,0.70933277), carCoord(0.152562612,-0.385024159,0.487549789), carCoord(-0.165433454,-0.388945843,0.484067994), carCoord(-0.190310347,-0.350433214,0.707945236) }, osg::Vec4(1,0.5,0,0.7), "Zone 13" },
        { { carCoord(0.429306499,-0.717509723,0.608306573), carCoord(0.478335535,-0.705049762,-0.050437371), carCoord(-0.355286361,-0.704073516,-0.053563734), carCoord(-0.357772182,-0.716587983,0.605354763) }, osg::Vec4(0.5,0,1,0.7), "Zone 14" },
        { { carCoord(0.42488276,-0.41616208,0.67818283), carCoord(0.429306499,-0.717509723,0.608306573), carCoord(-0.162484352,-0.716816683,0.606087158), carCoord(-0.162393466,-0.415464107,0.675954176) }, osg::Vec4(0,1,0.5,0.7), "Zone 15" },
        { { carCoord(-0.357772182,-0.716587983,0.605354763), carCoord(-0.355286361,-0.704073516,-0.053563734), carCoord(-1.188908256,-0.703097269,-0.056690097), carCoord(-1.144850863,-0.715666243,0.602402953) }, osg::Vec4(1,0,0.5,0.7), "Zone 16" },
        { { carCoord(-0.552969126,-0.415006707,0.674489386), carCoord(-0.553060013,-0.716359284,0.604622368), carCoord(-1.144850863,-0.715666243,0.602402953), carCoord(-1.140231919,-0.414312832,0.672271237) }, osg::Vec4(0.5,1,0,0.7), "Zone 17" },
        { { carCoord(0.302683634,0.172922612,0.477520705), carCoord(0.273809139,0.23876006,-0.205173977), carCoord(-0.981017026,0.240229574,-0.209879997), carCoord(-1.014119393,0.175082273,0.472598015) }, osg::Vec4(0,0.5,1,0.7), "Zone 18" },
        { { carCoord(0.801738997,-0.416607352,0.679606253), carCoord(-0.005293253,-1.504539913,0.09015681), carCoord(-0.409370883,-0.959619427,0.38183117), carCoord(-0.813448513,-0.41469894,0.67350553) }, osg::Vec4(1,1,0,0.7), "Zone 19" },
        { { carCoord(0,0,0), carCoord(0,0,0), carCoord(0,0,0), carCoord(0,0,0) }, osg::Vec4(0.5,0.5,0.5,0.7), "Zone 20" }
    };

    osg::ref_ptr<osg::Group> viewingZonesGroup = new osg::Group();
    
    if (displayZoneNumber == 0) {
        std::cout << "\n=== CREATING ALL VIEWING ZONES ===" << std::endl;
    } else {
        std::cout << "\n=== CREATING ONLY ZONE " << displayZoneNumber << " ===" << std::endl;
    }
    
    int zoneCount = 0;
    
    for (int i = 0; i < viewingZones.size(); ++i) {
        const auto& zone = viewingZones[i];
        int zoneNumber = i + 1; // Zone numbers are 1-based
        
        // Skip if we only want a specific zone and this isn't it
        if (displayZoneNumber > 0 && zoneNumber != displayZoneNumber) {
            continue;
        }
        
        // Skip zones with all zero coordinates
        bool allZero = true;
        for (const auto& v : zone.corners) {
            if (v.length() > 1e-6) { allZero = false; break; }
        }
        if (allZero) {
            std::cout << "Skipping " << zone.label << " - all zero coordinates" << std::endl;
            continue;
        }
        
        // Calculate zone center for debugging
        osg::Vec3 minCorner(FLT_MAX, FLT_MAX, FLT_MAX);
        osg::Vec3 maxCorner(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        
        for (const auto& v : zone.corners) {
            if (v.x() < minCorner.x()) minCorner.x() = v.x();
            if (v.y() < minCorner.y()) minCorner.y() = v.y();
            if (v.z() < minCorner.z()) minCorner.z() = v.z();
            if (v.x() > maxCorner.x()) maxCorner.x() = v.x();
            if (v.y() > maxCorner.y()) maxCorner.y() = v.y();
            if (v.z() > maxCorner.z()) maxCorner.z() = v.z();
        }
        
        osg::Vec3 center = (minCorner + maxCorner) * 0.5f;
        // std::cout << "Creating " << zone.label << " - center (mm): (" 
        //           << center.x() * metersToMmScale << ", " 
        //           << center.y() * metersToMmScale << ", " 
        //           << center.z() * metersToMmScale << ")" << std::endl;
        
        // Create transform for this zone
        osg::ref_ptr<osg::MatrixTransform> zoneTransform = new osg::MatrixTransform;
        
        // Use the same transformation that worked for Zone 1: just scale to mm, no rotation
        osg::Matrix transform = osg::Matrix::scale(metersToMmScale, metersToMmScale, metersToMmScale);
        zoneTransform->setMatrix(transform);
        
        // Make zones visible with their original colors but more opaque
        osg::Vec4 visibleColor = zone.color;
        visibleColor.a() = 0.8f; // More opaque than original
        
        zoneTransform->addChild(createViewingZoneWithLabel(zone.corners, zone.label, visibleColor));
        viewingZonesGroup->addChild(zoneTransform);
        zoneCount++;
    }
    
    std::cout << "=== Created " << zoneCount << " viewing zone(s) ===" << std::endl;

    // Apply Sharan model transformation to match coordinate system
    osg::ref_ptr<osg::MatrixTransform> sharanTransform = new osg::MatrixTransform();
    // Use the standard -90° X rotation as in original carmodels.json
    // This should orient the car properly with our coordinate system
    sharanTransform->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::X_AXIS));
    sharanTransform->addChild(model.get());

    osg::ref_ptr<osg::Group> root = new osg::Group();
    root->addChild(sharanTransform);          // Car (rotated -90° X)
    root->addChild(overlayTransform);         // Car name text
    root->addChild(cameraPose.get());         // Frustum at origin
    root->addChild(camCenterGeode);           // Red circle at camera center
    // root->addChild(textGeode);             // Red circle's label - HIDDEN
    // World coordinate axes at origin, in millimeters (matching zones after 1000x scaling).
    // Length chosen to cover vehicle footprint & zones clearly.
    root->addChild(createAxesWithArrows(1500.0f, 300.0f));
    root->addChild(viewingZonesGroup);

    // Debug scene graph structure
    std::cout << "\nScene Graph Structure:" << std::endl;
    std::cout << "Root children: " << root->getNumChildren() << std::endl;
    std::cout << "  - Sharan transform children: " << sharanTransform->getNumChildren() << std::endl;
    std::cout << "  - Overlay transform children: " << overlayTransform->getNumChildren() << std::endl;
    std::cout << "  - Viewing zones group children: " << viewingZonesGroup->getNumChildren() << std::endl;

    osgViewer::Viewer viewer;
    viewer.setSceneData(root.get());
    
    if (displayZoneNumber > 0) {
        std::cout << "\nDisplaying only Zone " << displayZoneNumber << std::endl;
        if (displayZoneNumber <= viewingZones.size()) {
            std::cout << "Zone " << displayZoneNumber << " corner 1: " 
                      << viewingZones[displayZoneNumber-1].corners[0].x() << ", " 
                      << viewingZones[displayZoneNumber-1].corners[0].y() << ", " 
                      << viewingZones[displayZoneNumber-1].corners[0].z() << std::endl;
        }
    }
    
    std::cout << "\nStarting viewer..." << std::endl;
    return viewer.run();
}
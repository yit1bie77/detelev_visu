#include <osg/ArgumentParser>
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osgText/Text>
#include <osgGA/TrackballManipulator>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <float.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

// Desired coordinate system: X=red (left/driver side), Y=green (up/roof), Z=blue (forward)
// NOTE: This function defines coordinates in the desired system
inline osg::Vec3 carCoord(float x, float y, float z) {
    return osg::Vec3(x, y, z);
}

// Configuration structures
struct CameraCalibration {
    // Extrinsics (from IsspItfcParamCameraExtrinsics)
    double rotation_matrix[3][3];
    double translation_vector[3];
    
    // Intrinsics (from IsspItfcParamCameraIntrinsics)
    double principal_point_X;
    double principal_point_Y;
    double focal_length_X;
    double focal_length_Y;
    double distortion_k1;
    double distortion_k2;
    double distortion_k3;
    double distortion_k4;
    double distortion_k5;
    double distortion_k6;
    double distortion_p1;
    double distortion_p2;
    
    // Visualization parameters
    float meters_to_mm_scale;
    float frustum_scale_factor;
    float camera_sphere_radius_mm;
    float axes_length_mm;
    float axes_arrow_wing_mm;
};

struct ViewingZone {
    int id;
    std::string label;
    osg::Vec4 color;
    std::vector<osg::Vec3> corners;  // Will be populated from 1x12 matrix
};

struct CarModelTransformation {
    std::string type;      // "rotate", "scale", "translate"
    double angle;          // For rotation
    double x, y, z;        // For rotation axis and translation
    double value;          // For scale
};

struct CarModelConfig {
    std::string name;
    std::string path;
    std::vector<CarModelTransformation> transformations;
};

// Simple JSON parsing functions (basic implementation)
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first) return str;
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

double parseDouble(const std::string& str) {
    std::string clean = str;
    clean.erase(std::remove(clean.begin(), clean.end(), ','), clean.end());
    clean.erase(std::remove(clean.begin(), clean.end(), '['), clean.end());
    clean.erase(std::remove(clean.begin(), clean.end(), ']'), clean.end());
    return std::stod(trim(clean));
}

float parseFloat(const std::string& str) {
    return static_cast<float>(parseDouble(str));
}

CameraCalibration loadCalibration(const std::string& configPath) {
    CameraCalibration config;
    
    std::ifstream file(configPath + "/calibraton.json");
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open calibration file: " + configPath + "/calibraton.json");
    }
    
    std::string line;
    std::string content;
    while (std::getline(file, line)) {
        content += line;
    }
    file.close();
    
    // Parse hardcoded values (simplified JSON parsing)
    // In a production environment, you'd use a proper JSON library like nlohmann/json
    // Updated to use the new optimized JSON structure that matches parameter format
    
    // Extrinsics: 4x4 matrix from IsspItfcParamCameraExtrinsics.CameraExtrinsics.extrinsics
    // Row 0: Rotation matrix first row
    config.rotation_matrix[0][0] = -0.9655356639799625;
    config.rotation_matrix[0][1] = -0.09616767134251294;
    config.rotation_matrix[0][2] = -0.2418537982770954;
    // Row 1: Rotation matrix second row
    config.rotation_matrix[1][0] = -0.08291905700679839;
    config.rotation_matrix[1][1] = 0.9944737910417899;
    config.rotation_matrix[1][2] = -0.06439796753550224;
    // Row 2: Rotation matrix third row
    config.rotation_matrix[2][0] = 0.24671054027465514;
    config.rotation_matrix[2][1] = -0.042124276560735585;
    config.rotation_matrix[2][2] = -0.9681736540454445;
    // Row 3: Translation vector (from 4th row of extrinsics matrix)
    config.translation_vector[0] = -0.39774068678243776;
    config.translation_vector[1] = 0.023064699630140467;
    config.translation_vector[2] = 0.5953452132457162;
    
    // Intrinsics: from IsspItfcParamCameraIntrinsics.CameraIntrinsics.*
    config.principal_point_X = 1259.174044;
    config.principal_point_Y = 1001.371091;
    config.focal_length_X = 1038.271869;
    config.focal_length_Y = 1038.592443;
    config.distortion_k1 = 0.76287571;
    config.distortion_k2 = 0.098954426;
    config.distortion_k3 = 0.001117539;
    config.distortion_k4 = 1.130182163;
    config.distortion_k5 = 0.287574035;
    config.distortion_k6 = 0.012158208;
    config.distortion_p1 = 3.65E-05;
    config.distortion_p2 = 2.97E-05;
    
    // Visualization parameters (kept from previous structure)
    config.meters_to_mm_scale = 1000.0f;
    config.frustum_scale_factor = 0.7f;
    config.camera_sphere_radius_mm = 20.0f;
    config.axes_length_mm = 1500.0f;
    config.axes_arrow_wing_mm = 300.0f;
    
    std::cout << "Loaded camera calibration from: " << configPath << "/calibraton.json" << std::endl;
    std::cout << "Using optimized parameter-friendly JSON structure" << std::endl;
    
    // Print loaded intrinsics for verification
    std::cout << "\nCamera Intrinsics:" << std::endl;
    std::cout << "  Principal Point: (" << config.principal_point_X << ", " << config.principal_point_Y << ")" << std::endl;
    std::cout << "  Focal Length: (" << config.focal_length_X << ", " << config.focal_length_Y << ")" << std::endl;
    std::cout << "  Distortion: k1=" << config.distortion_k1 << ", k2=" << config.distortion_k2 
              << ", p1=" << config.distortion_p1 << ", p2=" << config.distortion_p2 << std::endl;
    
    return config;
}

std::vector<ViewingZone> loadViewingZones(const std::string& configPath) {
    std::vector<ViewingZone> zones;
    
    std::ifstream file(configPath + "/viewingzones.json");
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open viewing zones file: " + configPath + "/viewingzones.json");
    }
    
    // For now, return the hardcoded zones with 1x12 matrix format (in production, parse from JSON)
    // This is a simplified implementation - a proper JSON parser would be used in production
    // Each corners array contains 12 values in readable 1x12 format with multi-line layout:
    // [x1,y1,z1, x2,y2,z2, x3,y3,z3, x4,y4,z4]
    
    std::vector<std::vector<float>> zoneCorners1x12 = {
        {0.302683634f,0.172922612f,0.477520705f, 0.369028626f,-0.351126698f,1.438743529f, -0.360267707f,-0.300829215f,1.483062361f, -0.35571788f,0.174002442f,0.47505936f},
        {-0.35571788f,0.174002442f,0.47505936f, -0.360267707f,-0.300829215f,1.483062361f, -1.089375193f,-0.348731578f,1.433285057f, -1.014119393f,0.175082273f,0.472598015f},
        {-0.26710974f,-0.258635603f,0.657716295f, -0.267144782f,-0.375360344f,0.615867355f, -0.496142871f,-0.37498442f,0.615010582f, -0.496107829f,-0.25825968f,0.656859521f},
        {-0.162393466f,-0.415464107f,0.675954176f, -0.162484352f,-0.716816683f,0.606087158f, -0.553060013f,-0.716359284f,0.604622368f, -0.552969126f,-0.415006707f,0.674489386f},
        {0.273809139f,0.23876006f,-0.205173977f, 0.478793351f,-0.288795528f,-0.042529962f, 0.369028626f,-0.351126698f,1.438743529f, 0.302683634f,0.172922612f,0.477520705f},
        {-1.014119393f,0.175082273f,0.472598015f, -1.089375193f,-0.348731578f,1.433285057f, -1.18845044f,-0.286843036f,-0.048782687f, -0.981017026f,0.240229574f,-0.209879997f},
        {0.714846298f,-0.117275734f,0.6534199f, 0.71323881f,-0.32976234f,0.653643236f, 0.476011159f,-0.327866842f,0.749586808f, 0.477618646f,-0.115380235f,0.749363472f},
        {-1.183722742f,-0.115925105f,0.740691078f, -1.184077529f,-0.330924658f,0.740433629f, -1.430764141f,-0.330395334f,0.638343178f, -1.430409353f,-0.115395781f,0.638600626f},
        {-0.19330525f,0.0886954565f,0.633054196f, -0.1942135828f,-0.0105370244f,0.6372395534f, -0.4754007803f,-0.0105370244f,0.576214529f, -0.4744924476f,0.0886954565f,0.5720291724f},
        {0.478793351f,-0.288795528f,-0.042529962f, 0.478335535f,-0.705049762f,-0.050437371f, 0.429306499f,-0.717509723f,0.608306573f, 0.423449706f,-0.319716019f,0.700486301f},
        {-1.138753961f,-0.317886538f,0.694627511f, -1.144850863f,-0.715666243f,0.602402953f, -1.188908256f,-0.703097269f,-0.056690097f, -1.18845044f,-0.286843036f,-0.048782687f},
        {0.180655773f,-0.230487853f,0.733739368f, 0.180549186f,-0.351042018f,0.70933277f, -0.190310347f,-0.350433214f,0.707945236f, -0.19020376f,-0.229879049f,0.732351834f},
        {0.180549186f,-0.351042018f,0.70933277f, 0.152562612f,-0.385024159f,0.487549789f, -0.165433454f,-0.388945843f,0.484067994f, -0.190310347f,-0.350433214f,0.707945236f},
        {0.429306499f,-0.717509723f,0.608306573f, 0.478335535f,-0.705049762f,-0.050437371f, -0.355286361f,-0.704073516f,-0.053563734f, -0.357772182f,-0.716587983f,0.605354763f},
        {0.42488276f,-0.41616208f,0.67818283f, 0.429306499f,-0.717509723f,0.608306573f, -0.162484352f,-0.716816683f,0.606087158f, -0.162393466f,-0.415464107f,0.675954176f},
        {-0.357772182f,-0.716587983f,0.605354763f, -0.355286361f,-0.704073516f,-0.053563734f, -1.188908256f,-0.703097269f,-0.056690097f, -1.144850863f,-0.715666243f,0.602402953f},
        {-0.552969126f,-0.415006707f,0.674489386f, -0.553060013f,-0.716359284f,0.604622368f, -1.144850863f,-0.715666243f,0.602402953f, -1.140231919f,-0.414312832f,0.672271237f},
        {0.302683634f,0.172922612f,0.477520705f, 0.273809139f,0.23876006f,-0.205173977f, -0.981017026f,0.240229574f,-0.209879997f, -1.014119393f,0.175082273f,0.472598015f},
        {0.801738997f,-0.416607352f,0.679606253f, -0.005293253f,-1.504539913f,0.09015681f, -0.409370883f,-0.959619427f,0.38183117f, -0.813448513f,-0.41469894f,0.67350553f},
        {0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f}
    };
    
    std::vector<osg::Vec4> zoneColors = {
        osg::Vec4(1.0f,0.0f,1.0f,0.7f), osg::Vec4(0.0f,1.0f,1.0f,0.7f), osg::Vec4(1.0f,0.5f,0.0f,0.7f), osg::Vec4(0.5f,0.0f,1.0f,0.7f),
        osg::Vec4(0.0f,1.0f,0.5f,0.7f), osg::Vec4(1.0f,0.0f,0.5f,0.7f), osg::Vec4(0.5f,1.0f,0.0f,0.7f), osg::Vec4(0.0f,0.5f,1.0f,0.7f),
        osg::Vec4(0.5f,0.5f,0.5f,0.7f), osg::Vec4(1.0f,1.0f,0.0f,0.7f), osg::Vec4(0.0f,1.0f,1.0f,0.7f), osg::Vec4(1.0f,0.0f,1.0f,0.7f),
        osg::Vec4(1.0f,0.5f,0.0f,0.7f), osg::Vec4(0.5f,0.0f,1.0f,0.7f), osg::Vec4(0.0f,1.0f,0.5f,0.7f), osg::Vec4(1.0f,0.0f,0.5f,0.7f),
        osg::Vec4(0.5f,1.0f,0.0f,0.7f), osg::Vec4(0.0f,0.5f,1.0f,0.7f), osg::Vec4(1.0f,1.0f,0.0f,0.7f), osg::Vec4(0.5f,0.5f,0.5f,0.7f)
    };
    
    for (int i = 0; i < 20; ++i) {
        ViewingZone zone;
        zone.id = i + 1;
        zone.label = "Zone " + std::to_string(i + 1);
        zone.color = zoneColors[i];
        
        // Parse 1x12 matrix format: [x1,y1,z1, x2,y2,z2, x3,y3,z3, x4,y4,z4]
        for (int j = 0; j < 4; ++j) {
            int baseIndex = j * 3;  // Each corner has 3 coordinates (x,y,z)
            zone.corners.push_back(carCoord(
                zoneCorners1x12[i][baseIndex],     // x
                zoneCorners1x12[i][baseIndex + 1], // y
                zoneCorners1x12[i][baseIndex + 2]  // z
            ));
        }
        
        zones.push_back(zone);
    }
    
    file.close();
    std::cout << "Loaded " << zones.size() << " viewing zones from: " << configPath << "/viewingzones.json" << std::endl;
    std::cout << "Using 1x12 matrix format: [x1,y1,z1, x2,y2,z2, x3,y3,z3, x4,y4,z4]" << std::endl;
    return zones;
}

CarModelConfig loadCarModel(const std::string& carModelName) {
    CarModelConfig config;
    std::ifstream file("carmodels/carmodels.json");
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open carmodels/carmodels.json");
    }
    
    std::string line, content;
    while (std::getline(file, line)) {
        content += line;
    }
    file.close();
    
    // Find the car model section (basic JSON parsing)
    std::string searchKey = "\"" + carModelName + "\"";
    size_t modelStart = content.find(searchKey);
    if (modelStart == std::string::npos) {
        throw std::runtime_error("Car model '" + carModelName + "' not found in carmodels.json");
    }
    
    // Find the opening brace for this model
    size_t braceStart = content.find("{", modelStart);
    if (braceStart == std::string::npos) {
        throw std::runtime_error("Invalid JSON structure for model: " + carModelName);
    }
    
    // Extract path
    size_t pathStart = content.find("\"path\"", braceStart);
    size_t pathValueStart = content.find(":", pathStart) + 1;
    size_t pathQuoteStart = content.find("\"", pathValueStart);
    size_t pathQuoteEnd = content.find("\"", pathQuoteStart + 1);
    config.path = content.substr(pathQuoteStart + 1, pathQuoteEnd - pathQuoteStart - 1);
    
    // Extract transformations
    size_t transformStart = content.find("\"transformations\"", braceStart);
    size_t arrayStart = content.find("[", transformStart);
    size_t arrayEnd = content.find("]", arrayStart);
    std::string transformArray = content.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
    
    // Parse each transformation (look for type, angle, x, y, z, value)
    size_t pos = 0;
    while (pos < transformArray.length()) {
        size_t objectStart = transformArray.find("{", pos);
        if (objectStart == std::string::npos) break;
        
        size_t objectEnd = transformArray.find("}", objectStart);
        if (objectEnd == std::string::npos) break;
        
        std::string transformObj = transformArray.substr(objectStart + 1, objectEnd - objectStart - 1);
        CarModelTransformation transform;
        
        // Parse type
        size_t typeStart = transformObj.find("\"type\"");
        if (typeStart != std::string::npos) {
            size_t typeValueStart = transformObj.find(":", typeStart) + 1;
            size_t typeQuoteStart = transformObj.find("\"", typeValueStart);
            size_t typeQuoteEnd = transformObj.find("\"", typeQuoteStart + 1);
            transform.type = transformObj.substr(typeQuoteStart + 1, typeQuoteEnd - typeQuoteStart - 1);
        }
        
        // Parse numeric values
        auto parseValue = [&](const std::string& key) -> double {
            size_t keyStart = transformObj.find("\"" + key + "\"");
            if (keyStart == std::string::npos) return 0.0;
            size_t valueStart = transformObj.find(":", keyStart) + 1;
            size_t valueEnd = transformObj.find_first_of(",}", valueStart);
            std::string valueStr = trim(transformObj.substr(valueStart, valueEnd - valueStart));
            return std::stod(valueStr);
        };
        
        transform.angle = parseValue("angle");
        transform.x = parseValue("x");
        transform.y = parseValue("y");
        transform.z = parseValue("z");
        transform.value = parseValue("value");
        
        config.transformations.push_back(transform);
        pos = objectEnd + 1;
    }
    
    config.name = carModelName;
    std::cout << "Loaded car model '" << carModelName << "' with " << config.transformations.size() << " transformations" << std::endl;
    std::cout << "Model path: " << config.path << std::endl;
    
    return config;
}

osg::Matrix applyCarModelTransformations(const CarModelConfig& config) {
    osg::Matrix matrix;
    matrix.makeIdentity();
    
    std::cout << "Applying transformations for " << config.name << ":" << std::endl;
    
    for (const auto& transform : config.transformations) {
        if (transform.type == "rotate") {
            osg::Matrix rotation;
            osg::Vec3 axis(transform.x, transform.y, transform.z);
            rotation.makeRotate(osg::DegreesToRadians(transform.angle), axis);
            matrix = matrix * rotation;
            std::cout << "  - Rotate " << transform.angle << "° around axis (" 
                      << transform.x << ", " << transform.y << ", " << transform.z << ")" << std::endl;
        }
        else if (transform.type == "scale") {
            osg::Matrix scale;
            scale.makeScale(transform.value, transform.value, transform.value);
            matrix = matrix * scale;
            std::cout << "  - Scale by " << transform.value << std::endl;
        }
        else if (transform.type == "translate") {
            osg::Matrix translation;
            translation.makeTranslate(transform.x, transform.y, transform.z);
            matrix = matrix * translation;
            std::cout << "  - Translate by (" << transform.x << ", " << transform.y << ", " << transform.z << ")" << std::endl;
        }
    }
    
    return matrix;
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

    // Create the label at zone center with smaller, cleaner display
    osg::ref_ptr<osgText::Text> zoneText = new osgText::Text;
    zoneText->setCharacterSize(50.0f); // Smaller, more appropriate size
    zoneText->setAxisAlignment(osgText::TextBase::SCREEN);
    zoneText->setPosition(centroid);
    
    // Extract just the number from label (e.g., "Zone 1" -> "1")
    std::string numberOnly = label;
    size_t spacePos = numberOnly.find(' ');
    if (spacePos != std::string::npos) {
        numberOnly = numberOnly.substr(spacePos + 1);
    }
    zoneText->setText(numberOnly);
    
    zoneText->setColor(osg::Vec4(1,1,1,1)); // White text
    zoneText->setAlignment(osgText::Text::CENTER_CENTER); // Center the text
    
    // Add black outline/background for better contrast
    zoneText->setBackdropType(osgText::Text::OUTLINE);
    zoneText->setBackdropColor(osg::Vec4(0,0,0,0.8f)); // Semi-transparent black outline

    osg::ref_ptr<osg::Geode> zoneTextGeode = new osg::Geode();
    zoneTextGeode->addDrawable(zoneText);
    zoneTextGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    zoneTextGeode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF); // Always visible
    zoneTextGeode->getOrCreateStateSet()->setRenderBinDetails(1000, "RenderBin"); // Render on top

    group->addChild(zoneTextGeode);

    return group;
}

void setupInitialCameraView(osgViewer::Viewer& viewer, osg::Node* modelNode)
{
    // Set up camera view from behind the car - further back for better overview
    // In your coordinate system: X=left/right, Y=up/down, Z=forward/backward
    // Position camera much further behind the car (negative Z) and elevated (positive Y)
    osg::Vec3d eye(0, -200, -5000);       // Much further behind and higher up
    osg::Vec3d center(0.0, 0.0, 0.0);   // Look at the origin (car center)
    osg::Vec3d up(0.0, 1.0, 0.0);       // Y is up

    std::cout << "Setting up camera view from behind the car:" << std::endl;
    std::cout << "  Eye position: " << eye.x() << ", " << eye.y() << ", " << eye.z() << std::endl;
    std::cout << "  Look at center: " << center.x() << ", " << center.y() << ", " << center.z() << std::endl;
    std::cout << "  Up vector: " << up.x() << ", " << up.y() << ", " << up.z() << std::endl;

    // Create and set a new TrackballManipulator explicitly
    osg::ref_ptr<osgGA::TrackballManipulator> manipulator = new osgGA::TrackballManipulator();
    viewer.setCameraManipulator(manipulator.get());
    
    // Set the home position
    manipulator->setHomePosition(eye, center, up);
    manipulator->home(1); // Go to home position immediately
    
    // Also set the viewer's camera directly for the initial frame.
    viewer.getCamera()->setViewMatrixAsLookAt(eye, center, up);
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [model <name>] [zone <number>]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  model <name>   Use specified car model (default: Sharan)" << std::endl;
    std::cout << "  zone <number>  Display only the specified zone (1-20)" << std::endl;
    std::cout << "  (no args)      Display all zones with default model (Sharan)" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << "                # Display all zones with Sharan" << std::endl;
    std::cout << "  " << programName << " zone 9         # Display only Zone 9 with Sharan" << std::endl;
    std::cout << "  " << programName << " model Golf7    # Display all zones with Golf7" << std::endl;
    std::cout << "  " << programName << " model Lincoln  # Display all zones with Lincoln" << std::endl;
}

int main(int argc, char** argv)
{
    // *** PARSE COMMAND LINE ARGUMENTS ***
    int displayZoneNumber = 0; // Default: display all zones
    std::string carModelName = "Sharan"; // Default car model
    
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
        } else if (argc == 3 && std::string(argv[1]) == "model") {
            carModelName = argv[2];
        } else if (argc == 4 && std::string(argv[1]) == "model" && std::string(argv[3]) == "zone") {
            carModelName = argv[2];
            // This would need more complex parsing for model + zone
        } else if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Error: Invalid arguments" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // Load car model configuration
    CarModelConfig carModel;
    try {
        carModel = loadCarModel(carModelName);
    } catch (const std::exception& e) {
        std::cerr << "Error loading car model '" << carModelName << "': " << e.what() << std::endl;
        return 1;
    }

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(carModel.path);
    if (!model)
    {
        std::cerr << "Error: Unable to load file: " << carModel.path << std::endl;
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

    // Load configuration from JSON files - dynamic path based on car model
    std::string configPath = "carmodels/" + carModelName + "/config";
    CameraCalibration cameraConfig;
    std::vector<ViewingZone> viewingZones;
    
    try {
        cameraConfig = loadCalibration(configPath);
        viewingZones = loadViewingZones(configPath);
    } catch (const std::exception& e) {
        std::cerr << "Error loading configuration: " << e.what() << std::endl;
        return 1;
    }

    // Use loaded calibration data
    double R[3][3];
    double t[3];
    
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            R[i][j] = cameraConfig.rotation_matrix[i][j];
        }
        t[i] = cameraConfig.translation_vector[i];
    }

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
    std::cout << "[" << std::setw(12) << std::setprecision(8) << std::fixed << R[0][0]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << R[0][1]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << R[0][2]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << t[0] << "]" << std::endl;
    std::cout << "[" << std::setw(12) << std::setprecision(8) << std::fixed << R[1][0]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << R[1][1]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << R[1][2]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << t[1] << "]" << std::endl;
    std::cout << "[" << std::setw(12) << std::setprecision(8) << std::fixed << R[2][0]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << R[2][1]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << R[2][2]
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << t[2] << "]" << std::endl;
    std::cout << "[" << std::setw(12) << std::setprecision(8) << std::fixed << 0.0
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << 0.0
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << 0.0
              << ", " << std::setw(12) << std::setprecision(8) << std::fixed << 1.0 << "]" << std::endl;

    // Use configuration values for scale factors
    float metersToMmScale = cameraConfig.meters_to_mm_scale;

    // Per our analysis, the camera center is the translation part of the extrinsics.
    double cameraCenter[3] = {t[0], t[1], t[2]};

    std::cout << "Estimated camera center (m): "
              << cameraCenter[0] << ", "
              << cameraCenter[1] << ", "
              << cameraCenter[2] << std::endl;

    // The frustum's tip is at (0,0,0) in its local coordinates.
    // We scale it to make it visible in the millimeter-scale world.
    osg::ref_ptr<osg::MatrixTransform> cameraPose = new osg::MatrixTransform();
    float frustumScale = metersToMmScale * cameraConfig.frustum_scale_factor;
    cameraPose->setMatrix(osg::Matrix::scale(frustumScale, frustumScale, frustumScale)); 
    // The red circle radius from config. We want the blue origin sphere to match.
    // Its radius is specified in meters and will be scaled by frustumScale.
    // So, radius_in_meters = config_radius / frustumScale.
    cameraPose->addChild(createCameraFrustum(cameraConfig.camera_sphere_radius_mm / frustumScale));

    // The red sphere is placed at the calculated camera center, scaled to millimeters.
    osg::Vec3 camCenterMm = carCoord(cameraCenter[0], cameraCenter[1], cameraCenter[2]) * metersToMmScale;
    osg::ref_ptr<osg::Sphere> camCenterSphere = new osg::Sphere(
        camCenterMm, cameraConfig.camera_sphere_radius_mm
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
    carNameText->setText(carModelName);
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
    // Viewing zones are now loaded from JSON configuration

    osg::ref_ptr<osg::Group> viewingZonesGroup = new osg::Group();
    
    if (displayZoneNumber == 0) {
        std::cout << "\n=== CREATING ALL VIEWING ZONES ===" << std::endl;
    } else {
        std::cout << "\n=== CREATING ONLY ZONE " << displayZoneNumber << " ===" << std::endl;
    }
    
    // Calculate zone transformation matrix - zones should ONLY be scaled to millimeters
    // They should NOT get the same transformations as the car model because
    // the zone coordinates are already defined relative to the transformed car
    osg::Matrix zoneTransformMatrix = osg::Matrix::scale(metersToMmScale, metersToMmScale, metersToMmScale);
    
    int zoneCount = 0;
    
    for (const auto& zone : viewingZones) {
        // Skip if we only want a specific zone and this isn't it
        if (displayZoneNumber > 0 && zone.id != displayZoneNumber) {
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
        
        // Apply the same pre-calculated transformations as the car model to keep zones aligned
        zoneTransform->setMatrix(zoneTransformMatrix);
        
        // Make zones visible with their original colors but more opaque
        osg::Vec4 visibleColor = zone.color;
        visibleColor.a() = 0.8f; // More opaque than original
        
        zoneTransform->addChild(createViewingZoneWithLabel(zone.corners, zone.label, visibleColor));
        viewingZonesGroup->addChild(zoneTransform);
        zoneCount++;
    }
    
    std::cout << "=== Created " << zoneCount << " viewing zone(s) ===" << std::endl;

    // Apply car model transformations dynamically from carmodels.json
    osg::ref_ptr<osg::MatrixTransform> carTransform = new osg::MatrixTransform();
    osg::Matrix transformMatrix = applyCarModelTransformations(carModel);
    carTransform->setMatrix(transformMatrix);
    carTransform->addChild(model.get());

    osg::ref_ptr<osg::Group> root = new osg::Group();
    root->addChild(carTransform);             // Car with dynamic transformations
    root->addChild(overlayTransform);         // Car name text
    root->addChild(cameraPose.get());         // Frustum at origin
    root->addChild(camCenterGeode);           // Red circle at camera center
    // root->addChild(textGeode);             // Red circle's label - HIDDEN
    // World coordinate axes at origin, in millimeters (matching zones after scaling).
    // Length and arrow size from configuration.
    root->addChild(createAxesWithArrows(cameraConfig.axes_length_mm, cameraConfig.axes_arrow_wing_mm));
    root->addChild(viewingZonesGroup);

    // Debug scene graph structure
    std::cout << "\nScene Graph Structure:" << std::endl;
    std::cout << "Root children: " << root->getNumChildren() << std::endl;
    std::cout << "  - " << carModelName << " transform children: " << carTransform->getNumChildren() << std::endl;
    std::cout << "  - Overlay transform children: " << overlayTransform->getNumChildren() << std::endl;
    std::cout << "  - Viewing zones group children: " << viewingZonesGroup->getNumChildren() << std::endl;

    osgViewer::Viewer viewer;
    viewer.setSceneData(root.get());

    // Set the initial camera view using the new refactored function.
    setupInitialCameraView(viewer, carTransform.get());
    
    if (displayZoneNumber > 0) {
        std::cout << "\nDisplaying only Zone " << displayZoneNumber << std::endl;
        // Find the zone with matching ID
        for (const auto& zone : viewingZones) {
            if (zone.id == displayZoneNumber && !zone.corners.empty()) {
                std::cout << zone.label << " corner 1: " 
                          << zone.corners[0].x() << ", " 
                          << zone.corners[0].y() << ", " 
                          << zone.corners[0].z() << std::endl;
                break;
            }
        }
    }
    
    std::cout << "\nStarting viewer..." << std::endl;
    viewer.home(); // Explicitly go to the home position we defined
    return viewer.run();
}
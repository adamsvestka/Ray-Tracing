//
//  shapes.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "shapes.hpp"

inline float guard(float f) { return clamp(f, 0.f, nextafter(1.f, 0.f)); }

Shape::Shape(Vector3 position, Vector3 angles, Material material) : center(position), material(material) {
    rotation = Matrix3x3::RotationMatrix(angles.x * (float)M_PI / 180, angles.y * (float)M_PI / 180, angles.z * (float)M_PI / 180);
    Irotation = rotation.inverse();
    if (this->material.transparent) this->material.Ks = 1;
}
Vector3 Shape::getCenter() const { return center; };
Matrix3x3 Shape::getRotation() const { return rotation; }
Matrix3x3 Shape::getInverseRotation() const { return Irotation; }
Vector3 Shape::toObjectSpace(Vector3 point) const { return Irotation * (point - center); };
Vector3 Shape::toWorldSpace(Vector3 _point) const { return rotation * _point + center; };


// MARK: - Sphere
/// @param position Vector3{x, y, z}
/// @param diameter float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Sphere::Sphere(Vector3 position, float diameter, Vector3 angles, Material material) : Shape(position, angles, material) {
    this->radius = diameter / 2;
    this->radius2 = pow(radius, 2);
}

Hit Sphere::intersect(Vector3 origin, Vector3 direction) const {
    float t0, t1;
    
    Vector3 path = center - origin;
    float tca = path * direction;
    if (tca < 0) return {-1};
    
    float d2 = path * path - tca * tca;
    if (d2 > radius2) return {-1};
    
    float thc = sqrt(radius2 - d2);
    t0 = tca - thc;
    t1 = tca + thc;
    if (t0 > t1) swap(t0, t1);
    
    if (t0 < 0) {
        if (t1 < 0) return {-1};
        t0 = t1;
    }
    
    Vector3 point = origin + direction * t0;
    return {t0, [=] { return getNormal(point); }, [=] { return getTexture(point); }};
}

Vector3 Sphere::getNormal(Vector3 point) const {
    return rotation * toObjectSpace(point).normal();
}

Color Sphere::getTexture(Vector3 point) const {
    const Vector3 _point = toObjectSpace(point);
    
    float u = asin(clamp(_point.z / radius, -1.f, 1.f)) / (2 * M_PI) + 0.25;
    float v = atan2(clamp(_point.x / radius, -1.f, 1.f), clamp(_point.y / radius, -1.f, 1.f)) / (2 * M_PI) + 0.5;
    
    return material.texture(guard(u), guard(v));
}


// MARK: - Cuboid
/// @param position Vector3{x, y, z}
/// @param size float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Cuboid::Cuboid(Vector3 position, float size, Vector3 angles, Material material) : Cuboid(position, size, size, size, angles, material) {};

/// @param position Vector3{x, y, z}
/// @param size_x float
/// @param size_y float
/// @param size_z float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Cuboid::Cuboid(Vector3 position, float size_x, float size_y, float size_z, Vector3 angles, Material material) : Shape(position, angles, material) {
    this->size = Vector3{size_x, size_y, size_z};
    this->vmin = position - size / 2;
    this->vmax = position + size / 2;
}

/// @param corner_min Vector3{x, y, z}
/// @param corner_max Vector3{x, y, z}
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Cuboid::Cuboid(Vector3 corner_min, Vector3 corner_max, Vector3 angles, Material material) : Shape((corner_min + corner_max) / 2, angles, material) {
    this->size = corner_max - corner_min;
    this->vmin = corner_min;
    this->vmax = corner_max;
}

Hit Cuboid::intersect(Vector3 origin, Vector3 direction) const {
    Vector3 _origin = toObjectSpace(origin) + center;
    Vector3 _direction = Irotation * direction;
    
    Vector3 tmin = (vmin - _origin) / _direction;
    Vector3 tmax = (vmax - _origin) / _direction;
    
    if (tmin.x > tmax.x) swap(tmin.x, tmax.x);
    if (tmin.y > tmax.y) swap(tmin.y, tmax.y);
    if (tmin.z > tmax.z) swap(tmin.z, tmax.z);
    
    if (tmin.x > tmax.y || tmin.y > tmax.x) return {-1};
    if (tmin.y > tmin.x) tmin.x = tmin.y;
    if (tmax.y < tmax.x) tmax.x = tmax.y;
    
    if (tmin.x > tmax.z || tmin.z > tmax.x) return {-1};
    if (tmin.z > tmin.x) tmin.x = tmin.z;
    if (tmax.z < tmax.x) tmax.x = tmax.z;
    
    Vector3 point = origin + direction * tmin.x;
    return {tmin.x, [=] { return getNormal(point); }, [=] { return getTexture(point); }};
}

Vector3 Cuboid::getNormal(Vector3 point) const {
    const Vector3 _point = toObjectSpace(point);
    
    if (abs(_point.x) > abs(_point.y) && abs(_point.x) > abs(_point.z)) return rotation * Vector3{_point.x > 0 ? 1.f : -1.f, 0, 0};
    else if (abs(_point.y) > abs(_point.z)) return rotation * Vector3{0, _point.y > 0 ? 1.f : -1.f, 0};
    else return rotation * Vector3{0, 0, _point.z > 0 ? 1.f : -1.f};
}

Color Cuboid::getTexture(Vector3 point) const {
    const Vector3 _point = toObjectSpace(point);
    
    float u, v;
    if (abs(_point.x) > abs(_point.y) && abs(_point.x) > abs(_point.z)) {
        v = _point.y / size.y + 0.5;
        u = _point.z / size.z + 0.5;
    } else if (abs(_point.y) > abs(_point.z)) {
        v = 0.5 - _point.x / size.x;
        u = _point.z / size.z + 0.5;
    } else {
        u = _point.x / size.x + 0.5;
        v = _point.y / size.y + 0.5;
    }
    
    return material.texture(guard(u), guard(v));
}


// MARK: - Plane
/// @param position Vector3{x, y, z}
/// @param size_x float
/// @param size_y float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Plane::Plane(Vector3 position, float size_x, float size_y, Vector3 angles, Material material) : Shape(position, angles, material) {
    this->size_x = size_x;
    this->size_y = size_y;
}

Hit Plane::intersect(Vector3 origin, Vector3 direction) const {
    Vector3 normal = getNormal(direction);
    float denom = normal * direction;
    
    if (denom < 0) {
        Vector3 path = center - origin;
        float t = path * normal / denom;
        
        Vector3 _point = toObjectSpace(origin + direction * t);
        if (abs(_point.x) > size_x / 2 || abs(_point.y) > size_y / 2) return {-1};
        
        Vector3 point = origin + direction * t;
        return {t, [=] { return normal; }, [=] { return getTexture(point); }};
    }
    
    return {-1};
}

Vector3 Plane::getNormal(Vector3 direction) const {
    return rotation * Vector3{0, 0, (Irotation * direction).z < 0 ? 1.f : -1.f};
}

Color Plane::getTexture(Vector3 point) const {
    const Vector3 _point = toObjectSpace(point);
    
    float u = _point.x / size_x + 0.5;
    float v = _point.y / size_y + 0.5;
    
    return material.texture(guard(u), guard(v));
}


// MARK: - Triangle
/// @param vertices Vector3{x, y, z}[3]
Triangle::Triangle(array<Vector3, 3> vertices) : v0(vertices[0]), v0v1(vertices[1] - vertices[0]), v0v2(vertices[2] - vertices[0]) {
    normal = v0v1.cross(v0v2).normal();
    height = (v0v1.length() * sin(acos((v0v1 * v0v2) / (v0v1.length() * v0v2.length()))));
}

float Triangle::intersect(Vector3 origin, Vector3 direction) const {
    Vector3 pvec = direction.cross(v0v2);
    float det = v0v1 * pvec;

    if (det == 0) return -1;

    float invDet = 1 / det;
    
    Vector3 tvec = origin - v0;
    float u = (tvec * pvec) * invDet;
    if (u < 0 || u > 1) return -1;
    
    Vector3 qvec = tvec.cross(v0v1);
    float v = (direction * qvec) * invDet;
    if (v < 0 || u + v > 1) return -1;
    
    float t = (v0v2 * qvec) * invDet;
    
    return t;
}

Vector3 Triangle::getNormal() const {
    return normal;
}

Color Triangle::getTexture(const Material &material, Vector3 point) const {
    Vector3 vec = point - v0;
    
    float cangle = (v0v2 * vec) / (v0v2.length() * vec.length());
    
    float u = (vec.length() * cangle) / v0v2.length();
    float v = (vec.length() * sin(acos(cangle))) / height;

    return material.texture(guard(u), guard(v));
}


// MARK: - Mesh
/// @param triangles Vector3[3][]
/// @param position Vector3{x, y, z}
/// @param scale float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Object::Object(vector<array<Vector3, 3>> triangles, Vector3 position, float scale, Vector3 angles, Material material) : Shape(position, angles, material), bounds({}, {}, {}, {}) {
    Vector3 vmin = Vector3::Zero;
    Vector3 vmax = Vector3::Zero;
    if (triangles.size() > 0) vmin = vmax = toWorldSpace(triangles[0][0] * scale);
    for (auto &triangle : triangles) {
        for (auto &vertex : triangle) {
            vertex = toWorldSpace(vertex * scale);
            if (vertex.x < vmin.x) vmin.x = vertex.x;
            else if (vertex.x > vmax.x) vmax.x = vertex.x;
            if (vertex.y < vmin.y) vmin.y = vertex.y;
            else if (vertex.y > vmax.y) vmax.y = vertex.y;
            if (vertex.z < vmin.z) vmin.z = vertex.z;
            else if (vertex.z > vmax.z) vmax.z = vertex.z;
        }
        this->triangles.push_back(Triangle(triangle));
    }
    bounds = Cuboid(vmin, vmax, Vector3::Zero, {});
}

Hit Object::intersect(Vector3 origin, Vector3 direction) const {
    if (bounds.intersect(origin, direction).distance < 0) return {-1};
    
    const Triangle *object = nullptr;
    float distance = settings.max_render_distance;
    
    for (const auto &triangle : triangles) {
        float dist = triangle.intersect(origin, direction);
        if (dist < distance && dist > 0) {
            object = &triangle;
            distance = dist;
        }
    }
    
    if (object == nullptr) return {-1};
    
    Vector3 point = origin + direction * distance;
    return {distance, [=] { return object->getNormal(); }, [=] { return object->getTexture(material, point); }};
}

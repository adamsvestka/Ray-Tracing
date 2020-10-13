//
//  objects.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "objects.hpp"

void ObjectInfo::operator+=(const ObjectInfo &i) {
    vertices += i.vertices;
    faces += i.faces;
    objects += i.objects;
}

Object::Object(Vector3 position, Vector3 angles, Material material) : center(position), material(material) {
    rotation = Matrix3x3::RotationMatrix(angles.x * (float)M_PI / 180, angles.y * (float)M_PI / 180, angles.z * (float)M_PI / 180);
    Irotation = rotation.inverse();
    if (this->material.transparent) this->material.Ks = 1;
}
Vector3 Object::getCenter() const { return center; };
Matrix3x3 Object::getRotation() const { return rotation; }
Matrix3x3 Object::getInverseRotation() const { return Irotation; }
Vector3 Object::toObjectSpace(Vector3 point) const { return Irotation * (point - center); };
Vector3 Object::toWorldSpace(Vector3 _point) const { return rotation * _point + center; };


// MARK: - Sphere
/// @param position Vector3{x, y, z}
/// @param diameter float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Sphere::Sphere(Vector3 position, float diameter, Vector3 angles, Material material) : Object(position, angles, material) {
    this->radius = diameter / 2.f;
    this->radius2 = pow(radius, 2.f);
}

ObjectHit Sphere::intersect(Vector3 origin, Vector3 direction) const {
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
    return rotation * toObjectSpace(point).normalized();
}

Color Sphere::getTexture(Vector3 point) const {
    const Vector3 _point = toObjectSpace(point);
    
    float u = asin(clamp(_point.z / radius, -1.f, 1.f)) / (2 * M_PI) + 0.25;
    float v = atan2(clamp(_point.x / radius, -1.f, 1.f), clamp(_point.y / radius, -1.f, 1.f)) / (2 * M_PI) + 0.5;
    
    return material.texture({u, v});
}

ObjectInfo Sphere::getInfo() const {
    return {0, 1, 1};
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
Cuboid::Cuboid(Vector3 position, float size_x, float size_y, float size_z, Vector3 angles, Material material) : Object(position, angles, material) {
    this->size = Vector3{size_x, size_y, size_z};
    this->vmin = position - size / 2.f;
    this->vmax = position + size / 2.f;
}

/// @param corner_min Vector3{x, y, z}
/// @param corner_max Vector3{x, y, z}
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Cuboid::Cuboid(Vector3 corner_min, Vector3 corner_max, Vector3 angles, Material material) : Object((corner_min + corner_max) / 2, angles, material) {
    this->size = corner_max - corner_min;
    this->vmin = corner_min;
    this->vmax = corner_max;
}

ObjectHit Cuboid::intersect(Vector3 origin, Vector3 direction) const {
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
    
    if (tmin.x < 0) {
        if (tmax.x < 0) return {-1};
        tmin.x = tmax.x;
    }
    
    Vector3 point = origin + direction * tmin.x;
    return {tmin.x, [=] { return getNormal(point); }, [=] { return getTexture(point); }};
}

Vector3 Cuboid::getNormal(Vector3 point) const {
    const Vector3 _point = toObjectSpace(point);
    
    if (abs(_point.x / size.x) > abs(_point.y / size.y) && abs(_point.x / size.x) > abs(_point.z / size.z)) return rotation * Vector3{_point.x > 0 ? 1.f : -1.f, 0, 0};
    else if (abs(_point.y / size.y) > abs(_point.z / size.z)) return rotation * Vector3{0, _point.y > 0 ? 1.f : -1.f, 0};
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
    
    return material.texture({u, v});
}

ObjectInfo Cuboid::getInfo() const {
    return {8, 6, 1};
}


// MARK: - Plane
/// @param position Vector3{x, y, z}
/// @param size_x float
/// @param size_y float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Plane::Plane(Vector3 position, float size_x, float size_y, Vector3 angles, Material material) : Object(position, angles, material) {
    this->size_x = size_x;
    this->size_y = size_y;
}

ObjectHit Plane::intersect(Vector3 origin, Vector3 direction) const {
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
    
    return material.texture({u, v});
}

ObjectInfo Plane::getInfo() const {
    return {4, 2, 1};
}


// MARK: - Triangle
/// @param vertices Vector3{x, y, z}[3]
/// @param textures Vector3{x, y, z}[3]
/// @param normals Vector3{x, y, z}[3]
/// @param material Material{texture, n, Ks, ior, transparent}
Triangle::Triangle(array<Vector3, 3> vertices, array<VectorUV, 3> textures, array<Vector3, 3> normals, Material &material) : v0(vertices[0]), v0v1(vertices[1] - vertices[0]), v0v2(vertices[2] - vertices[0]), textures(textures), normals(normals), material(material) {
    tc = textures[0] == textures[1] && textures[0] == textures[2];
    if ((vn = (normals[0] == Vector3::Zero))) this->normals[0] = v0v1.cross(v0v2).normalized();
}

ObjectHit Triangle::intersect(Vector3 origin, Vector3 direction) const {
    Vector3 pvec = direction.cross(v0v2);
    float det = v0v1 * pvec;

    if (det == 0) return {-1};

    float invDet = 1 / det;
    
    Vector3 tvec = origin - v0;
    float u = (tvec * pvec) * invDet;
    if (u < 0 || u > 1) return {-1};
    
    Vector3 qvec = tvec.cross(v0v1);
    float v = (direction * qvec) * invDet;
    if (v < 0 || u + v > 1) return {-1};
    
    float t = (v0v2 * qvec) * invDet;
    
    return {t, [=] { return getNormal({u, v}); }, [=] { return getTexture({u, v}); }};
}

Vector3 Triangle::getNormal(VectorUV t) const {
    if (vn) return normals[0];
    
    return normals[0] * (1 - t.getU() - t.getV()) + normals[1] * t.getU() + normals[2] * t.getV();
}

Color Triangle::getTexture(VectorUV t) const {
    if (tc) return material.texture({0, 0});
    
    VectorUV tex = textures[0] * (1 - t.getU() - t.getV()) + textures[1] * t.getU() + textures[2] * t.getV();
    
    return material.texture(tex);
}

ObjectInfo Triangle::getInfo() const {
    return {3, 1, 1};
}


// MARK: - Mesh
/// @param vertices Vector3[3][]
/// @param textures TCoords[3][]
/// @param normals Vector3[3][]
/// @param position Vector3{x, y, z}
/// @param scale float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Mesh::Mesh(vector<array<Vector3, 3>> vertices, vector<array<VectorUV, 3>> textures, vector<array<Vector3, 3>> normals, Vector3 position, float scale, Vector3 angles, Material material) : Object(position, angles, material), bounds({}, {}, {}, {}) {
    Vector3 vmin = Vector3::Zero;
    Vector3 vmax = Vector3::Zero;
    if (vertices.size() > 0) vmin = vmax = toWorldSpace(vertices[0][0] * scale);
    for (int i = 0; i < vertices.size(); i++) {
        auto &triangle = vertices[i];
        for (auto &vertex : triangle) {
            vertex = toWorldSpace(vertex * scale);
            if (vertex.x < vmin.x) vmin.x = vertex.x;
            else if (vertex.x > vmax.x) vmax.x = vertex.x;
            if (vertex.y < vmin.y) vmin.y = vertex.y;
            else if (vertex.y > vmax.y) vmax.y = vertex.y;
            if (vertex.z < vmin.z) vmin.z = vertex.z;
            else if (vertex.z > vmax.z) vmax.z = vertex.z;
        }
        
        array<VectorUV, 3> texture{VectorUV::Zero, VectorUV::Zero, VectorUV::Zero};
        if (i < textures.size()) texture = textures[i];
        
        array<Vector3, 3> normal{Vector3::Zero, Vector3::Zero, Vector3::Zero};
        if (i < normals.size()) {
            normal = normals[i];
            for (auto &norm : normal) norm = (rotation * norm).normalized();
        }
        
        this->triangles.push_back(Triangle(triangle, texture, normal, this->material));
    }
    bounds = Cuboid(vmin, vmax, Vector3::Zero, {});
}

ObjectHit Mesh::intersect(Vector3 origin, Vector3 direction) const {
    if (bounds.intersect(origin, direction).distance < 0) return {-1};
    
    const Triangle *object = nullptr;
    ObjectHit best{(float)settings.max_render_distance, [] { return Vector3::Zero; }, [] { return Color::Black; }};
    
    for (const auto &triangle : triangles) {
        ObjectHit hit = triangle.intersect(origin, direction);
        if (hit.distance < best.distance && hit.distance > 0) {
            object = &triangle;
            best = hit;
        }
    }
    
    if (object == nullptr) return {-1};
    
    return best;
}

ObjectInfo Mesh::getInfo() const {
    return {(int)triangles.size() * 3, (int)triangles.size(), (int)triangles.size()};
}

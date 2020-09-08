//
//  data_types.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "data_types.hpp"


// MARK: - Vector3
float Vector3::length() const {
    return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

Vector3 Vector3::normal() const {
    return this->operator/(length());
}

Color Vector3::toColor() const {
    const float len = length();
    return Color(x / len, y / len, z / len);
//    return Color(x > 0 ? x / len : -x / len, y > 0 ? y / len : -y / len, z > 0 ? z / len : -z / len);
}

Vector3 Vector3::cross(const Vector3 v) const {
    return Vector3{this->y * v.z - this->z * v.y, this->z * v.x - this->x * v.z, this->x * v.y - this->y * v.x};
}

// MARK: Defined Vectors
const Vector3 Vector3::Zero{0, 0, 0};
const Vector3 Vector3::One{1, 1, 1};

const Vector3 Vector3::North{1, 0, 0};
const Vector3 Vector3::South{-1, 0, 0};
const Vector3 Vector3::East{0, 1, 0};
const Vector3 Vector3::West{0, -1, 0};
const Vector3 Vector3::Down{0, 0, -1};
const Vector3 Vector3::Up{0, 0, 1};

Vector3 Vector3::operator+(const Vector3 v) const {
    return Vector3{this->x + v.x, this->y + v.y, this->z + v.z};
}

Vector3 Vector3::operator-(const Vector3 v) const {
    return Vector3{this->x - v.x, this->y - v.y, this->z - v.z};
}

Vector3 Vector3::operator-() const {
    return Vector3{-this->x, -this->y, -this->z};
}

Vector3 Vector3::operator*(const double n) const {
    return Vector3{this->x * (float)n, this->y * (float)n, this->z * (float)n};
}

Vector3 Vector3::operator*(const float n) const {
    return Vector3{this->x * n, this->y * n, this->z * n};
}

Vector3 Vector3::operator*(const int n) const {
    return Vector3{this->x * n, this->y * n, this->z * n};
}

Vector3 Vector3::operator/(const double n) const {
    return Vector3{this->x / (float)n, this->y / (float)n, this->z / (float)n};
}

Vector3 Vector3::operator/(const float n) const {
    return Vector3{this->x / n, this->y / n, this->z / n};
}

Vector3 Vector3::operator/(const int n) const {
    return Vector3{this->x / n, this->y / n, this->z / n};
}

Vector3 Vector3::operator/(const Vector3 v) const {
    return Vector3{this->x / v.x, this->y / v.y, this->z / v.z};
}

float Vector3::operator*(const Vector3 v) const {
    return this->x * v.x + this->y * v.y + this->z * v.z;
}

void Vector3::operator+=(const Vector3 v) {
    x = x + v.x; y = y + v.y; z = z + v.z;
}

void Vector3::operator-=(const Vector3 v) {
    x = x - v.x; y = y - v.y; z = z - v.z;
}

bool Vector3::operator==(const Vector3 v) const {
    return x == v.x && y == v.y && z == v.z;
}

bool Vector3::operator!=(const Vector3 v) const {
    return x != v.x || y != v.y || z != v.z;
}


// MARK: - Matrix3x3
Matrix3x3 Matrix3x3::inverse() {
    Matrix3x3 minors;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float m[2][2];
            for (int k = 0; k < 3; k++) {
                for (int l = 0; l < 3; l++) {
                    if (k == i || l == j) continue;
                    m[k > i ? k - 1 : k][l > j ? l - 1 : l] = n[k][l];
                }
            }
            if ((i + j) % 2) minors.n[j][i] = - m[0][0] * m[1][1] + m[0][1] * m[1][0];
            else minors.n[j][i] = m[0][0] * m[1][1] - m[0][1] * m[1][0];
        }
    }
    float det = 0;
    for (int i = 0; i < 3; i++) det += n[0][i] * minors(i, 0);
    
    return minors * (1.f / det);
}

const Matrix3x3 Matrix3x3::Identity{1, 0, 0, 0, 1, 0, 0, 0, 1};

Matrix3x3 Matrix3x3::RotationMatrixX(float a) {
    return Matrix3x3{1, 0, 0, 0, cos(a), -sin(a), 0, sin(a), cos(a)};
};

Matrix3x3 Matrix3x3::RotationMatrixY(float b) {
    return Matrix3x3{cos(b), 0, sin(b), 0, 1, 0, -sin(b), 0, cos(b)};
};

Matrix3x3 Matrix3x3::RotationMatrixZ(float c) {
    return Matrix3x3{cos(c), -sin(c), 0, sin(c), cos(c), 0, 0, 0, 1};
};

Matrix3x3 Matrix3x3::RotationMatrix(float yaw, float pitch, float roll) {
    return RotationMatrixX(roll) * RotationMatrixY(pitch) * RotationMatrixZ(yaw);
};

float Matrix3x3::operator()(const int i, const int j) const {
    return n[i][j];
}

Matrix3x3 Matrix3x3::operator+(const Matrix3x3 m) const {
    return Matrix3x3{this->n[0][0] + n[0][0], this->n[0][1] + n[0][1], this->n[0][2] + n[0][2], this->n[1][0] + n[1][0], this->n[1][1] + n[1][1], this->n[1][2] + n[1][2], this->n[2][0] + n[2][0], this->n[2][1] + n[2][1], this->n[2][2] + n[2][2]};
}

Matrix3x3 Matrix3x3::operator-(const Matrix3x3 m) const {
    return Matrix3x3{this->n[0][0] - n[0][0], this->n[0][1] - n[0][1], this->n[0][2] - n[0][2], this->n[1][0] - n[1][0], this->n[1][1] - n[1][1], this->n[1][2] - n[1][2], this->n[2][0] - n[2][0], this->n[2][1] - n[2][1], this->n[2][2] - n[2][2]};
}

Matrix3x3 Matrix3x3::operator*(const float n) const {
    return Matrix3x3{this->n[0][0] * n, this->n[0][1] * n, this->n[0][2] * n, this->n[1][0] * n, this->n[1][1] * n, this->n[1][2] * n, this->n[2][0] * n, this->n[2][1] * n, this->n[2][2] * n};
}

Matrix3x3 Matrix3x3::operator*(const Matrix3x3 m) const {
    return Matrix3x3{this->n[0][0] * m(0, 0) + this->n[1][0] * m(0, 1) + this->n[2][0] * m(0, 2), this->n[0][1] * m(0, 0) + this->n[1][1] * m(0, 1) + this->n[2][1] * m(0, 2), this->n[0][2] * m(0, 0) + this->n[1][2] * m(0, 1) + this->n[2][2] * m(0, 2), this->n[0][0] * m(1, 0) + this->n[1][0] * m(1, 1) + this->n[2][0] * m(1, 2), this->n[0][1] * m(1, 0) + this->n[1][1] * m(1, 1) + this->n[2][1] * m(1, 2), this->n[0][2] * m(1, 0) + this->n[1][2] * m(1, 1) + this->n[2][2] * m(1, 2), this->n[0][0] * m(2, 0) + this->n[1][0] * m(2, 1) + this->n[2][0] * m(2, 2), this->n[0][1] * m(2, 0) + this->n[1][1] * m(2, 1) + this->n[2][1] * m(2, 2), this->n[0][2] * m(2, 0) + this->n[1][2] * m(2, 1) + this->n[2][2] * m(2, 2)};
}

Vector3 Matrix3x3::operator*(const Vector3 v) const {
    return Vector3{this->n[0][0] * v.x + this->n[1][0] * v.y + this->n[2][0] * v.z, this->n[0][1] * v.x + this->n[1][1] * v.y + this->n[2][1] * v.z, this->n[0][2] * v.x + this->n[1][2] * v.y + this->n[2][2] * v.z};
}


// MARK: - Color
Color::Color() {
    r = g = b = 0;
}

Color::Color(float r, float g, float b) : r(r), g(g), b(b) {}

Color::Color(int r, int g, int b) : r(r / 256.f), g(g / 256.f), b(b / 256.f) {}

float Color::value() const {
    return (r + g + b) / 3.f;
}

array<unsigned char, 3> Color::array() const {
    return std::array<unsigned char, 3>{(unsigned char)clamp((this->r * 256.f), 0.f, 255.f), (unsigned char)clamp((this->g * 256.f), 0.f, 255.f), (unsigned char)clamp((this->b * 256.f), 0.f, 255.f)};
}

string Color::css() const {
    stringstream ss;
    ss << "rgb(" << r * 255 << ", " << g * 255 << ", " << b * 255 << ")";
    return ss.str();
}

Color Color::light() const {
    return *this * 3.f;
}

Color Color::dark() const {
    return *this / 3.f;
}

// MARK: Defined Colors
const Color Color::White{255, 255, 255};
const Color Color::Gray{142, 142, 147};
const Color Color::Black{0, 0, 0};

const Color Color::Red{255, 59, 48};
const Color Color::Orange{255, 149, 0};
const Color Color::Yellow{255, 204, 0};
const Color Color::Pink{255, 45, 85};
const Color Color::Brown{162, 132, 94};
const Color Color::Mocha{148, 82, 0};
const Color Color::Asparagus{146, 144, 0};

const Color Color::Lime{142, 250, 0};
const Color Color::Green{40, 205, 65};
const Color Color::Moss{0, 144, 81};
const Color Color::Fern{79, 143, 0};

const Color Color::Blue{0, 122, 255};
const Color Color::Cyan{0, 253, 255};
const Color Color::Magenta{255, 64, 255};
const Color Color::Teal{90, 200, 250};
const Color Color::Indigo{88, 86, 214};
const Color Color::Purple{175, 82, 222};

Color::operator int() const {
    return ((int)(fmin(fmax(r, 0.0), 1.0) * 255) << 16) + ((int)(fmin(fmax(g, 0.0), 1.0) * 255) << 8) + (int)(fmin(fmax(b, 0.0), 1.0) * 255);
}

bool Color::operator==(const Color c) const {
    return this->r == c.r && this->g == c.g && this->b == c.b;
}

Color Color::operator+(const Color c) const {
    return Color(this->r + c.r, this->g + c.g, this->b + c.b);
}

Color Color::operator-(const Color c) const {
    return Color(this->r - c.r, this->g - c.g, this->b - c.b);
}

Color Color::operator-() const {
    return Color(-this->r, -this->g, -this->b);
}

Color Color::operator*(const double n) const {
    return Color(this->r * (float)n, this->g * (float)n, this->b * (float)n);
}

Color Color::operator*(const float n) const {
    return Color(this->r * n, this->g * n, this->b * n);
}

Color Color::operator*(const int n) const {
    return Color(this->r * n, this->g * n, this->b * n);
}

Color Color::operator/(const double n) const {
    return Color(this->r / (float)n, this->g / (float)n, this->b / (float)n);
}

Color Color::operator/(const float n) const {
    return Color(this->r / n, this->g / n, this->b / n);
}

Color Color::operator/(const int n) const {
    return Color(this->r / n, this->g / n, this->b / n);
}

Color Color::operator*(const Color c) const {
    return Color(this->r * c.r, this->g * c.g, this->b * c.b);
}

void Color::operator+=(Color c) {
    *this = *this + c;
}

void Color::operator-=(Color c) {
    *this = *this - c;
}

void Color::operator*=(Color c) {
    *this = *this * c;
}

void Color::operator/=(Color c) {
    *this = *this / c;
}


// MARK: - NeuralNetwork
NeuralNetwork::NeuralNetwork(vector<vector<vector<float>>> &nodes) {
    this->nodes = nodes;
}

vector<float> NeuralNetwork::eval(vector<float> &input) {
    vector<vector<float>> layers(nodes.size() + 1);
    layers[0] = input;
    
    for (int i = 0; i < nodes.size(); i++) {
        for (int j = 0; j < nodes[i].size(); j++) {
            for (int k = 0; k < nodes[i][j].size(); k++) {
                if (abs(nodes[i][j][k]) >= layers[i+1].size()) layers[i+1].resize(abs(nodes[i][j][k])+1, 0);
                if (nodes[i][j][k] > 0) layers[i+1][nodes[i][j][k]] += (nodes[i][j][k] - (int)nodes[i][j][k]) * layers[i][j];
                else layers[i+1][-nodes[i][j][k]] += (nodes[i][j][k] - (int)nodes[i][j][k]) * layers[i][j];
            }
        }
        if (i < nodes.size() - 1) for (int j = 0; j < layers[i+1].size(); j++) layers[i+1][j] = layers[i+1][j] != 0 ? 1 : 0;
    }
    
    return layers[layers.size()-1];
}

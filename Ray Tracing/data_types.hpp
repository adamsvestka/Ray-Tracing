//
//  data_types.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct Vector3;
struct Matrix3x3;
struct Color;
struct Material;

#pragma once

#include <cmath>
#include <vector>
#include <thread>
#include <queue>
#include <atomic>

using namespace std;

struct Vector3 {
    float x, y, z;
    
    float length() const;
    Vector3 normal() const;
    
    Color toColor() const;
    Vector3 operator+(const Vector3) const;
    Vector3 operator-(const Vector3) const;
    Vector3 operator-() const;
    Vector3 operator*(const double) const;
    Vector3 operator*(const float) const;
    Vector3 operator*(const int) const;
    Vector3 operator/(const double) const;
    Vector3 operator/(const float) const;
    Vector3 operator/(const int) const;
    float operator*(const Vector3) const;
    void operator+=(const Vector3);
    void operator-=(const Vector3);
};

#define Zero    Vector3{0, 0, 0}
#define Unit    Vector3{1, 1, 1}

struct Matrix3x3 {
    float n[3][3];
    
    Matrix3x3 inverse();
    
    float operator()(const int, const int) const;
    Matrix3x3 operator+(const Matrix3x3) const;
    Matrix3x3 operator-(const Matrix3x3) const;
    Matrix3x3 operator*(const float) const;
    Matrix3x3 operator*(const Matrix3x3) const;
    Vector3 operator*(const Vector3) const;
};

#define Identity    Matrix3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}
#define rot_mat_x(a)    Matrix3x3{1, 0, 0, 0, cos(a), -sin(a), 0, sin(a), cos(a)}
#define rot_mat_y(b)    Matrix3x3{cos(b), 0, sin(b), 0, 1, 0, -sin(b), 0, cos(b)}
#define rot_mat_z(c)    Matrix3x3{cos(c), -sin(c), 0, sin(c), cos(c), 0, 0, 0, 1}
#define rot_mat(yaw, pitch, roll)   (rot_mat_z(roll) * rot_mat_y(pitch) * rot_mat_x(yaw))

struct Color {
    float r, g, b;
    
    Color();
    Color(float, float, float);
    
    float value() const;
    
    operator int() const;
    bool operator==(const Color) const;
    Color operator+(const Color) const;
    Color operator-(const Color) const;
    Color operator*(const double) const;
    Color operator*(const float) const;
    Color operator*(const int) const;
    Color operator/(const double) const;
    Color operator/(const float) const;
    Color operator/(const int) const;
    Color operator*(const Color) const;
    void operator+=(const Color);
    void operator-=(const Color);
};

#define White       Color{1.0, 1.0, 1.0}
#define Gray        Color{0.1, 0.1, 0.1}
#define Black       Color{0.0, 0.0, 0.0}

#define DarkRed     Color{0.3, 0.03, 0.03}
#define Red         Color{1.0, 0.1, 0.1}
#define Orange      Color{1.0, 0.5, 0.1}
#define DarkYellow  Color{0.3, 0.3, 0.03}
#define Yellow      Color{1.0, 1.0, 0.1}
#define Lime        Color{0.5, 1.0, 0.1}

#define DarkGreen     Color{0.03, 0.3, 0.03}
#define Green       Color{0.1, 1.0, 0.1}
#define Turquoise   Color{0.1, 0.7, 0.7}
#define Cyan        Color{0.1, 1.0, 1.0}

#define DarkBlue     Color{0.03, 0.03, 0.3}
#define Blue        Color{0.1, 0.1, 1.0}
#define Purple      Color{0.6, 0.1, 1.0}
#define Magenta     Color{1.0, 0.1, 1.0}
#define Pink        Color{1.0, 0.4, 0.7}

struct NeuralNetwork {
    vector<vector<vector<float>>> nodes;
    
    NeuralNetwork(vector<vector<vector<float>>>);
    
    vector<float> eval(vector<float>);
};


template<typename T>
class ConcurrentQueue {
private:
    queue<T> queue_;
    mutex mutex_;
    condition_variable cond_;
    atomic<bool> exit_ = {false};
    
public:
    void push(T const& data) {
        exit_.store(false);
        unique_lock<mutex> lk(mutex_);
        queue_.push(data);
        lk.unlock();
        cond_.notify_one();
    }
    
    bool empty() {
        unique_lock<mutex> lk(mutex_);
        return queue_.empty();
    }
    
    bool pop(T& popped_value) {
        unique_lock<mutex> lk(mutex_);
        cond_.wait(lk, [&]() -> bool { return !queue_.empty() || exit_.load(); });
        if (exit_.load()) return false;
        popped_value = queue_.front();
        queue_.pop();
        return true;
    }
    
    void stop() {
        exit_.store(true);
        cond_.notify_all();
    }
};

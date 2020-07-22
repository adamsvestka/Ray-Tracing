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
    
    static Vector3 Zero, One, North, South, East, West, Up, Down;
    
    Vector3 operator+(const Vector3) const;
    Vector3 operator-(const Vector3) const;
    Vector3 operator-() const;
    Vector3 operator*(const double) const;
    Vector3 operator*(const float) const;
    Vector3 operator*(const int) const;
    Vector3 operator/(const double) const;
    Vector3 operator/(const float) const;
    Vector3 operator/(const int) const;
    Vector3 operator/(const Vector3) const;
    float operator*(const Vector3) const;
    void operator+=(const Vector3);
    void operator-=(const Vector3);
    bool operator==(const Vector3) const;
    bool operator!=(const Vector3) const;
};


struct Matrix3x3 {
    float n[3][3];
    
    Matrix3x3 inverse();
    
    static Matrix3x3 Identity;
    static Matrix3x3 RotationMatrixX(float);
    static Matrix3x3 RotationMatrixY(float);
    static Matrix3x3 RotationMatrixZ(float);
    static Matrix3x3 RotationMatrix(float, float, float);
    
    float operator()(const int, const int) const;
    Matrix3x3 operator+(const Matrix3x3) const;
    Matrix3x3 operator-(const Matrix3x3) const;
    Matrix3x3 operator*(const float) const;
    Matrix3x3 operator*(const Matrix3x3) const;
    Vector3 operator*(const Vector3) const;
};


struct Color {
    float r, g, b;
    
    Color();
    Color(float, float, float);
    
    float value() const;
    
    static Color White, LightGray, Gray, DarkGray, Black,
        DarkRed, Red, Orange, DarkYellow, Yellow, Lime,
        DarkGreen, Green, Turquoise, Cyan,
        DarkBlue, Blue, Azure, Purple, Magenta, Pink;
    
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

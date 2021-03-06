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
struct NeuralNetwork;
template<typename T> class ConcurrentQueue;

#pragma once

#include <cmath>
#include <vector>
#include <array>
#include <sstream>

using namespace std;

typedef vector<vector<Color>> Buffer;

struct Vector3 {
    float x, y, z;
    
    float length() const;
    Vector3 normalized() const;
    Color asColor() const;
    Vector3 cross(const Vector3) const;
    
    const static Vector3 Zero, One, North, South, East, West, Up, Down;
    
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
    
    const static Matrix3x3 Identity;
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
    Color(int, int, int);
    
    inline static int guard(float f);
    
    float asValue() const;
    array<unsigned char, 3> cimg() const;
    string css() const;
    Color light() const;
    Color dark() const;
    
    const static Color White, Gray, Black,
        Red, Orange, Yellow, Pink, Brown, Mocha, Asparagus,
        Lime, Green, Moss, Fern,
        Blue, Cyan, Magenta, Teal, Indigo, Purple;
    
    operator int() const;
    unsigned char operator[](short) const;
    bool operator==(const Color) const;
    Color operator+(const Color) const;
    Color operator-(const Color) const;
    Color operator-() const;
    Color operator*(const double) const;
    Color operator*(const float) const;
    Color operator*(const int) const;
    Color operator/(const double) const;
    Color operator/(const float) const;
    Color operator/(const int) const;
    Color operator*(const Color) const;
    void operator+=(const Color);
    void operator-=(const Color);
    void operator*=(const Color);
    void operator/=(const Color);
};


struct VectorUV {
    float u, v;
    
    VectorUV();
    VectorUV(float, float);
    VectorUV(int, int);
    
    inline static float guard(float f);
    
    const static VectorUV Zero;
    
    bool operator==(const VectorUV) const;
    bool operator!=(const VectorUV) const;
    VectorUV operator+(const VectorUV) const;
    VectorUV operator-(const VectorUV) const;
    VectorUV operator-() const;
    VectorUV operator*(const double) const;
    VectorUV operator*(const float) const;
    VectorUV operator*(const int) const;
    VectorUV operator/(const double) const;
    VectorUV operator/(const float) const;
    VectorUV operator/(const int) const;
    
    float getU() const;
    float getV() const;
};


struct NeuralNetwork {
    vector<vector<vector<float>>> nodes;
    
    explicit NeuralNetwork(vector<vector<vector<float>>> &);
    
    vector<float> eval(vector<float> &);
};

#ifndef __EMSCRIPTEN__

#include <thread>
#include <queue>
#include <atomic>

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

#endif

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

// Vlastní datový typ na držení bitmapy
typedef vector<vector<Color>> Buffer;

// Struktura 3D vektoru
struct Vector3 {
    float x, y, z;
    
    // Metody vektorů
    float length() const;   // Délka vektoru
    Vector3 normalized() const; // Normalizovaný vektor
    Color asColor() const;  // (x, y, x) převedeno na (r, g, b)
    Vector3 cross(const Vector3) const; // Vektorový součin
    
    // Předdefinované, často používané vektory
    const static Vector3 Zero, One, North, South, East, West, Up, Down;
    
    // Vektorové operace
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


// Struktura matice 3 krát 3
struct Matrix3x3 {
    float n[3][3];
    
    // Metody matic
    Matrix3x3 inverse();    // Inverzní matice
    
    // Předdefinované, často používané matice a inicializátory
    const static Matrix3x3 Identity;
    static Matrix3x3 RotationMatrixX(float);
    static Matrix3x3 RotationMatrixY(float);
    static Matrix3x3 RotationMatrixZ(float);
    static Matrix3x3 RotationMatrix(float, float, float);
    
    // Maticové operace
    float operator()(const int, const int) const;
    Matrix3x3 operator+(const Matrix3x3) const;
    Matrix3x3 operator-(const Matrix3x3) const;
    Matrix3x3 operator*(const float) const;
    Matrix3x3 operator*(const Matrix3x3) const;
    Vector3 operator*(const Vector3) const;
};


// Struktura barvy
struct Color {
    float r, g, b;
    
    // Inicializátory
    Color();
    Color(float, float, float);
    Color(int, int, int);
    
    // Pomocná funkce na kontrolu integrity
    inline static int guard(float f);
    
    // Metody barev
    float asValue() const;  // Převod na skalár
    array<unsigned char, 3> cimg() const;   // Převod na CImg formát
    string css() const; // Převod na CSS formát
    Color light() const;    // Zesvětlení
    Color dark() const; // Ztmavení
    
    // Předdefinované, často používané barvy
    const static Color White, Gray, Black,
        Red, Orange, Yellow, Pink, Brown, Mocha, Asparagus,
        Lime, Green, Moss, Fern,
        Blue, Cyan, Magenta, Teal, Indigo, Purple;
    
    // Barevné operace
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


// Struktura dvojvektoru na texturové souřadnice
struct VectorUV {
    float u, v;
    
    // Inicializátory
    VectorUV();
    VectorUV(float, float);
    VectorUV(int, int);
    
    // Pomocná funkce na kontrolu integrity
    inline static float guard(float f);
    
    // Předdefinované, často používané vektory
    const static VectorUV Zero;
    
    // Vektorové operace
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
    
    // Metody vektorů
    float getU() const;
    float getV() const;
};


// Struktura jednoduché nervové sítě
struct NeuralNetwork {
    // Uloženo ve formě: pole vrstev > buňky v poli > spojení buňky s buňkami v následující vrstvě > váha spojení
    vector<vector<vector<float>>> nodes;
    
    // Inicializace
    explicit NeuralNetwork(vector<vector<vector<float>>> &);
    
    // Vyhodnocení
    vector<float> eval(vector<float> &);
};

#ifndef __EMSCRIPTEN__

#include <thread>
#include <queue>
#include <atomic>

// Struktura na správu pole dat napříč několik vláken (FIFO)
template<typename T>
class ConcurrentQueue {
private:
    queue<T> queue_;
    mutex mutex_;
    condition_variable cond_;
    atomic<bool> exit_ = {false};
    
public:
    // Přidání prvku
    void push(T const& data) {
        exit_.store(false);
        unique_lock<mutex> lk(mutex_);
        queue_.push(data);
        lk.unlock();
        cond_.notify_one();
    }
    
    // Zda je pole prázdné
    bool empty() {
        unique_lock<mutex> lk(mutex_);
        return queue_.empty();
    }
    
    // Získání nejstaršího prvku
    bool pop(T& popped_value) {
        unique_lock<mutex> lk(mutex_);
        cond_.wait(lk, [&]() -> bool { return !queue_.empty() || exit_.load(); });
        if (exit_.load()) return false;
        popped_value = queue_.front();
        queue_.pop();
        return true;
    }
    
    // Nucené ukončení všech vláken
    void stop() {
        exit_.store(true);
        cond_.notify_all();
    }
};

#endif

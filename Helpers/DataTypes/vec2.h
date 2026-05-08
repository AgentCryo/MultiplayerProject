#ifndef MULTIPLAYERPROJECT_VEC2_H
#define MULTIPLAYERPROJECT_VEC2_H
#include <cmath>

using namespace std;

class vec2
{
    public:
    float x, y;

    vec2() : x(0), y(0) {}
    vec2(const float x, const float y) : x(x), y(y) {}
    static vec2 identity() { return {0,0}; }

    vec2 operator+(const vec2& v) const { return {x + v.x, y + v.y}; }
    vec2 operator-(const vec2& v) const { return {x - v.x, y - v.y}; }

    vec2 operator*(const vec2& v) const { return {x * v.x, y * v.y}; }
    vec2 operator/(const vec2& v) const { return {x / v.x, y / v.y}; }

    vec2 operator*(const float s) const { return {x * s, y * s}; }
    vec2 operator/(const float s) const { return {x / s, y / s}; }

    vec2& operator+=(const vec2& v) { x += v.x; y += v.y; return *this; }
    vec2& operator-=(const vec2& v) { x -= v.x; y -= v.y; return *this; }
    vec2& operator*=(const float s) { x *= s; y *= s;     return *this; }
    vec2& operator/=(const float s) { x /= s; y /= s;     return *this; }
    
    static float dot(const vec2& a, const vec2& b) {return a.x*b.x + a.y*b.y;}
    static float dist(const vec2& a, const vec2& b) {return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));}
    static vec2 normalize(const vec2& v) {
        const float len = sqrt(v.x*v.x + v.y*v.y);
        if (len == 0) return {0,0};
        return {v.x/len, v.y/len};
    }
};

#endif //MULTIPLAYERPROJECT_VEC2_H
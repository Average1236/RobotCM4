#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>


/************************************************************************/
/*                        CVector                                       */
/************************************************************************/
double CNormalize(double angle);
class CVector {
  public:
    CVector() : _x(0), _y(0) {}
    CVector(double x, double y) : _x(x), _y(y) {}
    CVector(const CVector& v) : _x(v.x()), _y(v.y()) {}
    bool setVector(double x, double y) {
        _x = x;
        _y = y;
        return true;
    }
    double mod() const {
        return std::sqrt(_x * _x + _y * _y);
    }
    double mod2() const {
        return (_x * _x + _y * _y);
    }
    double dir() const {
        return std::atan2(y(), x());
    }
    double theta(const CVector& v) {
        //计算v到自身的夹角
        double _theta;
        _theta = std::atan2(_y, _x) - std::atan2(v.y(), v.x());
        if (_theta > 3.14159265358979323846) return _theta - 2 * 3.14159265358979323846 ;
        if (_theta < -3.14159265358979323846) return _theta + 2 * 3.14159265358979323846;
        return _theta;
    }
    CVector rotate(double angle) const;
    CVector unit() const {
        CVector vector(_x, _y);
        if (vector.mod() < 1e-8) {
            std::cout << "WARNING Vector too small to have unit vector!\n";
            return CVector(1, 0);
        }
        return CVector(vector.x() / vector.mod(),
                       vector.y() / vector.mod());
    }
    double x() const {
        return _x;
    }
    double y() const {
        return _y;
    }
    double value(double angle) const {
        return mod() * std::cos(dir() - angle);
    }
    double crossProductZ(const CVector& v) const { //wang lijie add 向量叉乘Z分量
        return _x * v.y() - _y * v.x();
    }
    CVector operator +(const CVector& v) const {
        return CVector(_x + v.x(), _y + v.y());
    }
    CVector operator -(const CVector& v) const {
        return CVector(_x - v.x(), _y - v.y());
    }
    CVector operator *(double a) const {
        return CVector(_x * a, _y * a);
    }
    friend CVector operator *(double a, CVector b){
        return CVector(b._x * a, b._y * a);
    }
    double operator *(CVector b) const {
        return double(_x * b.x() + _y * b.y());    //向量点乘
    }
    CVector operator /(double a) const {
        return CVector(_x / a, _y / a);
    }
    CVector operator -() const {
        return CVector(-1 * _x, -1 * _y);
    }
    friend std::ostream& operator <<(std::ostream& os, const CVector& v) {
        return os << "(" << v.x() << ":" << v.y() << ")";
    }

  private:
    double _x, _y;
};

/************************************************************************/
/*                       CGeoPoint                                      */
/************************************************************************/
class CGeoPoint {
  public:
    CGeoPoint() : _x(0), _y(0) {}
    ~CGeoPoint() {}
    CGeoPoint(double x, double y) : _x(x), _y(y) {}
    CGeoPoint(const CGeoPoint& p) : _x(p.x()), _y(p.y()) {}
    bool operator==(const CGeoPoint& rhs) {
        return ((this->x() == rhs.x()) && (this->y() == rhs.y()));
    }
    double x() const {
        return _x;
    }
    double y() const {
        return _y;
    }
    void setX(double x) {
        _x = x;    // 2014/2/28 新增 设置x坐标 yys
    }
    void setY(double y) {
        _y = y;    // 2014/2/28 新增 设置y坐标 yys
    }
    bool fill(double x, double y) {
        _x = x;    //2018/4/14  新增 同时设置 wayne
        _y = y;
        return true;
    }
    double dist(const CGeoPoint& p) const {
        return CVector(p - CGeoPoint(_x, _y)).mod();
    }
    double dist2(const CGeoPoint& p) const {
        return CVector(p - CGeoPoint(_x, _y)).mod2();
    }
    CGeoPoint operator+(const CVector& v) const {
        return CGeoPoint(_x + v.x(), _y + v.y());
    }
    CGeoPoint operator*(const double& a) const {
        return CGeoPoint(_x * a, _y * a);
    }
    CVector operator-(const CGeoPoint& p) const {
        return CVector(_x - p.x(), _y - p.y());
    }
    CGeoPoint midPoint(const CGeoPoint& p) const {
        return CGeoPoint((_x + p.x()) / 2, (_y + p.y()) / 2);
    }
    friend std::ostream& operator <<(std::ostream& os, const CGeoPoint& v) {
        return os << "(" << v.x() << ":" << v.y() << ")";
    }

  private:
    double _x, _y;
};

#endif


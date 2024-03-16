#pragma once

#include <cmath>
#include <Defines.h>
#include <ostream>
#include <sstream>
#include <string>

namespace YAM{
    class Vector3 {
    public:
        flt x;
        flt y;
        flt z;

        Vector3() : x(0), y(0), z(0) {}

        explicit Vector3(flt x) : x(x), y(x), z(x) {}

        Vector3(flt X, flt Y, flt Z) : x(X), y(Y), z(Z) {}

        flt Length() const { return std::sqrt(x * x + y * y + z * z); }
        flt SquaredLength() const { return x * x + y * y + z * z; }

        Vector3 Normal() const {
            const double length = this->Length();
            if (length < SmallNumber)
                return Vector3{0.};

            return *this / length;
        }

        bool IsNear(const Vector3& vector3, flt error = SmallNumber) const {
            return std::abs(this->Length() - vector3.Length()) < error;
        }

        flt Dot(const Vector3& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }
        static flt Dot(const Vector3& a, const Vector3& b) { return a.Dot(b); }

        flt Angle(const Vector3& rhs) const { return acos(this->Dot(rhs) / (this->Length() * rhs.Length())); }

        Vector3 Abs() const { return {std::abs(x), std::abs(y), std::abs(z)}; }

        Vector3 Cross(const Vector3& rhs) const {
            return {
                this->y * rhs.z - this->z * rhs.y, this->z * rhs.x - this->x * rhs.z,
                this->x * rhs.y - this->y * rhs.x
            };
        }

        static Vector3 Cross(const Vector3& a, const Vector3& b) { return a.Cross(b); }

        Vector3& operator=(Vector3 const& another) {
            if (this == &another) {
                return *this;
            }

            this->x = another.x;
            this->y = another.y;
            this->z = another.z;
            return *this;
        }

        Vector3 operator-() const {
            Vector3 result;
            result.x = -this->x;
            result.y = -this->y;
            result.z = -this->z;
            return result;
        }

        Vector3 operator+(Vector3 const& another) const {
            Vector3 result;
            result.x = this->x + another.x;
            result.y = this->y + another.y;
            result.z = this->z + another.z;
            return result;
        }

        Vector3 operator-(Vector3 const& another) const {
            Vector3 result;
            result.x = this->x - another.x;
            result.y = this->y - another.y;
            result.z = this->z - another.z;
            return result;
        }

        void operator+=(Vector3 const& another) { *this = *this + another; }
        void operator-=(Vector3 const& another) { *this = *this - another; }

        Vector3 operator*(flt const& scalar) const {
            Vector3 result;
            result.x = this->x * scalar;
            result.y = this->y * scalar;
            result.z = this->z * scalar;
            return result;
        }

        friend Vector3 operator*(flt scalar, const Vector3& vector) {
            return vector * scalar;
        }

        Vector3 operator/(flt const& scalar) const {
            Vector3 result;
            result.x = this->x / scalar;
            result.y = this->y / scalar;
            result.z = this->z / scalar;
            return result;
        }

        void operator*=(flt const& scalar) { *this = *this * scalar; }
        void operator/=(flt const& scalar) { *this = *this / scalar; }
        bool operator==(const Vector3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }

        bool operator!=(const Vector3& rhs) const { return !(rhs == *this); }
        bool operator<(const Vector3& rhs) const { return this->Length() < rhs.Length(); }

        bool operator>(const Vector3& rhs) const { return rhs < *this; }
        bool operator<=(const Vector3& rhs) const { return !(rhs < *this); }
        bool operator>=(const Vector3& rhs) const { return !(*this < rhs); }

        friend std::ostream& operator<<(std::ostream& Os, const Vector3& vector3) {
            Os << "[" << vector3.x << "," << vector3.y << "," << vector3.z << "]";
            return Os;
        }

        std::string str() const {
            std::ostringstream result;
            result << *this;
            return result.str();
        }
    };
} // namespace SG

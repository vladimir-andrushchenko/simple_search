#include <vector>
#include <iostream>
#include <numeric>
#include <sstream>

#include "Test.h"

using namespace std::string_literals;

class Rational {
public:
    Rational() = default;

    Rational(int numerator)
        : numerator_(numerator)
        , denominator_(1) {
    }

    Rational(int numerator, int denominator)
        : numerator_(numerator)
        , denominator_(denominator) {
        Normalize();
    }
    
public:
    Rational& operator+=(Rational right) {
        numerator_ = numerator_ * right.Denominator() + right.Numerator() * denominator_;
        denominator_ *= right.Denominator();
        Normalize();
        return *this;
    }
    
    Rational& operator-=(Rational right) {
        return *this += Rational{-right.Numerator(), right.Denominator()};
    }
    
    Rational& operator*=(Rational right) {
        numerator_ *= right.Numerator();
        denominator_ *= right.Denominator();
        Normalize();
        return *this;
    }
    
    Rational& operator/=(Rational right) {
        return *this *= Rational{right.Denominator(), right.Numerator()};
    }
    
public:
    int Numerator() const {
        return numerator_;
    }

    int Denominator() const {
        return denominator_;
    }

private:
    void Normalize() {
        if (denominator_ < 0) {
            numerator_ = -numerator_;
            denominator_ = -denominator_;
        }
        const int divisor = std::gcd(numerator_, denominator_);
        numerator_ /= divisor;
        denominator_ /= divisor;
    }

private:
    int numerator_ = 0;
    int denominator_ = 1;
};

std::ostream& operator<<(std::ostream& output, const Rational& rational) {
    output << rational.Numerator() << "/"s << rational.Denominator();
    return output;
}

std::istream& operator>>(std::istream& input, Rational& rational) {
    int x, y;
    char dash; // переменная для считывания запятой
    input >> x >> dash >> y;
    rational = Rational{x, y};
    return input;
}

Rational operator+(Rational rational) {
    return rational;
}

Rational operator+(Rational left, Rational right) {
    return left += right;
}

Rational operator-(Rational rational) {
    return {-rational.Numerator(), rational.Denominator()};
}

Rational operator-(Rational left, Rational right) {
    return left + (-right);
}

Rational operator*(Rational left, Rational right) {
    return left *= right;
}

Rational operator/(Rational left, Rational right) {
    return left /= right;
}

// tests
void TestPlusEqualsOperator() {
    {
        Rational rational(1, 2);
        rational += Rational{1, 2};
        
        std::stringstream ss;
        ss << rational;
        
        ASSERT_EQUAL(ss.str(), "1/1");
    }
    
    {
        Rational rational;
        rational += Rational{1, 2};
        
        std::stringstream ss;
        ss << rational;
        
        ASSERT_EQUAL(ss.str(), "1/2");
    }
    
    {
        Rational rational(3, 2);
        rational += Rational{2, 2};
        
        std::stringstream ss;
        ss << rational;
        
        ASSERT_EQUAL(ss.str(), "5/2");
    }
}

void TestMinusEqualsOperator() {
    {
        Rational rational(1, 1);
        rational -= Rational{1, 2};
        
        std::stringstream ss;
        ss << rational;
        
        ASSERT_EQUAL(ss.str(), "1/2");
    }
    
    {
        Rational rational(1, 2);
        rational -= Rational{1, 2};
        
        std::stringstream ss;
        ss << rational;
        
        ASSERT_EQUAL(ss.str(), "0/1");
    }
    
    {
        Rational rational(6, 17);
        rational -= Rational{2, 17};
        
        std::stringstream ss;
        ss << rational;
        
        ASSERT_EQUAL(ss.str(), "4/17");
    }
}

void TestMultiplyEqualsOperator() {
    {
        Rational rational(1, 2);
        rational *= Rational{1, 2};
        
        std::stringstream ss;
        ss << rational;
        
        ASSERT_EQUAL(ss.str(), "1/4");
    }
    
    {
        Rational rational(1, 2);
        rational *= Rational{0};
        
        std::stringstream ss;
        ss << rational;
        
        ASSERT_EQUAL(ss.str(), "0/1");
    }
}

void TestDivideEqualsOperator() {
    {
        Rational rational(1, 2);
        rational /= Rational{1, 2};
        
        std::stringstream ss;
        ss << rational;
        
        ASSERT_EQUAL(ss.str(), "1/1");
    }
    
    {
        Rational rational(1, 2);
        rational *= Rational{0};
        
        std::stringstream ss;
        ss << rational;
        
        ASSERT_EQUAL(ss.str(), "0/1");
    }
}

void TestPlusOperatorRational() {
    {
        Rational rational(1, 2);
        
        std::stringstream ss;
        ss << (rational + rational);
        
        ASSERT_EQUAL(ss.str(), "1/1");
    }
    
    {
        Rational rational(1, 2);
        
        std::stringstream ss;
        ss << (rational + rational + rational);
        
        ASSERT_EQUAL(ss.str(), "3/2");
    }
}

void TestMinusOperatorRational() {
    {
        Rational rational(1, 2);
        
        std::stringstream ss;
        ss << (rational - rational);
        
        ASSERT_EQUAL(ss.str(), "0/1");
    }
    
    {
        Rational rational(1, 2);
        
        std::stringstream ss;
        ss << (rational - rational - rational);
        
        ASSERT_EQUAL(ss.str(), "-1/2");
    }
}

void TestMultiplyOperatorRational() {
    {
        Rational rational(1, 2);
        
        std::stringstream ss;
        ss << (rational * rational);
        
        ASSERT_EQUAL(ss.str(), "1/4");
    }
    
    {
        Rational rational(1, 2);
        
        std::stringstream ss;
        ss << (rational * Rational{0});
        
        ASSERT_EQUAL(ss.str(), "0/1");
    }
}

void TestDivideOperatorRational() {
    {
        Rational rational(1, 2);
        
        std::stringstream ss;
        ss << (rational / rational);
        
        ASSERT_EQUAL(ss.str(), "1/1");
    }
    
    {
        Rational rational(1, 2);
        
        std::stringstream ss;
        ss << (rational / Rational{1});
        
        ASSERT_EQUAL(ss.str(), "1/2");
    }
}

void TestRational() {
    RUN_TEST(TestPlusEqualsOperator);
    RUN_TEST(TestMinusEqualsOperator);
    RUN_TEST(TestMultiplyEqualsOperator);
    RUN_TEST(TestDivideEqualsOperator);
    RUN_TEST(TestPlusOperatorRational);
    RUN_TEST(TestMinusOperatorRational);
    RUN_TEST(TestMultiplyOperatorRational);
    RUN_TEST(TestDivideOperatorRational);
}
// tests ^^^

int main() {
    TestRational();
}

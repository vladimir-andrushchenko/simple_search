#include <vector>
#include <iostream>
#include <numeric>

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

    int numerator_ = 0;
    int denominator_ = 1;
};

Rational Add(Rational r1, Rational r2) {
    int numerator = r1.Numerator() * r2.Denominator() + r2.Numerator() * r1.Denominator();
    int denominator = r1.Denominator() * r2.Denominator();

    // Создаём и возвращаем дробь с заданным числителем и знаменателем
    return Rational{numerator, denominator};
}

std::ostream& operator<<(std::ostream& output, const Rational& rational) {
    output << rational.Numerator() << "/"s << rational.Denominator();
    return output;
}

// ввод
std::istream& operator>>(std::istream& input, Rational& rational) {
    int x, y;
    char dash; // переменная для считывания запятой
    input >> x >> dash >> y;
    rational = Rational{x, y};
    return input;
}

int main() {
    Rational rational;
    std::cin >> rational;
    std::cout << rational << std::endl;
}

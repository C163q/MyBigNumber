#pragma once
#include"integer.h"
#include<ratio>


namespace C163q {
	class rational_number {
	public:
		using number_base = ::C163q::integer;
	private:
		number_base denominator;	// 分母
		number_base numerator;		// 分子

	public:
		rational_number() : denominator(1U), numerator() {}
		rational_number(const integer& num) : denominator(1U), numerator(num) {}
		rational_number(integer&& num) : denominator(1U), numerator(::std::move(num)) {}
		rational_number(const integer& numerator, const integer& denominator) : numerator(numerator), denominator(denominator) { normalize(); }
		rational_number(integer&& numerator, integer&& denominator) : numerator(::std::move(numerator)), denominator(::std::move(denominator)) { normalize(); }
		rational_number(const integer& numerator, integer&& denominator) : numerator(numerator), denominator(::std::move(denominator)) { normalize(); }
		rational_number(integer&& numerator, const integer& denominator) : numerator(::std::move(numerator)), denominator(denominator) { normalize(); }
		rational_number(rational_number&& other) noexcept : denominator(::std::move(other.denominator)), numerator(::std::move(other.numerator)) {}
		rational_number(const rational_number& other) : denominator(other.denominator), numerator(other.numerator) {}

		template<::std::intmax_t Num, ::std::intmax_t Denom = 1>
		rational_number(const ::std::ratio<Num, Denom>& rational) : numerator(::std::ratio<Num, Denom>::type::num), denominator(::std::ratio<Num, Denom>::type::den) {}

		rational_number& operator=(const rational_number& other) {
			if (this == ::std::addressof(other)) return *this;
			numerator = other.numerator;
			denominator = other.denominator;
			return *this;
		}

		rational_number& operator=(rational_number&& other) noexcept {
			if (this == ::std::addressof(other)) return *this;
			numerator = ::std::move(other.numerator);
			denominator = ::std::move(other.denominator);
			return *this;
		}

		[[nodiscard]] static rational_number positive_inf() {
			return rational_number(1U, 0U);
		}

		[[nodiscard]] static rational_number negative_inf() {
			return rational_number(-1, 0U);
		}

		[[nodiscard]] static rational_number NaN() {
			return rational_number(0U, 0U);
		}

		[[nodiscard]] bool is_NaN() const noexcept {
			return numerator.is_zero() && denominator.is_zero();
		}

		[[nodiscard]] bool is_infinity() const noexcept {
			return !numerator.is_zero() && denominator.is_zero();
		}

		[[nodiscard]] bool is_negative() const noexcept {
			return (numerator.is_negative() != denominator.is_negative()) && !numerator.is_zero();
		}

		[[nodiscard]] bool is_negative_inf() const noexcept {
			return is_infinity() && numerator.is_negative();
		}

		[[nodiscard]] bool is_positive_inf() const noexcept {
			return is_infinity() && !numerator.is_negative();
		}

		[[nodiscard]] bool is_zero() const noexcept {
			return numerator.is_zero() && !denominator.is_zero();
		}

		[[nodiscard]] explicit operator bool() const noexcept {
			return !is_zero();
		}

		[[nodiscard]] rational_number operator+(const rational_number& rhs) const {
			if (is_NaN() || rhs.is_NaN() || is_infinity() || rhs.is_infinity()) return NaN();
			integer lcm_res(lcm(denominator, rhs.denominator));
			integer lhs_num(lcm_res / denominator * numerator);
			integer rhs_num(lcm_res / rhs.denominator * rhs.numerator);
			rational_number ret(lhs_num + rhs_num, ::std::move(lcm_res));
			ret.normalize();
			return ret;
		}

		rational_number& operator+=(const rational_number& rhs) {
			return operator=(operator+(rhs));
		}

		[[nodiscard]] rational_number operator-(const rational_number& rhs) const {
			if (is_NaN() || rhs.is_NaN() || is_infinity() || rhs.is_infinity()) return NaN();
			integer lcm_res(lcm(denominator, rhs.denominator));
			integer lhs_num(lcm_res / denominator * numerator);
			integer rhs_num(lcm_res / rhs.denominator * rhs.numerator);
			rational_number ret(lhs_num - rhs_num, ::std::move(lcm_res));
			ret.normalize();
			return ret;
		}

		rational_number& operator-=(const rational_number& rhs) {
			return operator=(operator-(rhs));
		}

		[[nodiscard]] bool operator!() const noexcept {
			return !is_zero();
		}

		[[nodiscard]] bool operator>(const rational_number& rhs) const noexcept {
			if (is_NaN() || rhs.is_NaN() || is_negative_inf() || rhs.is_positive_inf()) return false;
			if ((is_positive_inf() && !rhs.is_positive_inf()) || (rhs.is_negative_inf() && !is_negative_inf)) return true;
			if (is_infinity() || rhs.is_infinity()) return false;
			integer lcm_res(lcm(denominator, rhs.denominator));
			integer lhs_num(lcm_res / denominator * numerator);
			integer rhs_num(lcm_res / rhs.denominator * rhs.numerator);
			return lcm_res > rhs_num;
		}

		[[nodiscard]] bool operator<(const rational_number& rhs) const noexcept {
			if (is_NaN() || rhs.is_NaN() || is_positive_inf() || rhs.is_negative_inf()) return false;
			if ((is_negative_inf() && !rhs.is_negative_inf()) || (rhs.is_positive_inf() && !is_positive_inf)) return true;
			if (is_infinity() || rhs.is_infinity()) return false;
			integer lcm_res(lcm(denominator, rhs.denominator));
			integer lhs_num(lcm_res / denominator * numerator);
			integer rhs_num(lcm_res / rhs.denominator * rhs.numerator);
			return lcm_res < rhs_num;
		}

		[[nodiscard]] bool operator==(const rational_number& rhs) const noexcept {
			if (is_NaN() || rhs.is_NaN() || is_infinity() || rhs.is_infinity()) return false;
			integer lcm_res(lcm(denominator, rhs.denominator));
			integer lhs_num(lcm_res / denominator * numerator);
			integer rhs_num(lcm_res / rhs.denominator * rhs.numerator);
			return lcm_res == rhs_num;
		}

		[[nodiscard]] bool operator<=(const rational_number& rhs) const noexcept {
			if (is_NaN() || rhs.is_NaN() || is_positive_inf() || rhs.is_negative_inf()) return false;
			if ((is_negative_inf() && !rhs.is_negative_inf()) || (rhs.is_positive_inf() && !is_positive_inf)) return true;
			if (is_infinity() || rhs.is_infinity()) return false;
			return !operator>(rhs);
		}

		[[nodiscard]] bool operator>=(const rational_number& rhs) const noexcept {
			if (is_NaN() || rhs.is_NaN() || is_negative_inf() || rhs.is_positive_inf()) return false;
			if ((is_positive_inf() && !rhs.is_positive_inf()) || (rhs.is_negative_inf() && !is_negative_inf)) return true;
			if (is_infinity() || rhs.is_infinity()) return false;
			return !operator<(rhs);
		}

		[[nodiscard]] bool operator!=(const rational_number& rhs) const noexcept {
			if (is_NaN() || rhs.is_NaN()) return true;
			if (is_infinity() || rhs.is_infinity()) return false;
			return !operator==(rhs);
		}

		[[nodiscard]] rational_number operator*(const rational_number& rhs) const {
			rational_number ret(numerator * rhs.numerator, denominator * rhs.denominator);
			ret.normalize();
			return ret;
		}

		rational_number& operator*=(const rational_number& rhs) {
			numerator *= rhs.numerator;
			denominator *= rhs.denominator;
			normalize();
			return *this;
		}

		[[nodiscard]] rational_number operator/(const rational_number& rhs) const {
			rational_number ret(numerator * rhs.denominator, denominator * rhs.numerator);
			ret.normalize();
			return ret;
		}

		rational_number& operator/=(const rational_number& rhs) {
			numerator *= rhs.denominator;
			denominator *= rhs.numerator;
			normalize();
			return *this;
		}

		


	private:
		
		void reduction() {
			integer factor(gcd(numerator, denominator));
			if (factor.is_zero()) return;
			denominator /= factor;
			numerator /= factor;
		}

		void normalize();

		

	};

}
#pragma once
#include<iostream>
#include<utility>
#include<iterator>
#include<cstddef>
#include<string>
#include<cassert>
#include"integer_container.h"




namespace C163q {
	class rational_number;
	class integer : private integer_container {
		friend rational_number;
	public:
		using container_base = integer_container::container_base_t;
		using container = integer_container;

	private:
		bool negative = false;	// 默认正数

	private:
		// 清除高位无意义的0.同时,-0会标准化为+0.
		void normalize() {
			container::normalize();
			if (container::is_zero()) negative = false;
		}

		void set_one_sign_unchange() {
			clear();
			push_back(1);
		}

	public:
		integer() noexcept : container(), negative() {}
		integer(const bool num) : container(static_cast<unit_t>(num)), negative() {}
		integer(const signed char num) : container(static_cast<unit_t>(num < 0 ? -num : num)), negative(num < 0 ? true : false) {}
		integer(const unsigned char num) : container(static_cast<unit_t>(num)), negative() {}
		integer(const signed short num) : container(static_cast<unit_t>(num < 0 ? -num : num)), negative(num < 0 ? true : false) {}
		integer(const unsigned short num) : container(static_cast<unit_t>(num)), negative() {}
		integer(const signed int num) : container(static_cast<unit_t>(num < 0 ? -num : num)), negative(num < 0 ? true : false) {}
		integer(const unsigned int num) : container(static_cast<unit_t>(num)), negative() {}
		integer(const signed long num) : container(static_cast<double_unit_t>(num < 0 ? -num : num)), negative(num < 0 ? true : false) {}
		integer(const unsigned long num) : container(static_cast<double_unit_t>(num)), negative() {}
		integer(const signed long long num) : container(static_cast<double_unit_t>(num < 0 ? -num : num)), negative(num < 0 ? true : false) {}
		integer(const unsigned long long num) : container(static_cast<double_unit_t>(num)), negative() {}

		explicit integer(const char* num) : integer(::std::string(num)) {}
		explicit integer(const ::std::string& num);

		// 用`integer_container`构造`integer`,不指定符号将默认为正号.
		explicit integer(const container& base) : container(base), negative(false) {}
		explicit integer(container&& base) noexcept : container(::std::move(base)), negative(false) {}
		// `neg`用于指定符号.
		explicit integer(const container& base, const bool neg) : container(base), negative(neg) {}
		explicit integer(container&& base, const bool neg) noexcept : container(::std::move(base)), negative(neg) {}

		integer(const integer& other) : container(other), negative(other.negative) {}
		integer(integer&& other) noexcept : container(::std::move(other)), negative(::std::exchange(other.negative, false)) {}

		integer& operator=(const integer& other) {
			if (this == ::std::addressof(other)) return *this;
			container::operator=(other);
			negative = other.negative;
			return *this;
		}

		integer& operator=(integer&& other) noexcept {
			if (this == ::std::addressof(other)) return *this;
			container::operator=(::std::move(other));
			negative = ::std::exchange(other.negative, false);
			return *this;
		}

		explicit operator bool() const noexcept {
			return !is_zero();
		}

		// 相当于(*this = 0),但更快.
		void set_zero() noexcept {
			container::set_zero();
			negative = false;
		}

		// 相当于(*this == 0),但更快.
		[[nodiscard]] bool is_zero() const noexcept {
			return container::is_zero();
		}

		[[nodiscard]] bool is_one() const noexcept {
			return (container::is_one() && !negative);
		}

		// return num.abs() == 1
		[[nodiscard]] bool is_one_abs() const noexcept {
			return (container::is_one());
		}

		// 返回绝对值,但会创建一份拷贝.
		[[nodiscard]] integer abs() const {
			return integer(container(*this));
		}

		// 相反数,但会创建拷贝
		[[nodiscard]] integer opposite() const {
			return integer(container(*this), !negative);
		}

		// 绝对值
		[[nodiscard]] integer& make_abs() noexcept {
			negative = false;
			return *this;
		}

		// 相反数
		[[nodiscard]] integer& make_opposite() noexcept {
			negative = !negative;
			return *this;
		}

		[[nodiscard]] bool is_negative() const noexcept {
			return negative;
		}

		[[nodiscard]] bool is_positive() const noexcept {
			return !negative && !is_zero();
		}

		[[nodiscard]] bool operator==(const integer& other) const noexcept {
			return (container::operator==(other) && negative == other.negative);
		}

		[[nodiscard]] bool operator!=(const integer& other) const noexcept {
			return !operator==(other);
		}

		[[nodiscard]] bool operator>(const integer& other) const noexcept {
			if (negative != other.negative) {
				if (negative) return false;
				return true;
			}
			if (negative) return container::operator<(other);
			return container::operator>(other);
		}

		[[nodiscard]] bool operator<(const integer& other) const noexcept {
			if (negative != other.negative) {
				if (negative) return true;
				return false;
			}
			if (negative) return container::operator>(other);
			return container::operator<(other);
		}

		[[nodiscard]] bool operator>=(const integer& other) const noexcept {
			return !operator<(other);
		}

		[[nodiscard]] bool operator<=(const integer& other) const noexcept {
			return !operator>(other);
		}

		integer& operator>>=(const size_t& bit) noexcept {
			container::operator>>=(bit);
			return *this;
		}

		[[nodiscard]] integer operator>>(const size_t& bit) const {
			return integer(container::operator>>(bit), negative);
		}

		integer& operator<<=(const size_t& bit) {
			container::operator<<=(bit);
			return *this;
		}

		[[nodiscard]] integer operator<<(const size_t& bit) const {
			return integer(container::operator<<(bit), negative);
		}

		// return lhs.abs() | rhs.abs() and lhs.is_negative() || rhs.is_negative()
		[[nodiscard]] integer operator|(const integer& other) const {
			return integer(container::operator|(other), negative || other.negative);
		}

		integer& operator|=(const integer& other) {
			container::operator|=(other);
			negative = negative || other.negative;
			return *this;
		}

		// return lhs.abs() & lhs.abs() and lhs.is_negative() && rhs.is_negative()
		[[nodiscard]] integer operator&(const integer& other) const {
			return integer(container::operator&(other), negative && other.negative);
		}

		integer& operator&=(const integer& other) {
			container::operator&=(other);
			negative = negative && other.negative;
			return *this;
		}

		// return lhs.abs() ^ rhs.abs() and lhs.is_negative() != rhs.is_negative()
		[[nodiscard]] integer operator^(const integer& other) const {
			return integer(container::operator^(other), negative != other.negative);
		}

		integer& operator^=(const integer& other) {
			container::operator^=(other);
			negative = negative != other.negative;
			return *this;
		}

		[[nodiscard]] integer operator+(const integer& other) const;

		integer& operator+=(const integer& other) {
			if (negative == other.negative) {
				return abs_add(other);
			}
			return abs_sub(other);
		}

		[[nodiscard]] integer operator-(const integer& other) const;

		integer& operator-=(const integer& other) {
			if (negative == other.negative) {
				return abs_sub(other);
			}
			return abs_add(other);
		}

		[[nodiscard]] integer operator*(const integer& other) const;

		integer& operator*=(const integer& other) {
			operator=(operator*(other));
			return *this;
		}

		[[nodiscard]] integer operator/(const integer& other) const;

		integer& operator/=(const integer& other) {
			operator=(operator/(other));
			return *this;
		}

		[[nodiscard]] integer operator%(const integer& other) const;

		integer& operator%=(const integer& other) {
			operator=(operator%(other));
			return *this;
		}

		integer& operator++() {
			if (!negative) {
				abs_self_incre();
			}
			else {
				abs_self_decre();	// 此处不为0
			}
			normalize();	// necessary
			return *this;
		}

		integer operator++(int) {
			integer ret(*this);
			operator++();
			return ret;
		}

		integer& operator--() {
			if (!negative && !is_zero()) {
				abs_self_decre();
			}
			else if (is_zero()) {
				push_back(1);
				negative = true;
			}
			else {
				abs_self_incre();
			}
			normalize();	// necessary
			return *this;
		}

		integer operator--(int) {
			integer ret(*this);
			operator--();
			return ret;
		}

		[[nodiscard]] ::std::string ToString() const;

	private:
		// return lhs.abs() += rhs.abs(), return *this!!!
		integer& abs_add(const integer& other);

		// return lhs.abs() -= rhs.abs(), return *this!!! Note: lhs.abs() >= rhs.abs()
		integer& abs_sub_abs(const integer& other);

		// return lhs.abs() -= rhs.abs(), return *this!!! Note: if lhs.abs() < rhs.abs(), then lhs.negative = !lhs.negative
		integer& abs_sub(const integer& other);

		// return lhs.abs() * rhs.abs()
		[[nodiscard]] integer abs_mult(const integer& other) const;

		// return lhs.abs() * unit_t
		[[nodiscard]] integer abs_mult_unit(const unit_t& other) const;

		// return lhs.abs() / rhs.abs(). Note: rhs != 0
		[[nodiscard]] integer abs_div(const integer& other) const {
#if _DEBUG
			assert(!other.is_zero());
#endif
			if (container::operator<(other)) return {};
			if (other.size() == 1) return make_div_unit(other[0]).first;
			return make_div(other).first;
		}

		// return lhs.abs() % rhs.abs(). Note: rhs != 0
		[[nodiscard]] integer abs_mod(const integer& other) const {
#if _DEBUG
			assert(!other.is_zero());
#endif
			if (container::operator<(other)) return *this;
			if (other.size() == 1) return make_div_unit(other[0]).second;
			return make_div(other).second;
		}

		// Note: lhs.abs() >= rhs.abs(), 返回左商,右余数
		[[nodiscard]] ::std::pair<integer, integer> make_div(const integer& other) const;

		// 针对除以unit_t的加速, 返回左商,右余数
		[[nodiscard]] ::std::pair<integer, integer::unit_t> make_div_unit(const unit_t& rhs) const;

		// Note: index < size()
		bool self_incre_guard(const size_t& index) noexcept {
#if _DEBUG
			assert(index < size());
#endif
			if (operator[](index) == unit_max) {
				return false;
			}
			else {
				++operator[](index);
				return true;
			}
		}

		// ++abs()
		void abs_self_incre() {
			size_t index = 0;
			bool guard_res{};
			for (; (index < size()) && !(guard_res = self_incre_guard(index)); ++index) {
				operator[](index) = 0;
			}
			if (!guard_res) {
				push_back(1);
			}
		}

		// Note: index < size()
		bool self_decre_guard(const size_t& index) {
#if _DEBUG
			assert(index < size());
#endif
			if (operator[](index) == 0) {
				return false;
			}
			else {
				--operator[](index);
				return true;
			}
		}

		// Note: *this != 0
		void abs_self_decre() noexcept {
#if _DEBUG
			assert(!is_zero());
#endif
			size_t index = 0;
			bool guard_res{};
			for (; (index < size()) && !(guard_res = self_decre_guard(index)); ++index) {
				operator[](index) = unit_max;
			}
			normalize();
		}

		friend integer gcd(const integer& first, const integer& second);

		inline friend integer lcm(const integer& first, const integer& second);

	};

	[[nodiscard]] inline integer lcm(const integer& first, const integer& second) {
		integer gcd_res(gcd(first, second));
		if (gcd_res.is_zero()) {
			if (first.is_zero()) return second.abs();
			return first.abs();
		}
		return (first / gcd_res * second).make_abs();
	}


}








#include"integer.h"
#include<assert.h>
#include<algorithm>

namespace C163q {

	integer::integer(const ::std::string& num) {
		::std::string::const_iterator it = num.cbegin();
		bool tmp_neg = false;
		if (*it == '+') {
			tmp_neg = false;
			++it;
		}
		else if (*it == '-') {
			tmp_neg = true;
			++it;
		}
		try {
			for (; it != num.cend(); ++it) {
				if (*it < '0' || *it > '9') throw ::std::invalid_argument("Invalid number.");
				operator=(abs_mult_unit(10U));
				operator+=(*it - '0');
			}
			negative = tmp_neg;
			normalize();
		}
		catch (...) {
			clear();
			throw;
		}
	}

	integer& integer::abs_add(const integer& other) {
		reserve(::std::max(size(), other.size()) + 1);	// 预留空间,最多会有这么多位
		double_unit_t unit_add{};
		unit_t advance{};		// 存储进位
		size_t i{};
		for (; i < ::std::min(size(), other.size()); ++i) {
			unit_add = static_cast<double_unit_t>(operator[](i)) + other[i] + advance;
			advance = high_bit(unit_add);
			operator[](i) = unit_add;		// 高位用于进位,低位即为结果
		}
		while (i < size()) {		// size() > other.size()时
			unit_add = static_cast<double_unit_t>(operator[](i)) + advance;
			advance = high_bit(unit_add);
			operator[](i) = unit_add;
			++i;
		}
		while (i < other.size()) {
			unit_add = static_cast<double_unit_t>(other[i]) + advance;
			advance = high_bit(unit_add);
			push_back(unit_add);
			++i;
		}
		push_back(advance);
		normalize();
		return *this;
	}

	integer& integer::abs_sub_abs(const integer& other) {
#if _DEBUG
		assert(container::operator>=(other));	// 该减法不能求出负数
#endif
		unit_t unit_sub{};
		unit_t borrow{};	// 借位
		size_t i = 0;
		for (; i < other.size(); ++i) {
			if (operator[](i) < other[i] + borrow) {
				unit_sub = low_bit(unit_division + operator[](i) - other[i] - borrow);
				operator[](i) = unit_sub;
				borrow = 1;
			}
			else {
				unit_sub = operator[](i) - other[i] - borrow;
				operator[](i) = unit_sub;
				borrow = 0;
			}
		}
		while (i < size()) {
			if (operator[](i) < borrow) {
				unit_sub = low_bit(unit_division + operator[](i) - borrow);
				operator[](i) = unit_sub;
				borrow = 1;
			}
			else {
				unit_sub = operator[](i) - borrow;
				operator[](i) = unit_sub;
				borrow = 0;
			}
		}
		normalize();
		return *this;
	}

	integer& integer::abs_sub(const integer& other) {
		if (container::operator>=(other)) {
			return abs_sub_abs(other);
		}
		integer tmp(other);
		tmp.abs_sub_abs(*this);
		tmp.negative = !negative;
		operator=(::std::move(tmp));
		return *this;
	}

	[[nodiscard]] integer integer::abs_mult(const integer& other) const {
		integer ret;
		for (size_t i = 0; i < other.size(); ++i) {
			integer&& res = abs_mult_unit(other[i]);
			res.insert(res.begin(), i, 0);
			ret += res;
		}
		return ret;
	}

	[[nodiscard]] integer integer::abs_mult_unit(const unit_t& other) const {
		integer ret;
		ret.reserve(size() + 1);
		double_unit_t in_res{};
		unit_t advance{};
		for (container::const_iterator it = cbegin(); it != cend(); ++it) {
			in_res = static_cast<double_unit_t>(*it) * other + advance;
			advance = high_bit(in_res);
			ret.push_back(low_bit(in_res));
		}
		ret.push_back(advance);
		return ret;
	}

	[[nodiscard]] ::std::pair<integer, integer> integer::make_div(const integer& other) const {
		size_t rhs_bit_width = static_cast<size_t>(other.bit_cend() - other.bit_cbegin());	// 获取other位宽度
		const_bit_iterator my_bit_end = bit_cend();
		const_bit_iterator tmp_it = my_bit_end - rhs_bit_width;	// 指向准备计算的被除位置
		integer im_int;
		im_int.reserve(my_bit_end.get_base_it() - tmp_it.get_base_it() + 1);
		for (const_iterator it = tmp_it.get_base_it(); it != my_bit_end.get_base_it(); ++it) {	// 拷贝被除数部分
			im_int.push_back(*it);
		}
		if (my_bit_end.get_pos()) { im_int.push_back(*(my_bit_end.get_base_it())); }
		im_int >>= tmp_it.get_pos();
		integer ret(container::container_base_t(size() - other.size() + 1, 0));
		bit_reverse_iterator res_apply = ::std::make_reverse_iterator(++(ret.bit_begin() + (tmp_it - bit_cbegin())));	// 商是逐bit求出的
		do {
			if (im_int.container::operator>=(other)) {	// (逐bit)结果只有1和0
				*res_apply = true;
				im_int.abs_sub_abs(other);
			}
			++res_apply;
			if (res_apply != ret.bit_rend()) {	// 阻止越界访问
				im_int <<= 1;
				*(im_int.bit_begin()) = *--tmp_it;
			}
		} while (res_apply != ret.bit_rend());
		ret.normalize();
		im_int.normalize();
		return { ret, im_int };	// 左商,右余数
	}
	
	[[nodiscard]] ::std::pair<integer, integer::unit_t> integer::make_div_unit(const unit_t& rhs) const {
		double_unit_t divided_v{};
		integer ret(container_base_t(this->size()));
		for (difference_type i = size() - 1; i >= 0; --i) {
			divided_v <<= unit_bit;
			divided_v |= operator[](i);
			ret[i] = static_cast<unit_t>(divided_v / rhs);
			divided_v -= (static_cast<double_unit_t>(ret[i]) * rhs);
		}
		ret.normalize();
		return { ret, static_cast<unit_t>(divided_v) };
	}

	[[nodiscard]] integer integer::operator+(const integer& other) const {
		if (negative == other.negative) {
			integer ret(*this);
			ret.abs_add(other);
			return ret;
		}
		integer ret(*this);
		// 200 + (-100) -> 200 - 100 -> 100
		// 100 + (-200) -> 100 - 200 -> -(200 - 100) -> -100
		// -100 + 200 -> -(100 - 200) -> +(200 - 100) -> 100
		// -200 + 100 -> -(200 - 100) -> -100
		ret.abs_sub(other);
		return ret;
	}

	[[nodiscard]] integer integer::operator-(const integer& other) const {
		if (negative == other.negative) {
			integer ret(*this);
			// 200 - 100 -> 100
			// 100 - 200 -> -(200 - 100) -> -100
			// -100 - (-200) -> -(100 - 200) -> +(200 - 100) -> 100
			// -200 - (-100) -> -(200 - 100) -> -100
			ret.abs_sub(other);
			return ret;
		}
		integer ret(*this);
		ret.abs_add(other);
		return ret;
	}

	[[nodiscard]] integer integer::operator*(const integer& other) const {
		if (is_zero() || other.is_zero()) {
			return {};
		}
		if (is_one_abs()) {
			integer ret(other);
			ret.negative = (negative != other.negative);
			return ret;
		}
		if (other.is_one_abs()) {
			integer ret(*this);
			ret.negative = (negative != other.negative);
			return ret;
		}
		integer ret(abs_mult(other));
		ret.negative = (negative != other.negative);
		return ret;
	}

	[[nodiscard]] integer integer::operator/(const integer& other) const {
		if (other.is_zero()) throw ::std::domain_error("Divided by zero.");
		if (is_zero()) return {};
		if (other.is_one_abs()) return integer(container(*this), negative != other.negative);
		integer ret(abs_div(other));
		ret.negative = (negative != other.negative);
		ret.normalize();
		return ret;
	}

	[[nodiscard]] integer integer::operator%(const integer& other) const {
		if (other.is_zero()) throw ::std::domain_error("Moded by zero.");
		if (is_zero()) return {};
		if (other.is_one_abs()) return {};
		integer ret(abs_mod(other));
		ret.negative = (negative != other.negative);
		ret.normalize();
		return ret;
	}

	[[nodiscard]] ::std::string integer::ToString() const {
		integer tmp(*this);
		tmp.negative = false;
		if (is_zero()) return "0";
		::std::string ret;
		while (!tmp.is_zero()) {
			auto&& div_mod = tmp.make_div_unit(10U);
			ret.push_back(div_mod.second + '0');
			tmp = ::std::move(div_mod.first);
		}
		if (negative) ret.push_back('-');
		::std::reverse(ret.begin(), ret.end());
		return ret;
	}


	[[nodiscard]] integer gcd(const integer& first, const integer& second) {
		if (first.is_zero() || second.is_zero()) return {};
		if (first == second) return first.abs();
		integer m;
		integer n;
		if (first.integer::container::operator>=(second)) {
			m = first.abs();
			n = second.abs();
		}
		else {
			m = second.abs();
			n = first.abs();
		}
		integer r(m % n);
		while (r) {
			m = ::std::move(n);
			n = ::std::move(r);
			r = ::std::move(m % n);
		}
		return n;
	}

}

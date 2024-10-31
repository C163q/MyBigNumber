#include"integer_container.h"

namespace C163q {


	[[nodiscard]] bool integer_container::operator==(const integer_container& other) const noexcept {
		if (this == ::std::addressof(other)) return true;
		if (size() == other.size()) {
			for (size_t i = 0; i < size(); ++i) {
				if (operator[](i) != other[i]) {
					return false;
				}
			}
			return true;
		}
		return false;
	}

	[[nodiscard]] bool integer_container::operator>(const integer_container& other) const noexcept {
		if (this == ::std::addressof(other)) return false;
		if (size() > other.size()) return true;
		if (size() < other.size()) return false;
		for (ptrdiff_t i = size() - 1; i >= 0; --i) {
			if (operator[](i) > other[i]) return true;
			else if (operator[](i) < other[i]) return false;
		}
		return false;
	}

	[[nodiscard]] bool integer_container::operator<(const integer_container& other) const noexcept {
		if (this == ::std::addressof(other)) return false;
		if (size() > other.size()) return false;
		if (size() < other.size()) return true;
		for (ptrdiff_t i = size() - 1; i >= 0; --i) {
			if (operator[](i) < other[i]) return true;
			else if (operator[](i) > other[i]) return false;
		}
		return false;
	}

	integer_container& integer_container::operator>>=(const size_t& bit) noexcept {
		const unsigned bit_shift = static_cast<unsigned>(bit % unit_bit);
		const size_t drop_byte = ::std::min(bit / unit_bit, size());
		erase(begin(), begin() + drop_byte);
		if (!bit_shift) return *this;	// necessary
		unit_t last_bit{};
		unit_t now_bit{};
		for (reverse_iterator it = rbegin(); it != rend(); ++it) {
			now_bit = unit_max >> (unit_bit - bit_shift);
			now_bit &= *it;
			*it >>= bit_shift;
			last_bit <<= (unit_bit - bit_shift);
			*it |= last_bit;
			last_bit = now_bit;
		}
		normalize();
		return *this;
	}

	integer_container& integer_container::operator<<=(const size_t& bit) {
		const unsigned bit_shift = static_cast<unsigned>(bit % unit_bit);
		const size_t insert_byte = bit / unit_bit;
		unit_t last_bit{};
		unit_t now_bit{};
		if (bit_shift) {	// necessary
			for (iterator it = begin(); it != end(); ++it) {
				now_bit = unit_max << (unit_bit - bit_shift);
				now_bit &= *it;
				*it <<= bit_shift;
				last_bit >>= (unit_bit - bit_shift);
				*it |= last_bit;
				last_bit = now_bit;
			}
			push_back(last_bit >> (unit_bit - bit_shift));
		}
		insert(begin(), insert_byte, 0);
		normalize();
		return *this;
	}

	[[nodiscard]] integer_container integer_container::operator|(const integer_container& other) const {
		integer_container res;
		res.reserve(::std::max(size(), other.size()));
		size_t i = 0;
		for (; i < ::std::min(size(), other.size()); ++i) {
			res.push_back(operator[](i) | other[i]);
		}
		while (i < size()) {
			res.push_back(operator[](i));
			++i;
		}
		while (i < other.size()) {
			res.push_back(other[i]);
			++i;
		}
		return res;
	}

	[[nodiscard]] integer_container integer_container::operator&(const integer_container& other) const {
		integer_container res;
		size_t min_size{ ::std::min(size(), other.size()) };
		res.reserve(min_size);
		for (size_t i{ 0 }; i < min_size; ++i) {
			res.push_back(operator[](i) & other[i]);
		}
		res.normalize();
		return res;
	}
	
	[[nodiscard]] integer_container integer_container::operator^(const integer_container& other) const {
		integer_container res;
		res.reserve(::std::max(size(), other.size()));
		size_t i{ 0 };
		for (; i < ::std::min(size(), other.size()); ++i) {
			res.push_back(operator[](i) ^ other[i]);
		}
		while (i < size()) {
			res.push_back(operator[](i));
			++i;
		}
		while (i < other.size()) {
			res.push_back(other[i]);
			++i;
		}
		return res;
	}

	using const_bit_iterator = integer_container::const_bit_iterator;
	using bit_iterator = integer_container::bit_iterator;


}


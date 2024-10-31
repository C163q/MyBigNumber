#pragma once
#include<vector>
#include<iostream>
#include<utility>
#include<exception>
#include<type_traits>
#include<numeric>
#include<cstddef>
#include<climits>
#include<iterator>
#include<limits>


namespace C163q {
	/// @brief 该类用于存储无符号整数的二进制数据,低位(32位为一个单元方便简化运算)存储在低(vector)索引中
	/// @note 例如,对于二进制数字1111'0000'1111'0000'1111'0000'1111'0000'1010'1010'1010'1010'1010'1010'1010'1010
	/// 将会存储为 {0b1111'0000'1111'0000'1111'0000'1111'0000, 0b1010'1010'1010'1010'1010'1010'1010'1010}
	class integer_container : public ::std::vector<::std::make_unsigned_t<__int32>> {
	public:
		class const_bit_iterator;
		class bit_iterator;

	public:
		using signed_unit = __int32;							// signed 32 bit
		using unit_t = ::std::make_unsigned_t<signed_unit>;		// unsigned 32 bit, 数据存储的最小单元
		using double_unit_t = ::std::make_unsigned_t<__int64>;	// unsigned 64 bit
		using container_base_t = ::std::vector<unit_t>;

	public:
		using const_bit_iterator = const_bit_iterator;
		using const_bit_reverse_iterator = ::std::reverse_iterator<const_bit_iterator>;
		using bit_iterator = bit_iterator;
		using bit_reverse_iterator = ::std::reverse_iterator<bit_iterator>;

	public:
		constexpr static unit_t unit_max = ::std::numeric_limits<unit_t>::max();	// unit_t最大值
		constexpr static double_unit_t unit_division = static_cast<double_unit_t>(unit_max) + 1;
		constexpr static unsigned unit_bit = sizeof(unit_t) * CHAR_BIT;				// 32

	public:
		// 清除高位的0, 这些0没有意义
		inline void normalize() noexcept {
			while (!empty() && !back()) {
				pop_back();
			}
		}

	public:
		/// @brief 用于绑定到某一个`unit_t`,以便修改单个位
		class bit_modifier {
		private:
			unit_t& unit_ref;	// 绑定
			int pos;			// 0 <= pos <= 32, int用于防止溢出

		public:
			bit_modifier(unit_t& ref, const int pos = 0) noexcept : unit_ref(ref), pos(pos% unit_bit) {}

			bit_modifier& operator=(const bit_modifier&) = default;

			operator bool() const noexcept {
				return get();
			}

			bool get() const noexcept {
				return ((static_cast<unit_t>(1U) << pos) & unit_ref);
			}

			int get_pos() const noexcept {
				return pos;
			}

			unit_t& get_unit() const noexcept {
				return unit_ref;
			}

			bit_modifier& operator=(const bool bit) noexcept {
				set_value(bit);
				return *this;
			}

			void set_value(const bool bit) noexcept {
				if (bit) {
					unit_ref |= (static_cast<unit_t>(1U) << pos);
				}
				else {
					unit_ref &= ((static_cast<unit_t>(1U) << pos) ^ unit_max);
				}
			}
		};

		class bit_iterator;

		/// @brief `integer_container`的常迭代器,允许迭代单个位
		class const_bit_iterator {
			friend bit_iterator;
		public:
			using iterator_category = ::std::random_access_iterator_tag;
			using value_type = bool;		// 由于是常迭代器,无法赋值,所以value_type设置为bool就可以了
			using difference_type = ptrdiff_t;
			using pointer = const bool*;
			using reference = const bool&;

		public:
			using apply_type = integer_container;
			using value_base = apply_type::unit_t;
			using base_type = apply_type::container_base_t;
			using const_base_iterator = base_type::const_iterator;
			using bit_pos_t = int;		// signed,防止溢出

		private:
			const_base_iterator it;
			bit_pos_t pos;
		public:
			const_bit_iterator() noexcept : it(), pos() {}
			explicit const_bit_iterator(const const_base_iterator& it, const bit_pos_t pos = 0) noexcept : it(it), pos(pos % unit_bit) {}

			const_bit_iterator(const const_bit_iterator& other) noexcept : it(other.it), pos(other.pos) {}
			const_bit_iterator(const_bit_iterator&& other) noexcept : it(::std::move(other.it)), pos(::std::move(other.pos)) {}
			const_bit_iterator(const bit_iterator& other) noexcept : it(other.it), pos(other.pos) {}

			const_bit_iterator& operator=(const const_bit_iterator& other) noexcept {
				if (this == ::std::addressof(other)) return *this;
				it = other.it;
				pos = other.pos;
				return *this;
			}

			const_bit_iterator& operator=(const_bit_iterator&& other) noexcept {
				if (this == ::std::addressof(other)) return *this;
				it = ::std::move(other.it);
				pos = ::std::move(other.pos);
				return *this;
			}

			const_base_iterator get_base_it() const noexcept {
				return it;
			}

			bit_pos_t get_pos() const noexcept {
				return pos;
			}

			// next bit
			const_bit_iterator& operator++() noexcept {
				if (!(++pos %= unit_bit)) {		// ++pos, pos==32 -> pos=0 and condition -> true
					++it;
				}
				return *this;
			}

			const_bit_iterator operator++(int) noexcept {
				const_bit_iterator ret(*this);
				operator++();
				return ret;
			}

			// prev bit
			const_bit_iterator& operator--() noexcept {
				if (!(pos--)) {		// pos==0 -> cond->true; pos!=0 -> (pos-1 and cond->false)
					--it;
					pos = unit_bit - 1;
				}
				return *this;
			}

			const_bit_iterator operator--(int) noexcept {
				const_bit_iterator ret(*this);
				operator--();
				return ret;
			}

			const_bit_iterator& operator+=(const size_t& bit) noexcept {
				const size_t it_advance{ bit / unit_bit };
				const unsigned bit_advance{ bit % unit_bit };	// bit_advance < unit_bit
				it += it_advance;
				if ((pos += bit_advance) >= unit_bit) {
					++it;
					pos -= unit_bit;
				}
				return *this;
			}

			[[nodiscard]] const_bit_iterator operator+(const size_t& bit) const noexcept {
				const_bit_iterator ret(*this);
				ret += bit;
				return ret;
			}

			const_bit_iterator& operator-=(const size_t& bit) noexcept {
				const size_t it_advance{ bit / unit_bit };
				const unsigned bit_advance{ bit % unit_bit };	// bit_advance < unit_bit
				it -= it_advance;
				if ((pos -= bit_advance) < 0) {
					--it;
					pos += unit_bit;
				}
				return *this;
			}

			[[nodiscard]] const_bit_iterator operator-(const size_t& bit) const noexcept {
				const_bit_iterator ret(*this);
				ret -= bit;
				return ret;
			}

			[[nodiscard]] difference_type operator-(const const_bit_iterator& rhs) const noexcept {
				return (it - rhs.it) * unit_bit + (static_cast<difference_type>(pos) - rhs.pos);
			}

			[[nodiscard]] const value_type operator*() const noexcept {
				return ((static_cast<unit_t>(1U) << pos) & (*it));
			}

			[[nodiscard]] const value_type operator[](const size_t& bit) const noexcept {
				return *(operator+(bit));
			}

			[[nodiscard]] bool operator==(const const_bit_iterator& other) const noexcept {
				return (it == other.it) && (pos == other.pos);
			}

			[[nodiscard]] bool operator!=(const const_bit_iterator& other) const noexcept {
				return !operator==(other);
			}

			[[nodiscard]] bool operator<(const const_bit_iterator & other) const noexcept {
				return it < other.it || ((it == other.it) && (pos < other.pos));
			}

			[[nodiscard]] bool operator>(const const_bit_iterator& other) const noexcept {
				return it > other.it || ((it == other.it) && (pos > other.pos));
			}

			[[nodiscard]] bool operator<=(const const_bit_iterator& other) const noexcept {
				return !operator>(other);
			}

			[[nodiscard]] bool operator>=(const const_bit_iterator& other) const noexcept {
				return !operator<(other);
			}

		};

		/// @brief `integer_container`的迭代器,允许迭代每一个位.
		class bit_iterator {
			friend const_bit_iterator;
		public:
			using iterator_category = ::std::random_access_iterator_tag;
			using value_type = bit_modifier;		// 要支持赋值某一个位,需要借助`bit_modifier`
			using difference_type = ptrdiff_t;
			using pointer = bit_modifier*;
			using reference = bit_modifier;

		public:
			using apply_type = integer_container;
			using value_base = apply_type::unit_t;
			using base_type = apply_type::container_base_t;
			using base_iterator = base_type::iterator;
			using bit_pos_t = int;		// signed,防止溢出

		private:
			base_iterator it;
			bit_pos_t pos;

		public:
			bit_iterator() noexcept : it(), pos() {}
			explicit bit_iterator(const base_iterator& it, const bit_pos_t pos = 0) noexcept : it(it), pos(pos% unit_bit) {}

			bit_iterator(const bit_iterator& other) noexcept : it(other.it), pos(other.pos) {}
			bit_iterator(bit_iterator&& other) noexcept : it(::std::move(other.it)), pos(::std::move(other.pos)) {}

			bit_iterator& operator=(const bit_iterator& other) noexcept {
				if (this == ::std::addressof(other)) return *this;
				it = other.it;
				pos = other.pos;
				return *this;
			}

			bit_iterator& operator=(bit_iterator&& other) noexcept {
				if (this == ::std::addressof(other)) return *this;
				it = ::std::move(other.it);
				pos = ::std::move(other.pos);
				return *this;
			}

			base_iterator get_base_it() const noexcept {
				return it;
			}

			bit_pos_t get_pos() const noexcept {
				return pos;
			}

			// next bit
			bit_iterator& operator++() noexcept {
				if (!(++pos %= unit_bit)) {		// ++pos, pos==32 -> pos=0 and condition -> true 
					++it;
				}
				return *this;
			}

			bit_iterator operator++(int) noexcept {
				bit_iterator ret(*this);
				operator++();
				return ret;
			}

			// prev bit
			bit_iterator& operator--() noexcept {
				if (!(pos--)) {		// pos==0 -> cond->true; pos!=0 -> (pos-1 and cond->false)
					--it;
					pos = unit_bit - 1;
				}
				return *this;
			}

			bit_iterator operator--(int) noexcept {
				bit_iterator ret(*this);
				operator--();
				return ret;
			}

			bit_iterator& operator+=(const size_t& bit) noexcept {
				const size_t it_advance{ bit / unit_bit };
				const unsigned bit_advance{ bit % unit_bit };	// bit_advance < unit_bit
				it += it_advance;
				if ((pos += bit_advance) >= unit_bit) {
					++it;
					pos -= unit_bit;
				}
				return *this;
			}

			[[nodiscard]] bit_iterator operator+(const size_t& bit) const noexcept {
				bit_iterator ret(*this);
				ret += bit;
				return ret;
			}

			bit_iterator& operator-=(const size_t& bit) noexcept {
				const size_t it_advance{ bit / unit_bit };
				const unsigned bit_advance{ bit % unit_bit };	// bit_advance < unit_bit
				it -= it_advance;
				if ((pos -= bit_advance) < 0) {
					--it;
					pos += unit_bit;
				}
				return *this;
			}

			[[nodiscard]] bit_iterator operator-(const size_t& bit) const noexcept {
				bit_iterator ret(*this);
				ret -= bit;
				return ret;
			}

			[[nodiscard]] difference_type operator-(const bit_iterator& rhs) const noexcept {
				return (it - rhs.it) * unit_bit + (static_cast<difference_type>(pos) - rhs.pos);
			}

			[[nodiscard]] value_type operator*() const noexcept {
				return value_type(*it, pos);
			}

			[[nodiscard]] value_type operator[](const size_t& bit) const noexcept {
				return *(operator+(bit));
			}

			[[nodiscard]] bool operator==(const bit_iterator& other) const noexcept {
				return (it == other.it) && (pos == other.pos);
			}

			[[nodiscard]] bool operator!=(const bit_iterator& other) const noexcept {
				return !operator==(other);
			}

			[[nodiscard]] bool operator<(const bit_iterator& other) const noexcept {
				return it < other.it || ((it == other.it) && (pos < other.pos));
			}

			[[nodiscard]] bool operator>(const bit_iterator& other) const noexcept {
				return it > other.it || ((it == other.it) && (pos > other.pos));
			}

			[[nodiscard]] bool operator<=(const bit_iterator& other) const noexcept {
				return !operator>(other);
			}

			[[nodiscard]] bool operator>=(const bit_iterator& other) const noexcept {
				return !operator<(other);
			}

			// 兼容bit_iterator和const_bit_iterator比较的情况.
			[[nodiscard]] bool operator==(const const_bit_iterator& other) const noexcept {
				return (it == other.it) && (pos == other.pos);
			}

			[[nodiscard]] bool operator!=(const const_bit_iterator& other) const noexcept {
				return !operator==(other);
			}

			[[nodiscard]] bool operator<(const const_bit_iterator& other) const noexcept {
				return it < other.it || ((it == other.it) && (pos < other.pos));
			}

			[[nodiscard]] bool operator>(const const_bit_iterator& other) const noexcept {
				return it > other.it || ((it == other.it) && (pos > other.pos));
			}

			[[nodiscard]] bool operator<=(const const_bit_iterator& other) const noexcept {
				return !operator>(other);
			}

			[[nodiscard]] bool operator>=(const const_bit_iterator& other) const noexcept {
				return !operator<(other);
			}

		};

	public:
		/// 获取`double_unit_t`的高位(不是指内存地址)
		[[nodiscard]] static unit_t high_bit(const double_unit_t& v) noexcept {
			return static_cast<unit_t>(v >> unit_bit);
		}
		
		/// 获取`double_unit_t`的低位
		[[nodiscard]] static unit_t low_bit(const double_unit_t& v) noexcept {
			return static_cast<unit_t>(v);
		}

		// 高位和低位并为一个`double_unit_t`,即如果高位: 0x22...22, 低位: 0x11..11 -> return 0x22..2211..11
		[[nodiscard]] static double_unit_t combine_bit(const unit_t& high, const unit_t& low) {
			return (static_cast<double_unit_t>(high) << unit_bit) | low;
		}

		// 空代表0
		integer_container() noexcept : container_base_t() {}

		explicit integer_container(const unit_t& num) : container_base_t({ num }) { normalize(); }

		// { low bit, high bit }
		explicit integer_container(const double_unit_t& num) :
			container_base_t({ low_bit(num), high_bit(num) }) {
			normalize();
		}

		integer_container(const integer_container& other) : container_base_t(other) {}

		integer_container(integer_container&& other) noexcept : container_base_t(::std::move(other)) {}

		integer_container(const container_base_t& other) : container_base_t(other) {}

		integer_container(container_base_t&& other) noexcept : container_base_t(::std::move(other)) {}

		integer_container& operator=(const integer_container& other) {
			if (this == ::std::addressof(other)) return *this;
			container_base_t::operator=(other);
			return *this;
		}

		integer_container& operator=(integer_container&& other) noexcept {
			if (this == ::std::addressof(other)) return *this;
			container_base_t::operator=(::std::move(other));
			return *this;
		}

		// 0 -> 清空
		void set_zero() noexcept {
			clear();
		}

		[[nodiscard]] bool is_zero() const noexcept {
			return empty();
		}

		[[nodiscard]] bool is_one() const noexcept {
			return (size() == 1 && operator[](0) == 1);
		}

		[[nodiscard]] bool operator==(const integer_container& other) const noexcept;
		
		[[nodiscard]] bool operator>(const integer_container& other) const noexcept;

		[[nodiscard]] bool operator<(const integer_container& other) const noexcept;

		[[nodiscard]] bool operator!=(const integer_container& other) const noexcept {
			return !operator==(other);
		}

		[[nodiscard]] bool operator>=(const integer_container& other) const noexcept {
			return !operator<(other);
		}

		[[nodiscard]] bool operator<=(const integer_container& other) const noexcept {
			return !operator>(other);
		}

		[[nodiscard]] bool operator!() const noexcept {
			return is_zero();
		}

		integer_container& operator>>=(const size_t& bit) noexcept;

		[[nodiscard]] integer_container operator>>(const size_t& bit) const {
			const size_t drop_byte = ::std::min(bit / unit_bit, size());
			integer_container ret(container_base_t(begin() + drop_byte, end()));
			return ret.operator>>=(bit % unit_bit);
		}

		integer_container& operator<<=(const size_t& bit);

		[[nodiscard]] integer_container operator<<(const size_t& bit) const {
			const size_t insert_byte = bit / unit_bit;
			integer_container ret;
			ret.reserve(insert_byte + size() + 1);
			ret.insert(ret.end(), insert_byte, 0);
			ret.insert(ret.end(), begin(), end());
			return ret.operator<<=(bit % unit_bit);
		}

		[[nodiscard]] integer_container operator|(const integer_container& other) const;

		integer_container& operator|=(const integer_container& other) {
			operator=(::std::move(operator|(other)));
			return *this;
		}

		[[nodiscard]] integer_container operator&(const integer_container& other) const;

		integer_container& operator&=(const integer_container& other) {
			operator=(::std::move(operator&(other)));
			return *this;
		}

		[[nodiscard]] integer_container operator^(const integer_container& other) const;

		integer_container& operator^=(const integer_container& other) {
			operator=(::std::move(operator^(other)));
			return *this;
		}

		// 返回指向最低位的`bit_iterator`. eg. 0b1001100...00101`1`
		[[nodiscard]] bit_iterator bit_begin() noexcept {
			return bit_iterator(begin(), 0);
		}

		[[nodiscard]] const_bit_iterator bit_cbegin() const noexcept {
			return const_bit_iterator(cbegin(), 0);
		}

		[[nodiscard]] const_bit_iterator bit_begin() const noexcept {
			return bit_cbegin();
		}

		// 返回指向非0最高位后一位的`bit_iterator`. eg. 0b`0`100100..001011
		[[nodiscard]] bit_iterator bit_end() noexcept {
			if (is_zero()) { return bit_iterator(end(), 0); }
			container_base_t::iterator byte_it = ::std::prev(end());
			bit_iterator::bit_pos_t bit_it{};
			unit_t tmp = *byte_it;
			while (tmp) {
				tmp >>= 1;
				++bit_it;
			}
			if (bit_it >= unit_bit) {
				++byte_it;
				bit_it -= unit_bit;
			}
			return bit_iterator(byte_it, bit_it);
		}

		[[nodiscard]] const_bit_iterator bit_cend() const noexcept {
			if (is_zero()) { return const_bit_iterator(cend(), 0); }
			container_base_t::const_iterator byte_it = ::std::prev(cend());
			bit_iterator::bit_pos_t bit_it{};
			unit_t tmp = *byte_it;
			while (tmp) {
				tmp >>= 1;
				++bit_it;
			}
			if (bit_it >= unit_bit) {
				++byte_it;
				bit_it -= unit_bit;
			}
			return const_bit_iterator(byte_it, bit_it);
		}

		[[nodiscard]] const_bit_iterator bit_end() const noexcept {
			return bit_cend();
		}

		[[nodiscard]] bit_reverse_iterator bit_rbegin() noexcept {
			return ::std::make_reverse_iterator(bit_end());
		}

		[[nodiscard]] const_bit_reverse_iterator bit_crbegin() const noexcept {
			return ::std::make_reverse_iterator(bit_cend());
		}

		[[nodiscard]] const_bit_reverse_iterator bit_rbegin() const noexcept {
			return bit_crbegin();
		}

		[[nodiscard]] bit_reverse_iterator bit_rend() noexcept {
			return ::std::make_reverse_iterator(bit_begin());
		}

		[[nodiscard]] const_bit_reverse_iterator bit_crend() const noexcept {
			return ::std::make_reverse_iterator(bit_cbegin());
		}

		[[nodiscard]] const_bit_reverse_iterator bit_rend() const noexcept {
			return bit_crend();
		}

		

	};

}


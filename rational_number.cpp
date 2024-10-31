#include "rational_number.h"


namespace C163q {
	void rational_number::normalize() {
		if (numerator.is_zero() && !denominator.is_zero()) {
			denominator.set_one_sign_unchange();
			denominator.negative = false;
			return;
		}
		if (denominator.is_zero() && !numerator.is_zero()) {
			numerator.set_one_sign_unchange();
			return;
		}
		if (numerator.is_zero() && denominator.is_zero()) return;
		if (numerator.is_negative() != denominator.is_negative()) {
			numerator.negative = true;
			denominator.negative = false;
		}
		else {
			numerator.negative = denominator.negative = false;
		}
		reduction();
	}

}

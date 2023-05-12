from opcodes import SUB, GT
from rule import Rule
from util import BVUnsignedMax, BVUnsignedUpCast
from z3 import BVSubNoUnderflow, BitVec, Not

"""
Overflow checked unsigned integer subtraction.
"""

n_bits = 256
for type_bits in range(8, n_bits + 1, 8):

	rule = Rule()

	# Input vars
	X_short = BitVec('X', type_bits)
	Y_short = BitVec('Y', type_bits)

	# Z3's overflow condition
	actual_overflow = Not(BVSubNoUnderflow(X_short, Y_short, False))

	# cast to full n_bits values
	X = BVUnsignedUpCast(X_short, n_bits)
	Y = BVUnsignedUpCast(Y_short, n_bits)
	diff = SUB(X, Y)

	# Constants
	maxValue = BVUnsignedMax(type_bits, n_bits)

	# Overflow check in YulUtilFunction::overflowCheckedIntSubFunction
	overflow_check = GT(diff, X) if type_bits == 256 else GT(diff, maxValue)
	rule.check(overflow_check != 0, actual_overflow)

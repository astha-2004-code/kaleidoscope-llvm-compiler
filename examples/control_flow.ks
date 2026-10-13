# Demonstrates conditionals, loops, and unary operations

def test_clamp(val low high)
  if val < low then
    low
  else if val > high then
    high
  else
    val;

def sum_squares(n)
  for i = 1, i < n, 1 in
    i * i;

# Test constant folding on complex expressions
def folded_math()
  (10 + 20) * 3 - (50 / 2) + (-5);

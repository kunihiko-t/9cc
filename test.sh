#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [[ "$actual" = "$expected" ]]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 200 '200;'
try 3 '1 + 2;'
try 21 '5+20-4;'
try 41 " 12 + 34 - 5 ;"
try 47 "5+6*7;"
try 10 "-10+20;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 15 "-3 * -5 + (10 - 10);"
try 1 "10 == 10;"
try 0 "9 == 10;"
try 1 "(10*3) == (10+20);"
try 0 "(10*4) == (10+20);"
try 1 "(10/2) <= 5;"
try 1 "(12*4) <= 50;"
try 0 "(12*4) <= 47;"
try 1 "1 < 3;"
try 0 "3 < 1;"
try 1 "-3 < 1;"
try 1 "-3 < 1*5;"
try 0 "3 >= 5;"
try 1 "30 >= 5*5;"
try 1 "(33-20 >= 5);"
try 1 "30 >= 30;"
try 1 "30 >= 30;"
try 1 "3 > 2;"
try 0 "2 > 3;"
try 0 "3 > 3;"
try 1 "(3*10) > (3*9);"
try 10 "ab = 10 ;"
try 20 "b = 10 * 2;"
try 1 "foo = 1;"
try 2 "return 2;"
try 6 "foo = 3; bar=2; return foo * bar;"
try 14 "a = 3;
b = 5 * 6 - 8;
return a + b / 2;"
try 1 "if(1 == 1) 1; else 2;"
try 1 "a = 1; if(1 != 1) a = 2; a;"
echo OK


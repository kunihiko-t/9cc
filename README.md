# 9cc

WIP

ステップ7: 比較演算子の最初のあたりまで

## Build

```asm
make
```

## Test
```asm
make test
```

## EBNF

```
program    = stmt*
stmt       = expr ";"
           | "return" expr ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? term
term       = num | ident | "(" expr ")"
```
References: https://www.sigbus.info/compilerbook
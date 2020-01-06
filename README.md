# 6502compiler

## ebnf

program = ("int" | "double" | ...) ident ( ) { stmt* }

stmt =  expr ";"
     |  "{" stmt* "}"
     |  "if" "(" expr ")" stmt ( "else" stmt )?
     |  "while" "(" expr ")" stmt
     |  "return" expr ";"

expr = ("int" | "double" | ...)? assign

assign = ("*"? equality | \[ num \] )?("=" assign)?

equality = relational ("==" relational | "!=" relational)*

relational = add ("<" add | "<=" add | ">" add | ">=" add)*

add = mul ("+" mul | "-" mul)*

mul = unary ("*" unary | "/" unary)*

unary =  |"+"? primary | "-"? primary | "*"? unary | "&"? unary | "0x"? unary | "0b"? unary

primary = num |  ident ( "( num )" | "\[ num \]" )? | "(" expr ")" | { "(num)"* ','? } 
  
## 6502に移植

前提としてスタックに計算したい値２つがあらかじめ積んである。
また、x86との違いとそれの対処方も記する
  
### レジスタの違いの対処方

- 算術用レジスタが一つしかない
  $0 メモリを算術用のメモリとして使用する

- rbpが存在しない。
  $1 をrbpとして代用する。



# 6502compiler

## 9ccからの移植について
## 6502に移植

前提としてスタックに計算したい値２つがあらかじめ積んである。
また、x86との違いとそれの対処方も記す

### レジスタの違いの対処方

- 算術用レジスタが一つしかない
  $0 メモリを算術用のメモリとして使用する

- rbpが存在しない。
  $1 をrbpとして代用する。



### add

clc    // キャリーフラグをクリア
pla  // aにポップ
sta $0
pla  // aにポップ
adc $0//
pha   // aをプッシュ

### sub

sec    // キャリーフラグをセット
pla  // aにポップ
sta $0
pla  // aにポップ
sbc $0 // なんか正気なのかしらないけど、キャリーフラグは反転してから引き算するらしい
pha   // aをプッシュ


# basic-scheme
Это интерпретатор lisp подобного языка программирования scheme.
На данный момент поддержано выполнение простых функций(вроде суммы, сравнений, и подобных операций)
Также есть возиожность создания переменных и лямбда функций.
Пока не реализован сборщик мусора, поэтому в программе есть утечки памяти.

Синтаксис языка: https://groups.csail.mit.edu/mac/ftpdir/scheme-7.4/doc-html/

Простейшие примеры использования:
>> (+ 1 2 3)

>> 6

>> (define x (- 5 4 2))

>> (> x 2)

>> #f

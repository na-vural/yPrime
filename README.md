# yPrime

Yet another prime checker. Of course the name is a result of an inspiration coming from the name of [yay](https://github.com/Jguer/yay).

## Purpose

Find whether a number is prime or not as fast as possible.

## Features

- Divisors are not even numbers and multiples of 3, 5, 7, 11. ([Sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes)) 

- Deal with big numbers such big (> 10^22) that maybe calculation never ends, thanks to gmp library.

- Use the hundred percent of all the cpu cores, thanks to pthread library.

- If your computer's fan works properly, then the COVID-19 hanging in the air will see what the hell is. Otherwise, your computer can see...

## Installation

Just clone the repo, and `make all`.

## Usage

```
$ ./yprime <number>
```

## Any suggestions are welcome

[![The Comic](https://imgs.xkcd.com/comics/code_quality.png "xkcd.com/1513")](https://xkcd.com/1513)

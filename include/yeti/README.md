# Yeti

## The parser hierarchy

__Parser function__: `parser_fn: P -> R -> T -> E -> bool`

1. Is a move constructible type.
2. If `P` defines a `::type` then R matches it.
3. Can be invoked with an `R` to produce a `result<R, T, E>`.
4. Invocation must be valid for all value categories (VC).
5. Invocations in (3) must return the same type independent of the VC.

If `R` is `void` then No. 2-5 are not required. If `T`/`E` is `void` then
the `T`/`E` of the `result` in No. 3 is not required to match `void`.

The purpose of No. 4-5 is to make static type checking possible for typed
parsers before they are invoked.

__Parser__: `parser: P -> R -> T -> E -> bool`

1. Is a `parser_fn<P, R, T, E>`
2. Has `.skip()` and `.mute()` methods.
3. The methods in (2) must be valid for all VC.
4. The methods in (3) must return the same type independent of the VC.
5. The result of `.skip()` is a `parser<'R unit, E>`.
6. The result of `.mute()` is a `parser<'R, T unit>`.

In No 5-6, if `'R` is the static type of P if it is not `void` otherwise `R`.

Similar to `parser_fn`, the purpose of No 3-4 is to make static type checking
possible for typed parsers before they are invoked.

# Yeti

## The parser hierarchy

__Parser function__: `parser_fn: P -> S -> T -> E -> bool`

1. Is a move constructible type.
2. If `P` defines a `::type` then `X <- ::type` else `X <- void`.
3. If `X` and `S` are both non-void then `X` is `std::common_with` `S`
4. Can be invoked with an `S` to produce a `result<yeti::decay_t<S>, T, E>`.
5. Can be invoked with an `X` to produce a `result<yeti::decay_t<X>, T, E>`.
6. Invocations in 4 and 5 must return the same type.
7. Invocations in 4-5 must be valid for all value categories (VC).
8. Invocations in 4-5 must return the same type independent of the VC.

If `S` or `X` is `void` the requirements corresponding to their invocation are dropped
and the parser is assumed to be generic.
If `T`/`E` is `void` then the `T`/`E` of the `result` in 4-5 are not required to match `void`.

The purpose of No. 6-8 is to make static type checking possible for typed
parsers before they are invoked.

__Parser__: `parser: P -> S -> T -> E -> bool`

1. Is a `parser_fn<P, S, T, E>`
2. Has `.skip()` and `.mute()` methods.
3. The methods in (2) must be valid for all VC.
4. The methods in (3) must return the same type independent of the VC.
5. The result of `.skip()` is a `parser<'S, U, E>`.
6. The result of `.mute()` is a `parser<'S, T, U>`.

In No 5-6, `'S` is the static type of P if it is not `void` otherwise `S`, similarly `U` is `void`
if `'S` is `void` else its is `unit`.

Similar to `parser_fn`, the purpose of No 3-4 is to make static type checking
possible for typed parsers before they are invoked.

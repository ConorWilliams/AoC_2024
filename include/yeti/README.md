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

The purpose of 4 and 5 is to make static type checking possible for typed
parsers before they are invoked.

- parser_typed_fn: P -> R -> T -> E -> bool

- parser_object: P -> R -> T -> E -> bool

- 5. Has `.skip()` and `.mute()` methods.
- 6. The methods in (5) must be valid for all VC.
- 7. The methods in (5) must return the same type independent of the VC.

- parser: P -> R -> T -> E -> bool

- 8. The result of `.skip()` is a `parser<R unit, E>`.
- 9. The result of `.mute()` is a `parser<R, T unit>`.

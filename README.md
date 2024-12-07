# AoC_2024

KISS principled C++23 (no modules), file `==` self-contained solution.

```sh
make -S . -B build && cmake --build build
```

Solutions are in `./build/day_*`

:30: note: because '

is_constructible_v<
    parser_lift::skip_parser<parser_lift::lifted<fail_imp::fail>> &&,
    parser_lift::skip_parser<parser_lift::lifted<fail_impl::fail>> &
    >

  160 |       = destructible<_Tp> && is_constructible_v<_Tp, _Args...>;

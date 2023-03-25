# Carl

A statically typed, jit compiled scripting language built for fun (wip).<br>
The main point is to have some fancy functional features to work with functions 🏗️.

**Passing functions around**:
```rust
fn call_other_fn(a: int, f: (int, int : int)) : int {
    return f(a, 1 + a);
}
fn add(a: int, b: int) : int {
    return a + b;
}
let result = call_other_fn(1, add);
__debug(result); // assert == 3
```

**Composition**:
```rust
fn add(a: float, b: float) : float {
    let result = a + 2;
    return result;
}

let add_two = add(_, 2);
let add_three = add(_, 3);
let add_five = add_three . add_two;

let result = add_five(1) == 6;
```

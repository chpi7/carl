# Carl

A statically typed, jit compiled scripting language built for fun (wip).

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

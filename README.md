# Carl

A work in progress (experimental) language that:
- Has partial function application
- Has nice chaining function applications via the `|` operator
- Compiles to bytecode
- Easly understandable handwritten table-driven operator precedence parser


```
struct Cat {
    name: string,
    age: int
}

fn changeName(cat: Cat, to: string) : Cat {
    Cat {
        name: to,
        age: cat.age
    }
}

nameTom = changeName(_, to);

anonymous = Cat {
        name: name,
        age: 5 
    };
tom = anonymous | nameTom;
```
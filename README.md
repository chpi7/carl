# Carl

A work in progress (experimental) language combining features I like from other languages.

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
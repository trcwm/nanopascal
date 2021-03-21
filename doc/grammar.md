
# EXTENDED BNF GRAMMAR FOR PL/0

```
program = block "." .

block = { "const" ident "=" number {"," ident "=" number} ";"}
        { "var" ident {"," ident} ";"}
        { "procedure" ident ";" block ";" } statement .

statement = [ ident ":=" expression | "call" ident 
              | "?" ident | "!" expression {"," expression}
              | "begin" statement {";" statement } "end" 
              | "if" condition "then" statement [ "else" statement ]
              | "while" condition "do" statement ].

condition = "odd" expression |
            expression ("="|"#"|"<"|"<="|">"|">=") expression .

expression = [ "+"|"-"] term { ("+"|"-") term}.

term = factor {("*"|"/") factor}.

factor = ident | number | "(" expression ")".)
```
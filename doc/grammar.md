
# EXTENDED BNF GRAMMAR FOR NANO PASCAL

```
program = block "." .

simple_type = "integer" | "char".

type  =  simple_type
          | "array" "[" (number | const_id) "]" "of" simple_type.

block = { "const" ident "=" number {"," ident "=" number} ";"}
        { "var" ident {"," ident} ":" type ";"}
        { "procedure" ident ";" block ";" } statement .

const_id    = ident.
variable_id = ident [ '[' expression ']' ].

statement = [ variable_id ":=" expression | "call" ident 
              | "?" variable_id | "!" expression {"," expression}
              | "begin" statement {";" statement } "end" 
              | "if" condition "then" statement [ "else" statement ]
              | "for" variable_id ":=" expression ("to"|"downto") expression "do" statement.
              | "while" condition "do" statement ].

condition = "odd" expression |
            expression ("="|"#"|"<"|"<="|">"|">=") expression .

expression = [ "+"|"-"] term { ("+"|"-") term}
                | "shr" expression
                | "shl" expression
                | "sar" expression.

term = factor {("*"|"/") factor}.

factor = variable_id | const_id | number | "(" expression ")".
```
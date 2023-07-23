# Toy Project 2
## DerkWithT at `replit.com`

### Summary
This might be a toy language interpreter using the tree walk approach on a generated AST.

### Features of Rubel
 1. Paradigm(s):
    - Imperative
 2. Type System:
    - Variable types are inferred.
    - Mismatched types for an operator causes runtime errors
    - Variables can be mutable or not
    - Booleans are written as `$T` and `$F`!
 3. Ruby-like syntax and uses similar C-like operators:
    - Example 1: logical operators are `&&, ||`.
    - Example 2: comparison operators are `==, !=, <, <=, >, >=`
    - Example 3: math operators are `+, -, *, /`

### Keywords of Rubel
 - `use`: Includes a module into a script. Specifically, the names of other procedures and constants become visible.
 - `let`: Declares a mutable variable.
 - `const`: Declares an immutable variable. Lists should be fixed size with this specifier as well.
 - `proc`: Declares a procedure.
 - `if`: Executes a block of statements if its conditional is true.
 - `otherwise`: Executes a block of statements if the last `if` failed.
 - `while`: Executes a block of statements as long as the conditional is true.
 - `end`: Marks the end of a block.
 - return: Returns a value from an expression in a procedure.

### Examples of Rubel
 - See `tests` to know Rubel's syntax.
